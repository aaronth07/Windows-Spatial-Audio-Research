"""Native VBAP object renderer and real Equalizer APO 16-bus convolution proof."""
import argparse
import os
import json
from pathlib import Path
import struct
import subprocess
import numpy as np
from scipy.spatial import ConvexHull
from scipy.signal import fftconvolve
import netCDF4
import soundfile as sf

HERE=Path(__file__).resolve().parent


def layout(count):
    if count==4:
        p=np.asarray([[1,1,1],[1,-1,-1],[-1,1,-1],[-1,-1,1]],dtype=float)/np.sqrt(3)
    else:
        z=1-2*(np.arange(count)+.5)/count
        az=np.arange(count)*np.pi*(3-np.sqrt(5))
        p=np.column_stack((np.sqrt(1-z*z)*np.cos(az),np.sqrt(1-z*z)*np.sin(az),z))
    p=p.astype(np.float32)
    hull=ConvexHull(p)
    if not np.all(hull.equations[:,3]<-1e-6):raise ValueError('Speaker layout does not surround the listener')
    return p,hull.simplices.astype(np.uint32)


def weights(p,faces,direction):
    direction=direction.astype(float);direction/=np.linalg.norm(direction)
    candidates=[]
    for indices in faces:
        solved=np.linalg.solve(p[indices].astype(float).T,direction)
        if solved.min()>=-1e-6:candidates.append((solved.min(),indices,solved))
    if not candidates:raise ValueError('Direction outside speaker coverage')
    _,indices,solved=max(candidates,key=lambda item:item[0])
    solved=np.maximum(solved,0);solved/=np.linalg.norm(solved)
    result=np.zeros(len(p),dtype=np.float32);result[indices]=solved
    return result


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('output',type=Path);parser.add_argument('--skip-apo',action='store_true');args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=False);rng=np.random.RandomState(1901);cases=[]
    for channels,objects in [(4,1),(8,8),(16,16),(16,64),(24,16),(32,16)]:
        p,faces=layout(channels);blocks=64;frames=128
        meta=np.zeros((blocks,objects,4),dtype=np.float32)
        meta[:,:,:3]=rng.normal(size=(blocks,objects,3));meta[:,:,3]=rng.uniform(.2,1,(blocks,objects))
        # Exercise exact speaker directions before random continuous transitions.
        for obj in range(objects):meta[0,obj,:3]=p[obj%channels]
        x=rng.normal(0,.001,(blocks,objects,frames)).astype(np.float32)
        name='c%d_o%d'%(channels,objects);directory=args.output/name;directory.mkdir()
        carrier=directory/'carrier.bin'
        with carrier.open('wb') as f:
            f.write(struct.pack('<6I',0x31504256,channels,objects,blocks,frames,len(faces)))
            f.write(p.astype('<f4').tobytes());f.write(faces.astype('<u4').tobytes());f.write(meta.astype('<f4').tobytes());f.write(x.astype('<f4').tobytes())
        run=subprocess.run([str(HERE/'vbap_probe.exe'),str(carrier),str(directory/'bus.f32'),str(directory/'metrics.json')],capture_output=True,text=True,timeout=30)
        (directory/'worker.log').write_text(run.stdout+run.stderr)
        if run.returncode:raise AssertionError(run.stderr)
        bus=np.fromfile(str(directory/'bus.f32'),dtype=np.float32).reshape(blocks,frames,channels)
        ref=np.zeros(bus.shape,dtype=np.float32);previous=np.zeros((objects,channels),dtype=np.float32);old_volume=meta[0,:,3].copy()
        alpha=(np.arange(1,frames+1,dtype=np.float32)/frames)[:,None]
        for block in range(blocks):
            for obj in range(objects):
                target=weights(p,faces,meta[block,obj,:3])
                if block==0:previous[obj]=target
                interpolated=previous[obj][None,:]*(1-alpha)+target[None,:]*alpha
                norm=np.sqrt(np.sum(interpolated.astype(float)**2,axis=1,keepdims=True))
                gain=(interpolated/norm).astype(np.float32)
                volume=old_volume[obj]*(1-alpha[:,0])+meta[block,obj,3]*alpha[:,0]
                ref[block]+=((x[block,obj]*volume)[:,None]*gain).astype(np.float32)
                previous[obj]=target;old_volume[obj]=meta[block,obj,3]
        error=float(np.max(abs(bus-ref)));metrics=json.loads((directory/'metrics.json').read_text())
        assert error<3e-7 and metrics['maximum_unit_power_error']<1e-6 and metrics['maximum_target_direction_reconstruction_error']<1e-5
        cases.append({'name':name,'maximum_reference_error':error,'metrics':metrics})
        if channels==16 and objects==16:
            main_bus=bus.reshape(-1,16);main_layout=p.copy();main_directory=directory
    if args.skip_apo:
        (args.output/'result.json').write_text(json.dumps({'passed':True,'cases':cases,'device_stream':False},indent=2))
        print(json.dumps({'passed':True,'panner_cases':len(cases)}));return
    # Full 4096-tap SOFA responses for each virtual speaker; 32 temporary APO
    # channels express the two-ear convolution matrix without changing a device.
    root=HERE.parents[2];sofa=root/'fixtures/generated/synthetic.sofa'
    with netCDF4.Dataset(str(sofa)) as f:
        position=np.asarray(f['SourcePosition'][:]);az,el=np.deg2rad(position[:,0]),np.deg2rad(position[:,1]);grid=np.column_stack((np.cos(el)*np.cos(az),np.cos(el)*np.sin(az),np.sin(el)))
        selected=np.argmax(main_layout@grid.T,axis=1);ir=np.stack([np.asarray(f['Data.IR'][int(i)],dtype=np.float32).T for i in selected])
    h=ir.transpose(1,0,2).reshape(4096,32)
    total=((len(main_bus)+len(h)-1+127)//128)*128
    padded=np.zeros((total,16),dtype=np.float32);padded[:len(main_bus)]=main_bus
    directory=args.output/'object_bus_to_sofa_eqapo';directory.mkdir();padded.tofile(str(directory/'input.f32'));sf.write(str(directory/'ir.wav'),h,48000,subtype='FLOAT')
    names=['LEFT%d'%i if ear==0 else 'RIGHT%d'%i for i in range(1,17) for ear in (0,1)]
    config='Stage: post-mix\nCopy: '+' '.join('%s=%d'%(name,(i//2)+1) for i,name in enumerate(names))+'\nChannel: '+' '.join(names)+'\nConvolution: ir.wav\nCopy: 1='+'+'.join(names[::2])+' 2='+'+'.join(names[1::2])+' '+' '.join('%d=0.0'%i for i in range(3,17))+'\n'
    (directory/'config.txt').write_text(config)
    run=subprocess.run([str(HERE/'eqapo_host.exe'),str(HERE/'runtime/EqualizerAPO.dll'),str(directory),str(directory/'input.f32'),str(directory/'output.f32'),str(directory/'metrics.json'),'16','128','adapted'],capture_output=True,text=True,timeout=30)
    (directory/'worker.log').write_text(run.stdout+run.stderr)
    if run.returncode:raise AssertionError(run.stderr)
    output=np.fromfile(str(directory/'output.f32'),dtype=np.float32).reshape(-1,16);expected=np.zeros(output.shape)
    for source in range(16):
        for ear in range(2):expected[:,ear]+=fftconvolve(padded[:,source].astype(float),ir[source,:,ear].astype(float))[:len(padded)]
    convolution_error=float(np.max(abs(output-expected)));assert convolution_error<3e-6 and np.all(output[:,2:]==0)
    report={'scope':'Application-owned positioned PCM objects, native VBAP to arbitrary spherical speaker buses, then actual Equalizer APO16-to-stereo full-SOFA convolution; no device or game interception','coordinate_system':'+X front,+Y left,+Z up; listener-relative directions','gain_contract':'128-sample normalized panning-weight transitions and linear source-gain ramps; supplied gain applied once; no implicit distance attenuation','cases':cases,'equalizer_chain':{'channels':16,'sofa_indices':selected.tolist(),'maximum_direct_reference_error':convolution_error,'metrics':json.loads((directory/'metrics.json').read_text()),'response_taps':4096,'complete_tail_retained':True},'not_verified':['Actual live WSSAPI source interception','Windows endpoint16-channel routing','Physical speaker localization','Perceptual equivalence to direct per-object SOFA rendering'],'reference':'https://aes2.org/publications/elibrary-page/?id=7853'}
    (args.output/'result.json').write_text(json.dumps(report,indent=2));print(json.dumps({'native_panner_cases':len(cases),'eqapo_chain_error':convolution_error,'passed':True}))


if __name__=='__main__':main()
