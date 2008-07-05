#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

LIB	= $(obj)lib$(BOARD).a

COBJS	:= netstar.o flash.o nand.o
SOBJS	:= setup.o crcek.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c) eeprom.c \
		eeprom_start.S
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

LOAD_ADDR = 0x10400000
LDSCRIPT = $(TOPDIR)/board/$(BOARDDIR)/eeprom.lds
lnk = $(if $(obj),$(obj),.)

HOST_CFLAGS = -Wall -pedantic -I$(TOPDIR)/include

all:	$(obj).depend $(LIB) $(obj)eeprom.srec $(obj)eeprom.bin \
		$(obj)crcek.srec $(obj)crcek.bin $(obj)crcit

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) $(ARFLAGS) $@ $^

$(obj)eeprom.srec:	$(obj)eeprom.o $(obj)eeprom_start.o
	cd $(lnk) && $(LD) -T $(LDSCRIPT) -g -Ttext $(LOAD_ADDR) \
		-o $(<:.o=) -e eeprom eeprom.o eeprom_start.o \
		-L$(obj)../../examples -lstubs \
		-L$(obj)../../lib_generic -lgeneric \
		-L$(gcclibdir) -lgcc
	$(OBJCOPY) -O srec $(<:.o=) $@

$(obj)eeprom.bin:	$(obj)eeprom.srec
	$(OBJCOPY) -I srec -O binary $< $@ 2>/dev/null

$(obj)crcek.srec:	$(obj)crcek.o
	$(LD) -g -Ttext 0x00000000 \
		-o $(<:.o=) -e crcek $^
	$(OBJCOPY) -O srec $(<:.o=) $@

$(obj)crcek.bin:	$(obj)crcek.srec
	$(OBJCOPY) -I srec -O binary $< $@ 2>/dev/null

$(obj)crcit:		$(obj)crcit.o $(obj)crc32.o
	$(HOSTCC) $(HOST_CFLAGS) -o $@ $^

$(obj)crcit.o:	crcit.c
	$(HOSTCC) $(HOST_CFLAGS) -o $@ -c $<

$(obj)crc32.o:	$(OBJTREE)/tools/crc32.c
	$(HOSTCC) $(HOST_CFLAGS) -DUSE_HOSTCC -o $@ -c $<

clean:
	rm -f $(SOBJS) $(OBJS) $(obj)eeprom $(obj)eeprom.srec \
		$(obj)eeprom.bin $(obj)crcek $(obj)crcek.srec \
		$(obj)crcek.bin

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
