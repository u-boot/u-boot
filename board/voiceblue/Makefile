# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de
#
# (C) Copyright 2005
# Ladislav Michl, 2N Telekomunikace, michl@2n.cz
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
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

OBJS	:= voiceblue.o
SOBJS	:= setup.o

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

LOAD_ADDR = 0x10400000
LDSCRIPT = $(TOPDIR)/board/$(BOARDDIR)/eeprom.lds

all:	$(LIB) eeprom.srec eeprom.bin

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

eeprom.srec:	eeprom.o eeprom_start.o
	$(LD) -T $(LDSCRIPT) -g -Ttext $(LOAD_ADDR) \
		-o $(<:.o=) -e $(<:.o=) $^ \
		-L../../examples -lstubs \
		-L../../lib_generic -lgeneric \
		-L$(gcclibdir) -lgcc
	$(OBJCOPY) -O srec $(<:.o=) $@

eeprom.bin:	eeprom.srec
	$(OBJCOPY) -I srec -O binary $< $@ 2>/dev/null

clean:
	rm -f $(SOBJS) $(OBJS) eeprom eeprom.srec eeprom.bin

distclean:	clean
	rm -f $(LIB) core config.tmp *.bak .depend

#########################################################################

.depend:	Makefile $(SOBJS:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CPPFLAGS) $(SOBJS:.o=.S) $(OBJS:.o=.c) > $@

-include .depend

#########################################################################
