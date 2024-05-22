@echo off


pushd ..
pushd Core\Vendor\Assimp
cmake .
popd
Vendor\Binaries\Premake\Windows\premake5.exe --file=Build.lua vs2022
powershell -ExecutionPolicy Bypass -File Scripts/GetDXC.ps1 Core/Vendor/DXC
popd


pause