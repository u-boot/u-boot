@echo off
REM Setup environment variables for Visual C++ 4.2 32 bit edition

REM: First setup for Win32 console development
call vc40-c32.bat > NUL

REM: Extra stuff to set up for Windows 9x DDK development
set MASTER_MAKE=1
set DDKROOT=%W95_DDKROOT%
set SDKROOT=%MSSDK%
set C16_ROOT=%VC_PATH%
set C32_ROOT=%VC4_PATH%

if .%CHECKED%==.1 goto checked_build
echo Release build enabled.
goto done
:checked_build
echo Checked debug build enabled.
goto done
:done
echo Visual C++ 4.2 Windows 9x driver compilation environment set up
