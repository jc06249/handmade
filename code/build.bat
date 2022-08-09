@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64"
set path=p:\handmade\misc;%path%

mkdir ..\..\build
pushd ..\..\build
cl -FC -Zi ..\handmade\code\win32_handmade.cpp user32.lib gdi32.lib
popd