"""Run offline reference examples without changing Windows audio settings."""
import os, subprocess, sys, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def run(*args):
    subprocess.run([str(a) for a in args],cwd=ROOT,check=True)
def main():
    run(sys.executable,'-m','unittest','discover','-s','tests','-v')
    for source in ('open_object_decoder/scan_uhd_samples.py','live_object_latency/analyze.py'):
        run(sys.executable,ROOT/'examples/native'/source,'--self-test')
    if os.name=='nt':
        for group,script in [('sofa_xtc_runtime','build.ps1'),('eqapo16','build_vbap.ps1')]:
            run('powershell','-NoProfile','-ExecutionPolicy','Bypass','-File',ROOT/'examples/native'/group/script)
        run(ROOT/'examples/native/sofa_xtc_runtime/probe.exe','--selftest')
        with tempfile.TemporaryDirectory() as tmp:
            run(sys.executable,ROOT/'examples/native/eqapo16/run_vbap.py',Path(tmp)/'panner','--skip-apo')
if __name__=='__main__':main()
