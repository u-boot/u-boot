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
# Descripton:   Rules makefile definitions, which define the rules used to
#				build targets. We include them here at the end of the
#				makefile so the generic project makefiles can override
#				certain things with macros (such as linking C++ programs
#				differently).
#
#############################################################################

# Take out PMLIB if we don't need to link with it

.IF $(NO_PMLIB)
PMLIB :=
.ENDIF

.IF $(USE_CXX_LINKER)
LD	:= $(LDXX)
.ENDIF

# Implicit generation rules for making object files from source files
%$O: %.c ;
.IF $(SHOW_ARGS)
    $(CC) -c $(CFLAGS) $<
.ELSE
    @$(ECHO) $(CC) $(SHOW_CFLAGS) $<
    @$(CC) -c $(CFLAGS) $<
.ENDIF

%$O: %$P ;
.IF $(SHOW_ARGS)
    $(CXX) -c $(CFLAGS) $<
.ELSE
    @$(ECHO) $(CXX) $(SHOW_CFLAGS) $<
    @$(CXX) -c $(CFLAGS) $<
.ENDIF

%$O: %$A ;
.IF $(SHOW_ARGS)
    $(AS) -o $@ $(ASFLAGS) $<
.ELSE
    @$(ECHO) $(AS) $(SHOW_ASFLAGS) $<
    @$(AS) @$(mktmp -o $@ $(ASFLAGS)) $<
.ENDIF

# Implicit rule for building a library file
.IF $(BUILD_DLL)
%$L:     ;
.IF $(SHOW_ARGS)
    $(LIB) $(LIBFLAGS) -Wl,-soname,$@.$(VERSIONMAJ) -o $@ $& $(LIBS)
.ELSE
    @$(ECHO) $(LIB) $@
    @$(LIB) $(LIBFLAGS) -Wl,-soname,$@.$(VERSIONMAJ) -o $@ $& $(LIBS)
.ENDIF
.ELSE
%$L:     ;
.IF $(SHOW_ARGS)
    $(LIB) $(LIBFLAGS) $@ $&
.ELSE
    @$(ECHO) $(LIB) $@
    @$(LIB) $(LIBFLAGS) $@ $&
.ENDIF
.ENDIF

# Implicit rule for building an executable file
%$E:     ;
.IF $(SHOW_ARGS)
    $(LD) $(LDFLAGS) -o $@ $& $(EXELIBS) $(PMLIB) -lm
.ELSE
    @$(ECHO) ld $@
    @$(LD) $(LDFLAGS) -o $@ $& $(EXELIBS) $(PMLIB) -lm
.ENDIF
