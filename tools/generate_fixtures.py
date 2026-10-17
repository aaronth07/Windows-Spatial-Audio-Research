"""Generate a reproducible synthetic SOFA and matching native filter banks."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import netCDF4
import numpy as np

ROOT = Path(__file__).resolve().parents[1]


def create(output, count=710, taps=4096):
    output = Path(output)
    if output.exists(): raise FileExistsError('Use a new fixture directory')
    if not 3 <= count <= 2048 or taps != 4096: raise ValueError('Unsupported fixture dimensions')
    output.mkdir(parents=True)
    z = 1-2*(np.arange(count)+.5)/count
    angle = np.arange(count)*np.pi*(3-np.sqrt(5))
    xyz = np.column_stack((np.sqrt(1-z*z)*np.cos(angle), np.sqrt(1-z*z)*np.sin(angle), z))
    azimuth = np.rad2deg(np.arctan2(xyz[:,1], xyz[:,0]))
    elevation = np.rad2deg(np.arcsin(z))
    banks = np.zeros((count,taps,2), dtype='<f4')
    rng = np.random.default_rng(90172)
    time = np.arange(taps)
    for i, direction in enumerate(xyz):
        for ear, sign in enumerate((1,-1)):
            delay = 5+int(round(4*(1-sign*direction[1])))
            banks[i,delay,ear] = np.float32(.7+.15*sign*direction[1])
            banks[i,delay+29,ear] = np.float32(.08+.02*direction[0])
            tail = rng.normal(0,.00015,taps)*np.exp(-time/700)
            tail[:delay+32] = 0
            banks[i,:,ear] += tail.astype('<f4')
    windows = np.column_stack((-xyz[:,1],xyz[:,2],-xyz[:,0])).astype('<f4')
    with netCDF4.Dataset(str(output/'synthetic.sofa'),'w',format='NETCDF4') as ds:
        for name,n in [('M',count),('R',2),('N',taps),('C',3),('I',1),('E',1)]: ds.createDimension(name,n)
        attributes = {'Conventions':'SOFA','Version':'2.1','SOFAConventions':'SimpleFreeFieldHRIR',
            'SOFAConventionsVersion':'1.0','DataType':'FIR','RoomType':'free field',
            'Title':'Synthetic numerical fixture','DatabaseName':'Synthetic',
            'ListenerShortName':'Synthetic','License':'MIT','ApplicationName':'Research examples',
            'ApplicationVersion':'1.0','Organization':'Research contributors',
            'History':'Generated artificial filters; no personal or acoustic measurement',
            'DateCreated':'2026-10-16 00:00:00','DateModified':'2026-10-16 00:00:00',
            'APIName':'netCDF4','APIVersion':netCDF4.__version__,'AuthorContact':'',
            'Comment':'Nominal receiver geometry and artificial filters; not an anatomical HRTF',
            'Origin':'Synthetic','References':'','EmitterDescription':'Point source','ReceiverDescription':'Nominal synthetic ear pair'}
        for key,value in attributes.items(): ds.setncattr(key,value)
        def variable(name,dims,data,kind=None,units=None):
            value = ds.createVariable(name,'f8',dims); value[:]=data
            if kind: value.Type=kind
            if units: value.Units=units
            return value
        variable('Data.IR',('M','R','N'),banks.transpose(0,2,1))
        variable('Data.SamplingRate',('I',),[48000],units='hertz')
        variable('Data.Delay',('I','R'),[[0,0]])
        variable('SourcePosition',('M','C'),np.column_stack((azimuth,elevation,np.ones(count))),'spherical','degree, degree, metre')
        variable('ReceiverPosition',('R','C','I'),np.array([[0,.09,0],[0,-.09,0]])[:,:,None],'cartesian','metre')
        variable('ListenerPosition',('I','C'),[[0,0,0]],'cartesian','metre')
        variable('ListenerView',('I','C'),[[1,0,0]],'cartesian','metre')
        variable('ListenerUp',('I','C'),[[0,0,1]],'cartesian','metre')
        variable('EmitterPosition',('E','C','I'),np.zeros((1,3,1)),'cartesian','metre')
    (output/'full_sofa_banks.bin').write_bytes(struct.pack('<4If',0x324b4e42,count,taps,0,1)+banks.tobytes())
    windows.tofile(str(output/'full_sofa_directions_windows.f32'))
    xyz.astype('<f4').tofile(str(output/'full_sofa_directions_sofa.f32'))
    result = {'synthetic':True,'directions':count,'taps':taps,'sample_rate':48000,
              'files':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in output.iterdir() if p.is_file()}}
    (output/'fixture.json').write_text(json.dumps(result,indent=2)+'\n')
    return result


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=ROOT/'fixtures/generated'); args=parser.parse_args()
    result=create(args.output);print(json.dumps({'synthetic':True,'directions':result['directions'],'taps':result['taps']}))


if __name__=='__main__':main()
