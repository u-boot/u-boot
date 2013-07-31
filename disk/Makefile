#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

#CFLAGS += -DET_DEBUG -DDEBUG

LIB	= $(obj)libdisk.o

COBJS-$(CONFIG_PARTITIONS) 	+= part.o
COBJS-$(CONFIG_MAC_PARTITION)   += part_mac.o
COBJS-$(CONFIG_DOS_PARTITION)   += part_dos.o
COBJS-$(CONFIG_ISO_PARTITION)   += part_iso.o
COBJS-$(CONFIG_AMIGA_PARTITION) += part_amiga.o
COBJS-$(CONFIG_EFI_PARTITION)   += part_efi.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
