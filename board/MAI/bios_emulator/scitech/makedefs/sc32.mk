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
#               Symantec C++ 6.x/7.x 32 bit version. Supports the DOSX
#               extender, FlashTek X32 and Phar Lap's TNT DOS Extender
#               and 32 bit Windows development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : USE_TNT USE_X32 USE_X32VM SC_LIBBASE

# We are compiling for a 32 bit envionment
   _32BIT_      := 1

# Default commands for compiling, assembling linking and archiving
   CC           := sc       # C-compiler and flags
   CFLAGS       := -Jm
.IF $(USE_TASM32)
   AS           := tasm32
.ELIF $(USE_TASMX)
   AS           := tasmx    # Assembler and flags
.ELSE
   AS           := tasm     # Assembler and flags
.ENDIF
.IF $(USE_WIN32)
   ASFLAGS      := /t /mx /m /D__FLAT__ /iINCLUDE /i$(SCITECH)\INCLUDE
.ELSE
   ASFLAGS      := /t /mx /m /DES_NOT_DS /D__COMM__ /i$(SCITECH)\INCLUDE
.ENDIF
   LD           := sc       # Loader and flags
   LD_FLAGS      =
   RC           := rcc      # WIndows resource compiler
   RCFLAGS      := -32      # Mark as Win32 compatible resources
   LIB          := lib      # Librarian
   LIBFLAGS     := /N /B
   ILIB         := implib   # Import librarian
   ILIBFLAGS    :=

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -g       # Turn on debugging for C compiler (FlashView)
.IF $(USE_TNT)
   LDFLAGS      += -fullsym # Turn on debugging for TNT 386link linker
.END
.IF $(USE_X32) or $(USE_X32VM)
   LDFLAGS      += -L/map   # Turn on debugging for FlashView debugger
.END
.ELSE
   ASFLAGS      += /q       # Suppress object records not needed for linking
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += -5 -o+all
.ELIF $(OPT_SIZE)
   CFLAGS       += -5 -o+space
.END

# Optionally turn on direct i387 FPU instructions

.IF $(FPU)
   CFLAGS       += -ff -DFPU387
   ASFLAGS      += -DFPU387 -DFPU_REG_RTN
.END

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += -DBETA
   ASFLAGS      += -DBETA
.END

# User a larger stack if requested

.IF $(STKSIZE)
    LDFLAGS     += =$(STKSIZE)
.ENDIF

.IF $(USE_TNT)                  # Use Phar Lap's TNT DOS Extender
   CFLAGS       += -mp
   DX_CFLAGS    += -DTNT
   ASFLAGS      += /D__FLAT__
   DX_ASFLAGS   += -DTNT
   LD           := 386link
   LDFLAGS      += @sc32.dos -exe $@
   LIB_OS       = DOS32
.ELIF $(USE_X32VM)              # Use FlashTek X-32VM DOS extender
   CFLAGS       += -mx
   DX_CFLAGS    += -DX32VM
   ASFLAGS      += /D__X386__
   DX_ASFLAGS   += -DX32VM
   LD           := sc
   LDFLAGS      += $(CFLAGS) x32v.lib
   LIB_OS       = DOS32
.ELIF $(USE_X32)                # Use FlashTek X-32 DOS extender
   CFLAGS       += -mx
   DX_CFLAGS    += -DX32VM
   ASFLAGS      += /D__X386__
   DX_ASFLAGS   += -DX32VM
   LD           := sc
   LDFLAGS      += $(CFLAGS) x32.lib
   LIB_OS       = DOS32
.ELIF $(USE_WIN32)              # Build 32 bit Windows NT app
.IF $(BUILD_DLL)
   CFLAGS       += -WD -mn
   ASFLAGS      += -DBUILD_DLL
.ELSE
   CFLAGS       += -WA -mn
.ENDIF
   DX_ASFLAGS   += -D__WINDOWS32__
   LIB_OS       = WIN32
.ELSE                           # Use default Symantec DOSX extender
   USE_DOSX     := 1
   USE_REALDOS	:= 1
   CFLAGS       += -mx
   DX_CFLAGS    += -DDOSX
   ASFLAGS      += /D__X386__
   DX_ASFLAGS   += -DDOSX
   LD           := sc
   LDFLAGS      += $(CFLAGS)
   LIB_OS       = DOS32
.END

# Place to look for PMODE library files

.IF $(USE_TNT)
PMLIB           := tnt\pm.lib
.ELIF $(USE_X32)
PMLIB           := x32\pm.lib
.ELSE
PMLIB           := dosx\pm.lib
.END

# Define the base directory for library files

.IF $(CHECKED)
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\debug
CFLAGS			+= -DCHECKED=1
.ELSE
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\release
.ENDIF

# Define where to install library files
   LIB_BASE     := $(LIB_BASE_DIR)\$(LIB_OS)\$(SC_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Define which file contains our rules

   RULES_MAK	:= sc32.mk
