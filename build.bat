@echo off

REM rmdir /s /q build
del /s /q build\player*
del /s /q build\*.pdb
del /s /q build\*.obj
del /s /q build\*.ilk

pushd build

SET NAME="player.exe"
SET MYINC=-IC:\Lib\tracy\public -IC:\Lib\tracy\public\tracy^
 -I..\include -IC:\Lib\SDL3\include

SET MYLIB=SDL3.lib SDL3_image.lib
SET MYFILES= ..\src\*.c
rem SET MYFILES=/Tc ..\src\vector.c /Tc ..\src\main.c /Tc ..\src\editor.c^
rem  /Tc ..\src\errors.c /Tc ..\src\audio_setup.c /Tc ..\src\file_process.c
rem  /Tp ..\src\TracyClient.cpp

rem SET DFL=/DTRACY_ENABLE /Zi /Od
rem SET DFL= /fsanitize=address /Zi /Od
rem SET DFL= /Zi /Od
SET DFL= /EHsc /D_DEBUG /MDd /Zi /Od

cl /Fe%NAME% %DFL% %MYLIB% %MYFILES% %MYINC% /link /libpath:C:\Lib\SDL3\lib
rem clang -D_DEBUG -g3 -O0 -I..\include -IC:\Lib\SDl3\include ..\src\*.c -LC:\Lib\SDL3\lib -lSDL3

rem cl /Fe"editor.exe" /Zi /Od SDL3.lib SDL3_ttf.lib SDL3_mixer.lib ..\src\*.c^
rem -I..\include -IC:\Lib\SDL3\include /link /libpath:C:\Lib\SDL3\lib
popd
