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

# Turn on pre-compiled headers as neccessary
.IF $(PRECOMP_HDR)
   CFLAGS       += -YX"$(PRECOMP_HDR)"
.ENDIF

# Turn on runtime type information as necessary
.IF $(USE_RTTI)
	CFLAGS		+= /GR
.ENDIF

# Turn on C++ exception handling as necessary
.IF $(USE_CPPEXCEPT)
	CFLAGS		+= /GX
.ENDIF

# Take out PMLIB if we don't need to link with it

.IF $(NO_PMLIB)
PMLIB :=
.ENDIF

# Implicit generation rules for making object files
%$O: %.c ; $(CC) /nologo @$(mktmp $(CFLAGS:s/\/\\)) /c $(<:s,/,\)
%$O: %$P ; $(CC) /nologo @$(mktmp $(CFLAGS:s/\/\\)) /c $(<:s,/,\)
.IF $(USE_NASM)
%$O: %$A ; $(AS) @$(mktmp -o $@ $(ASFLAGS:s/\/\\)) $(<:s,/,\)
.ELSE
%$O: %$A ; $(AS) @$(mktmp $(ASFLAGS:s/\/\\)) $(<:s,/,\)
.ENDIF

# Implicit rule for building resource files
%$R: %.rc ; $(RC) $(RCFLAGS) -r $<

# Implicit rules for building NT device drivers

%.sys: ;
	$(LD) /nologo @$(mktmp $(LDFLAGS) /Fe$@ $(&:t"\n"s/\/\\) $(PMLIB) $(EXELIBS) $(DEF_LIBS) $(LDENDFLAGS))
.IF $(DBG)
.IF $(USE_SOFTICE)
	$(NMSYM) $(NMSYMFLAGS);$(SI_SOURCE) $@
.ENDIF
.ENDIF

# Implicit rule for building a DLL using a response file
.IF $(IMPORT_DLL)
.ELSE
.IF $(NO_RUNTIME)
%$D: ; $(LD) /nologo @$(mktmp $(LDFLAGS) /Fe$@ $(&:t"\n"s/\/\\) $(EXELIBS) $(DEF_LIBS) $(LDENDFLAGS))
.ELSE
%$D: ;
	makedef -v $*
	$(LD) /nologo @$(mktmp $(LDFLAGS) /Fe$@ $(&:t"\n"s/\/\\) $(PMLIB) $(EXELIBS) $(DEF_LIBS) $(LDENDFLAGS))
.IF $(DBG)
.IF $(USE_SOFTICE)
	$(NMSYM) $(NMSYMFLAGS);$(SI_SOURCE) $@
.ENDIF
.ENDIF
.ENDIF
.ENDIF

# Implicit rule for building a library file using response file. Note that
# we use a special .VCD file that contains the EXPORT definitions for the
# Microsoft compiler, since the LIB utility automatically adds leading
# underscores to exported functions.
.IF $(IMPORT_DLL)
%$L: ;
	makedef -v $(?:b)
    @$(RM) $@
	$(ILIB) $(ILIBFLAGS) /DEF:$(?:b).def /OUT:$@
.ELSE
%$L: ;
    @$(RM) $@
    $(LIB) $(LIBFLAGS) /out:$@ @$(mktmp $(&:t"\n")\n)
.ENDIF

# Implicit rule for building an executable file using response file
.IF $(USE_WIN32)
%$E: ;
	$(LD) /nologo @$(mktmp $(LDFLAGS) /Fe$@ $(&:t"\n"s/\/\\) $(PMLIB) $(EXELIBS) $(DEF_LIBS) $(LDENDFLAGS))
.IF $(DBG)
.IF $(USE_SOFTICE)
	$(NMSYM) $(NMSYMFLAGS);$(SI_SOURCE) $@
.ENDIF
.ENDIF
.ELSE
%$E: ;
	@$(LD) /nologo @$(mktmp $(LDFLAGS) /Fe$@ $(&:t"\n"s/\/\\) $(PMLIB) $(EXELIBS) $(DEF_LIBS) $(LDENDFLAGS))
.IF $(DOSSTYLE)
	@markphar $@
.ENDIF
.ENDIF
