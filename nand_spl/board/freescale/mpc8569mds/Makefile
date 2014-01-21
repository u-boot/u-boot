#
# (C) Copyright 2007
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# Copyright 2009-2011 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

NAND_SPL := y
CONFIG_SYS_TEXT_BASE_SPL := 0xfff00000
PAD_TO := 0xfff01000

include $(TOPDIR)/config.mk

nandobj	:= $(OBJTREE)/nand_spl/

LDSCRIPT= $(TOPDIR)/$(CPUDIR)/u-boot-nand_spl.lds
LDFLAGS := -T $(nandobj)u-boot-nand_spl.lds -Ttext $(CONFIG_SYS_TEXT_BASE_SPL) \
		$(LDFLAGS) $(LDFLAGS_FINAL)
AFLAGS	+= -DCONFIG_NAND_SPL
CFLAGS	+= -DCONFIG_NAND_SPL

SOBJS	= start.o resetvec.o
COBJS	= cache.o cpu_init_early.o spl_minimal.o fsl_law.o law.o \
	  nand_boot.o nand_boot_fsl_elbc.o ns16550.o tlb.o tlb_table.o

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(nandobj)board/$(BOARDDIR)

all: $(nandobj)u-boot-spl.bin $(nandobj)u-boot-spl-16k.bin

$(nandobj)u-boot-spl-16k.bin: $(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(PAD_TO) -O binary $< $@

$(nandobj)u-boot-spl.bin:	$(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(nandobj)u-boot-spl:	$(OBJS) $(nandobj)u-boot-nand_spl.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) $(PLATFORM_LIBS) \
		-Map $(nandobj)u-boot-spl.map -o $@

$(nandobj)u-boot-nand_spl.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -I$(nandobj)/board/$(BOARDDIR) \
		-ansi -D__ASSEMBLY__ -P - <$< >$@

# create symbolic links for common files

$(obj)cache.c:
	@rm -f $@
	ln -sf $(SRCTREE)/arch/powerpc/lib/cache.c $@

$(obj)cpu_init_early.c:
	@rm -f $@
	ln -sf $(SRCTREE)/arch/powerpc/cpu/mpc85xx/cpu_init_early.c $@

$(obj)spl_minimal.c:
	@rm -f $@
	ln -sf $(SRCTREE)/arch/powerpc/cpu/mpc85xx/spl_minimal.c $@

$(obj)fsl_law.c:
	@rm -f $@
	ln -sf $(SRCTREE)/arch/powerpc/cpu/mpc8xxx/law.c $@

$(obj)law.c:
	@rm -f $@
	ln -sf $(SRCTREE)/board/$(BOARDDIR)/law.c $@

$(obj)nand_boot_fsl_elbc.c:
	@rm -f $@
	ln -sf $(SRCTREE)/nand_spl/nand_boot_fsl_elbc.c $@

$(obj)ns16550.c:
	@rm -f $@
	ln -sf $(SRCTREE)/drivers/serial/ns16550.c $@

$(obj)resetvec.S:
	@rm -f $@
	ln -s $(SRCTREE)/$(CPUDIR)/resetvec.S $@

$(obj)fixed_ivor.S:
	@rm -f $@
	ln -sf $(SRCTREE)/arch/powerpc/cpu/mpc85xx/fixed_ivor.S $@

$(obj)start.S: $(obj)fixed_ivor.S
	@rm -f $@
	ln -sf $(SRCTREE)/arch/powerpc/cpu/mpc85xx/start.S $@

$(obj)tlb.c:
	@rm -f $@
	ln -sf $(SRCTREE)/arch/powerpc/cpu/mpc85xx/tlb.c $@

$(obj)tlb_table.c:
	@rm -f $@
	ln -sf $(SRCTREE)/board/$(BOARDDIR)/tlb.c $@

ifneq ($(OBJTREE), $(SRCTREE))
$(obj)nand_boot.c:
	@rm -f $@
	ln -s $(SRCTREE)/nand_spl/board/$(BOARDDIR)/nand_boot.c $@
endif

#########################################################################

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
