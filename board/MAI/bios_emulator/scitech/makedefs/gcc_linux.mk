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
#               Linux version for GNU C/C++.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)/makedefs/startup.mk"

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

# Define that we are compiling for Linux
   USE_LINUX    := 1

# Default commands for compiling, assembling linking and archiving.
   CC           := gcc
   CFLAGS       := -Wall -I. -Iinclude -I$(SCITECH:s,\,/)/include -I$(PRIVATE:s,\,/)/include
   SHOW_CFLAGS	:= -c
   CXX          := g++
   AS           := nasm
   ASFLAGS      := -t -f elf -d__FLAT__ -d__GNUC__ -iinclude -i$(SCITECH)/include -d__NOU__
   SHOW_ASFLAGS	:= -f elf
   LD           := gcc
   LDXX			:= g++
   LDFLAGS      := -L.
   LIB          := ar
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

# Disable standard C runtime library

.IF $(NO_RUNTIME)
CFLAGS			+= -fno-builtin -nostdinc
.ENDIF

# Compile flag for whether to build X11 or non-X11 lib
.IF $(USE_X11)
   CFLAGS       += -D__X11__
.ENDIF

# Target environment dependant flags
   CFLAGS       += -D__LINUX__
   ASFLAGS      += -d__LINUX__ -d__UNIX__

# Define the base directory for library files

.IF $(CHECKED)
LIB_BASE_DIR	:= $(SCITECH_LIB)/lib/debug
CFLAGS		    += -DCHECKED=1
SHOW_CFLAGS	    += -DCHECKED=1
.ELSE
LIB_BASE_DIR	:= $(SCITECH_LIB)/lib/release
.ENDIF

# Define where to install library files
.IF $(LIBC)
   LIB_DEST_SHARED  := $(LIB_BASE_DIR)/linux/gcc/libc.so
   LIB_DEST_STATIC  := $(LIB_BASE_DIR)/linux/gcc/libc
.ELSE
   LIB_DEST_SHARED  := $(LIB_BASE_DIR)/linux/gcc/glibc.so
   LIB_DEST_STATIC  := $(LIB_BASE_DIR)/linux/gcc/glibc
.ENDIF

# Link to static libraries if requested
.IF $(STATIC_LIBS_ALL)
   LDFLAGS      += -static
   STATIC_LIBS  := 1
.ENDIF

# Link to static libraries if requested
.IF $(STATIC_LIBS)
   LDFLAGS      += -L$(LIB_DEST_STATIC)
.ELSE
   LDFLAGS      += -L$(LIB_DEST_SHARED) -L$(LIB_DEST_STATIC)
.ENDIF

# Optionally enable some  dynamic libraries to be built
.IF $(BUILD_DLL)
.IF $(VERSIONMAJ)
.ELSE
   VERSIONMAJ	:= 5
   VERSIONMIN	:= 0
.ENDIF
   VERSION      := $(VERSIONMAJ).$(VERSIONMIN)
   LIB		    := gcc -shared
   LIBFLAGS	    :=
   L		    := .so
   CFLAGS	    += -fPIC
   SHOW_CFLAGS	+= -fPIC
   ASFLAGS      += -D__PIC__
   SHOW_ASFLAGS += -D__PIC__
   LIB_DEST     := $(LIB_DEST_SHARED)
.ELSE
   LIB_DEST     := $(LIB_DEST_STATIC)
.ENDIF

# Place to look for PMODE library files

PMLIB           := -lpm

# Define which file contains our rules

   RULES_MAK	:= gcc_linux.mk
