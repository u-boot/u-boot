@echo off
REM Setup for compiling with Borland C++ 3.1.

if .%CHECKED%==.1 goto checked_build
SET LIB=%SCITECH_LIB%\LIB\RELEASE\DOS16\BC3;%BC3_PATH%\LIB;.
echo Release build enabled.
goto setvars

:checked_build
SET LIB=%SCITECH_LIB%\LIB\DEBUG\DOS16\BC3;%BC3_PATH%\LIB;.
echo Checked debug build enabled.
goto setvars

:setvars
SET INCLUDE=INCLUDE;%SCITECH%\INCLUDE;%PRIVATE%\INCLUDE;%BC3_PATH%\INCLUDE;
SET MAKESTARTUP=%SCITECH%\MAKEDEFS\BC3.MK
SET USE_DPMI16=
SET USE_WIN16=
SET USE_WIN32=
SET USE_SNAP=
PATH %SCITECH_BIN%;%BC3_PATH%\BIN;%DEFPATH%

REM: Create Borland compile/link configuration scripts
echo -I%INCLUDE% > %BC3_PATH%\BIN\turboc.cfg
echo -L%LIB% >> %BC3_PATH%\BIN\turboc.cfg
echo -L%LIB% > %BC3_PATH%\BIN\tlink.cfg

echo Borland C++ 3.1 DOS compilation configuration set up.
