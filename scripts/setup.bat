@echo off

pushd ..
external\bin\premake\premake5.exe --file=build.lua vs2022
popd
pause
