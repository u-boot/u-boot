#############################################################################
#
#                                       SciTech Multi-platform Graphics Library
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
#                               build targets. We include them here at the end of the
#                               makefile so the generic project makefiles can override
#                               certain things with macros (such as linking C++ programs
#                               differently).
#
#############################################################################

# Take out PMLIB if we don't need to link with it

.IF $(NO_PMLIB)
PMLIB :=
.ENDIF

# Implicit generation rules for making object files
%$O: %.c ; $(CC) -c @$(mktmp $(CFLAGS:s/\/\\))  $(<:s,/,\)
%$O: %$P ; $(CPP) -c @$(mktmp $(CFLAGS:s/\/\\))  $(<:s,/,\)
.IF $(USE_NASM)
%$O: %$A ; $(AS) @$(mktmp -o $@ $(ASFLAGS:s/\/\\)) $(<:s,/,\)
.ELSE
%$O: %$A ; $(AS) @$(mktmp $(ASFLAGS:s/\/\\)) $(<:s,/,\)
.ENDIF

# Implicit rule for building resource files
%$R: %.rc ; $(RC) $(RCFLAGS) -r $<

# Implicit rule for building a DLL using a response file
.IF $(USE_OS2GUI)
%$D: ; rclink $(LD) $(RC) $@ $(mktmp $(LDFLAGS) $(&:t"+\n":s/\/\\)\n$@\n$*.map\n$(EXELIBS) $(PMLIB)\n$*.def\n)
.ELSE
%$D: ; $(LD) /nofree /nol @$(mktmp $(LDFLAGS) $(&:t"+\n":s/\/\\)\n$@\n$*.map\n$(EXELIBS) $(PMLIB)\n$*.def\n)
.ENDIF

# Implicit rule for building a library file using response file
.IF $(BUILD_DLL)
%$L: ; $(ILIB) $(ILIBFLAGS) /out:$@ $?
.ELIF $(IMPORT_DLL)
%$L: ; $(ILIB) $(ILIBFLAGS) /out:$@ $?
.ELSE
%$L: ; $(LIB) $(LIBFLAGS) /nowarn:86 /out:$@ @$(mktmp $(?:t"\n":s/\/\\))
.ENDIF

# Implicit rule for building an executable file using response file
.IF $(USE_OS2GUI)
%$E: ;
	rclink $(LD) $(RC) $@ $(mktmp $(LDFLAGS) $(&:t"+\n":s/\/\\)\n$@\n$*.map\n$(EXELIBS) $(PMLIB)\n$*.def\n)
.IF $(LXLITE)
	lxlite $@
.ENDIF
.ELSE
%$E: ;
	rclink $(LD) $(RC) $@ $(mktmp $(LDFLAGS) $(&:t"+\n":s/\/\\)\n$@\n$*.map\n$(EXELIBS) $(PMLIB)\n\n)
.IF $(LXLITE)
	lxlite $@
.ENDIF
.ENDIF
