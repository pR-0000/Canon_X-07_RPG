@echo off
setlocal
pushd "%~dp0" || exit /b 1
set "ASM=sjasmplus"
if exist "..\sjasmplus.exe" set "ASM=..\sjasmplus.exe"
if not exist "bin\" mkdir "bin" || goto :fail
"%ASM%" -Isrc -DBUILD_LANG_EN=0 --raw=bin/quest_FR.bin src/quest.z80 || goto :fail
"%ASM%" -Isrc -DBUILD_LANG_EN=1 --raw=bin/quest_EN.bin src/quest.z80 || goto :fail
popd
exit /b 0

:fail
popd
exit /b 1
