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
#               Borland C++ 4.x 16 bit version. Supports 16 bit DOS,
#               DPMI16 DOS extender and 16 bit Windows development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : USE_WIN16 USE_BC5 BC_LIBBASE USE_WIN95

# Default commands for compiling, assembling linking and archiving
   CC           := bcc
   CFLAGS       := -ml -H=bcc.sym -i60 -d -dc -4 -f287
.IF $(USE_TASM32)
   AS           := tasm32
.ELIF $(USE_TASMX)
   AS           := tasmx
.ELSE
   AS           := tasm
.ENDIF
   ASFLAGS      := /t /mx /m /iINCLUDE /iINCLUDE /i$(SCITECH)\INCLUDE
   LD           := bclink tlink.exe
   LDFLAGS      := -c
   RC           := brc
   RCFLAGS      :=
.IF $(USE_BC5)
.IF $(USE_WIN95)
   WIN_VERSION  := -V4.0
.ENDIF
.ENDIF
   LIBR         := tlib
   LIBFLAGS     := /C /P32
   ILIB         := implib
   ILIBFLAGS    := -c

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -v
   LDFLAGS      += -v
   ASFLAGS      += /zi
   LIBFLAGS     += /P128
.ELSE
   LDFLAGS      += -x
   ASFLAGS      += /q
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += -O2 -k-
.ELIF $(OPT_SIZE)
   CFLAGS       += -O1 -k-
.END

# Optionally turn on direct i387 FPU instructions

.IF $(FPU)
   CFLAGS       += -DFPU387
   ASFLAGS      += -DFPU387
.END

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += -DBETA
   ASFLAGS      += -DBETA
.END

# Optionally compile as Win16
.IF $(USE_WIN16)
.IF $(BUILD_DLL)
   CFLAGS       += -WD -Fs- -DBUILD_DLL
   ASFLAGS      += -DBUILD_DLL
.ELSE
   CFLAGS       += -W -Fs-
.ENDIF
   DEF_LIBS		:= import.lib mathwl.lib cwl.lib
   DX_ASFLAGS   += -D__WINDOWS16__
   LIB_OS       = WIN16
.ELSE
   USE_REALDOS	:= 1
   DEF_LIBS     := mathl.lib fp87.lib cl.lib
   LIB_OS       = DOS16
.END

# Place to look for PMODE library files

.IF $(USE_DPMI16)
PMLIB           := dpmi16\pm.lib
.ELSE
PMLIB           := pm.lib
.END

# Define the base directory for library files

.IF $(CHECKED)
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\debug
CFLAGS			+= -DCHECKED=1
.ELSE
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\release
.ENDIF

# Define where to install library files
   LIB_BASE     := $(LIB_BASE_DIR)\$(LIB_OS)\$(BC_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Define which file contains our rules

   RULES_MAK	:= bc16.mk
