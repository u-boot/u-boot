#
# (C) Copyright 2000
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

OBJS	= $(BOARD).o flash.o
SOBJS	= init.o

# HACK: depend needs bootscript.c, which needs tools/mkimage, which is not
# built in the depend stage.  So... put bootscript.o here, not in OBJS
$(LIB):	$(OBJS) $(SOBJS) bootscript.o
	$(AR) crv $@ $^

clean:
	rm -f $(SOBJS) $(OBJS) bootscript.c bootscript.image bootscript.o

distclean:	clean
	rm -f $(LIB) core *.bak .depend

$(BOARD).o : $(BOARD).c bootscript.o

bootscript.c: bootscript.image
	od -t x1 -v -A x $^ | awk -f x2c.awk > $@

bootscript.image: bootscript.hush Makefile
	-$(TOPDIR)/tools/mkimage -A ppc -O linux -T script -C none -a 0 -e 0 -n bootscript -d  bootscript.hush $@

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
