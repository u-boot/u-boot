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
#               Common startup script that defines all variables common to
#				all startup scripts. These define the DMAKE runtime
#				environment and the values are dependant on the version of
#				DMAKE in use.
#
#############################################################################

# Disable warnings for macros redefined here that were given
# on the command line.
__.SILENT       := $(.SILENT)
.SILENT         := yes

# Import enivornment variables that we use common to all compilers
.IMPORT .IGNORE : TEMP SHELL COMSPEC INCLUDE LIB SCITECH PRIVATE SCITECH_LIB
.IMPORT .IGNORE : DBG OPT OPT_SIZE SHW BETA USE_WIN32 FPU BUILD_DLL BUILD_FOR_DLL
.IMPORT .IGNORE : IMPORT_DLL USE_TASMX WIN32_GUI USE_WIN16 USE_NASM CHECKED
.IMPORT .IGNORE : OS2_SHELL SOFTICE_PATH MAX_WARN USE_SOFTICE USE_TASM32
.IMPORT .IGNORE : DLL_START_TASM USE_SNAP USE_X11 USE_LINUX STATIC_LIBS LIBC
.IMPORT .IGNORE : SHOW_ARGS BOOT_STRAP_DMAKE
   TMPDIR := $(TEMP)

# Determine if the host machine is a Windows/DOS or Unix box
.IF $(COMSPEC)
   WIN32_HOST   := 1
.ELSE
   USE_NASM     := 1
   UNIX_HOST    := 1
.ENDIF

# Setup to either user NASM or TASM as the assembler
.IF $(USE_NASM)
.ELSE
   USE_TASM		:= 1
.ENDIF

.IF $(UNIX_HOST)
# Standard file suffix definitions
#
# NOTE: Linux/Unix does not require any extenion for executeable files, but you
#       can use an extension if you wish. We use the .exe extension for building
#       executeable files so that we can use implicit rules to make the
#       makefiles simpler and more portable between systems (exe also makes it
#       easier for cross-compile/debugging situations). When you install
#       the files to a local bin directory, you will probably want to remove
#       the .exe extension.
   L            := .a   	# Libraries
   E            := .exe   	# Executables for glibc
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

.ELSE
# Standard file DOS/Win/OS2 suffix definitions
   L            := .lib   	# Libraries
.IF $(USE_SNAP)
   E            := .sxe   	# Snap Executables
   D            := .sll  	# Snap Dynamic Link Library file
.ELSE
   E            := .exe   	# Executables
   D            := .dll   	# Dynamic Link Library file
.ENDIF
   O            := .obj   	# Objects
   A            := .asm   	# Assembler sources
   P            := .cpp   	# C++ sources
   R            := .res   	# Compiled resource file
   S			:= .s		# Assyntax.h style assembler

# File prefix/suffix definitions. The following prefixes are defined, and are
# used primarily to abstract between the Unix style libXX.a naming convention
# and the DOS/Windows/OS2 naming convention of XX.lib.
   LP           := 			# LP - Library file prefix (name of file on disk)
   LL           := 			# Library link prefix (name of library on link command line)
   LE           := .lib		# Library link suffix (extension of library on link command line)

# We use the DOS/Win/OS2 style shell at all times
   SHELL        := $(COMSPEC)
   GROUPSHELL   := $(SHELL)
   SHELLFLAGS   := $(SWITCHAR)c
   GROUPFLAGS   := $(SHELLFLAGS)
   SHELLMETAS   := *"?<>
.IF $(OS2_SHELL)
   GROUPSUFFIX  := .cmd
.ELSE
   GROUPSUFFIX  := .bat
.ENDIF
   DIRSEPSTR    := \\
   DIVFILE       = $(TMPFILE:s,/,\)

.ENDIF

# Standard Unix style shell commands. Since these do not exist on
# regular DOS/Win/OS2 installations we use our own '' versions
# instead. To boostrtap a new OS you may wish to use the regular
# unix versions.

.IF $(BOOT_STRAP_DMAKE)
   CP			:= cp
   MD			:= mkdir
   RM			:= rm
   ECHO			:= echo
.ELSE
   CP			:= k_cp
   MD			:= k_md
   RM			:= k_rm
   ECHO			:= k_echo
.ENDIF

# Definition of $(MAKE) macro for recursive makes.
   MAKE = $(MAKECMD) $(MFLAGS)

# Macro to install a library file
   INSTALL      := $(CP)

# DMAKE uses this recipe to remove intermediate targets
.REMOVE :; $(RM) -f $<

# Turn warnings back to previous setting.
.SILENT := $(__.SILENT)

# We dont use TABS in our makefiles
.NOTABS         := yes
