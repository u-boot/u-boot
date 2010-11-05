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

LIB	= $(obj)lib$(BOARD).o

COBJS	:= voiceblue.o
SOBJS	:= setup.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

LOAD_ADDR = 0x10400000

#########################################################################

all:	$(obj).depend $(LIB) $(obj)eeprom.srec $(obj)eeprom.bin

$(LIB):	$(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $^)

$(obj)eeprom_start.o:
	echo "b eeprom" | $(CC) $(AFLAGS) -c -x assembler -o $@ -

$(obj)eeprom:		$(obj)eeprom_start.o $(obj)eeprom.o
	$(LD) -Ttext $(LOAD_ADDR) -e eeprom -o $@ $^ \
		-L$(obj)../../examples/standalone -lstubs \
		$(PLATFORM_LIBS)

$(obj)eeprom.srec:	$(obj)eeprom
	$(OBJCOPY) -S -O srec $(<:.o=) $@

$(obj)eeprom.bin:	$(obj)eeprom
	$(OBJCOPY) -S -O binary $< $@

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
