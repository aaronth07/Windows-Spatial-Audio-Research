"""Offline swept-sine mechanics; common stereo timing and no normalization."""
import numpy as np
from scipy.signal import fftconvolve


def sweep_inverse(rate=48000, seconds=.5, low=20., high=20000.):
    if not isinstance(rate,int) or rate<1000 or not np.isfinite([seconds,low,high]).all() or not .05<=seconds<=30 or not 0<low<high<rate/2:
        raise ValueError('Invalid sweep rate, duration or band')
    count=int(round(rate*seconds));time=np.arange(count)/rate
    constant=seconds/np.log(high/low)
    sweep=np.sin(2*np.pi*low*constant*np.expm1(time/constant))
    fade=min(int(.01*rate),count//4)
    ramp=.5-.5*np.cos(np.linspace(0,np.pi,fade))
    sweep[:fade]*=ramp;sweep[-fade:]*=ramp[::-1]
    inverse=sweep[::-1]*np.exp(-time/constant)
    calibration=fftconvolve(sweep,inverse)
    reference=int(np.argmax(np.abs(calibration)))
    scale=float(calibration[reference])
    if not np.isfinite(scale) or abs(scale)<1e-12: raise ValueError('Degenerate calibration')
    inverse/=scale
    return sweep,inverse,reference


def deconvolve(stereo,inverse,reference,frames):
    stereo=np.asarray(stereo,dtype=float);inverse=np.asarray(inverse,dtype=float)
    if stereo.ndim!=2 or stereo.shape[1]!=2 or inverse.ndim!=1 or not len(inverse) or not np.isfinite(stereo).all() or not np.isfinite(inverse).all(): raise ValueError('Finite stereo recording and mono inverse required')
    if type(reference) is not int or reference<0 or type(frames) is not int or frames<1: raise ValueError('Invalid extraction extent')
    full=np.column_stack([fftconvolve(stereo[:,ear],inverse) for ear in range(2)])
    if reference+frames>len(full): raise ValueError('Recording does not cover requested response')
    return full[reference:reference+frames].copy()


def trim_stereo(stereo,samples=4096,relative_threshold=1e-4,pre_samples=0,fade_samples=64):
    stereo=np.asarray(stereo,dtype=float)
    if stereo.ndim!=2 or stereo.shape[1]!=2 or not len(stereo) or not np.isfinite(stereo).all(): raise ValueError('Finite stereo response required')
    if any(type(x) is not int for x in (samples,pre_samples,fade_samples)) or samples<1 or pre_samples<0 or not 0<=fade_samples<=samples or not np.isfinite(relative_threshold) or not 0<relative_threshold<1: raise ValueError('Invalid trim parameters')
    peak=float(np.max(np.abs(stereo)))
    if peak<=1e-15: raise ValueError('Silent response')
    active=np.flatnonzero(np.max(np.abs(stereo),axis=1)>=peak*relative_threshold)
    start=max(0,int(active[0])-pre_samples)
    output=np.zeros((samples,2));count=min(samples,len(stereo)-start);output[:count]=stereo[start:start+count]
    if fade_samples>1: output[-fade_samples:]*=(.5+.5*np.cos(np.linspace(0,np.pi,fade_samples)))[:,None]
    return output,start


def smooth_power(frequencies,power,bands_per_octave=6):
    frequency=np.asarray(frequencies,dtype=float);power=np.asarray(power,dtype=float)
    if frequency.ndim!=1 or power.shape!=frequency.shape or not len(frequency) or not np.isfinite(frequency).all() or not np.isfinite(power).all() or np.any(frequency<=0) or np.any(np.diff(frequency)<=0) or np.any(power<0) or type(bands_per_octave) is not int or bands_per_octave<1: raise ValueError('Sorted positive frequencies and finite nonnegative power required')
    factor=2**(1/(2*bands_per_octave));output=np.empty_like(power)
    for i,centre in enumerate(frequency):
        selected=(frequency>=centre/factor)&(frequency<=centre*factor)
        output[i]=np.mean(power[selected])
    return output
