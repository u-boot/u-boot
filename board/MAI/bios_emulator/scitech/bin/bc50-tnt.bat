@echo off
REM Setup for compiling with Borland C++ 5.0 in 32 bit mode with Phar Lap TNT

if .%CHECKED%==.1 goto checked_build
SET LIB=%SCITECH_LIB%\LIB\RELEASE\DOS32\BC5;%BC5_PATH%\LIB;%TNT_PATH%\LIB;.
echo Release build enabled.
goto setvars

:checked_build
SET LIB=%SCITECH_LIB%\LIB\DEBUG\DOS32\BC5;%BC5_PATH%\LIB;%TNT_PATH%\LIB;.
echo Checked debug build enabled.
goto setvars

:setvars
SET INCLUDE=INCLUDE;%SCITECH%\INCLUDE;%PRIVATE%\INCLUDE;%BC5_PATH%\INCLUDE;%TNT_PATH%\INCLUDE;
SET MAKESTARTUP=%SCITECH%\MAKEDEFS\BC32.MK
SET USE_DPMI16=
SET USE_WIN16=
SET USE_WIN32=
SET USE_VXD=
SET USE_TNT=1
SET USE_SMX32=
SET USE_SMX16=
SET USE_BC5=1
SET WIN32_GUI=
SET USE_SNAP=
SET BC_LIBBASE=BC5
PATH %SCITECH_BIN%;%BC5_PATH%\BIN;%TNT_PATH%\BIN;%DEFPATH%%BC_CD_PATH%

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

SET DOSSTYLE=

REM: Create Borland compile/link configuration scripts
echo -I%INCLUDE% > %BC5_PATH%\BIN\bcc32.cfg
echo -L%LIB% >> %BC5_PATH%\BIN\bcc32.cfg
echo -L%LIB% > %BC5_PATH%\BIN\tlink32.cfg

echo Borland C++ 5.0 32 bit DOS compilation configuration set up (TNT).
