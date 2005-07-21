@echo off
REM Setup environment variables for Visual C++ 1.52c 16 bit edition

if .%CHECKED%==.1 goto checked_build
set LIB=%SCITECH_LIB%\LIB\RELEASE\WIN16\VC4;%VC_PATH%\LIB;.
echo Release build enabled.
goto setvars

:checked_build
set LIB=%SCITECH_LIB%\LIB\DEBUG\WIN16\VC4;%VC_PATH%\LIB;.
echo Checked debug build enabled.
goto setvars

:setvars
set TOOLROOTDIR=%VC_PATH%
set INCLUDE=.;INCLUDE;%SCITECH%\INCLUDE;%PRIVATE%\INCLUDE;%VC_PATH%\INCLUDE;
set INIT=%VC_PATH%
SET MAKESTARTUP=%SCITECH%\MAKEDEFS\VC16.MK
SET USE_WIN16=1
SET USE_WIN32=
SET VC_LIBBASE=VC4
SET USE_RTTARGET=
SET USE_SNAP=
PATH %SCITECH_BIN%;%VC_PATH%\BIN;%DEFPATH%%VC_CD_PATH%

echo Visual C++ 1.52c 16 bit Windows compilation environment set up.
