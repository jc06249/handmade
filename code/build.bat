@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64"
set path=p:\handmade\misc;%path%

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build
REM W4 switch
cl -MT -nologo -Gm- -GR- -EHsc -EHa- -Od -Oi -WX -W3 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7 -Fmwin32_handmade.map ..\handmade\code\win32_handmade.cpp /link -opt:ref -subsystem:windows user32.lib gdi32.lib
popd