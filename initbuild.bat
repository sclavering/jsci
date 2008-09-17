@echo off

rem This script sets environment variables needed to use VC command-line tools

if "%jsext_initbuild%"=="" goto doset
goto endit

:doset

set config=Release
set jsext_initbuild=yes
rem call "%programfiles%\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
rem call "%programfiles%\Microsoft Platform SDK\setenv.cmd" /RETAIL /2000

:endit
