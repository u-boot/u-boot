#
# (C) Copyright 2000-2006
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

LIB	= $(obj)lib$(BOARD).a

COBJS	:= trab.o flash.o vfd.o cmd_trab.o memory.o tsc2000.o auto_update.o
SOBJS	:= lowlevel_init.o

COBJS_FKT := trab_fkt.o rs485.o tsc2000.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c) $(COBJS_FKT:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

OBJS_FKT := $(addprefix $(obj),$(COBJS_FKT))

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

LOAD_ADDR = 0xc100000

#########################################################################

all:	$(LIB) $(obj)trab_fkt.srec $(obj)trab_fkt.bin

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(SOBJS)

$(obj)trab_fkt.srec:	$(OBJS_FKT) $(LIB)
	$(LD) -g -Ttext $(LOAD_ADDR) -o $(<:.o=) -e trab_fkt $^ $(LIB) \
		-L$(obj)../../examples -lstubs \
		-L$(obj)../../lib_generic -lgeneric \
		-L$(gcclibdir) -lgcc
	$(OBJCOPY) -O srec $(<:.o=) $@

$(obj)trab_fkt.bin:	$(obj)trab_fkt.srec
	$(OBJCOPY) -I srec -O binary $< $@

clean:
	rm -f $(SOBJS) $(OBJS) $(OBJS_FKT)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
