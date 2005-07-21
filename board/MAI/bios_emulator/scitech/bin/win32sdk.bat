@echo off
REM: Set up environment variables for Microsoft Platform SDK development
REM: Note that we have hard coded this for Windows NT i386 development.

SET MSTOOLS=%MSSDK%
SET DXSDKROOT=%MSTOOLS%
SET INETSDK=%MSTOOLS%
SET BKOFFICE=%MSTOOLS%
SET BASEMAKE=%BKOFFICE%\INCLUDE\BKOffice.Mak
SET INCLUDE=.;INCLUDE;%SCITECH%\INCLUDE;%PRIVATE%\INCLUDE;%MSTOOLS%\INCLUDE;%C_INCLUDE%
if .%1%==.borland goto borland
SET LIB=%MSTOOLS%\LIB;%LIB%
goto notborland
:borland
SET LIB=%MSTOOLS%\LIB\BORLAND;%LIB%
:notborland
SET PATH=%MSTOOLS%\Bin\;%MSTOOLS%\Bin\WinNT;%PATH%
SET CPU=i386

echo Microsoft Platform SDK support enbabled.
