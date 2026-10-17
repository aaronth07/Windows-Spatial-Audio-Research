"""Construct nonzero causal object signals with zero two-ear FIR output."""
import argparse
import hashlib
import json
from pathlib import Path
import netCDF4
import numpy as np
from scipy.signal import fftconvolve


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('sofa', type=Path)
    parser.add_argument('output', type=Path)
    args=parser.parse_args()
    if args.output.exists(): raise FileExistsError(args.output)
    indices=[0,1,145]
    with netCDF4.Dataset(str(args.sofa)) as f:
        h=np.asarray(f['Data.IR'][indices], dtype=float)
        positions=np.asarray(f['SourcePosition'][indices], dtype=float)
    x=[]
    for b,c in [(1,2),(2,0),(0,1)]:
        x.append(fftconvolve(h[b,0],h[c,1])-fftconvolve(h[b,1],h[c,0]))
    peak=max(np.max(np.abs(s)) for s in x)
    if not np.isfinite(peak) or peak<=1e-12: raise ValueError('Degenerate example')
    x=np.asarray(x)*(.1/peak)
    ears=np.asarray([sum(fftconvolve(x[i],h[i,ear]) for i in range(3)) for ear in range(2)])
    residual=float(np.max(np.abs(ears)))
    assert residual<1e-12 and all(np.max(np.abs(s))>1e-8 for s in x)
    result={'claim':'The original objects cannot in general be uniquely recovered from their binaural mix alone.',
            'construction':'Three causal nonzero PCM sources lie in the null space of the known two-ear FIR mixing operator.',
            'scope':'Information loss after mixing; does not rule out accessing objects before mixing.',
            'sofa_sha256':hashlib.sha256(args.sofa.read_bytes()).hexdigest(),
            'source_indices':indices, 'source_positions':positions.tolist(),
            'source_peaks':np.max(abs(x),axis=1).tolist(), 'maximum_binaural_residual':residual,
            'arithmetic':'float64 convolution; algebraic cancellation is exact apart from numerical roundoff.'}
    args.output.write_text(json.dumps(result,indent=2))
    print(json.dumps({'maximum_binaural_residual':residual,'passed':True}))


if __name__=='__main__': main()
