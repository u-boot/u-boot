#
# (C) Copyright 2000-2002
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

OBJS	:= trab.o flash.o vfd.o cmd_trab.o memory.o tsc2000.o auto_update.o
SOBJS	:= lowlevel_init.o

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

LOAD_ADDR = 0xc100000

#########################################################################

all:	$(LIB) trab_fkt.srec trab_fkt.bin

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

trab_fkt.srec:	trab_fkt.o rs485.o tsc2000.o $(LIB)
	$(LD) -g -Ttext $(LOAD_ADDR) -o $(<:.o=) -e $(<:.o=) $^ $(LIB) \
		-L../../examples -lstubs \
		-L../../lib_generic -lgeneric \
		-L$(gcclibdir) -lgcc
	$(OBJCOPY) -O srec $(<:.o=) $@

trab_fkt.bin:	trab_fkt.srec
	$(OBJCOPY) -O binary $< $@ 2>/dev/null

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CPPFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

-include .depend

#########################################################################
