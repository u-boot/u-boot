@echo off
REM Setup for compiling with Watcom C/C++ 10.0a in 32 bit mode with Phar Lap TNT

if .%CHECKED%==.1 goto checked_build
SET LIB=%SCITECH_LIB%\LIB\RELEASE\DOS32\WC10A;%WC10A_PATH%\LIB386;%WC10A_PATH%\LIB386\DOS;%TNT_PATH%\LIB;.
echo Release build enabled.
goto setvars

:checked_build
SET LIB=%SCITECH_LIB%\LIB\DEBUG\DOS32\WC10A;%WC10A_PATH%\LIB386;%WC10A_PATH%\LIB386\DOS;%TNT_PATH%\LIB;.
echo Checked debug build enabled.
goto setvars

:setvars
SET EDPATH=%WC10A_PATH%\EDDAT
SET INCLUDE=INCLUDE;%SCITECH%\INCLUDE;%PRIVATE%\INCLUDE;%WC10A_PATH%\H;%WC10A_PATH%\H\NT;%TNT_PATH%\INCLUDE
SET WATCOM=%WC10A_PATH%
SET MAKESTARTUP=%SCITECH%\MAKEDEFS\WC32.MK
SET USE_TNT=1
SET USE_X32=
SET USE_X32VM=
SET USE_WIN16=
SET USE_WIN32=
SET USE_WIN386=
SET USE_OS216=
SET USE_OS232=
SET USE_OS2GUI=
SET USE_SNAP=
SET WC_LIBBASE=WC10A
PATH %SCITECH_BIN%;%WC10A_PATH%\BINNT;%WC10A_PATH%\BINB;%DEFPATH%%WC_CD_PATH%

REM If you set the following to a 1, a TNT DosStyle app will be created.
REM Otherwise a TNT NtStyle app will be created. NtStyle apps will *only*
REM run under real DOS when using our libraries, since we require access
REM to functions that the Win32 API does not support (such as direct access
REM to video memory, calling Int 10h BIOS functions etc). DosStyle apps
REM will however run fine in both DOS and a Win95 DOS box (NT DOS boxes don't
REM work too well).
REM
REM If you are using the RealTime DOS extender, your apps *must* be NtStyle,
REM and hence will never be able to run under Win95 or WinNT, only DOS.

SET DOSSTYLE=1

echo Watcom C/C++ 10.0a 32-bit DOS compilation environment set up (TNT).
