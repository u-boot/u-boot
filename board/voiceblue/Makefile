# (C) Copyright 2000-2006
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

LIB	= $(obj)lib$(BOARD).a

COBJS	:= voiceblue.o
SOBJS	:= setup.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c) eeprom.c eeprom_start.S
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

LOAD_ADDR = 0x10400000
LDSCRIPT = $(TOPDIR)/board/$(BOARDDIR)/eeprom.lds
lnk = $(if $(obj),$(obj),.)

all:	$(obj).depend $(LIB) $(obj)eeprom.srec $(obj)eeprom.bin

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(SOBJS)

$(obj)eeprom.srec:	$(obj)eeprom.o $(obj)eeprom_start.o
	cd $(lnk) && $(LD) -T $(LDSCRIPT) -g -Ttext $(LOAD_ADDR) \
		-o $(<:.o=) -e eeprom eeprom.o eeprom_start.o \
		-L$(obj)../../examples -lstubs \
		-L$(obj)../../lib_generic -lgeneric \
		-L$(gcclibdir) -lgcc
	$(OBJCOPY) -O srec $(<:.o=) $@

$(obj)eeprom.bin:	$(obj)eeprom.srec
	$(OBJCOPY) -I srec -O binary $< $@ 2>/dev/null

clean:
	rm -f $(SOBJS) $(OBJS) $(obj)eeprom \
		$(obj)eeprom.srec $(obj)eeprom.bin \
		$(obj)eeprom.o $(obj)eeprom_start.o


distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
