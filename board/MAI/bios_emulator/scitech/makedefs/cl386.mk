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
#               Microsoft 386 C 6.0 32 bit. Supports 32 bit
#               OS/2 development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : CL_LIBBASE USE_VDD
.IMPORT .IGNORE : USE_MASM

# Default commands for compiling, assembling linking and archiving
   CC           := cl386       # C-compiler and flags
   # NB: The -Zf flag is ABSOLUTELY NECESSARY to compile IBM's OS/2 headers.
   #     It isn't documented anywhere but obviously adds support for 48-bit
   #     far pointers (ie. _far is valid in 32-bit code). Great.
   CFLAGS       := -G3s -Zf -D__386__
   ASFLAGS      := /t /mx /m /oi /D__FLAT__ /DSTDCALL_MANGLE /D__NOU_VAR__ /iINCLUDE /i$(SCITECH)\INCLUDE
.IF $(USE_TASM32)
   AS           := tasm32
.ELIF $(USE_TASMX)
   AS           := tasmx   # Assembler and flags
.ELIF $(USE_MASM)
   AS           := masm    # Assembler and flags
   ASFLAGS      := /t /mx /D__FLAT__ /DSTDCALL_MANGLE /D__NOU_VAR__ /iINCLUDE /i$(SCITECH)\INCLUDE
.ELSE
   AS           := tasm     # Assembler and flags
.ENDIF
   LD           := link386  # Linker and flags
   LDFLAGS       = $(CFLAGS)
   RC           := rc       # Windows resource compiler
   RCFLAGS      :=
   LIB          := lib      # Librarian
   LIBFLAGS     := /NOI /NOE
   ILIB         := implib   # Import librarian
   ILIBFLAGS    := /noignorecase

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -Zi      # Turn on debugging for C compiler
   ASFLAGS      += /zi      # Turn on debugging for assembler
.ELSE
   ASFLAGS      += /q       # Suppress object records not needed for linking
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += /Ox
.END

# Optionally turn on direct i387 FPU instructions

.IF $(FPU)
   CFLAGS       += /FPi87 /DFPU387
   ASFLAGS      += /DFPU387 /DFPU_REG_RTN
.END

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += /DBETA
   ASFLAGS      += /DBETA
.END

# Use a larger stack during linking if requested ???? How the fuck do you
# specify linker options on the CL command line?????

.IF $(STKSIZE)
.ENDIF

# Place to look for PMODE library files

PMLIB           := pm.lib

# Define the base directory for library files

.IF $(CHECKED)
LIB_BASE_DIR    := $(SCITECH_LIB)\lib\debug
CFLAGS                  += -DCHECKED=1
.ELSE
LIB_BASE_DIR    := $(SCITECH_LIB)\lib\release
.ENDIF

# Define where to install library files
   LIB_OS       = os232
   LIB_BASE     := $(LIB_BASE_DIR)\$(LIB_OS)\$(CL_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Define which file contains our rules

   RULES_MAK    := cl386.mk
