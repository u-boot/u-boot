
include $(TOPDIR)/config.mk
include $(TOPDIR)/onenand_ipl/board/$(BOARDDIR)/config.mk

LDSCRIPT= $(TOPDIR)/onenand_ipl/board/$(BOARDDIR)/u-boot.onenand.lds
LDFLAGS	= -Bstatic -T $(onenandobj)u-boot.lds -Ttext $(CONFIG_SYS_TEXT_BASE) $(PLATFORM_LDFLAGS)
AFLAGS	+= -DCONFIG_SPL_BUILD -DCONFIG_ONENAND_IPL
CFLAGS	+= -DCONFIG_SPL_BUILD -DCONFIG_ONENAND_IPL
OBJCFLAGS += --gap-fill=0x00

SOBJS	:= low_levelinit.o
SOBJS	+= start.o
COBJS	:= apollon.o
COBJS	+= onenand_read.o
COBJS	+= onenand_boot.o

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(OBJTREE)/onenand_ipl/board/$(BOARDDIR)

onenandobj	:= $(OBJTREE)/onenand_ipl/

ALL	= $(onenandobj)onenand-ipl $(onenandobj)onenand-ipl.bin $(onenandobj)onenand-ipl-2k.bin $(onenandobj)onenand-ipl-4k.bin

all:	$(obj).depend $(ALL)

$(onenandobj)onenand-ipl-2k.bin:	$(onenandobj)onenand-ipl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=0x800 -O binary $< $@

$(onenandobj)onenand-ipl-4k.bin:	$(onenandobj)onenand-ipl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=0x1000 -O binary $< $@

$(onenandobj)onenand-ipl.bin:	$(onenandobj)onenand-ipl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(onenandobj)onenand-ipl:	$(OBJS) $(onenandobj)u-boot.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $$UNDEF_SYM $(__OBJS) \
		-Map $@.map -o $@

$(onenandobj)u-boot.lds:	$(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

# create symbolic links from common files

# from cpu directory
$(obj)start.S:
	@rm -f $@
	ln -s $(SRCTREE)/$(CPUDIR)/start.S $@

# from onenand_ipl directory
$(obj)onenand_ipl.h:
	@rm -f $@
	ln -s $(SRCTREE)/onenand_ipl/onenand_ipl.h $@

$(obj)onenand_boot.c:	$(obj)onenand_ipl.h
	@rm -f $@
	ln -s $(SRCTREE)/onenand_ipl/onenand_boot.c $@

$(obj)onenand_read.c:	$(obj)onenand_ipl.h
	@rm -f $@
	ln -s $(SRCTREE)/onenand_ipl/onenand_read.c $@

ifneq ($(OBJTREE), $(SRCTREE))
$(obj)apollon.c:
	@rm -f $@
	ln -s $(SRCTREE)/onenand_ipl/board/$(BOARDDIR)/apollon.c $@

$(obj)low_levelinit.S:
	@rm -f $@
	ln -s $(SRCTREE)/onenand_ipl/board/$(BOARDDIR)/low_levelinit.S $@
endif

#########################################################################

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)$.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
