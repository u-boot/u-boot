# Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
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

LIB := $(obj)libtpm.o

$(shell mkdir -p $(obj)slb9635_i2c)

# TODO: Merge tpm_tis_lpc.c with tpm.c
COBJS-$(CONFIG_TPM_ATMEL_TWI) += tpm_atmel_twi.o
COBJS-$(CONFIG_TPM_TIS_I2C) += tpm.o
COBJS-$(CONFIG_TPM_TIS_I2C) += tpm_tis_i2c.o
COBJS-$(CONFIG_TPM_TIS_LPC) += tpm_tis_lpc.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
