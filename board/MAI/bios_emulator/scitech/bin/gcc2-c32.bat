@echo off
REM Setup for compiling with GNU C compiler

if .%CHECKED%==.1 goto checked_build
set LIB=%SCITECH_LIB%\LIB\release\win32\gcc2
echo Release build enabled.
goto setvars

:checked_build
set LIB=%SCITECH_LIB%\LIB\debug\win32\gcc2
echo Checked debug build enabled.
goto setvars

:setvars
set INCLUDE=include;%SCITECH%\include;%PRIVATE%\include
set MAKESTARTUP=%SCITECH%\makedefs\gcc_win32.mk
set MAKE_MODE=
set USE_WIN16=
set USE_WIN32=1
set WIN32_GUI=
set USE_SNAP=
set GCC_LIBBASE=gcc2
PATH %SCITECH_BIN%;%GCC2_PATH%\NATIVE\BIN;%DEFPATH%

echo GCC 2.9.x 32-bit Win32 console compilation environment set up

