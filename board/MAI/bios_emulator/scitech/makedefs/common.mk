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
# Descripton:   Common makefile targets used by all SciTech Software
#               makefiles. This file includes targets for cleaning the
#               current directory, and maintaining the source files with
#               RCS.
#
#############################################################################

# Override global OpenGL includes when compiling against MGL version

.IF $(USE_MGL_OPENGL)
.IF $(UNIX_HOST)
CFLAGS		+= -I$(SCITECH)/include/mglgl
DEPEND_INC	+= $(SCITECH)/include/mglgl
.ELSE
CFLAGS		+= -I$(SCITECH)\include\mglgl
DEPEND_INC	+= $(SCITECH)\include/mglgl
.ENDIF
.ENDIF

# Define where to install all compiled DLL files

.IF $(UNIX_HOST)
.IF $(CHECKED)
DLL_DEST    := $(SCITECH_LIB)/redist/debug
.ELSE
DLL_DEST    := $(SCITECH_LIB)/redist/release
.ENDIF
.ELSE
.IF $(CHECKED)
DLL_DEST    := $(SCITECH_LIB)\redist\debug
.ELSE
DLL_DEST    := $(SCITECH_LIB)\redist\release
.ENDIF
.ENDIF

# Target to build the library and DLL file if specified

.IF $(LIBFILE)

lib: $(LIBFILE)

.IF $(DLLFILE)

# Build and install a DLL file, or simply build import library and install

.IF $(BUILD_DLL)

$(DLLFILE): $(OBJECTS)
$(LIBFILE): $(DLLFILE)
install: $(LIBFILE) $(DLLFILE)
	$(INSTALL) $(LIBFILE) $(LIB_DEST)$(LIB_EXTENDER)
	$(INSTALL) $(DLLFILE) $(DLL_DEST)
.IF $(USE_SOFTICE)
	$(INSTALL) $(DLLFILE:s/.dll/.nms) $(DLL_DEST)
.ENDIF
.ELSE

$(LIBFILE): $(DLL_DEST)\$(DLLFILE)
install: $(LIBFILE)
	$(INSTALL) $(LIBFILE) $(LIB_DEST)$(LIB_EXTENDER)

.ENDIF
.ELSE

.IF $(BUILD_DLL)

# Build and install a Unix shared library

$(LIBFILE): $(OBJECTS)
install: $(LIBFILE)
	$(INSTALL) $(LIBFILE) $(LIB_DEST)$(LIB_EXTENDER)
	$(INSTALL) $(LIBFILE) $(DLL_DEST)/$(LIBFILE).$(VERSION)

.ELSE

# Build and install a normal library file

.IF $(USE_DLL)
.ELSE
$(LIBFILE): $(OBJECTS)
install: $(LIBFILE)
	$(INSTALL) $(LIBFILE) $(LIB_DEST)$(LIB_EXTENDER)
.ENDIF
.ENDIF
.ENDIF
.ENDIF

# Build and install a VxD file, including debug information

.IF $(VXDFILE)
$(VXDFILE:s/.vxd/.dll): $(OBJECTS)
$(VXDFILE): $(VXDFILE:s/.vxd/.dll)
install: $(VXDFILE)
	$(INSTALL) $(VXDFILE) $(DLL_DEST)
.IF $(DBG)
	$(INSTALL) $(VXDFILE:s/.vxd/.nms) $(DLL_DEST)
.ENDIF
.ENDIF

# Clean up directory removing all files not needed to make the library.

__CLEAN_FILES := *.obj *.o *.sym *.bak *.tdk *.swp *.map *.err *.csm *.lib *.aps *.nms *.sys
__CLEAN_FILES += *.~* *.td *.tr *.tr? *.td? *.rws *.res *.exp *.ilk *.pdb *.pch *.a bcc32.*
__CLEAN_FILES += $(LIBCLEAN)
__CLEANEXE_FILES := $(__CLEAN_FILES) *$E *.drv *.rex *.dll *.vxd *.nms *.pel *.smf *.so.*

.PHONY clean:
	@$(RM) -f -S $(mktmp $(__CLEAN_FILES:t"\n"))

.PHONY cleanexe:
	@$(RM) -f -S $(mktmp $(__CLEANEXE_FILES:t"\n"))

# Define the source directories to find common files

.IF $(NO_SCITECH_COMMON)
.ELSE
.SOURCE:		   $(SCITECH)/src/common
.ENDIF

# Create the include file dependencies using the MKUTIL makedep program if
# the list of dependent object files is defined

.IF $(DEPEND_OBJ)
depend:
	@$(RM) -f makefile.dep
.IF $(DEPEND_SRC)
.IF $(DEPEND_INC)
	@makedep -amakefile.dep -r -s -I@$(mktmp $(DEPEND_INC:s/\/\\)) -S@$(mktmp $(DEPEND_SRC:s/\/\\);$(SCITECH)/src/common) @$(mktmp $(DEPEND_OBJ:t"\n")\n)
.ELSE
	@makedep -amakefile.dep -r -s -S@$(mktmp $(DEPEND_SRC:s/\/\\);$(SCITECH)/src/common) @$(mktmp $(DEPEND_OBJ:t"\n")\n)
.ENDIF
.ELSE
.IF $(DEPEND_INC)
	@makedep -amakefile.dep -r -s -I@$(mktmp $(DEPEND_INC:s/\/\\)) -S@$(mktmp $(SCITECH)/src/common) @$(mktmp $(DEPEND_OBJ:t"\n")\n)
.ELSE
	@makedep -amakefile.dep -r -s -S@$(mktmp $(SCITECH)/src/common) @$(mktmp $(DEPEND_OBJ:t"\n")\n)
.ENDIF
.ENDIF
	@$(ECHO) Object file dependency information generated.
.ENDIF

# Set up for compiling Snap executeables and dynamic link libraries

.IF $(USE_SNAP)
#CFLAGS     	+= -I$(PRIVATE)\include\drvlib -I$(SCITECH)\include\drvlib -D__SNAP__
CFLAGS     	+= -D__SNAP__
ASFLAGS   	+= -d__SNAP__
#EXELIBS		+= snap$L
.ENDIF

# Include rule definitions for the compiler

.INCLUDE: "$(SCITECH)/makedefs/rules/$(RULES_MAK)"

# Include file dependencies

.INCLUDE .IGNORE: "makefile.dep"
