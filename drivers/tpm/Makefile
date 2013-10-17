# Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
#
# SPDX-License-Identifier:	GPL-2.0+
#

$(shell mkdir -p $(obj)slb9635_i2c)

# TODO: Merge tpm_tis_lpc.c with tpm.c
obj-$(CONFIG_TPM_ATMEL_TWI) += tpm_atmel_twi.o
obj-$(CONFIG_TPM_TIS_I2C) += tpm.o
obj-$(CONFIG_TPM_TIS_I2C) += tpm_tis_i2c.o
obj-$(CONFIG_TPM_TIS_LPC) += tpm_tis_lpc.o
