#
# (C) Copyright 2002
# Sysgo Real-Time Solutions, GmbH <www.elinos.com>
# Marius Groeger <mgroeger@sysgo.de>
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

LIB	= lib$(BOARD).a

OBJS	:= lpc2292sodimm.o flash.o mmc.o spi.o mmc_hw.o eth.o
SOBJS	:= lowlevel_init.o iap_entry.o

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

# this MUST be compiled as thumb code!
iap_entry.o:
	arm-linux-gcc  -D__ASSEMBLY__ -g  -Os   -fno-strict-aliasing  \
	-fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ \
	-DTEXT_BASE=0x81500000  -I/home/garyj/proj/LPC/u-boot/include \
	-fno-builtin -ffreestanding -nostdinc -isystem \
	/opt/eldk/arm/usr/bin/../lib/gcc/arm-linux/4.0.0/include -pipe  \
	-DCONFIG_ARM -D__ARM__ -march=armv4t -mtune=arm7tdmi -mabi=apcs-gnu \
	-c -o iap_entry.o iap_entry.S

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CPPFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

-include .depend

#########################################################################
