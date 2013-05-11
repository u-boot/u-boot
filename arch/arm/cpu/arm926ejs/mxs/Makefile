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
