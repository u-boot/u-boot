
include $(TOPDIR)/config.mk
include $(TOPDIR)/include/config.mk
include $(TOPDIR)/onenand_ipl/board/$(BOARDDIR)/config.mk

LDSCRIPT= $(TOPDIR)/onenand_ipl/board/$(BOARDDIR)/u-boot.onenand.lds
LDFLAGS	= -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE) $(PLATFORM_LDFLAGS)
AFLAGS	+= -DCONFIG_ONENAND_IPL
CFLAGS	+= -DCONFIG_ONENAND_IPL
OBJCLFAGS += --gap-fill=0x00

SOBJS	= start.o low_levelinit.o # _memcpy32.o
COBJS	= apollon.o onenand_read.o onenand_boot.o

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(OBJTREE)/onenand_ipl/board/$(BOARDDIR)

onenandobj	:= $(OBJTREE)/onenand_ipl/

ALL	= $(onenandobj)onenand-ipl $(onenandobj)onenand-ipl.bin $(onenandobj)onenand-ipl-2k.bin

all:	$(obj).depend $(ALL)

$(onenandobj)onenand-ipl-2k.bin:	$(onenandobj)onenand-ipl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=0x800 -O binary $< $@

$(onenandobj)onenand-ipl.bin:	$(onenandobj)onenand-ipl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(onenandobj)onenand-ipl:	$(OBJS)
	cd $(LNDIR) && $(LD) $(LDFLAGS) $$UNDEF_SYM $(__OBJS) \
		-Map $(onenandobj)onenand-ipl.map \
		-o $(onenandobj)onenand-ipl

# create symbolic links from common files

# from cpu directory
$(obj)start.S:
	rm -f $(obj)start.S
	ln -s $(SRCTREE)/cpu/$(CPU)/start.S $(obj)start.S

# from onenand_ipl directory
$(obj)onenand_ipl.h:
	rm -f $(obj)onenand_ipl.h
	ln -s $(SRCTREE)/onenand_ipl/onenand_ipl.h $(obj)onenand_ipl.h

$(obj)onenand_boot.c:	$(obj)onenand_ipl.h
	rm -f $(obj)onenand_boot.c
	ln -s $(SRCTREE)/onenand_ipl/onenand_boot.c $(obj)onenand_boot.c

$(obj)onenand_read.c:	$(obj)onenand_ipl.h
	rm -f $(obj)onenand_read.c
	ln -s $(SRCTREE)/onenand_ipl/onenand_read.c $(obj)onenand_read.c

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)$.c
	$(CC) $(CFLAGS) -c -o $@ $<

include $(SRCTREE)/rules.mk

sinclude $(obj).depend
