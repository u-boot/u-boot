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
#               Watcom C++ 10.x 16 bit version. Supports 16-bit DOS,
#               16-bit Windows development and 16-bit OS/2 development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : WC_LIBBASE USE_WIN16 USE_OS216 USE_OS2GUI

# Default commands for compiling, assembling linking and archiving
   CC           := wcc      # C-compiler and flags
   CPP          := wpp      # C++-compiler and flags
   CFLAGS       := -ml-zq-j-w2-s-fh -fhq
.IF $(USE_TASM32)
   AS           := tasm32
.ELIF $(USE_TASMX)
   AS           := tasmx    # Assembler and flags
.ELSE
   AS           := tasm     # Assembler and flags
.ENDIF
   AS           := tasm     # Assembler and flags
   ASFLAGS      := /t /mx /m /D__LARGE__ /iINCLUDE /i$(SCITECH)\INCLUDE
   LD           := wlink    # Loader and flags
   LDFLAGS       =
   RC           := wrc      # Watcom resource compiler
   RCFLAGS      := /bt=windows
   LIB          := wlib     # Librarian
   LIBFLAGS     := -q
   ILIB         := wlib     # Import librarian
   ILIBFLAGS    := -c

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -d2      # Turn on debugging for C compiler
   LIBFLAGS     += -p=128   # Larger page size for libraries with debug info!
   ASFLAGS      += /zi      # Turn on debugging for assembler
   LDFLAGS      += D A      # Turn on debugging for linker
.ELSE
   ASFLAGS      += /q       # Suppress object records not needed for linking
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += -onatx-5
.ELIF $(OPT_SIZE)
   CFLAGS       += -onaslmr-5
.END

# Optionally turn on direct i387 FPU instructions optimised for Pentium

.IF $(FPU)
   CFLAGS       += -fpi87-fp5-DFPU387
   ASFLAGS      += -DFPU387
.END

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += -DBETA
   ASFLAGS      += -DBETA
.END

# Use a larger stack during linking if requested

.IF $(STKSIZE)
    LDFLAGS     += OP STACK=$(STKSIZE)
.ENDIF

.IF $(USE_OS216)
.IF $(BUILD_DLL)
    CFLAGS       += -bd-bt=os2-DBUILD_DLL
    ASFLAGS      += -DBUILD_DLL
.ELSE
    CFLAGS       += -bt=os2
.ENDIF
    DX_ASFLAGS   += -D__OS216__
    LIB_OS       = os216
.ELIF $(USE_WIN16)
.IF $(BUILD_DLL)
    CFLAGS       += -bd-bt=windows-D_WINDOWS-DBUILD_DLL
    ASFLAGS      += -DBUILD_DLL
.ELSE
    CFLAGS       += -bt=windows-D_WINDOWS
.ENDIF
    DX_ASFLAGS   += -D__WINDOWS16__
    LIB_OS       = WIN16
.ELSE
    USE_REALDOS  := 1
	LIB_OS       = DOS16
.END

# Place to look for PMODE library files

PMLIB           := pm.lib,

# Define the base directory for library files

.IF $(CHECKED)
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\debug
CFLAGS			+= -DCHECKED=1
.ELSE
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\release
.ENDIF

# Define where to install library files
   LIB_BASE     := $(LIB_BASE_DIR)\$(LIB_OS)\$(WC_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Define which file contains our rules

   RULES_MAK	:= wc16.mk
