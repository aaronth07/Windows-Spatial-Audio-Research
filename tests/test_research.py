import unittest, tempfile, struct, sys
from pathlib import Path
import numpy as np
import netCDF4
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from examples import profile_format as profile
from examples.sweep_measurement import sweep_inverse,deconvolve,trim_stereo,smooth_power
from tools.generate_fixtures import create

class ResearchTests(unittest.TestCase):
    def test_profile_checksum_and_isolation(self):
        values=[0]*15691; values[0]=0x7072;values[2:13]=[1,1,0,64,3,0,1,25,20,11,36]
        values[1600]=123; checksum=0xa569
        for v in values[2:]: checksum^=v&65535
        values[1]=checksum
        self.assertTrue(profile.inspect_rp(values)['extended_valid'])
        changed=profile.room_tap_proposal(values)
        self.assertEqual(values[1600],123);self.assertEqual(changed[1600],0)
        self.assertEqual(changed[1834:],tuple(values[1834:]))
        values[4000]^=1
        with self.assertRaises(ValueError):profile.inspect_rp(values)
    def test_payload_rejects_truncation_and_trailing(self):
        for data in (b'',b'\x02',b'\x00'*8+b'x'):
            with self.assertRaises(ValueError):profile.decode(data)
    def test_sweep_calibration(self):
        sweep,inverse,reference=sweep_inverse()
        response=deconvolve(np.column_stack((sweep,.5*sweep)),inverse,reference,128)
        self.assertAlmostEqual(response[0,0],1,places=10)
        np.testing.assert_allclose(response[:,1],.5*response[:,0],atol=1e-12)
    def test_common_stereo_trim(self):
        x=np.zeros((6000,2));x[300,0]=1;x[310,1]=.5
        out,start=trim_stereo(x)
        self.assertEqual(start,300);self.assertEqual(out.shape,(4096,2))
        self.assertEqual(out[0,0],1);self.assertEqual(out[10,1],.5)
        with self.assertRaises(ValueError):trim_stereo(np.zeros((4,2)))
    def test_smoothing(self):
        f=np.geomspace(20,20000,1024)
        np.testing.assert_array_equal(smooth_power(f,np.ones(1024)),np.ones(1024))
        with self.assertRaises(ValueError):smooth_power(f[::-1],np.ones(1024))
    def test_fixture_parity(self):
        with tempfile.TemporaryDirectory() as tmp:
            output=Path(tmp)/'fixture';create(output,count=8)
            raw=(output/'full_sofa_banks.bin').read_bytes()
            self.assertEqual(struct.unpack('<4If',raw[:20]),(0x324b4e42,8,4096,0,1))
            bank=np.frombuffer(raw[20:],dtype='<f4').reshape(8,4096,2)
            with netCDF4.Dataset(output/'synthetic.sofa') as sofa:
                np.testing.assert_array_equal(sofa['Data.IR'][:],bank.transpose(0,2,1))
                self.assertEqual(sofa.SOFAConventions,'SimpleFreeFieldHRIR')
            directions=np.fromfile(output/'full_sofa_directions_windows.f32',dtype='<f4').reshape(-1,3)
            np.testing.assert_allclose(np.linalg.norm(directions,axis=1),1,atol=1e-6)
            with self.assertRaises(FileExistsError):create(output,count=8)

if __name__=='__main__':unittest.main()
