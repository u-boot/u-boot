#############################################################################
#
#                                       SciTech Multi-platform Graphics Library
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
#               IBM VisualAge C++ 3.0 OS/2 32-bit version.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : VA_LIBBASE USE_OS232 USE_OS2GUI FULLSCREEN NOOPT MAX_WARN

# We are compiling for a 32 bit envionment
   _32BIT_      := 1

# Default commands for compiling, assembling linking and archiving
   CC           := icc
   CPP          := icc
   CFLAGS       := /Q /G5 /Gl+ /Fi /Si /J- /Ss+ /Sp1 /Gm+ /I.
.IF $(USE_NASM)
   AS           := nasm
   ASFLAGS      := -t -f obj -F null -d__FLAT__ -dSTDCALL_MANGLE -d__NOU_VAR__ -iINCLUDE -i$(SCITECH)\INCLUDE
.ELSE
.IF $(USE_TASM32)
   AS           := tasm32
.ELIF $(USE_TASMX)
   AS           := tasmx
.ELSE
   AS           := tasm
.ENDIF
   ASFLAGS      := /t /mx /m /D__FLAT__ /DSTDCALL_MANGLE /D__NOU_VAR__ /iINCLUDE /i$(SCITECH)\INCLUDE
.ENDIF
   LD           := ilink
   LDFLAGS       = /noi /exepack:2 /packcode /packdata /align:32 /map /noe
   RC           := rc
   RCFLAGS      := -n -x2
   LIB          := ilib
   LIBFLAGS     := /nologo
   ILIB         := implib
   ILIBFLAGS    := /nologo
   IPFC         := ipfc
   IPFCFLAGS    :=
   IBMCOBJ      := 1

# Set the compiler warning level
.IF $(MAX_WARN)
   CFLAGS       += /W3
.ELSE
   CFLAGS       += /W1
.ENDIF

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += /Ti
   LDFLAGS      += /DE
.ELSE
.IF $(USE_TASM)
   ASFLAGS      += /q
.ENDIF
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += /Gfi /O /Oi
.ELIF $(OPT_SIZE)
   CFLAGS       += /Gfi /O /Oc
.ELIF $(NOOPT)
   CFLAGS       += /O-
.END

# Optionally turn on direct i387 FPU instructions optimised for Pentium
.IF $(FPU)
   CFLAGS       += -DFPU387
   ASFLAGS      += -dFPU387
.END

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += -DBETA
   ASFLAGS      += -dBETA
.END

# Build 32-bit OS/2 apps
.IF $(BUILD_DLL)
   CFLAGS       += /Ge- /DBUILD_DLL
   LDFLAGS      += /DLL /NOE
   ASFLAGS      += -dBUILD_DLL
.ELSE
.IF $(USE_OS2GUI)
   CFLAGS       += -D__OS2_PM__
   LDFLAGS      += /PMTYPE:PM
.ELSE
.IF $(FULLSCREEN)
   LDFLAGS      += /PMTYPE:NOVIO
.ELSE
   LDFLAGS      += /PMTYPE:VIO
.ENDIF
.ENDIF
.ENDIF
   DX_ASFLAGS   += -d__OS2__
   LIB_OS       = os232

# Place to look for PMODE library files

.IF $(USE_OS2GUI)
.IF $(USE_SDDPMDLL)
#Note: This is OK for now but might need to be changed if the GUI PM library
#      were really different
PMLIB           := sddpmlib.lib
.ELSE
PMLIB           := pm_pm.lib
.ENDIF
.ELSE
.IF $(USE_SDDPMDLL)
PMLIB           := sddpmlib.lib
.ELSE
PMLIB           := pm.lib
.ENDIF
.ENDIF

# Define the base directory for library files

.IF $(CHECKED)
LIB_BASE_DIR    := $(SCITECH_LIB)\lib\debug
CFLAGS          += /DCHECKED=1
.ELSE
LIB_BASE_DIR    := $(SCITECH_LIB)\lib\release
.ENDIF

# Define where to install library files
   LIB_BASE     := $(LIB_BASE_DIR)\$(LIB_OS)\$(VA_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Define which file contains our rules

   RULES_MAK    := va32.mk
