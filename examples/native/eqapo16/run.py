"""Exercise the installed Equalizer APO DSP in an isolated, non-device COM host."""
import argparse
import os
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import numpy as np
from scipy.signal import fftconvolve
import soundfile as sf

HERE=Path(__file__).resolve().parent
PIN='fb73fc8b94f271464ae4f83918bb3cb3243c4f50a8035c07fa538ad16ad66aa4'


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output',type=Path)
    parser.add_argument('--installed',type=Path,default=Path(os.environ['ProgramFiles'])/'EqualizerAPO')
    parser.add_argument('--runtime',type=Path,default=HERE/'runtime')
    args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=False)
    original=args.installed/'EqualizerAPO.dll'
    if digest(original)!=PIN:raise ValueError('Unreviewed installed EqualizerAPO build')
    master=args.installed/'config/config.txt'; master_before=digest(master)
    args.runtime.mkdir(exist_ok=True)
    hashes={}
    for name in ['EqualizerAPO.dll','fftw3f.dll','sndfile.dll','msvcp140.dll','vcruntime140.dll','vcruntime140_1.dll','msvcp140_1.dll','msvcp140_2.dll']:
        source=args.installed/name
        if source.exists():
            target=args.runtime/name
            if not target.exists():shutil.copy2(str(source),str(target))
            if digest(source)!=digest(target):raise ValueError('Runtime source differs: '+name)
            hashes[name]=digest(target)
    rng=np.random.RandomState(286)
    cases=[]

    def run(name,x,h,block,mode='valid',configuration=None,expected=None):
        directory=args.output/name;directory.mkdir()
        sf.write(str(directory/'ir.wav'),h,48000,subtype='FLOAT')
        (directory/'config.txt').write_text(configuration or 'Stage: post-mix\nChannel: all\nConvolution: ir.wav\n',encoding='utf-8')
        x=np.asarray(x,dtype=np.float32);x.tofile(str(directory/'input.f32'))
        command=[str(HERE/'eqapo_host.exe'),str(args.runtime/'EqualizerAPO.dll'),str(directory.resolve()),str((directory/'input.f32').resolve()),str((directory/'output.f32').resolve()),str((directory/'metrics.json').resolve()),str(x.shape[1]),str(block),mode]
        worker=subprocess.run(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=45)
        (directory/'worker.log').write_bytes(worker.stdout)
        record={'name':name,'mode':mode,'exit_code':worker.returncode,'channels':x.shape[1],'block_frames':block,'filter_taps':len(h)}
        if worker.returncode:
            record['diagnostic']=worker.stdout.decode('utf-8',errors='replace')
            cases.append(record);return record,None
        y=np.fromfile(str(directory/'output.f32'),dtype=np.float32).reshape(x.shape)
        if expected is None:expected=np.column_stack([fftconvolve(x[:,c].astype(float),h[:,c%h.shape[1]].astype(float))[:len(x)] for c in range(x.shape[1])])
        record['maximum_reference_error']=float(np.max(abs(y-expected)))
        record['reference_passed']=record['maximum_reference_error']<3e-6
        record['nonzero_channels']=np.flatnonzero(np.max(abs(y),axis=0)>1e-10).tolist()
        record['metrics']=json.loads((directory/'metrics.json').read_text())
        cases.append(record)
        return record,y

    for channels,block,taps in [(8,128,4096),(16,48,4096),(16,64,4096),(16,128,4096),(16,240,4096),(16,480,4096),(16,512,4096),(16,1024,4096),(24,128,4096),(32,128,4096),(16,128,16384),(16,128,65536)]:
        n=4096;total=((n+taps-1+block-1)//block+2)*block
        x=np.zeros((total,channels),dtype=np.float32);x[:n]=rng.normal(0,.002,(n,channels))
        h=rng.normal(0,.0002,(taps,channels)).astype(np.float32);h[0]=np.arange(1,channels+1)/channels;h[-1]=.02
        record,_=run('c%d_b%d_t%d'%(channels,block,taps),x,h,block)
        if record['exit_code']!=0 or not record.get('reference_passed'):raise AssertionError(record)

    # Numeric channel selection for an otherwise unnamed 16-channel layout.
    x=rng.normal(0,.01,(2048,16)).astype(np.float32)
    expected=x.astype(float).copy();expected[:,15]*=.5
    record,_=run('select_channel16',x,np.ones((1,1),dtype=np.float32),128,configuration='Stage: post-mix\nChannel: 16\nPreamp: -6.020599913279624 dB\n',expected=expected)
    if not record.get('reference_passed'):raise AssertionError(record)

    # A 16-source-to-stereo convolution matrix expressed through 32 temporary
    # channels. Only the first two output channels carry the summed ear targets.
    h=rng.normal(0,.0003,(4096,32)).astype(np.float32);h[0]=.1;h[-1]=.003
    x=np.zeros((8320,16),dtype=np.float32);x[:4096]=rng.normal(0,.002,(4096,16))
    names=['LEFT%d'%i if ear==0 else 'RIGHT%d'%i for i in range(1,17) for ear in (0,1)]
    copies=' '.join('%s=%d'%(name,(i//2)+1) for i,name in enumerate(names))
    left='+'.join(names[::2]);right='+'.join(names[1::2])
    config='Stage: post-mix\nCopy: '+copies+'\nChannel: '+' '.join(names)+'\nConvolution: ir.wav\nCopy: 1='+left+' 2='+right+' '+' '.join('%d=0.0'%i for i in range(3,17))+'\n'
    reference=np.zeros(x.shape)
    for i in range(16):
        for ear in (0,1):reference[:,ear]+=fftconvolve(x[:,i].astype(float),h[:,2*i+ear].astype(float))[:len(x)]
    record,_=run('matrix_16_to_2',x,h,128,configuration=config,expected=reference)
    if not record.get('reference_passed'):raise AssertionError(record)

    # Constant maximum quantum is the conservative transport contract. Probe
    # changing frame counts independently and retain a failure as a route limit.
    x=np.zeros((8192,16),dtype=np.float32);x[0]=.1;h=np.zeros((4096,16),dtype=np.float32);h[0]=1;h[-1]=.25
    valid,valid_pcm=run('tail_valid_zeros',x,h,128)
    silent,silent_pcm=run('tail_silent_flag',x,h,128,mode='silent-tail')
    varying,_=run('varying_quantum',x,h,128,mode='vary')
    varying_small,_=run('varying_small_first',x,h,128,mode='vary-small-first')
    if not valid.get('reference_passed'):raise AssertionError(valid)
    silent['expected_tail_was_suppressed']=bool(silent_pcm is not None and np.max(abs(silent_pcm[128:]))==0 and np.max(abs(valid_pcm[128:]))>.01)
    varying['constant_quantum_required_for_this_build']=varying['exit_code']!=0 or not varying.get('reference_passed',False)
    varying_small['constant_quantum_required_for_this_build']=varying_small['exit_code']!=0 or not varying_small.get('reference_passed',False)
    adapted,_=run('adapted_packets_and_tail',x,h,128,mode='adapted')
    if not adapted.get('reference_passed'):raise AssertionError(adapted)
    master_after=digest(master)
    if master_after!=master_before or digest(original)!=PIN:raise AssertionError('Installed baseline changed')
    report={'installed_equalizer_version':'1.4.2.0','dll_sha256':PIN,'runtime_hashes':hashes,'master_config_sha256_before':master_before,'master_config_sha256_after':master_after,'master_unchanged':True,'device_streams_created':False,'registry_writes':False,'configuration_scope':'Process-local imported registry reads return only the isolated configuration; all other DLL registry opens fail closed','cases':cases,'source_commit':subprocess.check_output(['git','-C',str(HERE/'source'),'rev-parse','HEAD'],text=True).strip(),'latency_scope':'APO reported latency and offline callback times; no device or end-to-end measurement'}
    (args.output/'result.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps({'cases':len(cases),'passing_references':sum(c.get('reference_passed',False) for c in cases),'worker_failures':[c['name'] for c in cases if c['exit_code']],'silent_tail_cut':silent['expected_tail_was_suppressed'],'varying_quantum_limit':varying['constant_quantum_required_for_this_build'],'master_unchanged':True}))


if __name__=='__main__':main()
