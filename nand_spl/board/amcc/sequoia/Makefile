#
# (C) Copyright 2006
# Stefan Roese, DENX Software Engineering, sr@denx.de.
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

include $(TOPDIR)/config.mk
include board/$(BOARDDIR)/config.mk

LDSCRIPT= board/$(BOARDDIR)/u-boot.lds
LDFLAGS	= -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE) $(PLATFORM_LDFLAGS)
AFLAGS	+= -DCONFIG_NAND_SPL
CFLAGS	+= -DCONFIG_NAND_SPL

SOBJS	= start.o init.o resetvec.o
COBJS	= nand_boot.o ndfc.o sdram.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

ALL	= u-boot-spl u-boot-spl.bin u-boot-spl-16k.bin

all:	$(obj).depend $(ALL)

u-boot-spl-16k.bin: u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} --pad-to=$(PAD_TO) -O binary $< $@

u-boot-spl.bin:	u-boot-spl
	$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

u-boot-spl:	$(OBJS)
	$(LD) $(LDFLAGS) $$UNDEF_SYM $(OBJS) \
		-Map u-boot-spl.map -o u-boot-spl

# create symbolic links for common files

# from cpu directory
ndfc.c:
	@rm -f ndfc.c
	ln -s ../cpu/ppc4xx/ndfc.c ndfc.c

resetvec.S:
	@rm -f resetvec.S
	ln -s ../cpu/ppc4xx/resetvec.S resetvec.S

start.S:
	@rm -f start.S
	ln -s ../cpu/ppc4xx/start.S start.S

# from board directory
init.S:
	@rm -f init.S
	ln -s ../board/amcc/sequoia/init.S init.S

sdram.c:
	@rm -f sdram.c
	ln -s ../board/amcc/sequoia/sdram.c sdram.c

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
