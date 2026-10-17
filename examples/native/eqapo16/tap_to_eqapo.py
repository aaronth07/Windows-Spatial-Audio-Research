"""Render a verified native WSSAPI capture to sixteen buses and the real APO."""
import argparse
from functools import reduce
from math import gcd
import json
import hashlib
import os
from pathlib import Path
import struct
import subprocess
import netCDF4
import numpy as np
from scipy.signal import fftconvolve
import soundfile as sf
from run_vbap import HERE, layout, weights


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('capture',type=Path)
    parser.add_argument('sofa',type=Path)
    parser.add_argument('output',type=Path)
    args=parser.parse_args();args.output.mkdir(parents=True,exist_ok=False)
    master=Path(os.environ['ProgramFiles'])/'EqualizerAPO/config/config.txt'
    master_before=hashlib.sha256(master.read_bytes()).hexdigest() if master.is_file() else None
    native_result=json.loads((args.capture/'result.json').read_text(encoding='utf-8-sig'))
    assert native_result['passed'] and native_result['actual_windows_objects_forwarded'] and native_result['exact_reference_match']
    capture=json.loads((args.capture/'capture.json').read_text(encoding='utf-8-sig'))
    reference=json.loads((args.capture/'reference.json').read_text(encoding='utf-8-sig'))
    assert capture['rate']==reference['rate']==48000
    raw=np.fromfile(str(args.capture/'capture.f32'),dtype=np.float32)
    original=np.fromfile(str(args.capture/'reference.f32'),dtype=np.float32)
    records=capture['records'];expected=reference['records']
    assert len(records)==len(expected) and len(records)>0 and np.isfinite(raw).all()
    assert np.array_equal(raw,original)
    by_update={};identities={};owners={};extent=0
    for record,truth in zip(records,expected):
        fields=['update','source_index','frames','valid_frames','eos','type','position','volume','sample_offset']
        if 'streams' in native_result:fields+=['stream_id','generation']
        for key in fields:
            assert record[key]==truth[key],(key,record,truth)
        assert record['forward_end_hresult']==0 and record['type']==1
        assert type(record['frames']) is int and 0<record['frames']<=2048
        assert type(record['sample_offset']) is int and record['sample_offset']==extent;extent+=record['frames']
        slot=(record.get('stream_id',1),record['source_index'])
        lifetime=slot+(record.get('generation',0),);identity=record.get('object_id',record.get('id'))
        assert identity is not None and identity>0
        identities.setdefault(lifetime,identity);owners.setdefault(identity,lifetime)
        assert identities[lifetime]==identity and owners[identity]==lifetime and 0<=record['valid_frames']<=record['frames']
        assert 0<=record['volume']<=1
        update=by_update.setdefault(record['update'],{});assert slot not in update;update[slot]=record
    assert extent==len(raw)
    slots=sorted({slot for update in by_update.values() for slot in update});objects=len(slots)
    assert 1<=objects<=128 and len(set(identities.values()))==len(identities)
    updates=[by_update[k] for k in sorted(by_update)]
    assert sorted(by_update)==list(range(1,len(updates)+1))
    assert all(set(update)==set(slots) and len({r['frames'] for r in update.values()})==1 for update in updates)
    quantum=reduce(gcd,(int(update[slots[0]]['frames']) for update in updates))
    assert 1<=quantum<=2048
    blocks=sum(update[slots[0]]['frames']//quantum for update in updates)
    assert 0<blocks<=4096
    p,faces=layout(16);metadata=[];samples=[];lifetime_ids=[];last={}
    for update in updates:
        frames=update[slots[0]]['frames'];pcm=np.zeros((objects,frames),dtype=np.float32);meta=np.zeros((objects,4),dtype=np.float32);ids=[]
        for source,slot in enumerate(slots):
            record=update[slot];offset=record['sample_offset'];identity=record.get('object_id',record.get('id'));ids.append(identity)
            if slot in last:
                changed=identity!=last[slot].get('object_id',last[slot].get('id'))
                if changed or last[slot]['eos']==1:
                    assert changed and last[slot]['eos']==1 and record.get('generation',0)==last[slot].get('generation',0)+1
            last[slot]=record
            assert offset+frames<=len(raw)
            pcm[source]=raw[offset:offset+frames]
            assert np.all(pcm[source,record['valid_frames']:]==0)
            x,y,z=record['position'];meta[source]=[-z,-x,y,record['volume']]
        for start in range(0,frames,quantum):
            samples.append(pcm[:,start:start+quantum]);metadata.append(meta.copy());lifetime_ids.append(ids)
    x=np.asarray(samples);meta=np.asarray(metadata);lifetime_ids=np.asarray(lifetime_ids,dtype=np.uint64)
    carrier=args.output/'carrier.bin'
    with carrier.open('wb') as f:
        f.write(struct.pack('<6I',0x32504256,16,objects,blocks,quantum,len(faces)))
        for data in [p.astype('<f4'),faces.astype('<u4'),lifetime_ids.astype('<u8'),meta.astype('<f4'),x.astype('<f4')]:f.write(data.tobytes())
    bus_path=args.output/'bus.f32';metrics=args.output/'panner_metrics.json'
    run=subprocess.run([str(HERE/'vbap_probe.exe'),str(carrier),str(bus_path),str(metrics)],capture_output=True,timeout=45)
    (args.output/'panner.log').write_bytes(run.stdout+run.stderr);assert run.returncode==0
    bus=np.fromfile(str(bus_path),dtype=np.float32).reshape(blocks,quantum,16)
    ref=np.zeros(bus.shape,dtype=float);previous=None;volume=None
    alpha=np.arange(1,quantum+1,dtype=np.float32)/quantum
    for block in range(blocks):
        target=np.asarray([weights(p,faces,meta[block,i,:3]) for i in range(objects)])
        if previous is None:previous=target.copy();volume=meta[block,:,3].copy()
        reset=np.ones(objects,dtype=bool) if block==0 else lifetime_ids[block]!=lifetime_ids[block-1]
        previous[reset]=target[reset];volume[reset]=meta[block,reset,3]
        for i in range(objects):
            gains=previous[i]*(1-alpha[:,None])+target[i]*alpha[:,None]
            gains/=np.linalg.norm(gains.astype(float),axis=1,keepdims=True)
            level=volume[i]*(1-alpha)+meta[block,i,3]*alpha
            ref[block]+=(x[block,i]*level)[:,None]*gains
        previous=target;volume=meta[block,:,3].copy()
    panner_error=float(np.max(abs(bus-ref)));panner_peak=float(np.max(abs(ref)))
    assert panner_peak>1e-9 and panner_error<max(panner_peak*1e-5,1e-12)
    with netCDF4.Dataset(str(args.sofa)) as f:
        assert np.all(f['Data.SamplingRate'][:]==48000) and np.all(f['Data.Delay'][:]==0)
        position=np.deg2rad(np.asarray(f['SourcePosition'][:,:2],dtype=float))
        grid=np.column_stack((np.cos(position[:,1])*np.cos(position[:,0]),np.cos(position[:,1])*np.sin(position[:,0]),np.sin(position[:,1])))
        chosen=np.argmax(p@grid.T,axis=1)
        ir=np.asarray([np.asarray(f['Data.IR'][int(i)],dtype=np.float32).T for i in chosen])
    h=ir.transpose(1,0,2).reshape(ir.shape[1],32);flat=bus.reshape(-1,16)
    total=((len(flat)+len(h)-1+127)//128)*128
    padded=np.zeros((total,16),dtype=np.float32);padded[:len(flat)]=flat
    directory=args.output/'eqapo';directory.mkdir();padded.tofile(str(directory/'input.f32'))
    sf.write(str(directory/'ir.wav'),h,48000,subtype='FLOAT')
    names=[('LEFT%d'%i if ear==0 else 'RIGHT%d'%i) for i in range(1,17) for ear in (0,1)]
    config='Stage: post-mix\nCopy: '+' '.join('%s=%d'%(name,i//2+1) for i,name in enumerate(names))+'\nChannel: '+' '.join(names)+'\nConvolution: ir.wav\nCopy: 1='+'+'.join(names[::2])+' 2='+'+'.join(names[1::2])+' '+' '.join('%d=0.0'%i for i in range(3,17))+'\n'
    (directory/'config.txt').write_text(config)
    run=subprocess.run([str(HERE/'eqapo_host.exe'),str(HERE/'runtime/EqualizerAPO.dll'),str(directory),str(directory/'input.f32'),str(directory/'output.f32'),str(directory/'metrics.json'),'16','128','adapted'],capture_output=True,timeout=45)
    (directory/'worker.log').write_bytes(run.stdout+run.stderr);assert run.returncode==0
    output=np.fromfile(str(directory/'output.f32'),dtype=np.float32).reshape(-1,16)
    expected=np.zeros(output.shape,dtype=float)
    # Use the independent panner result so this checks the complete numerical chain.
    independent=np.zeros(padded.shape,dtype=float);independent[:len(flat)]=ref.reshape(-1,16)
    for source in range(16):
        for ear in range(2):expected[:,ear]+=fftconvolve(independent[:,source],ir[source,:,ear].astype(float))[:total]
    error=float(np.max(abs(expected-output)));peak=float(np.max(abs(expected)))
    assert peak>1e-10 and error<max(peak*2e-5,1e-12) and np.all(output[:,2:]==0)
    master_after=hashlib.sha256(master.read_bytes()).hexdigest() if master.is_file() else None
    assert master_after==master_before
    result={'passed':True,'scope':'Actual native Windows spatial objects captured before EndUpdating in a process-local forwarding layer, followed by offline native VBAP and actual Equalizer APO. Multiple streams are aligned at producer sample zero for this comparison; no hardware timestamp alignment, game injection or live custom-rendered output.',
            'native_updates_per_stream':len(updates),'concurrent_source_slots':objects,'unique_object_lifetimes':len(identities),'object_lifetimes':[dict(stream_id=k[0],source_index=k[1],generation=k[2],object_id=v) for k,v in sorted(identities.items())],'native_frame_counts':sorted({r['frames'] for r in records}),
            'captured_frame_extent':len(flat),'panner_frames':quantum,'panning_policy':'Weights and source volume ramp across each carrier quantum; native control events retain exact frame boundaries.',
            'panner_error':panner_error,'panner_relative_error':panner_error/panner_peak,
            'complete_chain_error':error,'complete_chain_relative_error':error/peak,
            'sofa_taps':len(h),'complete_tail_retained':True,'unused_outputs_zero':True,
            'master_sha256_before':master_before,'master_sha256_after':master_after,
            'panner_metrics':json.loads(metrics.read_text()),'apo_metrics':json.loads((directory/'metrics.json').read_text())}
    (args.output/'result.json').write_text(json.dumps(result,indent=2))
    print(json.dumps({'passed':True,'native_updates':len(updates),'complete_chain_relative_error':error/peak}))


if __name__=='__main__':main()
