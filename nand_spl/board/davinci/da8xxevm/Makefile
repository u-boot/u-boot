#
# (C) Copyright 2006-2007
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# (C) Copyright 2008
# Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

CONFIG_NAND_SPL	= y

include $(TOPDIR)/config.mk

nandobj	:= $(OBJTREE)/nand_spl/

LDSCRIPT= $(TOPDIR)/nand_spl/board/$(BOARDDIR)/u-boot.lds
LDFLAGS := -T $(nandobj)u-boot.lds -Ttext $(CONFIG_SYS_TEXT_BASE) $(LDFLAGS) \
	   $(LDFLAGS_FINAL)
AFLAGS	+= -DCONFIG_SPL_BUILD -DCONFIG_NAND_SPL
CFLAGS	+= -DCONFIG_SPL_BUILD -DCONFIG_NAND_SPL

SOBJS	= _divsi3.o \
	_udivsi3.o \
	start.o

COBJS	= cpu.o \
	davinci_nand.o \
	davinci_pinmux.o \
	div0.o \
	hawkboard_nand_spl.o \
	memsize.o \
	misc.o \
	nand_boot.o \
	ns16550.o \
	psc.o

SRCS	:= $(addprefix $(obj),$(SOBJS:.o=.S) $(COBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
__OBJS	:= $(SOBJS) $(COBJS)
LNDIR	:= $(nandobj)board/$(BOARDDIR)

ALL	= $(nandobj)u-boot-spl $(nandobj)u-boot-spl.bin \
	$(nandobj)u-boot-spl-16k.bin

all:	$(ALL)

$(nandobj)u-boot-spl-16k.bin: $(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(PAD_TO) -O binary $< $@

$(nandobj)u-boot-spl.bin:	$(nandobj)u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

$(nandobj)u-boot-spl:	$(OBJS) $(nandobj)u-boot.lds
	cd $(LNDIR) && $(LD) $(LDFLAGS) $(__OBJS) \
		-Map $(nandobj)u-boot-spl.map \
		-o $(nandobj)u-boot-spl

$(nandobj)u-boot.lds: $(LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

# create symbolic links for common files

# from board directory
$(obj)davinci_pinmux.c:
	@rm -f $@
	@ln -s $(TOPDIR)/board/davinci/common/davinci_pinmux.c $@

# from drivers/mtd/nand directory
$(obj)davinci_nand.c:
	@rm -f $@
	@ln -s $(TOPDIR)/drivers/mtd/nand/davinci_nand.c $@

# from nand_spl directory
$(obj)nand_boot.c:
	@rm -f $@
	@ln -s $(TOPDIR)/nand_spl/nand_boot.c $@

# from drivers/serial directory
$(obj)ns16550.c:
	@rm -f $@
	@ln -sf $(TOPDIR)/drivers/serial/ns16550.c $@

# from cpu directory
$(obj)start.S:
	@rm -f $@
	ln -s $(TOPDIR)/arch/arm/cpu/arm926ejs/start.S $@

# from lib directory
$(obj)_udivsi3.S:
	@rm -f $@
	ln -s $(TOPDIR)/arch/arm/lib/_udivsi3.S $@

# from lib directory
$(obj)_divsi3.S:
	@rm -f $@
	ln -s $(TOPDIR)/arch/arm/lib/_divsi3.S $@

# from lib directory
$(obj)div0.c:
	@rm -f $@
	ln -s $(TOPDIR)/arch/arm/lib/div0.c $@

# from SoC directory
$(obj)cpu.c:
	@rm -f $@
	@ln -s $(TOPDIR)/arch/arm/cpu/arm926ejs/davinci/cpu.c $@

# from board directory
$(obj)hawkboard_nand_spl.c:
	@rm -f $@
	ln -s $(TOPDIR)/board/davinci/da8xxevm/hawkboard_nand_spl.c $@

# from board directory
$(obj)misc.c:
	@rm -f $@
	ln -s $(TOPDIR)/board/davinci/common/misc.c $@

$(obj)psc.c:
	@rm -f $@
	ln -s $(TOPDIR)/arch/arm/cpu/arm926ejs/davinci/psc.c $@

# from common directory
$(obj)memsize.c:
	@rm -f $@
	ln -s $(TOPDIR)/common/memsize.c $@

#########################################################################

$(obj)%.o:	$(obj)%.S
	$(CC) $(AFLAGS) -c -o $@ $<

$(obj)%.o:	$(obj)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
