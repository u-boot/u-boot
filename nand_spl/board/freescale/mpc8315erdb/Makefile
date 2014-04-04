#
# (C) Copyright 2007
# Stefan Roese, DENX Software Engineering, sr@denx.de.
# (C) Copyright 2008 Freescale Semiconductor
#
# SPDX-License-Identifier:	GPL-2.0+
#

PAD_TO := 0xfff04000

nandobj	:= $(objtree)/nand_spl/

LDSCRIPT= $(srctree)/nand_spl/board/$(BOARDDIR)/u-boot.lds
LDFLAGS := -T $(nandobj)u-boot.lds -Ttext $(CONFIG_SYS_TEXT_BASE_SPL) \
	   $(LDFLAGS) $(LDFLAGS_FINAL)
asflags-y += -DCONFIG_NAND_SPL
ccflags-y += -DCONFIG_NAND_SPL

SOBJS	= start.o ticks.o
COBJS	= nand_boot_fsl_elbc.o $(BOARD).o sdram.o ns16550.o spl_minimal.o \
	  time.o cache.o

OBJS	:= $(addprefix $(obj)/,$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(nandobj)board/$(BOARDDIR)

targets += $(__OBJS)

all: $(nandobj)u-boot-spl.bin $(nandobj)u-boot-spl-16k.bin

$(nandobj)u-boot-spl-16k.bin: $(nandobj)u-boot-spl
	$(OBJCOPY) $(OBJCOPYFLAGS) --pad-to=$(PAD_TO) -O binary $< $@

$(nandobj)u-boot-spl.bin:	$(nandobj)u-boot-spl
	$(OBJCOPY) $(OBJCOPYFLAGS) -O binary $< $@

$(nandobj)u-boot-spl:	$(OBJS) $(nandobj)u-boot.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) $(PLATFORM_LIBS) \
		-Map $(nandobj)u-boot-spl.map -o $@

$(nandobj)u-boot.lds: $(LDSCRIPT)
	$(CPP) $(cpp_flags) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

# create symbolic links for common files

$(obj)/start.S:
	ln -sf $(srctree)/arch/powerpc/cpu/mpc83xx/start.S $@

$(obj)/nand_boot_fsl_elbc.c:
	ln -sf $(srctree)/nand_spl/nand_boot_fsl_elbc.c $@

$(obj)/sdram.c:
	ln -sf $(srctree)/board/$(BOARDDIR)/sdram.c $@

$(obj)/$(BOARD).c:
	ln -sf $(srctree)/board/$(BOARDDIR)/$(BOARD).c $@

$(obj)/ns16550.c:
	ln -sf $(srctree)/drivers/serial/ns16550.c $@

$(obj)/spl_minimal.c:
	ln -sf $(srctree)/arch/powerpc/cpu/mpc83xx/spl_minimal.c $@

$(obj)/cache.c:
	ln -sf $(srctree)/arch/powerpc/lib/cache.c $@

$(obj)/time.c:
	ln -sf $(srctree)/arch/powerpc/lib/time.c $@

$(obj)/ticks.S:
	ln -sf $(srctree)/arch/powerpc/lib/ticks.S $@
