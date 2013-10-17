#
# (C) Copyright 2003-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

SSTART	= start.o
CSTART  = traps.o
SOBJS  += io.o
SOBJS  += firmware_sc_task_bestcomm.impl.o
COBJS-y += i2c.o
COBJS-y += cpu.o
COBJS-y += cpu_init.o
COBJS-y += ide.o
COBJS-y += interrupts.o
COBJS-y += loadtask.o
COBJS-y += pci_mpc5200.o
COBJS-y += serial.o
COBJS-y += speed.o
COBJS-$(CONFIG_CMD_USB) += usb_ohci.o
COBJS-$(CONFIG_CMD_USB) += usb.o

ifdef CONFIG_SPL_BUILD
COBJS-y += spl_boot.o
endif

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS-y))
START	:= $(addprefix $(obj),$(SSTART) $(CSTART))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
