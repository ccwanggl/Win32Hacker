@echo off

mkdir ..\build
pushd ..\build

cl /Zi w:\handmade\code\win32_handmade.cpp user32.lib
popd
