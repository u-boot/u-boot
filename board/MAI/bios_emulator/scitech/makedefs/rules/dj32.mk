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

# Implicit generation rules for making object files
%$O: %.c ; $(CC) @$(mktmp $(CFLAGS:s/\/\\) -c) $(<:s,/,\)
%$O: %$P ; $(CC) @$(mktmp $(CFLAGS:s/\/\\) -c) $(<:s,/,\)
%$O: %$A ; $(AS) @$(mktmp -o $@ $(ASFLAGS:s/\/\\)) $(<:s,/,\)

# Implicit rule for building a library file using response file
%$L:      ; $(LIB) $(LIBFLAGS) $@ @$(mktmp $(&:s/\/\\)\n)

# Implicit rule for building an executable file using response file
%$E:      ; $(LD) $(LDFLAGS) $@ @$(mktmp $(&:s/\/\\) $(EXELIBS) $(PMLIB) -lstdcxx -lm)
