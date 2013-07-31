#
# Copyright 2010-2011 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#
NAND_SPL := y
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

ALL	= $(nandobj)u-boot-spl $(nandobj)u-boot-spl.bin $(nandobj)u-boot-spl-16k.bin

all:	$(obj).depend $(ALL)

$(nandobj)u-boot-spl-16k.bin: $(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(PAD_TO) -O binary $< $@

$(nandobj)u-boot-spl.bin:	$(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(nandobj)u-boot-spl:	$(OBJS) $(nandobj)u-boot-nand_spl.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) $(PLATFORM_LIBS) \
		-Map $(nandobj)u-boot-spl.map \
		-o $(nandobj)u-boot-spl

$(nandobj)u-boot-nand_spl.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -I$(nandobj)/board/$(BOARDDIR) \
		-ansi -D__ASSEMBLY__ -P - <$< >$@

# create symbolic links for common files

$(obj)cache.c:
	@rm -f $(obj)cache.c
	ln -sf $(SRCTREE)/arch/powerpc/lib/cache.c $(obj)cache.c

$(obj)cpu_init_early.c:
	@rm -f $(obj)cpu_init_early.c
	ln -sf $(SRCTREE)/$(CPUDIR)/cpu_init_early.c $(obj)cpu_init_early.c

$(obj)spl_minimal.c:
	@rm -f $(obj)spl_minimal.c
	ln -sf $(SRCTREE)/$(CPUDIR)/spl_minimal.c $(obj)spl_minimal.c

$(obj)fsl_law.c:
	@rm -f $(obj)fsl_law.c
	ln -sf $(SRCTREE)/arch/powerpc/cpu/mpc8xxx/law.c $(obj)fsl_law.c

$(obj)law.c:
	@rm -f $(obj)law.c
	ln -sf $(SRCTREE)/board/$(BOARDDIR)/law.c $(obj)law.c

$(obj)nand_boot_fsl_elbc.c:
	@rm -f $(obj)nand_boot_fsl_elbc.c
	ln -sf $(SRCTREE)/nand_spl/nand_boot_fsl_elbc.c \
	       $(obj)nand_boot_fsl_elbc.c

$(obj)ns16550.c:
	@rm -f $(obj)ns16550.c
	ln -sf $(SRCTREE)/drivers/serial/ns16550.c $(obj)ns16550.c

$(obj)resetvec.S:
	@rm -f $(obj)resetvec.S
	ln -s $(SRCTREE)/$(CPUDIR)/resetvec.S $(obj)resetvec.S

$(obj)fixed_ivor.S:
	@rm -f $(obj)fixed_ivor.S
	ln -sf $(SRCTREE)/$(CPUDIR)/fixed_ivor.S $(obj)fixed_ivor.S

$(obj)start.S: $(obj)fixed_ivor.S
	@rm -f $(obj)start.S
	ln -sf $(SRCTREE)/$(CPUDIR)/start.S $(obj)start.S

$(obj)tlb.c:
	@rm -f $(obj)tlb.c
	ln -sf $(SRCTREE)/$(CPUDIR)/tlb.c $(obj)tlb.c

$(obj)tlb_table.c:
	@rm -f $(obj)tlb_table.c
	ln -sf $(SRCTREE)/board/$(BOARDDIR)/tlb.c $(obj)tlb_table.c

ifneq ($(OBJTREE), $(SRCTREE))
$(obj)nand_boot.c:
	@rm -f $(obj)nand_boot.c
	ln -s $(SRCTREE)/nand_spl/board/$(BOARDDIR)/nand_boot.c $(obj)nand_boot.c
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
