@echo off
REM Setup for compiling with Watcom C/C++ 11.0 in 16-bit OS/2 mode

if .%CHECKED%==.1 goto checked_build
SET LIB=%SCITECH_LIB%\lib\release\os216\wc11;.
echo Release build enabled.
goto setvars

:checked_build
SET LIB=%SCITECH_LIB%\lib\debug\os216\wc11;.
echo Checked debug build enabled.
goto setvars

:setvars
SET EDPATH=%WC11_PATH%\eddat
SET INCLUDE=include;%SCITECH%\include;%PRIVATE%\include;%WC11_PATH%\h\os2;%WC11_PATH%\h
SET WATCOM=%WC11_PATH%
SET MAKESTARTUP=%SCITECH%\makedefs\wc16.mk
SET USE_WIN16=
SET USE_WIN32=
SET USE_WIN386=
SET USE_OS216=1
SET USE_OS232=
SET USE_OS2GUI=
SET USE_SNAP=
SET USE_QNX4=
SET WC_LIBBASE=wc11
SET EDPATH=%WC11_PATH%\EDDAT
PATH %SCITECH_BIN%;%WC11_PATH%\BINNT;%WC11_PATH%\BINW;%DEFPATH%%WC_CD_PATH%

echo Watcom C/C++ 11.0 16-bit OS/2 compilation environment set up.
