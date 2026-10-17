$ErrorActionPreference = 'Stop'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$installations = & $vswhere -all -prerelease -products '*' -format json | ConvertFrom-Json
$visualStudio = ($installations | Where-Object { Test-Path -LiteralPath (Join-Path $_.installationPath 'VC/Tools/MSVC') } | Sort-Object { [version]$_.installationVersion } -Descending | Select-Object -First 1).installationPath
if (-not $visualStudio) { throw 'MSVC tools unavailable' }
$compilerTools = (Get-ChildItem -LiteralPath (Join-Path $visualStudio 'VC/Tools/MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1).FullName
$sdkRoot = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits/10'
$sdkVersion = (Get-ChildItem -LiteralPath (Join-Path $sdkRoot 'Include') -Directory | Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'um/SpatialAudioClient.h') } | Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1).Name
$arguments = @('/nologo','/std:c++17','/EHsc','/W4','/WX','/MT','/O2',"/I$compilerTools/include")
foreach ($include in @('ucrt','shared','um','winrt')) { $arguments += "/I$sdkRoot/Include/$sdkVersion/$include" }
$arguments += @("$PSScriptRoot/controlled_source_adapter.cpp","/Fo$PSScriptRoot/controlled_source_adapter.obj","/Fe$PSScriptRoot/controlled_source_adapter.exe",'/link',"/LIBPATH:$compilerTools/lib/x64","/LIBPATH:$sdkRoot/Lib/$sdkVersion/ucrt/x64","/LIBPATH:$sdkRoot/Lib/$sdkVersion/um/x64",'ole32.lib')
& "$compilerTools/bin/Hostx64/x64/cl.exe" @arguments
if ($LASTEXITCODE) { throw 'Source adapter build failed' }
$dllArguments = @('/nologo','/std:c++17','/EHsc','/W4','/WX','/MT','/O2','/LD',"/I$compilerTools/include")
& "$compilerTools/bin/Hostx64/x64/ml64.exe" /nologo /c "/Fo$PSScriptRoot/regfree_forwarders.obj" "$PSScriptRoot/regfree_forwarders.asm"
if ($LASTEXITCODE) { throw 'DSP forwarding assembly failed' }
foreach ($include in @('ucrt','shared','um','winrt')) { $dllArguments += "/I$sdkRoot/Include/$sdkVersion/$include" }
$dllArguments += @("$PSScriptRoot/regfree_factory.cpp","$PSScriptRoot/regfree_forwarders.obj","/Fo$PSScriptRoot/regfree_factory.obj","/Fe$PSScriptRoot/regfree_factory.dll",'/link',"/LIBPATH:$compilerTools/lib/x64","/LIBPATH:$sdkRoot/Lib/$sdkVersion/ucrt/x64","/LIBPATH:$sdkRoot/Lib/$sdkVersion/um/x64",'ole32.lib','uuid.lib','bcrypt.lib',"/DEF:$PSScriptRoot/regfree_factory.def","/IMPLIB:$PSScriptRoot/regfree_factory.lib")
& "$compilerTools/bin/Hostx64/x64/cl.exe" @dllArguments
if ($LASTEXITCODE) { throw 'Isolated COM factory build failed' }
$bootstrapBase = @('/nologo','/std:c++17','/EHsc','/W4','/WX','/MT','/O2',"/I$compilerTools/include")
foreach ($include in @('ucrt','shared','um','winrt')) { $bootstrapBase += "/I$sdkRoot/Include/$sdkVersion/$include" }
$bootstrapLink = @('/link',"/LIBPATH:$compilerTools/lib/x64","/LIBPATH:$sdkRoot/Lib/$sdkVersion/ucrt/x64","/LIBPATH:$sdkRoot/Lib/$sdkVersion/um/x64",'ole32.lib','uuid.lib','propsys.lib','bcrypt.lib')
& "$compilerTools/bin/Hostx64/x64/cl.exe" @bootstrapBase /LD "$PSScriptRoot/bootstrap_apo.cpp" "/Fo$PSScriptRoot/bootstrap_apo.obj" "/Fe$PSScriptRoot/bootstrap_apo.dll" @bootstrapLink "/DEF:$PSScriptRoot/bootstrap_apo.def" "/IMPLIB:$PSScriptRoot/bootstrap_apo.lib"
if ($LASTEXITCODE) { throw 'Bootstrap APO build failed' }
& "$compilerTools/bin/Hostx64/x64/cl.exe" @bootstrapBase "$PSScriptRoot/bootstrap_host.cpp" "/Fo$PSScriptRoot/bootstrap_host.obj" "/Fe$PSScriptRoot/bootstrap_host.exe" @bootstrapLink
if ($LASTEXITCODE) { throw 'Bootstrap host build failed' }
