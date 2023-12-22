@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64"
set path=P:\handmade\misc;%path%

REM W4 switch
set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHsc -EHa- -Od -Oi -WX -W3 -wd4201 -wd4100 -wd4189 -wd4505 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref -subsystem:windows user32.lib gdi32.lib winmm.lib

IF NOT EXIST .\build mkdir .\build
pushd .\build
REM 32-bit
REM cl %CommonCompilerFlags%  ..\handmade\code\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%
REM 64-bit
del *.pdb > NUL 2> NUL
REM Optimixation switches /O2
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% ..\code\handmade.cpp -Fmwin32_handmade.map -LD /link -incremental:no /PDB:handmade_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
del lock.tmp
cl %CommonCompilerFlags% ..\code\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
popd