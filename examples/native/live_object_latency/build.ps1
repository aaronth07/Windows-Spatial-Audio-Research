$ErrorActionPreference = 'Stop'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$instances = & $vswhere -all -prerelease -products '*' -format json | ConvertFrom-Json
$vs = ($instances | Where-Object {Test-Path -LiteralPath (Join-Path $_.installationPath 'VC/Tools/MSVC')} | Sort-Object {[version]$_.installationVersion} -Descending | Select-Object -First 1).installationPath
if (-not $vs) {throw 'MSVC unavailable'}
$vc = (Get-ChildItem -LiteralPath (Join-Path $vs 'VC/Tools/MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1).FullName
$sdk = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits/10'
$ver = (Get-ChildItem -LiteralPath (Join-Path $sdk 'Include') -Directory | Where-Object {Test-Path -LiteralPath (Join-Path $_.FullName 'um/spatialaudioclient.h')} | Sort-Object {[version]$_.Name} -Descending | Select-Object -First 1).Name
$common = @('/nologo','/std:c++17','/EHsc','/W4','/WX','/MT','/O2',"/I$vc/include")
foreach ($inc in @('ucrt','shared','um','winrt')) {$common += "/I$sdk/Include/$ver/$inc"}
$libraries = @('/link',"/LIBPATH:$vc/lib/x64","/LIBPATH:$sdk/Lib/$ver/ucrt/x64","/LIBPATH:$sdk/Lib/$ver/um/x64",'ole32.lib','propsys.lib','uuid.lib')
& "$vc/bin/Hostx64/x64/cl.exe" @common "$PSScriptRoot/latency_probe.cpp" "/Fo$PSScriptRoot/latency_probe.obj" "/Fe$PSScriptRoot/latency_probe.exe" @libraries
if ($LASTEXITCODE) {throw 'Latency probe build failed'}
