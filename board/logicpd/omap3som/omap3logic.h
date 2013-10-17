/*
 * (C) Copyright 2011
 * Logic Product Development <www.logicpd.com>
 *
 * Author:
 * Peter Barada <peter.barada@logicpd.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _OMAP3LOGIC_H_
#define _OMAP3LOGIC_H_

/*
 * OMAP3 GPMC register settings for CS1 LAN922x
 */
#define NET_LAN92XX_GPMC_CONFIG1	0x00001000
#define NET_LAN92XX_GPMC_CONFIG2	0x00080801
#define NET_LAN92XX_GPMC_CONFIG3	0x00000000
#define NET_LAN92XX_GPMC_CONFIG4	0x08010801
#define NET_LAN92XX_GPMC_CONFIG5	0x00080a0a
#define NET_LAN92XX_GPMC_CONFIG6	0x03000280


const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"Logic DM37x/OMAP35x reference board",
	"NAND",
};


#endif
