@echo off
REM Setup for compiling with Borland C++ 4.5 in 32 bit Windows mode.

if .%CHECKED%==.1 goto checked_build
SET LIB=%SCITECH_LIB%\LIB\RELEASE\SNAP\BC4;%BC4_PATH%\LIB;.
echo Release build enabled.
goto setvars

:checked_build
SET LIB=%SCITECH_LIB%\LIB\DEBUG\SNAP\BC4;%BC4_PATH%\LIB;.
echo Checked debug build enabled.
goto setvars

:setvars
SET INCLUDE=INCLUDE;%SCITECH%\INCLUDE;%PRIVATE%\INCLUDE
SET MAKESTARTUP=%SCITECH%\MAKEDEFS\BC32.MK
SET USE_WIN16=
SET USE_WIN32=
SET USE_VXD=
SET USE_TNT=
SET USE_BC5=
SET WIN32_GUI=
SET USE_SNAP=1
SET BC_LIBBASE=BC4
PATH %SCITECH_BIN%;%BC4_PATH%\BIN;%DEFPATH%%BC_CD_PATH%

REM: Create Borland compile/link configuration scripts
echo -I%INCLUDE% > %BC4_PATH%\BIN\bcc32.cfg
echo -L%LIB% >> %BC4_PATH%\BIN\bcc32.cfg
echo -L%LIB% > %BC4_PATH%\BIN\tlink32.cfg

echo Borland C++ 4.5 Snap compilation configuration set up.
