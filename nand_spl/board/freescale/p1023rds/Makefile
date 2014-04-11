#
# Copyright 2010-2011 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

PAD_TO := 0xfff01000

nandobj	:= $(objtree)/nand_spl/

LDSCRIPT= $(srctree)/$(CPUDIR)/u-boot-nand_spl.lds
LDFLAGS := -T $(nandobj)u-boot-nand_spl.lds -Ttext $(CONFIG_SYS_TEXT_BASE_SPL) \
		$(LDFLAGS) $(LDFLAGS_FINAL)
asflags-y += -DCONFIG_NAND_SPL
ccflags-y += -DCONFIG_NAND_SPL

SOBJS	= start.o resetvec.o
COBJS	= cache.o cpu_init_early.o spl_minimal.o fsl_law.o law.o \
	  nand_boot.o nand_boot_fsl_elbc.o ns16550.o tlb.o tlb_table.o

OBJS	:= $(addprefix $(obj)/,$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(nandobj)board/$(BOARDDIR)

targets += $(__OBJS)

all: $(nandobj)u-boot-spl.bin $(nandobj)u-boot-spl-16k.bin

$(nandobj)u-boot-spl-16k.bin: $(nandobj)u-boot-spl
	$(OBJCOPY) $(OBJCOPYFLAGS) --pad-to=$(PAD_TO) -O binary $< $@

$(nandobj)u-boot-spl.bin:	$(nandobj)u-boot-spl
	$(OBJCOPY) $(OBJCOPYFLAGS) -O binary $< $@

$(nandobj)u-boot-spl:	$(OBJS) $(nandobj)u-boot-nand_spl.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) $(PLATFORM_LIBS) \
		-Map $(nandobj)u-boot-spl.map -o $@

$(nandobj)u-boot-nand_spl.lds: $(LDSCRIPT)
	$(CPP) $(cpp_flags) $(LDPPFLAGS) -I$(nandobj)/board/$(BOARDDIR) \
		-ansi -D__ASSEMBLY__ -P - <$< >$@

# create symbolic links for common files

$(obj)/cache.c:
	@rm -f $@
	ln -sf $(srctree)/arch/powerpc/lib/cache.c $@

$(obj)/cpu_init_early.c:
	@rm -f $@
	ln -sf $(srctree)/$(CPUDIR)/cpu_init_early.c $@

$(obj)/spl_minimal.c:
	@rm -f $@
	ln -sf $(srctree)/$(CPUDIR)/spl_minimal.c $@

$(obj)/fsl_law.c:
	@rm -f $@
	ln -sf $(srctree)/arch/powerpc/cpu/mpc8xxx/law.c $@

$(obj)/law.c:
	@rm -f $@
	ln -sf $(srctree)/board/$(BOARDDIR)/law.c $@

$(obj)/nand_boot_fsl_elbc.c:
	@rm -f $@
	ln -sf $(srctree)/nand_spl/nand_boot_fsl_elbc.c $@

$(obj)/ns16550.c:
	@rm -f $@
	ln -sf $(srctree)/drivers/serial/ns16550.c $@

$(obj)/resetvec.S:
	@rm -f $@
	ln -s $(srctree)/$(CPUDIR)/resetvec.S $@

$(obj)/start.S:
	@rm -f $@
	ln -sf $(srctree)/$(CPUDIR)/start.S $@

$(obj)/tlb.c:
	@rm -f $@
	ln -sf $(srctree)/$(CPUDIR)/tlb.c $@

$(obj)/tlb_table.c:
	@rm -f $@
	ln -sf $(srctree)/board/$(BOARDDIR)/tlb.c $@
