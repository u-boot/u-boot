#
# (C) Copyright 2005
# Ladislav Michl, 2N Telekomunikace, michl@2n.cz
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

OBJS	:= netstar.o flash.o nand.o
SOBJS	:= setup.o crcek.o

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

LOAD_ADDR = 0x10400000
LDSCRIPT = $(TOPDIR)/board/$(BOARDDIR)/eeprom.lds

HOST_CFLAGS = -Wall -pedantic -I$(TOPDIR)/include

all:	$(LIB) eeprom.srec eeprom.bin crcek.srec crcek.bin crcit

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $^

eeprom.srec:	eeprom.o eeprom_start.o
	$(LD) -T $(LDSCRIPT) -g -Ttext $(LOAD_ADDR) \
		-o $(<:.o=) -e $(<:.o=) $^ \
		-L../../examples -lstubs \
		-L../../lib_generic -lgeneric \
		-L$(gcclibdir) -lgcc
	$(OBJCOPY) -O srec $(<:.o=) $@

eeprom.bin:	eeprom.srec
	$(OBJCOPY) -I srec -O binary $< $@ 2>/dev/null

crcek.srec:	crcek.o
	$(LD) -g -Ttext 0x00000000 \
		-o $(<:.o=) -e $(<:.o=) $^
	$(OBJCOPY) -O srec $(<:.o=) $@

crcek.bin:	crcek.srec
	$(OBJCOPY) -I srec -O binary $< $@ 2>/dev/null

crcit:		crcit.o crc32.o
	$(HOSTCC) $(HOST_CFLAGS) -o $@ $^

crcit.o:	crcit.c
	$(HOSTCC) $(HOST_CFLAGS) -c $<

crc32.o:	$(TOPDIR)/tools/crc32.c
	$(HOSTCC) $(HOST_CFLAGS) -DUSE_HOSTCC -c $<

clean:
	rm -f $(SOBJS) $(OBJS) eeprom eeprom.srec eeprom.bin \
		crcek crcek.srec crcek.bin

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CPPFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
