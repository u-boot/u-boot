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

# Whether to link in real VBIOS library, or just the stub library

.IF $(USE_BIOS)
VBIOSLIB := -lvbios.lib
.ELSE
VBIOSLIB := -lvbstubs.lib
.END

# Require special privledges for Nucleus programs (requires root access)

.IF $(USE_NUCLEUS)
LDFLAGS		+= -T1
.ENDIF

# Implicit generation rules for making object files from source files
%$O: %.c ;
.IF $(SHOW_ARGS)
    $(CC) $(CFLAGS) $<
.ELSE
    @echo $(CC) -c $<
	+@$(CC) $(CFLAGS) $< > /dev/null
.ENDIF

%$O: %$P ;
.IF $(SHOW_ARGS)
	$(CXX) $(CFLAGS) $<
.ELSE
	@echo $(CXX) -c $<
	+@$(CXX) $(CFLAGS) $< > /dev/null
.ENDIF

%$O: %$A ;
.IF $(SHOW_ARGS)
    $(AS) -o $@ $(ASFLAGS) $<
.ELSE
    @echo $(AS) $<
	@$(AS) -o $@ $(ASFLAGS) $<
.ENDIF

# Implicit rule for building a library file
%$L:     ;
.IF $(SHOW_ARGS)
    $(LIB) $(LIBFLAGS) -q $@ $&
.ELSE
    @echo $(LIB) $@
	+@$(LIB) $(LIBFLAGS) -q $@ $& > /dev/null
.ENDIF


# Implicit rule for building an executable file
%$E:     ;
.IF $(SHOW_ARGS)
	$(LD) $(LDFLAGS) -o $@ $& $(EXELIBS) $(PMLIB) $(VBIOSLIB)
.ELSE
	@echo wlink $@
	+@$(LD) $(LDFLAGS) -o $@ $& $(EXELIBS) $(PMLIB) $(VBIOSLIB) > /dev/null
.ENDIF
