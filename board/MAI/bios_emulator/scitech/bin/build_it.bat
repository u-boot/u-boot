@echo off
rem Generic batch file to build a version of the library. This batch file
rem assumes that the correct batch files exist to setup the appropriate
rem compilation environments, and that the DMAKE.EXE program is available
rem somewhere on the path.
rem
rem Builds as release or debug depending on the value of the CHECKED
rem environment variable.

rem Unset all environment variables that change the compile process
set DBG=
set OPT=
set OPT_SIZE=
set BUILD_DLL=
set IMPORT_DLL=
set FPU=
set CHECKS=
set BETA=

if %1==bc31-d16 goto bc31-d16
if %1==bc45-d16 goto bc45-d16
if %1==bc45-d32 goto bc45-d32
if %1==bc45-tnt goto bc45-tnt
if %1==bc45-w16 goto bc45-w16
if %1==bc45-w32 goto bc45-w32
if %1==bc45-c32 goto bc45-c32
if %1==bc45-vxd goto bc45-vxd
if %1==bc45-snp goto bc45-snp
if %1==bc50-d16 goto bc50-d16
if %1==bc50-d32 goto bc50-d32
if %1==bc50-tnt goto bc50-tnt
if %1==bc50-w16 goto bc50-w16
if %1==bc50-w32 goto bc50-w32
if %1==bc50-c32 goto bc50-c32
if %1==bc50-vxd goto bc50-vxd
if %1==bc50-snp goto bc50-snp
if %1==gcc2-d32 goto gcc2-d32
if %1==gcc2-w32 goto gcc2-w32
if %1==gcc2-c32 goto gcc2-c32
if %1==gcc2-linux goto gcc2-linux
if %1==vc40-d16 goto vc40-d16
if %1==vc40-tnt goto vc40-tnt
if %1==vc40-w16 goto vc40-w16
if %1==vc40-w32 goto vc40-w32
if %1==vc40-c32 goto vc40-c32
if %1==vc40-drv9x goto vc40-drv9x
if %1==vc40-drvnt goto vc40-drvnt
if %1==vc40-rtt goto vc40-rtt
if %1==vc40-snp goto vc40-snp
if %1==vc50-d16 goto vc50-d16
if %1==vc50-tnt goto vc50-tnt
if %1==vc50-w16 goto vc50-w16
if %1==vc50-w32 goto vc50-w32
if %1==vc50-c32 goto vc50-c32
if %1==vc50-drv9x goto vc50-drv9x
if %1==vc50-drvnt goto vc50-drvnt
if %1==vc50-rtt goto vc50-rtt
if %1==vc50-snp goto vc50-snp
if %1==vc60-d16 goto vc60-d16
if %1==vc60-tnt goto vc60-tnt
if %1==vc60-w16 goto vc60-w16
if %1==vc60-w32 goto vc60-w32
if %1==vc60-c32 goto vc60-c32
if %1==vc60-drv9x goto vc60-drv9x
if %1==vc60-drvnt goto vc60-drvnt
if %1==vc60-drvw2k goto vc60-drvw2k
if %1==vc60-rtt goto vc60-rtt
if %1==vc60-snp goto vc60-snp
if %1==wc10ad16 goto wc10ad16
if %1==wc10ad32 goto wc10ad32
if %1==wc10atnt goto wc10atnt
if %1==wc10aw16 goto wc10aw16
if %1==wc10aw32 goto wc10aw32
if %1==wc10ac32 goto wc10ac32
if %1==wc10ao32 goto wc10ao32
if %1==wc10ap32 goto wc10ap32
if %1==wc10asnp goto wc10asnp
if %1==wc10-d16 goto wc10-d16
if %1==wc10-d32 goto wc10-d32
if %1==wc10-tnt goto wc10-tnt
if %1==wc10-w16 goto wc10-w16
if %1==wc10-w32 goto wc10-w32
if %1==wc10-c32 goto wc10-c32
if %1==wc10-o32 goto wc10-o32
if %1==wc10-p32 goto wc10-p32
if %1==wc10-snp goto wc10-snp
if %1==wc11-d16 goto wc11-d16
if %1==wc11-d32 goto wc11-d32
if %1==wc11-tnt goto wc11-tnt
if %1==wc11-w16 goto wc11-w16
if %1==wc11-w32 goto wc11-w32
if %1==wc11-c32 goto wc11-c32
if %1==wc11-o32 goto wc11-o32
if %1==wc11-p32 goto wc11-p32
if %1==wc11-snp goto wc11-snp

echo Usage: BUILD 'compiler_name' [DMAKE commands]
echo.
echo Where 'compiler_name' is of the form comp-os, where
echo 'comp' defines the compiler and 'os' defines the OS environment.
echo For instance 'bc50-w32' is for Borland C++ 5.0 for Win32.
echo The value of 'comp' can be any of the following:
echo.
echo    bc45 - Borland C++ 4.5x
echo    bc50 - Borland C++ 5.x
echo    vc40 - Visual C++ 4.x
echo    vc50 - Visual C++ 5.x
echo    vc60 - Visual C++ 6.x
echo    wc10 - Watcom C++ 10.6
echo    wc11 - Watcom C++ 11.0
echo    gcc2 - GNU C/C++ 2.9x
echo.
echo The value of 'os' can be one of the following:
echo.
echo    d16   - 16-bit DOS
echo    d32   - 32-bit DOS
echo    w16   - 16-bit Windows GUI mode
echo    c32   - 32-bit Windows console mode
echo    w32   - 32-bit Windows GUI mode
echo    o16   - 16-bit OS/2 console mode
echo    o32   - 32-bit OS/2 console mode
echo    p32   - 32-bit OS/2 Presentation Manager
echo    snp   - 32-bit SciTech Snap application
echo    linux - 32-bit Linux application
goto end

rem -------------------------------------------------------------------------
rem Setup for the specified compiler

:bc31-d16
call bc31-d16.bat
goto compileit

:bc45-d16
call bc45-d16.bat
goto compileit

:bc45-d32
call bc45-d32.bat
goto compileit

:bc45-tnt
call bc45-tnt.bat
goto compileit

:bc45-w16
call bc45-w16.bat
goto compileit

:bc45-w32
call bc45-w32.bat
goto compileit

:bc45-c32
call bc45-c32.bat
goto compileit

:bc45-vxd
call bc45-vxd.bat
goto compileit

:bc50-d16
call bc50-d16.bat
goto compileit

:bc50-d32
call bc50-d32.bat
goto compileit

:bc50-tnt
call bc50-tnt.bat
goto compileit

:bc50-w16
call bc50-w16.bat
goto compileit

:bc50-w32
call bc50-w32.bat
goto compileit

:bc50-c32
call bc50-c32.bat
goto compileit

:bc50-vxd
call bc50-vxd.bat
goto compileit

:gcc2-d32
call gcc2-d32.bat
goto compileit

:gcc2-w32
call gcc2-w32.bat
goto compileit

:gcc2-c32
call gcc2-c32.bat
goto compileit

:gcc2-linux
call gcc2-linux.bat
goto compileit

:sc70-d16
call sc70-d16.bat
goto compileit

:sc70-w16
call sc70-w16.bat
goto compileit

:sc70-tnt
call sc70-tnt.bat
goto compileit

:sc70-w32
call sc70-w32.bat
goto compileit

:sc70-c32
call sc70-c32.bat
goto compileit

:vc40-d16
call vc40-d16.bat
goto compileit

:vc40-tnt
call vc40-tnt.bat
goto compileit

:vc40-w16
call vc40-w16.bat
goto compileit

:vc40-w32
call vc40-w32.bat
goto compileit

:vc40-c32
call vc40-c32.bat
goto compileit

:vc40-drv9x
call vc40-drv9x.bat
goto compileit

:vc40-drvnt
call vc40-drvnt.bat
goto compileit

:vc40-rtt
call vc40-rtt.bat
goto compileit

:vc50-d16
call vc50-d16.bat
goto compileit

:vc50-tnt
call vc50-tnt.bat
goto compileit

:vc50-w16
call vc50-w16.bat
goto compileit

:vc50-w32
call vc50-w32.bat
goto compileit

:vc50-c32
call vc50-c32.bat
goto compileit

:vc50-drv9x
call vc50-drv9x.bat
goto compileit

:vc50-drvnt
call vc50-drvnt.bat
goto compileit

:vc50-rtt
call vc50-rtt.bat
goto compileit

:vc60-d16
call vc60-d16.bat
goto compileit

:vc60-tnt
call vc60-tnt.bat
goto compileit

:vc60-w16
call vc60-w16.bat
goto compileit

:vc60-w32
call vc60-w32.bat
goto compileit

:vc60-c32
call vc60-c32.bat
goto compileit

:vc60-drv9x
call vc60-drv9x.bat
goto compileit

:vc60-drvnt
call vc60-drvnt.bat
goto compileit

:vc60-drvw2k
call vc60-drvw2k.bat
goto compileit

:vc60-rtt
call vc60-rtt.bat
goto compileit

:wc10ad16
call wc10ad16.bat
goto compileit

:wc10ad32
call wc10ad32.bat
goto compileit

:wc10atnt
call wc10atnt.bat
goto compileit

:wc10aw16
call wc10aw16.bat
goto compileit

:wc10aw32
call wc10aw32.bat
goto compileit

:wc10ac32
call wc10ac32.bat
goto compileit

:wc10ao32
call wc10ao32.bat
goto compileit

:wc10ap32
call wc10ap32.bat
goto compileit

:wc10-d16
call wc10-d16.bat
goto compileit

:wc10-d32
call wc10-d32.bat
goto compileit

:wc10-tnt
call wc10-tnt.bat
goto compileit

:wc10-w16
call wc10-w16.bat
goto compileit

:wc10-w32
call wc10-w32.bat
goto compileit

:wc10-c32
call wc10-c32.bat
goto compileit

:wc10-o32
call wc10-o32.bat
goto compileit

:wc10-p32
call wc10-p32.bat
goto compileit

:wc11-d16
call wc11-d16.bat
goto compileit

:wc11-d32
call wc11-d32.bat
goto compileit

:wc11-tnt
call wc11-tnt.bat
goto compileit

:wc11-w16
call wc11-w16.bat
goto compileit

:wc11-w32
call wc11-w32.bat
goto compileit

:wc11-c32
call wc11-c32.bat
goto compileit

:wc11-o32
call wc11-o32.bat
goto compileit

:wc11-p32
call wc11-p32.bat
goto compileit

:compileit
k_rm -f *.lib *.a
dmake %2 %3 %4 %5 %6 %7 %8 %9
if errorlevel 1 goto errorend
goto end

:errorend
echo *************************************************
echo * An error occurred while building the library. *
echo *************************************************
:end
