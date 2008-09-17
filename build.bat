@echo off
rem This script builds jsext

call initbuild.bat

setlocal

if "%config%" == "" goto hasnoconfig
goto hasconfig
:hasnoconfig
set config=Release
:hasconfig

cd js\src\fdlibm
nmake /nologo CFG="fdlibm - Win32 %config%" /f fdlibm.mak
cd ..
nmake /nologo CFG="js - Win32 %config%" /f js.mak
cd ..\..
cd libjsext
vcbuild /useenv jsext.sln "%config%|Win32"
copy ..\js\src\%config%\js32.dll %config%
cd ..
vcbuild /useenv global.sln "%config%|Win32"
cd global\JSEXT
cd Cdb\+tinycdb-0.76
vcbuild /useenv libcdb.vcproj %config%
copy %config%\libcdb.dll ..\libcdb.dll
cd ..\..
cd C\+ctoxml
vcbuild /useenv ctoxml.sln "%config%|Win32"
copy %config%lib\*.dll ..
cd ..
mkdir "=include"
copy ..\..\..\js\src\*.h "=include"
copy ..\..\..\js\src\*.msg "=include"
copy ..\..\..\js\src\%config%\js32.lib
cd ..\..
..\libjsext\Release\jsext /r +makeclib.js /e ";"
cd ..\..


endlocal
