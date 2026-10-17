"""Exercise built CPU preparation gates in a fresh owned process, without DSP/audio."""
import argparse
import ctypes as c
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import threading

PTR, HR, U32 = c.c_void_p, c.c_long, c.c_uint32
UNEXPECTED, POINTER, INVALID = 0x8000FFFF, 0x80004003, 0x80070057
NATIVE_PIN = '4706985ccbc44d74a9060df72c6ae7c7f34e95aeee5d99669fd00ea8d74261f9'


def unsigned(hr):
    return hr & 0xffffffff


def child(folder):
    assert c.sizeof(PTR) == 8
    kernel = c.WinDLL('kernel32', use_last_error=True)
    kernel.GetModuleHandleW.argtypes = [c.c_wchar_p]
    kernel.GetModuleHandleW.restype = PTR
    kernel.GetSystemDirectoryW.argtypes = [c.c_wchar_p, U32]
    kernel.GetModuleFileNameW.argtypes = [PTR, c.c_wchar_p, U32]
    kernel.GetModuleHandleExW.argtypes = [U32, PTR, c.POINTER(PTR)]
    kernel.GetProcAddress.argtypes = [PTR, c.c_char_p]
    kernel.GetProcAddress.restype = PTR
    assert not kernel.GetModuleHandleW('HrtfDspCpu.dll'), 'Child is not cold'
    text = c.create_unicode_buffer(32768)
    assert 0 < kernel.GetSystemDirectoryW(text, len(text)) < len(text)
    native_path = Path(text.value) / 'HrtfDspCpu.dll'
    assert hashlib.sha256(native_path.read_bytes()).hexdigest() == NATIVE_PIN
    own_path = (folder / 'HrtfDspCpu.dll').resolve(strict=True)
    own = c.WinDLL(str(own_path))
    factory = own.CreateHrtfDspFactory
    factory.argtypes, factory.restype = [U32, c.POINTER(PTR)], HR
    prepare = own.InitializeCpuFactory
    prepare.argtypes, prepare.restype = [], HR
    helper_address = kernel.GetProcAddress(own._handle, b'CreateHrtfDataHelper')
    assert helper_address
    # Only inspect our own built thunk. This load must be the single pointer read.
    thunk = c.string_at(helper_address, 32)
    assert thunk[:3] == b'\x48\x8b\x05', thunk.hex()
    pointer_slot = helper_address + 7 + struct.unpack('<i', thunk[3:7])[0]
    assert c.c_void_p.from_address(pointer_slot).value is None
    helper_before = c.WINFUNCTYPE(HR)(helper_address)()
    assert unsigned(helper_before) == UNEXPECTED
    # No helper call is made after preparation: its successful ABI is not assumed.
    output = PTR(1)
    before_factory = factory(0, c.byref(output))
    assert unsigned(before_factory) == UNEXPECTED and output.value is None
    assert unsigned(factory(0, None)) == POINTER
    output = PTR(1)
    assert unsigned(factory(1, c.byref(output))) == INVALID and output.value is None
    assert unsigned(factory(1, None)) == POINTER
    ole = c.WinDLL('ole32')
    ole.CoInitializeEx.argtypes = [PTR, U32]
    ole.CoInitializeEx.restype = HR
    assert unsigned(ole.CoInitializeEx(None, 0)) in (0, 1)
    barrier = threading.Barrier(3)
    ready_barrier = threading.Barrier(2)
    prepared = threading.Event()
    results, failures = [], []
    result_lock = threading.Lock()

    def one_call(counts):
        value = PTR(1)
        hr = unsigned(factory(0, c.byref(value)))
        assert hr in (0, UNEXPECTED), hex(hr)
        if hr == 0:
            assert value.value
            release_address = None
            try:
                table = c.cast(value, c.POINTER(PTR))[0]
                release_address = c.cast(table, c.POINTER(PTR))[2]
                assert release_address
            finally:
                if value.value and release_address:
                    c.WINFUNCTYPE(U32, PTR)(release_address)(value)
                    counts['released'] += 1
            counts['success'] += 1
        else:
            assert value.value is None
            counts['not_ready'] += 1

    def worker(index):
        initialized = False
        counts = {'worker': index, 'before': {'success': 0, 'not_ready': 0, 'released': 0},
                  'preparation_window': {'success': 0, 'not_ready': 0, 'released': 0},
                  'ready': {'success': 0, 'not_ready': 0, 'released': 0}}
        try:
            assert unsigned(ole.CoInitializeEx(None, 0)) in (0, 1)
            initialized = True
            # Establish rejection on every worker before allowing preparation.
            for _ in range(8):
                one_call(counts['before'])
            assert counts['before']['success'] == 0
            barrier.wait(timeout=15)
            for _ in range(200):
                one_call(counts['preparation_window'])
            assert prepared.wait(timeout=15), 'Preparation did not signal ready'
            ready_barrier.wait(timeout=15)
            for _ in range(32):
                one_call(counts['ready'])
            assert counts['ready']['success'] == counts['ready']['released'] == 32 and counts['ready']['not_ready'] == 0
            with result_lock:
                results.append(counts)
        except BaseException as error:
            with result_lock:
                failures.append({'worker': index, 'error': repr(error)})
            try:
                barrier.abort()
                ready_barrier.abort()
            except Exception:
                pass
        finally:
            if initialized:
                ole.CoUninitialize()

    threads = [threading.Thread(target=worker, args=(index,), daemon=True) for index in range(2)]
    try:
        for thread in threads:
            thread.start()
        barrier.wait(timeout=15)
        status = unsigned(prepare())
        assert status == 0, hex(status)
        prepared.set()
        for thread in threads:
            thread.join(timeout=30)
            assert not thread.is_alive(), 'Worker timeout'
        assert not failures, failures
        assert len(results) == 2 and all(phase['success'] == phase['released'] for row in results for key, phase in row.items() if key != 'worker')
        after = {key: sum(row['ready'][key] for row in results) for key in ('success', 'not_ready', 'released')}
        assert unsigned(prepare()) == 0, 'Repeated preparation must be idempotent'
        native_helper = c.c_void_p.from_address(pointer_slot).value
        assert native_helper and native_helper != helper_address
        owner = PTR()
        assert kernel.GetModuleHandleExW(0x4 | 0x2, native_helper, c.byref(owner))
        assert 0 < kernel.GetModuleFileNameW(owner, text, len(text)) < len(text)
        owner_path = Path(text.value)
        assert owner_path.samefile(native_path)
        assert hashlib.sha256(owner_path.read_bytes()).hexdigest() == NATIVE_PIN
        assert kernel.GetProcAddress(owner, b'CreateHrtfDataHelper') == native_helper
        own.GetLiveDspCount.restype = U32
        assert own.GetLiveDspCount() == 0
        result = {'passed': True, 'custom_module_sha256': hashlib.sha256(own_path.read_bytes()).hexdigest(), 'sidecar_mode': 0,
                  'helper_before_preparation_hresult': hex(unsigned(helper_before)), 'factory_before_preparation_hresult': hex(unsigned(before_factory)),
                  'null_output_and_invalid_mode_rejected': True, 'workers': sorted(results, key=lambda item: item['worker']),
                  'post_ready_workers_total': after, 'ready_phase_workers': 2, 'ready_phase_start_barrier': True, 'repeated_preparation_succeeded': True,
                  'helper_owner_path': str(owner_path), 'helper_owner_sha256': NATIVE_PIN, 'helper_pointer_matches_owner_export': True,
                  'live_DSP_count': 0, 'successful_helper_calls': 0, 'DSP_calls': 0,
                  'scope': 'Fresh owned process. Raw factory creation/release during mode0 preparation only; no DSP processing, audio endpoint, helper successful ABI, system admission, or host retry/dispatch proof.'}
        (folder / 'result.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
        print(json.dumps(result))
    finally:
        ole.CoUninitialize()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    parser.add_argument('--dll', type=Path, default=Path(__file__).resolve().parent / 'regfree_factory.dll')
    parser.add_argument('--child', action='store_true', help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.child:
        child(args.output.resolve(strict=True))
        return
    args.output.mkdir(parents=True, exist_ok=False)
    folder = args.output.resolve()
    source = args.dll.resolve(strict=True)
    target = folder / 'HrtfDspCpu.dll'
    shutil.copyfile(source, target)
    digest = hashlib.sha256(source.read_bytes()).hexdigest()
    assert hashlib.sha256(target.read_bytes()).hexdigest() == digest
    (folder / 'HrtfDspCpu.sofa.bin').write_bytes(struct.pack('<8s6I32s', b'WSSSOFA1', 1, 0, 0, 0, 0, 0, hashlib.sha256(b'').digest()))
    result = subprocess.run([sys.executable, str(Path(__file__).resolve()), str(folder), '--child'], capture_output=True, text=True, timeout=60)
    (folder / 'worker.log').write_text(result.stdout + result.stderr, encoding='utf-8')
    assert result.returncode == 0, (result.returncode, result.stdout, result.stderr)
    assert hashlib.sha256(source.read_bytes()).hexdigest() == digest, 'Source binary changed during test'
    print('PASS early-entry rejection, concurrent factory calls, ready-state release and System32 helper ownership')


if __name__ == '__main__':
    main()
