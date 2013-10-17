# Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
#
# SPDX-License-Identifier:	GPL-2.0+
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
