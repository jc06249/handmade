@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64"
set path=P:\handmade\misc;%path%

REM W4 switch
set CommonCompilerFlags=-Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHsc -EHa- -Zo -Oi -WX -W3 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST .\build mkdir .\build
pushd .\build

del *.pdb > NUL 2> NUL

REM Asset file builder build
cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\code\test_asset_builder.cpp /link %CommonLinkerFlags%

REM 32-bit build
REM cl %CommonCompilerFlags%  ..\handmade\code\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
REM Optimisation switches /O2
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% -O2 -c ..\code\handmade_optimized.cpp -Fohandmade_optimized.obj -LD
cl %CommonCompilerFlags% ..\code\handmade.cpp handmade_optimized.obj -Fmwin32_handmade.map -LD /link -incremental:no -opt:ref -PDB:handmade_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
del lock.tmp
cl %CommonCompilerFlags% ..\code\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
popd