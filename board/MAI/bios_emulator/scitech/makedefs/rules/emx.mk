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
#               OS/2 version for EMX/GNU C/C++.
#
#############################################################################

# Take out PMLIB if we don't need to link with it

.IF $(NO_PMLIB)
PMLIB :=
.ENDIF

# Implicit generation rules for making object files
%$O: %.c ;
.IF $(SHOW_ARGS)
	$(CC) -c $(CFLAGS) $(<:s,\,/)
.ELSE
	@echo $(CC) -c $(<:s,\,/)
	@$(CC) -c $(CFLAGS) $(<:s,\,/)
.ENDIF

%$O: %$P ;
.IF $(SHOW_ARGS)
	$(CXX) -c $(CFLAGS) $(<:s,\,/)
.ELSE
	@echo $(CXX) -c $(<:s,\,/)
	@$(CXX) -c $(CFLAGS) $(<:s,\,/)
.ENDIF

%$O: %$A ;
.IF $(USE_NASM)
.IF $(SHOW_ARGS)
	$(AS) -o $@ $(ASFLAGS) $(<:s,\,/)
.ELSE
	@echo $(AS) $(<:s,\,/)
	@$(AS) @$(mktmp -o $@ $(ASFLAGS)) $(<:s,\,/)
.ENDIF
.ELSE
.IF $(SHOW_ARGS)

    $(AS)  @$(mktmp $(ASFLAGS:s/\/\\)) $(<:s,/,\)
.ELSE
	@echo $(AS) $(<:s,/,\)
    $(AS)  @$(mktmp $(ASFLAGS:s/\/\\)) $(<:s,/,\)
.ENDIF
.ENDIF

# Implicit rule for building a library file using response file
%$L:     ;
.IF $(SHOW_ARGS)
	$(LIB) $(LIBFLAGS) $@ $(&:s,\,/)
.ELSE
	@echo $(LIB) $@
	@$(LIB) $(LIBFLAGS) $@ @$(mktmp $(?:t"\n"))
.ENDIF

# Implicit rule for building an executable file using response file
%$E:     ;
.IF $(SHOW_ARGS)
	$(LD) $(LDFLAGS) -o $@ $(&:s,\,/) $(EXELIBS) $(PMLIB) -lgpp -lstdcpp
.ELSE
	@echo $(LD) $@
	@$(LD) $(LDFLAGS) -o $@ $(&:s,\,/) $(EXELIBS) $(PMLIB) -lgpp -lstdcpp
.ENDIF
