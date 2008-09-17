@echo off

rem This script creates a binary windows installer

setlocal

call build.bat


if "%config%" == "" goto hasnoconfig
goto hasconfig
:hasnoconfig
set config=Debug
:hasconfig

cd %~dp0

call jsext.bat /e "os.deltree('inst')"

cd global
call ..\jsext.bat =install.js install_dir "%~dp0inst"
cd ..
copy js\src\%config%\js32.dll inst
copy src\%config%win\jsextwin.exe inst\jsext.exe
copy src\%config%\libjsext.dll inst

"%ProgramFiles%\Inno Setup 5\compil32" /cc install.iss

endlocal
