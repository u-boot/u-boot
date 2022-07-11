/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Gateworks Corporation
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* SPL */
/* Location in NAND to read U-Boot from */

/* Falcon Mode */

/* Falcon Mode - MMC support: args@1MB kernel@2MB */

#include "imx6_spl.h"                  /* common IMX6 SPL configuration */
#include "mx6_common.h"

/* Serial */
#define CONFIG_MXC_UART_BASE	       UART2_BASE

/* NAND */
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR      0

/*
 * PCI express
 */

/*
 * PMIC
 */
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR	0x08
#define CONFIG_POWER_LTC3676
#define CONFIG_POWER_LTC3676_I2C_ADDR  0x3c

/* Various command support */

/* USB Configs */
#define CONFIG_MXC_USB_PORTSC     (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS      0
#define CONFIG_USBD_HS

/* Framebuffer and LCD */
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP
#define CONFIG_HIDE_LOGO_VERSION  /* Custom config to hide U-boot version */

/* Miscellaneous configurable options */
#define CONFIG_HWCONFIG

/* Memory configuration */

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

/*
 * MTD Command for mtdparts
 */

/* Persistent Environment Config */

/* Environment */
#define CONFIG_IPADDR             192.168.1.1
#define CONFIG_SERVERIP           192.168.1.146

#endif			       /* __CONFIG_H */
