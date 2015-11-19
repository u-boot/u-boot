/*
 * Copyright (c) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#undef CONFIG_CMD_SF_TEST

#undef CONFIG_TPM_TIS_BASE_ADDRESS

#undef CONFIG_CMD_IMLS

#undef CONFIG_X86_SERIAL
#undef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_IS_NOWHERE
#undef CONFIG_VIDEO
#undef CONFIG_CFB_CONSOLE
#undef CONFIG_SCSI_AHCI
#undef CONFIG_CMD_SCSI
#undef CONFIG_INTEL_ICH6_GPIO

#define CONFIG_STD_DEVICES_SETTINGS     "stdin=usbkbd,vga,serial\0" \
					"stdout=vga,serial\0" \
					"stderr=vga,serial\0"

#endif
