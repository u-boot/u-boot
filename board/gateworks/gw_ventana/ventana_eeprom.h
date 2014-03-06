/*
 * Copyright (C) 2013 Gateworks Corporation
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef _VENTANA_EEPROM_
#define _VENTANA_EEPROM_

struct ventana_board_info {
	u8 mac0[6];          /* 0x00: MAC1 */
	u8 mac1[6];          /* 0x06: MAC2 */
	u8 res0[12];         /* 0x0C: reserved */
	u32 serial;          /* 0x18: Serial Number (read only) */
	u8 res1[4];          /* 0x1C: reserved */
	u8 mfgdate[4];       /* 0x20: MFG date (read only) */
	u8 res2[7];          /* 0x24 */
	/* sdram config */
	u8 sdram_size;       /* 0x2B: enum (512,1024,2048) MB */
	u8 sdram_speed;      /* 0x2C: enum (100,133,166,200,267,333,400) MHz */
	u8 sdram_width;      /* 0x2D: enum (32,64) bit */
	/* cpu config */
	u8 cpu_speed;        /* 0x2E: enum (800,1000,1200) MHz */
	u8 cpu_type;         /* 0x2F: enum (imx6q,imx6d,imx6dl,imx6s) */
	u8 model[16];        /* 0x30: model string */
	/* FLASH config */
	u8 nand_flash_size;  /* 0x40: enum (4,8,16,32,64,128) MB */
	u8 spi_flash_size;   /* 0x41: enum (4,8,16,32,64,128) MB */

	/* Config1: SoC Peripherals */
	u8 config[8];        /* 0x42: loading options */

	u8 res3[4];          /* 0x4A */

	u8 chksum[2];        /* 0x4E */
};

/* config bits */
enum {
	EECONFIG_ETH0,
	EECONFIG_ETH1,
	EECONFIG_HDMI_OUT,
	EECONFIG_SATA,
	EECONFIG_PCIE,
	EECONFIG_SSI0,
	EECONFIG_SSI1,
	EECONFIG_LCD,
	EECONFIG_LVDS0,
	EECONFIG_LVDS1,
	EECONFIG_USB0,
	EECONFIG_USB1,
	EECONFIG_SD0,
	EECONFIG_SD1,
	EECONFIG_SD2,
	EECONFIG_SD3,
	EECONFIG_UART0,
	EECONFIG_UART1,
	EECONFIG_UART2,
	EECONFIG_UART3,
	EECONFIG_UART4,
	EECONFIG_IPU0,
	EECONFIG_IPU1,
	EECONFIG_FLEXCAN,
	EECONFIG_MIPI_DSI,
	EECONFIG_MIPI_CSI,
	EECONFIG_TZASC0,
	EECONFIG_TZASC1,
	EECONFIG_I2C0,
	EECONFIG_I2C1,
	EECONFIG_I2C2,
	EECONFIG_VPU,
	EECONFIG_CSI0,
	EECONFIG_CSI1,
	EECONFIG_CAAM,
	EECONFIG_MEZZ,
	EECONFIG_RES1,
	EECONFIG_RES2,
	EECONFIG_RES3,
	EECONFIG_RES4,
	EECONFIG_ESPCI0,
	EECONFIG_ESPCI1,
	EECONFIG_ESPCI2,
	EECONFIG_ESPCI3,
	EECONFIG_ESPCI4,
	EECONFIG_ESPCI5,
	EECONFIG_RES5,
	EECONFIG_RES6,
	EECONFIG_GPS,
	EECONFIG_SPIFL0,
	EECONFIG_SPIFL1,
	EECONFIG_GSPBATT,
	EECONFIG_HDMI_IN,
	EECONFIG_VID_OUT,
	EECONFIG_VID_IN,
	EECONFIG_NAND,
	EECONFIG_RES8,
	EECONFIG_RES9,
	EECONFIG_RES10,
	EECONFIG_RES11,
	EECONFIG_RES12,
	EECONFIG_RES13,
	EECONFIG_RES14,
	EECONFIG_RES15,
};

#endif
