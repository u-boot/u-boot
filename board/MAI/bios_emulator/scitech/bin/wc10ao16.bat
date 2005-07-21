@echo off
REM Setup for compiling with Watcom C/C++ 10.0a in 16-bit OS/2 mode

if .%CHECKED%==.1 goto checked_build
SET LIB=%SCITECH_LIB%\lib\release\os216\wc10;.
echo Release build enabled.
goto setvars

:checked_build
SET LIB=%SCITECH_LIB%\lib\debug\os216\wc10;.
echo Checked debug build enabled.
goto setvars

:setvars
SET EDPATH=%WC10A_PATH%\eddat
SET INCLUDE=include;%SCITECH%\include;%PRIVATE%\include;%WC10A_PATH%\h\os2;%WC10A_PATH%\h
SET WATCOM=%WC10A_PATH%
SET MAKESTARTUP=%SCITECH%\makedefs\wc16.mk
SET USE_WIN16=
SET USE_WIN32=
SET USE_WIN386=
SET USE_OS216=1
SET USE_OS232=
SET USE_OS2GUI=
SET USE_SNAP=
SET WC_LIBBASE=wc10
SET EDPATH=%WC10A_PATH%\EDDAT
PATH %SCITECH_BIN%;%WC10A_PATH%\BINNT;%WC10A_PATH%\BINB;%DEFPATH%%WC_CD_PATH%

echo Watcom C/C++ 10.0a 16-bit OS/2 compilation environment set up.
