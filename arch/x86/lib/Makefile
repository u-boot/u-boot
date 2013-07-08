#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(ARCH).o

COBJS-$(CONFIG_CMD_BOOTM) += bootm.o
COBJS-y	+= cmd_boot.o
COBJS-y	+= gcc.o
COBJS-y	+= init_helpers.o
COBJS-y	+= interrupts.o
COBJS-$(CONFIG_SYS_PCAT_INTERRUPTS) += pcat_interrupts.o
COBJS-$(CONFIG_SYS_PCAT_TIMER) += pcat_timer.o
COBJS-$(CONFIG_PCI) += pci_type1.o
COBJS-y	+= relocate.o
COBJS-y += physmem.o
COBJS-y	+= string.o
COBJS-$(CONFIG_SYS_X86_TSC_TIMER)	+= tsc_timer.o
COBJS-$(CONFIG_VIDEO_VGA)	+= video.o
COBJS-$(CONFIG_CMD_ZBOOT)	+= zimage.o

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(PREFIXED_LIBGCC): $(NORMAL_LIBGCC)
	$(OBJCOPY) $< $@ --prefix-symbols=__normal_

$(LIB): $(PREFIXED_LIBGCC)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
