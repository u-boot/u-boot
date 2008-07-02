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
ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)../common)
$(shell mkdir -p $(obj)../xilinx_enet)
$(shell mkdir -p $(obj)../xilinx_iic)
endif

INCS		:= -I../common -I../xilinx_enet -I../xilinx_iic
CFLAGS		+= $(INCS)
HOST_CFLAGS	+= $(INCS)

LIB	= $(obj)lib$(BOARD).a

COBJS	= $(BOARD).o \
	  serial.o \
	  ../xilinx_enet/emac_adapter.o  ../xilinx_enet/xemac.o \
	  ../xilinx_enet/xemac_options.o ../xilinx_enet/xemac_polled.o \
	  ../xilinx_enet/xemac_intr.o ../xilinx_enet/xemac_g.o \
	  ../xilinx_enet/xemac_intr_dma.o ../xilinx_iic/iic_adapter.o \
	  ../xilinx_iic/xiic_l.o ../common/xipif_v1_23_b.o \
	  ../common/xbasic_types.o ../common/xdma_channel.o \
	  ../common/xdma_channel_sg.o ../common/xpacket_fifo_v1_00_b.o \
	  ../common/xversion.o \

SOBJS	= init.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
