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
#               Cygwin port of GNU C/C++ to Win32.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : GCC2_LIBBASE

# Override some file suffix definitions
   L            := .a     # Libraries
   O            := .o     # Objects

# Override the file prefix/suffix definitions for library naming.
   LP           := lib		# LP - Library file prefix (name of file on disk)
   LL           := -l		# Library link prefix (name of library on link command line)
   LE           :=			# Library link suffix (extension of library on link command line)

# We are compiling for a 32 bit envionment
   _32BIT_      := 1

# Default commands for compiling, assembling linking and archiving
   CC           := gcc      # C-compiler and flags
   CFLAGS       := -Wall -I. -Iinclude -I$(SCITECH:s,\,/)/include -I$(PRIVATE:s,\,/)/include
   SHOW_CFLAGS	:= -c
   CXX          := g++
   AS			:= nasm
   ASFLAGS      := -t -f coff -F null -d__FLAT__ -d__GNUC__ -dSTDCALL_USCORE -iINCLUDE -i$(SCITECH)\INCLUDE
   SHOW_ASFLAGS	:= -f coff
   LD           := gcc    	# Loader and flags
   LDXX			:= g++
.IF $(WIN32_GUI)
   LDFLAGS      := -L. -mwindows -e _mainCRTStartup
.ELSE
   LDFLAGS      := -L.
.ENDIF
   RC           := windres
   RCFLAGS		:= -O coff
   LIB          := ar       # Librarian
   LIBFLAGS     := rcs
   YACC			:= bison -y
   LEX			:= flex
   SED			:= sed

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -g
   SHOW_CFLAGS  += -g
.ELSE
# NASM does not support debugging information yet
   ASFLAGS      +=
.ENDIF

# Optionally turn on optimisations
.IF $(OPT_MAX)
   CFLAGS       += -O6
   SHOW_CFLAGS  += -O6
.ELIF $(OPT)
   CFLAGS       += -O2
   SHOW_CFLAGS  += -O2
.ELIF $(OPT_SIZE)
   CFLAGS       += -O1
   SHOW_CFLAGS  += -O1
.ENDIF

# Optionally turn on direct i387 FPU instructions

.IF $(FPU)
   CFLAGS       += -DFPU387
   ASFLAGS      += -dFPU387
.END

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += -DBETA
   SHOW_CFLAGS  += -DBETA
   ASFLAGS      += -dBETA
   SHOW_ASFLAGS += -dBETA
.ENDIF

# DOS extender dependant flags
   DX_CFLAGS    +=
   DX_ASFLAGS   += -dGCC_WIN32

# Define the base directory for library files

.IF $(CHECKED)
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\debug
CFLAGS			+= -DCHECKED=1
SHOW_CFLAGS	    += -DCHECKED=1
.ELSE
LIB_BASE_DIR	:= $(SCITECH_LIB)\lib\release
.ENDIF

# Define where to install library files
   LIB_DEST     := $(LIB_BASE_DIR)\WIN32\$(GCC2_LIBBASE)
   LDFLAGS      += -L$(LIB_DEST)

# Place to look for PMODE library files

PMLIB           := -lpm

# Define which file contains our rules

   RULES_MAK	:= gcc_win32.mk
