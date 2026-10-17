"""Validate native object-panning quanta independently of the APO quantum."""
import argparse
import os
import json
from pathlib import Path
import struct
import subprocess
import numpy as np
from run_vbap import layout, weights, HERE


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output',type=Path)
    args=parser.parse_args();args.output.mkdir(parents=True,exist_ok=False)
    p,faces=layout(16);rng=np.random.RandomState(1907);cases=[]
    for frames in [1,48,127,480,1024,2048]:
        directory=args.output/('frames%d'%frames);directory.mkdir()
        objects=4;blocks=8
        meta=np.zeros((blocks,objects,4),dtype=np.float32)
        meta[:,:,:3]=rng.normal(size=(blocks,objects,3));meta[:,:,3]=rng.uniform(.1,.9,(blocks,objects))
        pcm=rng.normal(0,.001,(blocks,objects,frames)).astype(np.float32)
        carrier=directory/'carrier.bin'
        with carrier.open('wb') as f:
            f.write(struct.pack('<6I',0x31504256,16,objects,blocks,frames,len(faces)))
            for data in [p.astype('<f4'),faces.astype('<u4'),meta.astype('<f4'),pcm.astype('<f4')]:f.write(data.tobytes())
        run=subprocess.run([str(HERE/'vbap_probe.exe'),str(carrier),str(directory/'output.f32'),str(directory/'metrics.json')],capture_output=True,timeout=20)
        assert run.returncode==0,(frames,run.stderr)
        actual=np.fromfile(str(directory/'output.f32'),dtype=np.float32).reshape(blocks,frames,16)
        reference=np.zeros(actual.shape,dtype=float);previous=None;volume=None
        alpha=np.arange(1,frames+1,dtype=np.float32)/frames
        for block in range(blocks):
            target=np.asarray([weights(p,faces,meta[block,i,:3]) for i in range(objects)])
            if previous is None:previous=target;volume=meta[block,:,3].copy()
            for i in range(objects):
                gains=previous[i]*(1-alpha[:,None])+target[i]*alpha[:,None]
                gains=gains/np.linalg.norm(gains.astype(float),axis=1,keepdims=True)
                scalar=volume[i]*(1-alpha)+meta[block,i,3]*alpha
                reference[block]+=(pcm[block,i]*scalar)[:,None]*gains
            previous=target;volume=meta[block,:,3].copy()
        error=float(np.max(abs(reference-actual)));assert error<3e-7
        cases.append({'frames':frames,'maximum_reference_error':error,'metrics':json.loads((directory/'metrics.json').read_text())})
    (args.output/'result.json').write_text(json.dumps({'passed':True,'scope':'Fixed producer-transaction length per run; panning and gain ramp span that transaction. APO processing remains independently fixed at128frames.','cases':cases},indent=2))
    print(json.dumps({'cases':len(cases),'passed':True}))


if __name__=='__main__':main()
