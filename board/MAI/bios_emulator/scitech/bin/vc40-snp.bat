@echo off
REM Setup environment variables for Visual C++ 4.2 32 bit edition

if .%CHECKED%==.1 goto checked_build
set LIB=%SCITECH_LIB%\LIB\RELEASE\SNAP\VC4;%VC4_PATH%\LIB;%TNT_PATH%\LIB;.
echo Release build enabled.
goto setvars

:checked_build
set LIB=%SCITECH_LIB%\LIB\DEBUG\SNAP\VC4;%VC4_PATH%\LIB;%TNT_PATH%\LIB;.
echo Checked debug build enabled.
goto setvars

:setvars
set TOOLROOTDIR=%VC4_PATH%
set INCLUDE=.;INCLUDE;%SCITECH%\INCLUDE;%PRIVATE%\INCLUDE
set INIT=%VC4_PATH%
SET MAKESTARTUP=%SCITECH%\MAKEDEFS\VC32.MK
SET USE_TNT=
SET USE_WIN16=
SET USE_WIN32=
SET WIN32_GUI=
SET USE_VXD=
SET USE_NTDRV=
SET USE_RTTARGET=
SET USE_SNAP=1
SET VC_LIBBASE=VC4
PATH %SCITECH_BIN%;%VC4_PATH%\BIN;%TNT_PATH%\BIN;%DEFPATH%%VC32_CD_PATH%

echo Visual C++ 4.2 Snap compilation environment set up

