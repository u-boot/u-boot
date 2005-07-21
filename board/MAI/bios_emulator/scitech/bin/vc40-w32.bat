@echo off
REM Setup environment variables for Visual C++ 4.2 32 bit edition

if .%CHECKED%==.1 goto checked_build
set LIB=%SCITECH_LIB%\LIB\RELEASE\WIN32\VC4;%VC4_PATH%\LIB;%TNT_PATH%\LIB;.
echo Release build enabled.
goto setvars

:checked_build
set LIB=%SCITECH_LIB%\LIB\DEBUG\WIN32\VC4;%VC4_PATH%\LIB;%TNT_PATH%\LIB;.
echo Checked debug build enabled.
goto setvars

:setvars
set TOOLROOTDIR=%VC4_PATH%
set C_INCLUDE=%VC4_PATH%\INCLUDE;%TNT_PATH%\INCLUDE;
set INCLUDE=.;INCLUDE;%SCITECH%\INCLUDE;%PRIVATE%\INCLUDE;%C_INCLUDE%
set INIT=%VC4_PATH%
SET MAKESTARTUP=%SCITECH%\MAKEDEFS\VC32.MK
SET USE_TNT=
SET USE_WIN16=
SET USE_WIN32=1
SET WIN32_GUI=1
SET USE_VXD=
SET USE_NTDRV=
SET USE_RTTARGET=
SET USE_SNAP=
SET VC_LIBBASE=VC4
PATH %SCITECH_BIN%;%VC4_PATH%\BIN;%TNT_PATH%\BIN;%DEFPATH%%VC32_CD_PATH%

REM: Enable Win32 SDK if desired (sdk on command line)
if NOT .%1%==.sdk goto done
call win32sdk.bat

:done
echo Visual C++ 4.2 32 bit Windows compilation environment set up

