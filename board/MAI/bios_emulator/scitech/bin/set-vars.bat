@echo off
REM:=========================================================================
REM: Master batch file to set up all necessary environment variables for
REM: the SciTech makefile utilities. This batch file should be executed
REM: *first* before any other batch files when you start a command shell.
REM: You should not need to modify any batch files except this one to
REM: configure the makefile utilities.
REM:=========================================================================

REM: Set the place where SciTech Software is installed, and where each
REM: of the supported compilers is installed. These environment variables
REM: are used by the batch files in the SCITECH\BIN directory.
REM:
REM: Modify the as appropriate for your compiler configuration (you should
REM: only need to change things in this batch file).
REM:
REM: This version is for a normal MSDOS installation.

REM: The SCITECH variable points to where batch files, makefile startups,
REM: include files and source files will be found when compiling.

SET SCITECH=c:\scitech

REM: The SCITECH_LIB variable points to where the SciTech libraries live
REM: for installation and linking. This allows you to have the source and
REM: include files on local machines for compiling and have the libraries
REM: located on a common network machine (for network builds).

SET SCITECH_LIB=%SCITECH%

REM: The PRIVATE variable points to where private source files reside that
REM: do not live in the public source tree

SET PRIVATE=c:\private

REM: The following sets up the path to the SciTech command line utilities
REM: for the development operating system. We select either DOS hosted
REM: tools or Win32 hosted tools depending on whether you are running
REM: on NT or not. Windows 9x users can use the Win32 hosted tools but
REM: they run slower, but you will have long filenames if you do this.

IF .%OS%==.Windows_NT goto Win32_path
IF NOT .%WINDIR%==. goto Win32_path
SET SCITECH_BIN=%SCITECH%\bin;%SCITECH%\bin-dos
goto path_set

REM: The following sets up the path to the SciTech command line utilities
REM: for the development operating system. This version uses the Win32
REM: hosted tools by default, so you can use long filenames.

:Win32_path
SET SCITECH_BIN=%SCITECH%\bin;%SCITECH%\bin-win32

:path_set

REM: Set the TMP variable for dmake if this is not already set

SET TMP=%SCITECH%

REM: Set the following environment variable to use the Netwide Assembler
REM: (NASM) provided with the MGL tools to build all assembler modules.
REM: If you have Turbo Assembler 4.0 or later and you wish to use it,
REM: you can use it by removing the following line.

SET USE_NASM=1

REM: The following is used to set up DDK directories for device driver
REM: development. They can safely be ignored unless you are using the
REM: SciTech makefile utilities to build device drivers.

SET DDKDRIVE=c:
SET MSSDK=c:\c\win32sdk
SET W95_DDKROOT=c:\c\95ddk
SET W98_DDKROOT=c:\c\98ddk
SET NT_DDKROOT=c:\c\ntddk
SET W2K_DDKROOT=c:\c\2000ddk
SET MASM_ROOT=c:\c\masm611
SET VTOOLSD=c:\c\vtd95
SET SOFTICE_PATH=c:\c\sint

REM: The following define the locations of all the compilers that you may
REM: be using. Change them to reflect where you have installed your
REM: compilers.

SET BC3_PATH=c:\c\bc3
SET BC4_PATH=c:\c\bc45
SET BC5_PATH=c:\c\bc50
SET BCB5_PATH=c:\c\bcb50
SET VC_PATH=c:\c\msvc
SET VC4_PATH=c:\c\vc42
SET VC5_PATH=c:\c\vc50
SET VC6_PATH=c:\c\vc60
SET SC70_PATH=c:\c\sc75
SET WC10A_PATH=c:\c\wc10a
SET WC10_PATH=c:\c\wc10
SET WC11_PATH=c:\c\wc11
SET TNT_PATH=c:\c\tnt
SET DJ_PATH=c:\c\djgpp
SET GCC2_PATH=c:\unix\usr

REM: The following define the locations of the IDE and compiler path
REM: tools for Visual C++. If you do a standard installation, you wont
REM: need to change this. If however you did a custom install and changed
REM: the paths to these directory, you will need to modify this to suit.

SET VC5_MSDevDir=%VC5_PATH%\sharedide
SET VC5_MSVCDir=%VC5_PATH%\vc
SET VC6_MSDevDir=%VC6_PATH%\common\msdev98
SET VC6_MSVCDir=%VC6_PATH%\vc98

