> Anonymized archived research note. Experimental results are historical, not rerun by publication. See the current research status and public examples for supported reproduction. Local links and fixture-specific commands were removed.

# Packaging inventory

Inventory date: 2026-10-14. This note is local and ignored by Git.

## Local Python environment

`<local-path>` exists and is 64-bit CPython 3.8.10. Import-spec and installed-distribution checks succeeded for the six pinned application dependencies:

| Distribution / import | Installed version | Import found |
| --- | ---: | --- |
| numpy | 1.24.4 | yes |
| scipy | 1.10.1 | yes |
| soundfile | 0.13.1 | yes |
| sofar | 1.2.3 | yes |
| netCDF4 | 1.6.5 | yes |
| matplotlib | 3.7.5 | yes |
| cftime (netCDF4 dependency) | 1.6.4.post1 | yes |
| PyInstaller | not installed | no |

The versions of the six application dependencies match `requirements.txt`. The imports used by `src/measurement.py` and `src/export_hrir.py` were found in this interpreter. `sofar` ships convention JSON/CSV files; `matplotlib` ships its data/fonts. `soundfile` has a bundled `libsndfile` DLL in this interpreter's wheel.

## Local build environment commands

These commands created an isolated build environment in the repository's ignored `.venv` directory and installed the pinned app dependencies plus PyInstaller. Subprocesses ran hidden with stdout/stderr redirected into `validation/` logs. The install completed successfully; a second idempotent install exited 0, and `pip check` exited 0 with no broken requirements.

Historical workspace command omitted; use the public [example guide](../../../../examples/README.md).

The verified venv is 64-bit Python 3.8.10 with PyInstaller 6.22.3 and hooks-contrib 2026.8. All six pinned modules import successfully at the required versions. PyInstaller documents support for Python 3.8+. Build on Windows to produce a Windows executable; PyInstaller is not a cross-compiler. A candidate one-file worker build command, assuming `src/desktop_worker.py` remains the worker entry point, is:

Historical workspace command omitted; use the public [example guide](../../../../examples/README.md).

## Collection notes

- Keep normal direct imports for `numpy`, `scipy`, `soundfile`, `sofar`, and `netCDF4`; no extra hidden-import flags are indicated by the current source imports.
- Collect `sofar` package data because SOFA convention validation reads its bundled JSON/CSV definitions.
- Collect Matplotlib data because export plotting selects the `Agg` backend and uses the package data and fonts.
- PyInstaller installs `pyinstaller-hooks-contrib`, whose package hooks handle special package imports and binaries. The upstream hook changelog documents the Windows `netCDF4` DLL/load-order-file collection fix and hooks for `cftime` and `soundfile`. Preserve those hooks; add explicit `--hidden-import`/binary collection only if the build analysis or a frozen-runtime failure identifies a missing module or DLL.
- This inventory does not establish that the frozen worker launches, that native capture binaries are bundled, or that the final GUI/worker process protocol works. Those require a real build and end-to-end runtime checks after the application entry point and native build inputs are finalized.
- `--runtime-tmpdir` is a build-time static path. The current manual documents that POSIX bootloaders do not expand environment variables; it does not promise Windows `%LOCALAPPDATA%` expansion. Do not treat a literal `%LOCALAPPDATA%` value as verified. Windows one-file extraction starts before the frozen Python entry point, so that entry point cannot set `TMP`/`TEMP` in time for its own initial extraction. Build-time expansion would embed the build user's absolute profile path and would not be portable. Keeping all application preferences, captures, and outputs in `LOCALAPPDATA` is independent of the bootloader's extraction directory; if extraction itself must be in AppData, investigate a native launcher that sets `TMP`/`TEMP` before launching the one-file executable or validate a different packaging model.

## References

- PyInstaller manual (historical reference unavailable)
- PyInstaller usage (historical reference unavailable) `--collect-all`, `--collect-data`, and hidden-import options.
- pyinstaller-hooks-contrib changelog (historical reference unavailable) `netCDF4` DLL/load-order handling and `cftime`/`soundfile` hooks.
