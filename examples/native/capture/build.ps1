$ErrorActionPreference='Stop'
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$installations=& $vswhere -all -prerelease -products '*' -format json|ConvertFrom-Json
$installation=($installations|Where-Object {Test-Path -LiteralPath (Join-Path $_.installationPath 'VC\Tools\MSVC')}|Sort-Object {[version]$_.installationVersion} -Descending|Select-Object -First 1).installationPath
if(-not $installation){throw 'Build requires Visual Studio C++ x64 tools and Windows SDK.'}
$compiler=(Get-ChildItem -LiteralPath (Join-Path $installation 'VC\Tools\MSVC') -Directory|Sort-Object Name -Descending|Select-Object -First 1).FullName
$sdk=Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10'
$version=(Get-ChildItem -LiteralPath (Join-Path $sdk 'Include') -Directory|Where-Object {Test-Path -LiteralPath (Join-Path $_.FullName 'um\spatialaudioclient.h')}|Sort-Object {[version]$_.Name} -Descending|Select-Object -First 1).Name
if(-not $version){throw 'Windows SDK is missing.'}
$build=Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force -Path $build|Out-Null
$common=@('/nologo','/std:c++17','/utf-8','/EHsc','/W4','/WX','/MT','/O2','/Brepro','/experimental:deterministic',"/pathmap:$PSScriptRoot=.","/I$compiler\include")
foreach($part in @('ucrt','shared','um','winrt')){$common+="/I$sdk\Include\$version\$part"}
$link=@("/LIBPATH:$compiler\lib\x64","/LIBPATH:$sdk\Lib\$version\ucrt\x64","/LIBPATH:$sdk\Lib\$version\um\x64")
foreach($target in @('spatial_capture')){
 $source=if($target -eq 'spatial_capture'){'spatial_capture'}else{'desktop_app'}
 $arguments=$common+@("$PSScriptRoot\src\$source.cpp","/Fo$build\$target.obj","/Fe$build\$target.exe",'/link')+$link
 if($target -eq 'spatial_capture'){$arguments+=@('ole32.lib','uuid.lib','runtimeobject.lib','bcrypt.lib','advapi32.lib','avrt.lib')}
 else{$arguments+=@('/SUBSYSTEM:WINDOWS','user32.lib','gdi32.lib','comctl32.lib','shell32.lib','ole32.lib','uuid.lib',"/MANIFESTDEPENDENCY:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'")}
 & "$compiler\bin\Hostx64\x64\cl.exe" @arguments
 if($LASTEXITCODE -ne 0){throw "Native build failed: $target"}
}
