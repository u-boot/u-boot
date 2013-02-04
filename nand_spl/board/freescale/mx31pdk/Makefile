CONFIG_NAND_SPL	= y
PAD_TO	:= 2048

include $(TOPDIR)/config.mk

nandobj	:= $(OBJTREE)/nand_spl/

LDSCRIPT= $(TOPDIR)/nand_spl/board/$(BOARDDIR)/u-boot.lds
LSTSCRIPT= $(nandobj)/board/$(BOARDDIR)/u-boot.lst
LDFLAGS := -T $(nandobj)u-boot.lds -Ttext $(CONFIG_SYS_TEXT_BASE) $(LDFLAGS) \
	   $(LDFLAGS_FINAL)
AFLAGS	+= -DCONFIG_SPL_BUILD -DCONFIG_NAND_SPL
CFLAGS	+= -DCONFIG_SPL_BUILD -DCONFIG_NAND_SPL

SOBJS	= start.o crt0.o lowlevel_init.o
COBJS	= nand_boot_fsl_nfc.o

SRCS	:= $(SRCTREE)/nand_spl/nand_boot_fsl_nfc.c
SRCS	+= $(SRCTREE)/arch/arm/cpu/arm1136/start.S
SRCS	+= $(SRCTREE)/arch/arm/lib/crt0.S
SRCS	+= $(SRCTREE)/board/freescale/mx31pdk/lowlevel_init.S
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
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) \
		-Map $(nandobj)u-boot-spl.map \
		-o $@

# The following line expands into whole rule which generates $(LSTSCRIPT),
# the file containing u-boots LG-array linker section. This is included into
# $(LDSCRIPT). The function make_u_boot_list is defined in helper.mk file.
$(eval $(call make_u_boot_list, $(LSTSCRIPT), $(OBJS)))
$(nandobj)u-boot.lds: $(LDSCRIPT) $(LSTSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -I$(nandobj)/board/$(BOARDDIR) \
		-ansi -D__ASSEMBLY__ -P - <$< >$@

#########################################################################

$(obj)%.o:	$(SRCTREE)/arch/arm/cpu/arm1136/%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(SRCTREE)/arch/arm/lib/%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(SRCTREE)/board/freescale/mx31pdk/%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(SRCTREE)/nand_spl/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
