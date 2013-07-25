#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS	= clock.o mxs.o iomux.o timer.o

ifdef	CONFIG_SPL_BUILD
COBJS	+= spl_boot.o spl_lradc_init.o spl_mem_init.o spl_power_init.o
endif

SRCS	:= $(START:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

# Specify the target for use in elftosb call
ELFTOSB_TARGET-$(CONFIG_MX23) = imx23
ELFTOSB_TARGET-$(CONFIG_MX28) = imx28

$(OBJTREE)/u-boot.bd: $(SRCTREE)/$(CPUDIR)/$(SOC)/u-boot-$(ELFTOSB_TARGET-y).bd
	sed "s@OBJTREE@$(OBJTREE)@g" $^ > $@

$(OBJTREE)/u-boot.sb: $(OBJTREE)/u-boot.bin $(OBJTREE)/spl/u-boot-spl.bin $(OBJTREE)/u-boot.bd
		elftosb -zf $(ELFTOSB_TARGET-y) -c $(OBJTREE)/u-boot.bd -o $(OBJTREE)/u-boot.sb

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
