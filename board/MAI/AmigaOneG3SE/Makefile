#
# (C) Copyright 2002-2006
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
ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)../menu)
$(shell mkdir -p $(obj)../bios_emulator)
endif

LIB	= $(obj)lib$(BOARD).a

COBJS	= $(BOARD).o articiaS.o flash.o serial.o smbus.o articiaS_pci.o \
		via686.o i8259.o ../bios_emulator/x86interface.o	\
		../bios_emulator/bios.o ../bios_emulator/glue.o		\
		interrupts.o ps2kbd.o video.o usb_uhci.o enet.o	        \
		../menu/cmd_menu.o cmd_boota.o nvram.o

SOBJS	= board_asm_init.o memio.o

EMUDIR  = ../bios_emulator/scitech/src/x86emu/
EMUOBJ  = $(EMUDIR)decode.o $(EMUDIR)ops2.o $(EMUDIR)fpu.o $(EMUDIR)prim_ops.o \
		$(EMUDIR)ops.o $(EMUDIR)sys.o
EMUSRC  = $(EMUOBJ:.o=.c)

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))
EMUOBJ  := $(addprefix $(obj),$(EMUOBJ))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS) $(EMUSRC)
	make $(obj)libx86emu.a -C ../bios_emulator/scitech/src/x86emu -f makefile.uboot CROSS_COMPILE=$(CROSS_COMPILE)
	-rm $(LIB)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(SOBJS) $(EMUOBJ)


#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
