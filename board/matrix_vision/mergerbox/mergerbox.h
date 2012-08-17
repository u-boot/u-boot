/*
 * Copyright (C) 2011 Matrix Vision GmbH
 * Andre Schwarz <andre.schwarz@matrix-vision.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __MERGERBOX_H__
#define __MERGERBOX_H__

#define MV_GPIO

/*
 * GPIO Bank 1
 */
#define	TFT_SPI_EN	(0x80000000>>0)
#define	FPGA_CONFIG	(0x80000000>>1)
#define	FPGA_STATUS	(0x80000000>>2)
#define	FPGA_CONF_DONE	(0x80000000>>3)
#define	FPGA_DIN	(0x80000000>>4)
#define	FPGA_CCLK	(0x80000000>>5)
#define	MAN_RST		(0x80000000>>6)
#define	FPGA_SYS_RST	(0x80000000>>7)
#define	WD_WDI		(0x80000000>>8)
#define	TFT_RST		(0x80000000>>9)
#define	HISCON_GPIO1	(0x80000000>>10)
#define	HISCON_GPIO2	(0x80000000>>11)
#define	B2B_GPIO2	(0x80000000>>12)
#define	CCU_GPIN	(0x80000000>>13)
#define	CCU_GPOUT	(0x80000000>>14)
#define	TFT_GPIO0	(0x80000000>>15)
#define	TFT_GPIO1	(0x80000000>>16)
#define	TFT_GPIO2	(0x80000000>>17)
#define	TFT_GPIO3	(0x80000000>>18)
#define	B2B_GPIO0	(0x80000000>>19)
#define	B2B_GPIO1	(0x80000000>>20)
#define	TFT_SPI_CPLD_CS	(0x80000000>>21)
#define	TFT_SPI_CS	(0x80000000>>22)
#define	CCU_PWR_EN	(0x80000000>>23)
#define	B2B_GPIO3	(0x80000000>>24)
#define	CCU_PWR_STAT	(0x80000000>>25)

#define MV_GPIO1_DAT	(FPGA_CONFIG|CCU_PWR_EN|TFT_SPI_CPLD_CS)
#define MV_GPIO1_OUT	(TFT_SPI_EN|FPGA_CONFIG|FPGA_DIN|FPGA_CCLK|CCU_PWR_EN| \
			 TFT_SPI_CPLD_CS)
#define MV_GPIO1_ODE	(FPGA_CONFIG|MAN_RST)

/*
 * GPIO Bank 2
 */
#define	SPI_FLASH_WP	(0x80000000>>10)
#define	SYS_EEPROM_WP	(0x80000000>>11)
#define	SPI_FLASH_CS	(0x80000000>>22)

#define MV_GPIO2_DAT	(SYS_EEPROM_WP|SPI_FLASH_CS)
#define MV_GPIO2_OUT	(SPI_FLASH_WP|SYS_EEPROM_WP|SPI_FLASH_CS)
#define MV_GPIO2_ODE	0

void mergerbox_tft_dim(u16 value);

#endif
