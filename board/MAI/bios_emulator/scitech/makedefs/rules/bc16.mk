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
%$O: %.c ; $(CC) @$(mktmp $(CFLAGS:s/\/\\)) -c $<
%$O: %$P ; $(CC) @$(mktmp $(CFLAGS:s/\/\\)) -c $<
%$O: %$A ; $(AS) @$(mktmp $(ASFLAGS:s/\/\\)) $(<:s,/,\)

# Implicit rule for building resource files
%$R: %.rc ; $(RC) $(RCFLAGS) -r $<

# Implicit rule for building a DLL using a response file
%$D:      ; $(LD) $(mktmp $(LDFLAGS) -C -Twd c0dl.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS) $(EXELIBS)\n$*.def)

# Implicit rule for building a library file using response file
.IF $(BUILD_DLL)
%$L: ;
    @$(RM) $@
    $(ILIB) $(ILIBFLAGS) $@ $?
.ELIF $(IMPORT_DLL)
%$L: ;
    @$(RM) $@
    $(ILIB) $(ILIBFLAGS) $@ $?
.ELSE
%$L: ;
    @$(RM) $@
    $(LIBR) $(LIBFLAGS) $@ @$(mktmp +$(&:t" &\n+")\n)
.ENDIF

# Implicit rule for building an executable file using response file
.IF $(USE_WIN16)
%$E: ; $(LD) $(mktmp $(LDFLAGS) -C -Twe $(WIN_VERSION) c0wl.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS) $(EXELIBS)\n$*.def)
.ELSE
%$E: ; $(LD) $(mktmp $(LDFLAGS) -Tde c0l.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(PMLIB) $(DEF_LIBS) $(EXELIBS))
.ENDIF
