@echo off
REM Setup environment variables for Visual C++ 4.2 32 bit edition

REM: First setup for Win32 console development (with Platform SDK)
call vc40-c32.bat sdk > NUL

REM: Extra stuff to set up for Windows NT DDK development
SET BASEDIR=%NT_DDKROOT%
SET PATH=%NT_DDKROOT%\bin;%PATH%

if .%CHECKED%==.1 goto checked_build
echo Release build enabled.
goto done
:checked_build
echo Checked debug build enabled.
goto done
:done
echo Visual C++ 4.2 Windows NT driver compilation environment set up
