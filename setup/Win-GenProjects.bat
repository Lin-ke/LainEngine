@echo off
pushd %~dp0\..\
call setup\premake\premake5.exe vs2022
popd
PAUSE
