#
# (C) Copyright 2008
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/nand_spl/board/$(BOARDDIR)/config.mk

nandobj	:= $(OBJTREE)/nand_spl/

LDSCRIPT= $(TOPDIR)/nand_spl/board/$(BOARDDIR)/u-boot.lds
LDFLAGS := -T $(nandobj)u-boot.lds -Ttext $(CONFIG_SYS_TEXT_BASE) $(LDFLAGS) \
	   $(LDFLAGS_FINAL)
asflags-y += -DCONFIG_NAND_SPL
ccflags-y += -DCONFIG_NAND_SPL

SOBJS	:= start.o
SOBJS	+= init.o
SOBJS	+= resetvec.o
COBJS	:= ddr2_fixed.o
COBJS	+= nand_boot.o
COBJS	+= nand_ecc.o
COBJS	+= ndfc.o

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

# from cpu directory
$(obj)/ndfc.c:
	@rm -f $@
	ln -s $(SRCTREE)/drivers/mtd/nand/ndfc.c $@

$(obj)/resetvec.S:
	@rm -f $@
	ln -s $(SRCTREE)/arch/powerpc/cpu/ppc4xx/resetvec.S $@

$(obj)/start.S:
	@rm -f $@
	ln -s $(SRCTREE)/arch/powerpc/cpu/ppc4xx/start.S $@

# from board directory
$(obj)/init.S:
	@rm -f $@
	ln -s $(SRCTREE)/board/amcc/canyonlands/init.S $@

# from nand_spl directory
$(obj)/nand_boot.c:
	@rm -f $@
	ln -s $(SRCTREE)/nand_spl/nand_boot.c $@

# from drivers/mtd/nand directory
$(obj)/nand_ecc.c:
	@rm -f $@
	ln -s $(SRCTREE)/drivers/mtd/nand/nand_ecc.c $@
