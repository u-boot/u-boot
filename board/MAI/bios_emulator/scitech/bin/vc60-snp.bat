@echo off
REM Setup environment variables for Visual C++ 6.0 32 bit edition

SET MSDevDir=%VC6_MSDevDir%
SET MSVCDir=%VC6_MSVCDir%

if .%CHECKED%==.1 goto checked_build
set LIB=%SCITECH_LIB%\LIB\RELEASE\SNAP\VC6;%MSVCDir%\LIB;%TNT_PATH%\LIB;.
echo Release build enabled.
goto setvars

:checked_build
set LIB=%SCITECH_LIB%\LIB\DEBUG\SNAP\VC6;%MSVCDir%\LIB;%TNT_PATH%\LIB;.
echo Checked debug build enabled.
goto setvars

:setvars
set TOOLROOTDIR=%MSVCDir%
set INCLUDE=.;INCLUDE;%SCITECH%\INCLUDE;%PRIVATE%\INCLUDE
set INIT=%MSVCDir%
SET MAKESTARTUP=%SCITECH%\MAKEDEFS\VC32.MK
SET USE_TNT=
SET USE_WIN16=
SET USE_WIN32=
SET WIN32_GUI=
SET USE_VXD=
SET USE_NTDRV=
SET USE_RTTARGET=
SET USE_SNAP=1
SET VC_LIBBASE=vc6
PATH %SCITECH_BIN%;%MSVCDir%\BIN;%MSDevDir%\BIN;%TNT_PATH%\BIN;%DEFPATH%%VC32_CD_PATH%

echo Visual C++ 6.0 Snap compilation environment set up
