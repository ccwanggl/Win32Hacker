@echo off

REM
REM  To  run this at startup, use this as your shortcut target:
REM  %windir%\sytem32\cmd.exe /k w:\handmade\misc\shell.bat
REM


call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set path=w:\handmade\misc;%path%
w:
code .