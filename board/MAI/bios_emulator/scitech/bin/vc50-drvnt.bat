@echo off
REM Setup environment variables for Visual C++ 6.0 32 bit edition

REM: First setup for Win32 console development (with Platform SDK)
call vc60-c32.bat sdk > NUL

REM: Now setup stuff for the NT DDK build environment
call ntddk.bat

if .%CHECKED%==.1 goto checked_build
echo Release build enabled.
goto done
:checked_build
echo Checked debug build enabled.
goto done
:done
echo Visual C++ 6.0 Windows NT driver compilation environment set up
