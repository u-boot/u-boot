#############################################################################
#
#					SciTech Multi-platform Graphics Library
#
#  ========================================================================
#
#    The contents of this file are subject to the SciTech MGL Public
#    License Version 1.0 (the "License"); you may not use this file
#    except in compliance with the License. You may obtain a copy of
#    the License at http://www.scitechsoft.com/mgl-license.txt
#
#    Software distributed under the License is distributed on an
#    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
#    implied. See the License for the specific language governing
#    rights and limitations under the License.
#
#    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
#
#    The Initial Developer of the Original Code is SciTech Software, Inc.
#    All Rights Reserved.
#
#  ========================================================================
#
# Descripton:   Generic DMAKE startup makefile definitions file. Assumes
#               that the SCITECH environment variable has been set to point
#               to where all our stuff is installed. You should not need
#               to change anything in this file.
#
#               Microsoft Visual C++ 2.x 32 bit version. Supports Phar Lap
#               TNT DOS Extender and 32 bit Windows development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : TNT_PATH VC_LIBBASE DOSSTYLE USE_TNT USE_RTTARGET MSVCDIR
.IMPORT .IGNORE : USE_VXD USE_NTDRV USE_W2KDRV NT_DDKROOT USE_RTTI USE_CPPEXCEPT

# We are compiling for a 32 bit envionment
   _32BIT_      := 1

# Default commands for compiling, assembling linking and archiving
   CC           := cl       # C-compiler and flags
   CFLAGS       :=
.IF $(USE_NASM)
   AS			:= nasm
   ASFLAGS      := -t -f win32 -F null -d__FLAT__ -dSTDCALL_MANGLE -iINCLUDE -i$(SCITECH)\INCLUDE
.ELSE
.IF $(USE_TASM32)
   AS           := tasm32
.ELIF $(USE_TASMX)
   AS           := tasmx    # Assembler and flags
.ELSE
   AS           := tasm     # Assembler and flags
.ENDIF
   ASFLAGS      := /t /mx /m /D__FLAT__ /DSTDCALL_MANGLE /iINCLUDE /i$(SCITECH)\INCLUDE
.ENDIF
   LD           := cl
.IF $(USE_WIN32)
   LDFLAGS       = $(CFLAGS)
.IF $(USE_NTDRV)
   LDENDFLAGS   = -link /INCREMENTAL:NO /DRIVER /SUBSYSTEM:NATIVE,4.00 /VERSION:4.00 /MACHINE:I386 /NODEFAULTLIB /DEBUGTYPE:CV /PDB:NONE /ALIGN:0x20 /BASE:0x10000 /ENTRY:DriverEntry@8
	#/MERGE:_page=page /MERGE:_text=.text /MERGE:.rdata=.text
.ELIF $(WIN32_GUI)
   LDENDFLAGS   = -link /INCREMENTAL:NO /DEF:$(@:b).def /SUBSYSTEM:WINDOWS /MACHINE:I386 /DEBUGTYPE:CV /PDB:NONE
.ELSE
   LDENDFLAGS   = -link /INCREMENTAL:NO /SUBSYSTEM:CONSOLE /MACHINE:I386 /DEBUGTYPE:CV /PDB:NONE
.ENDIF
.ELSE
   LDFLAGS       = $(CFLAGS)
   LDENDFLAGS   := -link -stub:$(TNT_PATH:s/\/\\)\\bin\\gotnt.exe /PDB:NONE
.ENDIF
   RC           := rc       # Watcom resource compiler
   RCFLAGS      :=          # Mark as Win32 compatible resources
   LIB          := lib      # Librarian
   LIBFLAGS     :=
   ILIB         := lib      # Import librarian
   ILIBFLAGS    := /MACHINE:IX86
   INTEL_X86	:= 1
   NMSYM   		:= $(SOFTICE_PATH)\nmsym.exe
.IF $(USE_NTDRV)
   NMSYMFLAGS	:= /TRANSLATE:source,package,always /PROMPT /SOURCE:$(MSVCDIR)\crt\src\intel;$(SCITECH)\src\pm;$(SCITECH)\src\pm\common;$(SCITECH)\src\pm\ntdrv
.ELSE
   NMSYMFLAGS	:= /TRANSLATE:source,package,always /PROMPT /SOURCE:$(SCITECH)\src\pm;$(SCITECH)\src\pm\common;$(SCITECH)\src\pm\win32
.ENDIF

# Set the compiler warning level
.IF $(MAX_WARN)
   CFLAGS       += -W3
.ELSE
   CFLAGS       += -W1
.ENDIF

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += /Yd /Zi  # Turn on debugging for C compiler
.IF $(USE_TASM)
   ASFLAGS      += /zi      # Turn on debugging for assembler
.ENDIF
.ELSE
.IF $(USE_TASM)
   ASFLAGS      += /q       # Suppress object records not needed for linking
.ENDIF
.END

# Optionally turn on optimisations
.IF $(VC_LIBBASE) == vc5
.IF $(OPT)
   CFLAGS       += /G6 /O2 /Ox /Oi-
.ELIF $(OPT_SIZE)
   CFLAGS       += /G6 /O1
.END
.ELSE
.IF $(OPT)
   CFLAGS       += /G5 /O2 /Ox
.ELIF $(OPT_SIZE)
   CFLAGS       += /G5 /O1
.END
.ENDIF

# Optionally turn on direct i387 FPU instructions

.IF $(FPU)
   CFLAGS       += /DFPU387
   ASFLAGS      += -dFPU387
.END

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += /DBETA
   ASFLAGS      += -dBETA
.END

# Use a larger stack during linking if requested, or use a default stack
# of 50k. The usual default stack provided by Visual C++ is *way* to small
# for real 32 bit code development.

.IF $(USE_WIN32)
	# Not necessary for Win32 code.
.ELSE
.IF $(STKSIZE)
	LDENDFLAGS  += /STACK:$(STKSIZE)
.ELSE
	LDENDFLAGS  += /STACK:51200
.ENDIF
.ENDIF

# DOS extender dependant flags
.IF $(USE_NTDRV)				# Build 32 bit Windows NT driver
   CFLAGS       += /LD /Zl /Gy /Gz /GF /D__NT_DRIVER__ /D_X86_=1 /Di386=1
.IF $(DBG)
   CFLAGS       += /QIf
.ENDIF
   ASFLAGS      +=
   DEF_LIBS		:= int64.lib ntoskrnl.lib hal.lib
   DX_ASFLAGS   += -d__NT_DRIVER__
.IF $(USE_W2KDRV)				# Build 32 bit Windows 2000 driver
   LIB_OS       = W2KDRV
.ELSE
   LIB_OS       = NTDRV
.ENDIF
.ELIF $(USE_WIN32)              # Build 32 bit Windows NT app
.IF $(WIN32_GUI)
.ELSE
    CFLAGS       += -D__CONSOLE__
.ENDIF
.IF $(BUILD_DLL)
   CFLAGS       += /MT /LD /DBUILD_DLL
   ASFLAGS      += -dBUILD_DLL
.IF $(NO_RUNTIME)
   LDENDFLAGS   += /NODEFAULTLIB
   CFLAGS 		+= /Zl
   DEF_LIBS		:=
.ELSE
   DEF_LIBS		:= kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib winmm.lib comdlg32.lib comctl32.lib ole32.lib oleaut32.lib version.lib winspool.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib rpcrt4.lib
.ENDIF
.ELSE
   CFLAGS       += /MT
   DEF_LIBS		:= kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib winmm.lib comdlg32.lib comctl32.lib ole32.lib oleaut32.lib version.lib winspool.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib rpcrt4.lib
.ENDIF
   DX_ASFLAGS   += -d__WINDOWS32__
   LIB_OS       = WIN32
.ELIF $(USE_RTTARGET)
   CFLAGS       += -D__RTTARGET__
   DX_CFLAGS    +=
   DX_ASFLAGS   += -d__RTTARGET__
   USE_REALDOS	:=
   LIB_OS       = RTT32
   DEF_LIBS     := cw32mt.lib
.ELSE
   USE_TNT      := 1
   USE_REALDOS	:= 1
   CFLAGS       += /MT /D__MSDOS32__
   DX_CFLAGS    += -DTNT
   DX_ASFLAGS   += -dTNT
   LIB_OS       = DOS32
   DEF_LIBS		:= dosx32.lib tntapi.lib
.ENDIF

# Define the base directory for library files

.IF $(CHECKED)
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\debug
CFLAGS			+= /DCHECKED=1
.ELSE
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\release
.ENDIF

# Define where to install library files
   LIB_BASE     := $(LIB_BASE_DIR)\$(LIB_OS)\$(VC_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Place to look for PMODE library files

.IF $(USE_TNT)
PMLIB           := $(LIB_BASE:s/\/\\)\\tnt\\pm.lib
.ELSE
PMLIB           := $(LIB_BASE:s/\/\\)\\pm.lib
.ENDIF

# Define which file contains our rules

   RULES_MAK	:= vc32.mk
