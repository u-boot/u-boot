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
#               QNX Neutrino version for GNU C/C++
#
#############################################################################

# Disable warnings for macros redefined here that were given
# on the command line.
__.SILENT       := $(.SILENT)
.SILENT         := yes

# Import enivornment variables that we use common to all compilers
.IMPORT .IGNORE : TEMP SHELL INCLUDE LIB SCITECH PRIVATE SCITECH_LIB
.IMPORT .IGNORE : DBG OPT OPT_SIZE SHW BETA CHECKED USE_QNX USE_QNXNTO
.IMPORT .IGNORE : USE_EGCS USE_PHOTON USE_X11 USE_BIOS
   TMPDIR       := $(TEMP)

# Standard file suffix definitions
#
# NOTE: Qnx does not require any extension for executeable files, but you
#       can use an extension if you wish. We use the .x extension for building
#       executeable files so that we can use implicit rules to make the
#       makefiles simpler and more portable between systems. When you install
#       the files to a local bin directory, you will probably want to remove
#       the .x extension.
   L            := .a   	# Libraries
   E            := .x   	# Executables
   O            := .o   	# Objects
   A            := .asm   	# Assembler sources
   S            := .s       # GNU assembler sources
   P            := .cpp   	# C++ sources

# File prefix/suffix definitions. The following prefixes are defined, and are
# used primarily to abstract between the Unix style libXX.a naming convention
# and the DOS/Windows/OS2 naming convention of XX.lib.
   LP           := lib      # LP - Library file prefix (name of file on disk)
   LL           := -l		# Library link prefix (name of library on link command line)
   LE           := 		    # Library link suffix (extension of library on link command line)

# We use the Unix shell at all times
   SHELL		:= /bin/sh
   SHELLFLAGS   := -c

# Definition of $(MAKE) macro for recursive makes.
   MAKE = $(MAKECMD) $(MFLAGS)

# Macro to install a library file
   INSTALL      := cp

# DMAKE uses this recipe to remove intermediate targets
.REMOVE :; $(RM) -f $<

# Turn warnings back to previous setting.
.SILENT := $(__.SILENT)

# We dont use TABS in our makefiles
.NOTABS         := yes

# Define that we are compiling for QNX
   USE_QNX    	:= 1

# Default commands for compiling, assembling linking and archiving.
   CC           := qcc
   CFLAGS       := -Vgcc_ntox86 -I. -Iinclude $(INCLUDE)
   CPPFLAGS     := -Vgcc_ntox86 -I. -Iinclude $(INCLUDE)
   CXX          := QCC
   AS           := nasm
   ASFLAGS      := -t -f elf -d__FLAT__ -d__GNUC__ -dSTDCALL_MANGLE -iinclude -i$(SCITECH)/include -d__NOU__
   LD           := qcc
   LDFLAGS      := -Vgcc_ntox86 -L. -lm
   LIB          := ar
   LIBFLAGS     := rc

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -g2
   LDFLAGS      += -g2
.ELSE
# NASM does not support debugging information yet
   ASFLAGS      +=
.ENDIF

# Optionally turn on optimisations
.IF $(OPT_MAX)
   CFLAGS       += -Ot
.ELIF $(OPT)
   CFLAGS       += -O
.ELIF $(OPT_SIZE)
   CFLAGS       += -Os
.ENDIF

# Compile flag for whether to build photon or non-photon lib
.IF $(USE_PHOTON)
   CFLAGS       += -D__PHOTON__
.ENDIF

# Compile flag for whether to build X11 or non-X11 lib
.IF $(USE_X11)
   CFLAGS       += -D__X11__
.ENDIF

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += -DBETA
   ASFLAGS      += -dBETA
.ENDIF

# Target environment dependant flags
   CFLAGS       += -D__QNX__ -D__UNIX__
   ASFLAGS      += -d__QNX__ -d__UNIX__

# Define the base directory for library files

.IF $(CHECKED)
  LIB_BASE_DIR	:= $(SCITECH_LIB)/lib/debug
  CFLAGS		+= -DCHECKED=1
.ELSE
  LIB_BASE_DIR	:= $(SCITECH_LIB)/lib/release
.ENDIF

# Define where to install library files
   LIB_DEST	:= $(LIB_BASE_DIR)/qnxnto
   LDFLAGS      += -L$(LIB_DEST)

# Place to look for PMODE library files

PMLIB           := -lpm

# Define which file contains our rules

   RULES_MAK	:= qnxnto.mk
