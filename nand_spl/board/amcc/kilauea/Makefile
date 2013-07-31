#
# (C) Copyright 2007
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk
include $(TOPDIR)/nand_spl/board/$(BOARDDIR)/config.mk

nandobj	:= $(OBJTREE)/nand_spl/

LDSCRIPT= $(TOPDIR)/nand_spl/board/$(BOARDDIR)/u-boot.lds
LDFLAGS := -T $(nandobj)u-boot.lds -Ttext $(CONFIG_SYS_TEXT_BASE) $(LDFLAGS) \
	   $(LDFLAGS_FINAL)
AFLAGS	+= -DCONFIG_NAND_SPL
CFLAGS	+= -DCONFIG_NAND_SPL

SOBJS	= start.o resetvec.o cache.o
COBJS	= 44x_spd_ddr2.o nand_boot.o nand_ecc.o ndfc.o

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(nandobj)board/$(BOARDDIR)

ALL	= $(nandobj)u-boot-spl $(nandobj)u-boot-spl.bin $(nandobj)u-boot-spl-16k.bin

all:	$(obj).depend $(ALL)

$(nandobj)u-boot-spl-16k.bin: $(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(PAD_TO) -O binary $< $@

$(nandobj)u-boot-spl.bin:	$(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(nandobj)u-boot-spl:	$(OBJS) $(nandobj)u-boot.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) $(PLATFORM_LIBS) \
		-Map $(nandobj)u-boot-spl.map \
		-o $(nandobj)u-boot-spl

$(nandobj)u-boot.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

# create symbolic links for common files

# from cpu directory
$(obj)44x_spd_ddr2.c: $(obj)ecc.h
	@rm -f $(obj)44x_spd_ddr2.c
	ln -s $(SRCTREE)/arch/powerpc/cpu/ppc4xx/44x_spd_ddr2.c $(obj)44x_spd_ddr2.c

$(obj)cache.S:
	@rm -f $(obj)cache.S
	ln -s $(SRCTREE)/arch/powerpc/cpu/ppc4xx/cache.S $(obj)cache.S

$(obj)ecc.h:
	@rm -f $(obj)ecc.h
	ln -s $(SRCTREE)/arch/powerpc/cpu/ppc4xx/ecc.h $(obj)ecc.h

$(obj)ndfc.c:
	@rm -f $(obj)ndfc.c
	ln -s $(SRCTREE)/drivers/mtd/nand/ndfc.c $(obj)ndfc.c

$(obj)resetvec.S:
	@rm -f $(obj)resetvec.S
	ln -s $(SRCTREE)/arch/powerpc/cpu/ppc4xx/resetvec.S $(obj)resetvec.S

$(obj)start.S:
	@rm -f $(obj)start.S
	ln -s $(SRCTREE)/arch/powerpc/cpu/ppc4xx/start.S $(obj)start.S

# from nand_spl directory
$(obj)nand_boot.c:
	@rm -f $(obj)nand_boot.c
	ln -s $(SRCTREE)/nand_spl/nand_boot.c $(obj)nand_boot.c

# from drivers/nand directory
$(obj)nand_ecc.c:
	@rm -f $(obj)nand_ecc.c
	ln -s $(SRCTREE)/drivers/mtd/nand/nand_ecc.c $(obj)nand_ecc.c

#########################################################################

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
