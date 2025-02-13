@echo off

@REM build script to build the dependencies and copy the required files to the runtime directory
@REM ./b2 --build-dir=C:\main\trade-master\external\boost\build --toolset=msvc-14.3 --with-system --with-filesystem variant=debug,release link=static,shared threading=multi runtime-link=static,shared address-model=64 stage

external\bin\premake\premake5.exe --file=build.lua vs2022

pause
