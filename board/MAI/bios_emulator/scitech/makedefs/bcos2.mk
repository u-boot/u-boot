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
#               Borland C++ 2.0 32-bit OS/2 version.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : USE_OS2GUI BC_LIBBASE

# We are compiling for a 32 bit envionment
   _32BIT_      := 1

# Default commands for compiling, assembling linking and archiving
   CC           := bcc
   CFLAGS       := -w- -4 -H=bcc32.sym -i60 -d
.IF $(USE_NASM)
   AS           := nasm
   ASFLAGS      := -t -f obj -d__FLAT__ -iINCLUDE -i$(SCITECH)\INCLUDE
.ELSE
   AS           := tasm
   ASFLAGS      := /t /mx /m /D__FLAT__ /D__OS2__ /iINCLUDE /i$(SCITECH)\INCLUDE
.ENDIF
   LD           := bclink tlink.exe
   LDFLAGS      := -c
   RC           := brcc
   RCFLAGS      :=
   LIB          := tlib
   LIBFLAGS     := /C /P32
   ILIB         := implib
   ILIBFLAGS    := -c
.IF $(USE_OS2GUI)
   CFLAGS		+= -D__OS2_PM__
.ENDIF

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -v
   LDFLAGS      += -v
   LIBFLAGS     += /P128
.IF $(USE_NASM)
   ASFLAGS      += -F borland
.ELSE
   ASFLAGS      += /zi
.ENDIF
.ELSE
   LDFLAGS      += -x
.IF $(USE_NASM)
   ASFLAGS      += -F null
.ELSE
   ASFLAGS      += /q
.ENDIF
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += -5 -O2 -k-
.ELIF $(OPT_SIZE)
   CFLAGS       += -5 -O1 -k-
.END

# Optionally turn on direct i387 FPU instructions
.IF $(FPU)
   CFLAGS       += -DFPU387
   ASFLAGS      += -dFPU387
.END

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += -DBETA
   ASFLAGS      += -dBETA
.END

# Optionally use Phar Lap's TNT DOS Extender, otherwise use the DOS Power Pack
.IF $(BUILD_DLL)
   CFLAGS       += -sd -sm -DBUILD_DLL
   ASFLAGS      += -dBUILD_DLL
.ELSE
   CFLAGS       += -sm
.ENDIF
   DEF_LIBS	:= os2.lib c2mt.lib
   DX_ASFLAGS   += -d__OS2__
   LIB_OS       = os232

# Define the base directory for library files

.IF $(CHECKED)
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\debug
CFLAGS          += -DCHECKED=1
.ELSE
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\release
.ENDIF

# Define where to install library files
   LIB_BASE     := $(LIB_BASE_DIR)\$(LIB_OS)\$(BC_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Place to look for PMODE library files

.IF $(USE_OS2GUI)
DEF_LIBS        += pm_pm.lib
.ELSE
DEF_LIBS        += pm.lib
.ENDIF

# Define which file contains our rules

   RULES_MAK	:= bcos2.mk
