@echo off

rem This script cleans up

call initbuild.bat

setlocal

if "%config%" == "" goto hasnoconfig
goto hasconfig
:hasnoconfig
set config=Debug
:hasconfig

cd global
call ..\jsext.bat +clean.js
cd ..
cd js\src\fdlibm
nmake /nologo CFG="fdlibm - Win32 %config%" /f fdlibm.mak clean
cd ..
nmake /nologo CFG="js - Win32 %config%" /f js.mak clean
cd ..\..\libjsext
vcbuild /useenv /clean jsext.vcproj %config%
vcbuild /useenv /clean jsextwin.vcproj %config%
vcbuild /useenv /clean libjsext.vcproj %config%

endlocal
