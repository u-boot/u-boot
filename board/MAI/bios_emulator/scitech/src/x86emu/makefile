#############################################################################
#
#						Realmode X86 Emulator Library
#
#            	Copyright (C) 1996-1999 SciTech Software, Inc.
#
#  ========================================================================
#
#  Permission to use, copy, modify, distribute, and sell this software and
#  its documentation for any purpose is hereby granted without fee,
#  provided that the above copyright notice appear in all copies and that
#  both that copyright notice and this permission notice appear in
#  supporting documentation, and that the name of the authors not be used
#  in advertising or publicity pertaining to distribution of the software
#  without specific, written prior permission.  The authors makes no
#  representations about the suitability of this software for any purpose.
#  It is provided "as is" without express or implied warranty.
#
#  THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
#  EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
#  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
#  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
#  PERFORMANCE OF THIS SOFTWARE.
#
#  ========================================================================
#
# Descripton:   Generic makefile for the x86emu library. Requires
#               the SciTech Software makefile definitions package to be
#               installed, which uses the DMAKE make program.
#
#############################################################################

.IMPORT .IGNORE: DEBUG

#----------------------------------------------------------------------------
# Define the lists of object files
#----------------------------------------------------------------------------

OBJECTS			= sys$O decode$O ops$O ops2$O prim_ops$O fpu$O debug$O
CFLAGS      	+= -DSCITECH
.IF $(DEBUG)
CFLAGS			+= -DDEBUG
.ENDIF
LIBCLEAN  		= *.dll *.lib *.a
LIBFILE 		= $(LP)x86emu$L

#----------------------------------------------------------------------------
# Sample test programs
#----------------------------------------------------------------------------

all: $(LIBFILE)

validate$E: validate$O $(LIBFILE)

#----------------------------------------------------------------------------
# Define the list of object files to create dependency information for
#----------------------------------------------------------------------------

DEPEND_OBJ      =  validate$O $(OBJECTS)

.INCLUDE: "$(SCITECH)/makedefs/common.mk"
