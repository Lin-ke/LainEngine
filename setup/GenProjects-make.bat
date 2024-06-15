@echo off
pushd %~dp0\..\
call setup\premake\premake5.exe gmake2
popd
PAUSE