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

.IF $(USE_VXD)

# Implicit rule generation to build VxD's

%$O: %.c ;
	$(CC) @$(mktmp $(CFLAGS:s/\/\\)) -c $(<:s,/,\)
	@$(VTOOLSD)\bin\segalias.exe -p $(VTOOLSD)\include\default.seg $@

%$O: %$P ;
	$(CC) @$(mktmp $(CFLAGS:s/\/\\)) -c $(<:s,/,\)
	@$(VTOOLSD)\bin\segalias.exe -p $(VTOOLSD)\include\default.seg $@

%$O: %$A ; $(AS) @$(mktmp $(ASFLAGS:s/\/\\)) $(<:s,/,\)

%$L: ; $(LIB) $(LIBFLAGS) $@ @$(mktmp -+$(?:t" &\n-+")\n)

%.dll: ;
	@$(CP) $(mktmp EXPORTS\n_The_DDB @1) $*.def
	tlink32.exe @$(mktmp $(LDFLAGS) -Tpd $(VTOOLSD:s/\/\\)\lib\icrtbc4.obj+\n$(&:s/\/\\)\n$*.dll\n$*.map\n$(DEF_LIBS:s/\/\\) $(PMLIB:s/\/\\) $(EXELIBS:s/\/\\)\n$*.def)
	@$(RM) -S $(mktmp $*.def)

%.vxd: %.dll ;
	@$(CP) $(mktmp DYNAMIC\nATTRIB ICODE INIT\nATTRIB LCODE LOCKED\nATTRIB PCODE PAGEABLE\nATTRIB SCODE STATIC\nATTRIB DBOCODE DEBUG\nMERGE ICODE INITDAT0 INITDATA) $*.pel
	@$(VTOOLSD)\bin\vxdver.exe $*.vrc $*.res
	@$(VTOOLSD)\bin\pele.exe -d -s $*.smf -c $*.pel -o $@ -k 400 $*.dll
	@$(VTOOLSD)\bin\sethdr.exe -n $* -x $@ -r $*.res
.IF $(DBG)
	$(NMSYM) /TRANS:source,package /SOURCE:$(VXDSOURCE) $*.smf
.ENDIF
	@$(RM) -S $(mktmp $*.pel)

.ELSE

# Implicit generation rules for making object files, libraries and exe's

%$O: %.c ; $(CC) @$(mktmp $(CFLAGS:s/\/\\)) -c $(<:s,/,\)
%$O: %$P ; $(CC) @$(mktmp $(CFLAGS:s/\/\\)) -c $(<:s,/,\)
.IF $(USE_NASM)
%$O: %$A ; $(AS) @$(mktmp -o $@ $(ASFLAGS:s/\/\\)) $(<:s,/,\)
.ELSE
%$O: %$A ; $(AS) @$(mktmp $(ASFLAGS:s/\/\\)) $(<:s,/,\)
.ENDIF

# Implicit rule for building resource files
%$R: %.rc ; $(RC) $(RCFLAGS) -r $<

# Implicit rule for building a DLL using a response file
.IF $(IMPORT_DLL)
.ELSE
.IF $(NO_RUNTIME)
%$D: ; $(LD) $(mktmp $(LDFLAGS) -Tpd -aa $(&:s/\/\\)\n$@\n$*.map\n$(EXELIBS)\n$*.def)
.ELSE
%$D: ;
	makedef $(@:b)
	$(LD) $(mktmp $(LDFLAGS) -Tpd -aa c0d32.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS:s/\/\\) $(PMLIB:s/\/\\) $(EXELIBS)\n$*.def)
.IF $(DBG)
.IF $(USE_SOFTICE)
	$(NMSYM) $(NMSYMFLAGS);$(SI_SOURCE) $@
	tdstrp32 $@
.ENDIF
.ENDIF
.ENDIF
.ENDIF

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
    $(LIB) $(LIBFLAGS) $@ @$(mktmp +$(&:t" &\n+")\n)
.ENDIF

# Implicit rule for building an executable file using response file

.IF $(USE_WIN32)
.IF $(WIN32_GUI)
%$E: ;
	$(LD) $(mktmp $(LDFLAGS) -Tpe -aa $(WIN_VERSION) c0w32.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS:s/\/\\) $(PMLIB:s/\/\\) $(EXELIBS)\n$*.def)
.IF $(DBG)
.IF $(USE_SOFTICE)
	$(NMSYM) $(NMSYMFLAGS);$(SI_SOURCE) $@
	tdstrp32 $@
.ENDIF
.ENDIF
.ELSE
%$E: ;
	$(LD) $(mktmp $(LDFLAGS) -Tpe -ap c0x32.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS:s/\/\\) $(PMLIB:s/\/\\) $(EXELIBS)\n$*.def)
.IF $(USE_SOFTICE)
	$(NMSYM) $(NMSYMFLAGS);$(SI_SOURCE) $@
	tdstrp32 $@
.ENDIF
.ENDIF
.ELIF $(USE_TNT)
%$E: ;
	@$(CP) $(mktmp stub 'gotnt.exe') $*.def
	@$(LD) $(mktmp $(LDFLAGS) -Tpe -ap c0x32.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS) $(PMLIB:s/\/\\) $(EXELIBS)\n$*.def)
.IF $(DOSSTYLE)
	@markphar $@
.ENDIF
	@$(RM) -S $(mktmp $*.def)
.ELIF $(USE_SMX32)
%$E: ; $(LD) $(mktmp $(LDFLAGS) -Tpe -ap c0x32.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS) $(PMLIB:s/\/\\) $(EXELIBS)\n$*.def)
.ELSE
%$E: ; $(LD) $(mktmp $(LDFLAGS) -Tpe -ap c0x32.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS) $(PMLIB:s/\/\\) $(EXELIBS)\n$*.def)
.END

.ENDIF
