@echo off


pushd ..
pushd Core\Vendor\Assimp
cmake .
popd
Vendor\Binaries\Premake\Windows\premake5.exe --file=Build.lua vs2022
popd


pause