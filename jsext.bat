@echo off

rem This script runs the jsext interpreter from the development source tree,
rem avoiding pre-installed libraries.

setlocal

if "%config%" == "" goto hasnoconfig
goto hasconfig
:hasnoconfig
set config=Release
:hasconfig

path %~dp0js\src\%config%;%path%
%~dp0libjsext\%config%\jsext /r %~dp0global\=init.js %1 %2 %3 %4 %5 %6 %7 %8 %9

endlocal
