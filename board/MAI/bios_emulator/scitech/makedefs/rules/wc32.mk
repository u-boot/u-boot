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

# Use a larger stack during linking if requested, or use a default stack
# of 200k. The usual default stack provided by Watcom C++ is *way* to small
# for real 32 bit code development. We also need a *huge* stack for OpenGL
# software rendering also!
.IF $(USE_QNX4)
    # Not necessary for QNX code.
.ELSE
.IF $(STKSIZE)
    LDFLAGS		+= OP STACK=$(STKSIZE)
.ELSE
	LDFLAGS		+= OP STACK=204800
.ENDIF
.ENDIF

# Turn on runtime type information as necessary
.IF $(USE_RTTI)
	CPFLAGS		+= -xr
.ENDIF

# Optionally turn on pre-compiled headers
.IF $(PRECOMP_HDR)
	CFLAGS		+= -fhq
.ENDIF

.IF $(USE_QNX)
# Whether to link in real VBIOS library, or just the stub library
.IF $(USE_BIOS)
VBIOSLIB := vbios.lib,
.ELSE
VBIOSLIB := vbstubs.lib,
.END
# Require special privledges for Nucleus programs (requires root access)
.IF $(USE_NUCLEUS)
LDFLAGS		+= OP PRIV=1
.ENDIF
.ENDIF

# Implicit generation rules for making object files
.IF $(WC_LIBBASE) == WC10A
%$O: %.c ; $(CC) $(CFLAGS) $(<:s,/,\)
%$O: %$P ; $(CPP) $(CFLAGS) $(<:s,/,\)
.ELSE
%$O: %.c ; $(CC) @$(mktmp $(CFLAGS:s/\/\\)) $(<:s,/,\)
%$O: %$P ; $(CPP) @$(mktmp $(CPFLAGS:s/\/\\) $(CFLAGS:s/\/\\)) $(<:s,/,\)
.ENDIF
.IF $(USE_NASM)
%$O: %$A ; $(AS) @$(mktmp -o $@ $(ASFLAGS:s/\/\\)) $(<:s,/,\)
.ELSE
%$O: %$A ; $(AS) @$(mktmp $(ASFLAGS:s/\/\\)) $(<:s,/,\)
.ENDIF

# Implit rule to compile .S assembler files. The first version
# uses GAS directly and the second uses a pre-processor to
# produce NASM code.

.IF $(USE_GAS)
.IF $(WC_LIBBASE) == WC11
%$O: %$S ; $(GAS) -c @$(mktmp $(GAS_FLAGS:s/\/\\)) $(<:s,/,\)
.ELSE
# Black magic to build asm sources with Watcom 10.6 (requires sed)
%$O: %$S ;
	$(GAS) -c @$(mktmp $(GAS_FLAGS:s/\/\\)) $(<:s,/,\)
	wdisasm \\ -a $(*:s,/,\).o > $(*:s,/,\).lst
	sed -e "s/\.text/_TEXT/; s/\.data/_DATA/; s/\.bss/_BSS/; s/\.386/\.586/; s/lar *ecx,cx/lar ecx,ecx/" $(*:s,/,\).lst > $(*:s,/,\).asm
	wasm \\ $(WFLAGS) -zq -fr=nul -fp3 -fo=$@ $(*:s,/,\).asm
	$(RM) -S $(mktmp $(*:s,/,\).o)
	$(RM) -S $(mktmp $(*:s,/,\).lst)
	$(RM) -S $(mktmp $(*:s,/,\).asm)
.ENDIF
.ELSE
%$O: %$S ;
	@gcpp -DNASM_ASSEMBLER -D__WATCOMC__ -EP $(<:s,/,\) > $(*:s,/,\).asm
	nasm @$(mktmp -f obj -o $@) $(*:s,/,\).asm
	@$(RM) -S $(mktmp $(*:s,/,\).asm)
.ENDIF

# Special target to build dllstart.asm using Borland TASM
dllstart.obj: dllstart.asm
	$(DLL_TASM) @$(mktmp /t /mx /m /D__FLAT__ /i$(SCITECH)\INCLUDE /q) $(PRIVATE)\src\common\dllstart.asm

# Implicit rule for building resource files
%$R: %.rc ; $(RC) $(RCFLAGS) -r $<

# Implicit rule for building a DLL using a response file
.IF $(IMPORT_DLL)
.ELSE
.IF $(USE_OS232)
%$D: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet SYS os2v2 dll\nN $@\nF $(&:t",\n":s/\/\\)\nLIBR $(EXELIBS:t",")) $*.lnk
	rclink $(LD) $(RC) $@ $*.lnk
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.ELIF $(USE_WIN32)
%$D: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet SYS nt_dll\nN $@\nF $(&:t",\n":s/\/\\)\nLIBR $(PMLIB)$(DEFLIBS)$(EXELIBS:t",")) $*.lnk
	rclink $(LD) $(RC) $@ $*.lnk
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.ELSE
%$D: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet SYS win386\nN $*.rex\nF $(&:t",\n":s/\/\\)\nLIBR $(EXELIBS:t",")) $*.lnk
	rclink $(LD) $(RC) $@ $*.lnk
	wbind $* -d -q -n
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.ENDIF
.ENDIF

# Implicit rule for building a library file using response file
.IF $(BUILD_DLL)
%$L: ;
	@$(RM) $@
	$(ILIB) $(ILIBFLAGS) $@ +$?
.ELIF $(IMPORT_DLL)
%$L: ;
	@$(RM) $@
	$(ILIB) $(ILIBFLAGS) $@ +$?
.ELSE
%$L: ;
    @$(RM) $@
    $(LIB) $(LIBFLAGS) $@ @$(mktmp,$*.rsp +$(&:t"\n+":s/\/\\)\n)
.ENDIF

# Implicit rule for building an executable file using response file
.IF $(USE_X32)
%$E: ;
	@trimlib $(mktmp OP quiet\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(PMLIB)$(EXELIBS:t",")) $*.lnk
	$(LD) $(LDFLAGS) @$*.lnk
	x32fix $@
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.ELIF $(USE_OS232)
.IF $(USE_OS2GUI)
%$E: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet SYS os2v2_pm\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(PMLIB)$(EXELIBS:t",")) $*.lnk
	rclink $(LD) $(RC) $@ $*.lnk
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.IF $(LXLITE)
 	lxlite $@
.ENDIF
.ELSE
%$E: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet SYS os2v2\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(PMLIB)$(EXELIBS:t",")) $*.lnk
	rclink $(LD) $(RC) $@ $*.lnk
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.IF $(LXLITE)
 	lxlite $@
.ENDIF
.ENDIF
.ELIF $(USE_SNAP)
%$E: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet SYS nt\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(DEFLIBS)$(EXELIBS:t",")) $*.lnk
	rclink $(LD) $(RC) $@ $*.lnk
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.ELIF $(USE_WIN32)
.IF $(WIN32_GUI)
%$E: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet SYS win95\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(PMLIB)$(DEFLIBS)$(EXELIBS:t",")) $*.lnk
	rclink $(LD) $(RC) $@ $*.lnk
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.ELSE
%$E: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet SYS nt\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(PMLIB)$(DEFLIBS)$(EXELIBS:t",")) $*.lnk
	rclink $(LD) $(RC) $@ $*.lnk
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.ENDIF
.ELIF $(USE_WIN386)
%$E: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet SYS win386\nN $*.rex\nF $(&:t",":s/\/\\)\nLIBR $(PMLIB)$(EXELIBS:t",")) $*.lnk
	rclink $(LD) wbind $*.rex $*.lnk
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.ELIF $(USE_TNT)
%$E: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet\nN $@\nF $(&:t",":s/\/\\)\nLIBR dosx32.lib,tntapi.lib,$(PMLIB)$(EXELIBS:t",")) $*.lnk
	$(LD) @$*.lnk
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.IF $(DOSSTYLE)
	@markphar $@
.ENDIF
.ELIF $(USE_QNX4)
%$E: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(PMLIB)$(VBIOSLIB)$(EXELIBS:t",")) $*.lnk
	@+if exist $*.exe attrib -s $*.exe > NUL
	$(LD) @$*.lnk
	@attrib +s $*.exe
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.ELSE
%$E: ;
	@trimlib $(mktmp $(LDFLAGS) OP quiet\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(PMLIB)$(EXELIBS:t",")) $*.lnk
	$(LD) @$*.lnk
.IF $(LEAVE_LINKFILE)
.ELSE
	@$(RM) -S $(mktmp *.lnk)
.ENDIF
.ENDIF
