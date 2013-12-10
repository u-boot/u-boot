#
# (C) Copyright 2007
# Stefan Roese, DENX Software Engineering, sr@denx.de.
# (C) Copyright 2008 Freescale Semiconductor
# (C) Copyright Sheldon Instruments, Inc. 2008
#
# SPDX-License-Identifier:	GPL-2.0+
#

NAND_SPL := y

include $(TOPDIR)/config.mk

nandobj	:= $(OBJTREE)/nand_spl/

LDSCRIPT= $(TOPDIR)/nand_spl/board/$(BOARDDIR)/u-boot.lds
LDFLAGS := -T $(nandobj)u-boot.lds -Ttext $(CONFIG_SYS_TEXT_BASE_SPL) \
	   $(LDFLAGS) $(LDFLAGS_FINAL)
AFLAGS	+= -DCONFIG_NAND_SPL
CFLAGS	+= -DCONFIG_NAND_SPL

SOBJS	= start.o ticks.o
COBJS	= nand_boot_fsl_elbc.o $(BOARD).o sdram.o ns16550.o spl_minimal.o \
	  time.o cache.o

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(nandobj)board/$(BOARDDIR)

all: $(nandobj)u-boot-spl.bin $(nandobj)u-boot-spl-16k.bin

$(nandobj)u-boot-spl-16k.bin:	$(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(PAD_TO) -O binary $< $@

$(nandobj)u-boot-spl.bin:	$(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(nandobj)u-boot-spl:	$(OBJS) $(nandobj)u-boot.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) $(PLATFORM_LIBS) \
		-Map $(nandobj)u-boot-spl.map -o $@

$(nandobj)u-boot.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

# create symbolic links for common files

$(obj)start.S:
	@rm -f $@
	ln -s $(SRCTREE)/arch/powerpc/cpu/mpc83xx/start.S $@

$(obj)nand_boot_fsl_elbc.c:
	@rm -f $@
	ln -s $(SRCTREE)/nand_spl/nand_boot_fsl_elbc.c $@

$(obj)sdram.c:
	@rm -f $@
	ln -s $(SRCTREE)/board/$(BOARDDIR)/sdram.c $@

$(obj)$(BOARD).c:
	@rm -f $@
	ln -s $(SRCTREE)/board/$(BOARDDIR)/$(BOARD).c $@

$(obj)ns16550.c:
	@rm -f $@
	ln -s $(SRCTREE)/drivers/serial/ns16550.c $@

$(obj)spl_minimal.c:
	@rm -f $@
	ln -s $(SRCTREE)/arch/powerpc/cpu/mpc83xx/spl_minimal.c $@

$(obj)cache.c:
	@rm -f $@
	ln -s $(SRCTREE)/arch/powerpc/lib/cache.c $@

$(obj)time.c:
	@rm -f $@
	ln -s $(SRCTREE)/arch/powerpc/lib/time.c $@

$(obj)ticks.S:
	@rm -f $@
	ln -s $(SRCTREE)/arch/powerpc/lib/ticks.S $@

#########################################################################

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
