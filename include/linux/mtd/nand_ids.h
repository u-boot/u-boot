/*
 *  u-boot/include/linux/mtd/nand_ids.h
 *
 *  Copyright (c) 2000 David Woodhouse <dwmw2@mvhi.com>
 *                     Steven J. Hill <sjhill@cotw.com>
 *
 * $Id: nand_ids.h,v 1.1 2000/10/13 16:16:26 mdeans Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Info:
 *   Contains standard defines and IDs for NAND flash devices
 *
 *  Changelog:
 *   01-31-2000 DMW     Created
 *   09-18-2000 SJH     Moved structure out of the Disk-On-Chip drivers
 *			so it can be used by other NAND flash device
 *			drivers. I also changed the copyright since none
 *			of the original contents of this file are specific
 *			to DoC devices. David can whack me with a baseball
 *			bat later if I did something naughty.
 *   10-11-2000 SJH     Added private NAND flash structure for driver
 *   2000-10-13 BE      Moved out of 'nand.h' - avoids duplication.
 */

#ifndef __LINUX_MTD_NAND_IDS_H
#define __LINUX_MTD_NAND_IDS_H

#ifndef CFG_NAND_LEGACY
#error This module is for the legacy NAND support
#endif

static struct nand_flash_dev nand_flash_ids[] = {
	{"Toshiba TC5816BDC",     NAND_MFR_TOSHIBA, 0x64, 21, 1, 2, 0x1000, 0},
	{"Toshiba TC5832DC",      NAND_MFR_TOSHIBA, 0x6b, 22, 0, 2, 0x2000, 0},
	{"Toshiba TH58V128DC",    NAND_MFR_TOSHIBA, 0x73, 24, 0, 2, 0x4000, 0},
	{"Toshiba TC58256FT/DC",  NAND_MFR_TOSHIBA, 0x75, 25, 0, 2, 0x4000, 0},
	{"Toshiba TH58512FT",     NAND_MFR_TOSHIBA, 0x76, 26, 0, 3, 0x4000, 0},
	{"Toshiba TC58V32DC",     NAND_MFR_TOSHIBA, 0xe5, 22, 0, 2, 0x2000, 0},
	{"Toshiba TC58V64AFT/DC", NAND_MFR_TOSHIBA, 0xe6, 23, 0, 2, 0x2000, 0},
	{"Toshiba TC58V16BDC",    NAND_MFR_TOSHIBA, 0xea, 21, 1, 2, 0x1000, 0},
	{"Toshiba TH58100FT",     NAND_MFR_TOSHIBA, 0x79, 27, 0, 3, 0x4000, 0},
	{"Samsung KM29N16000",    NAND_MFR_SAMSUNG, 0x64, 21, 1, 2, 0x1000, 0},
	{"Samsung unknown 4Mb",   NAND_MFR_SAMSUNG, 0x6b, 22, 0, 2, 0x2000, 0},
	{"Samsung KM29U128T",     NAND_MFR_SAMSUNG, 0x73, 24, 0, 2, 0x4000, 0},
	{"Samsung KM29U256T",     NAND_MFR_SAMSUNG, 0x75, 25, 0, 2, 0x4000, 0},
	{"Samsung unknown 64Mb",  NAND_MFR_SAMSUNG, 0x76, 26, 0, 3, 0x4000, 0},
	{"Samsung KM29W32000",    NAND_MFR_SAMSUNG, 0xe3, 22, 0, 2, 0x2000, 0},
	{"Samsung unknown 4Mb",   NAND_MFR_SAMSUNG, 0xe5, 22, 0, 2, 0x2000, 0},
	{"Samsung KM29U64000",    NAND_MFR_SAMSUNG, 0xe6, 23, 0, 2, 0x2000, 0},
	{"Samsung KM29W16000",    NAND_MFR_SAMSUNG, 0xea, 21, 1, 2, 0x1000, 0},
	{"Samsung K9F5616Q0C",    NAND_MFR_SAMSUNG, 0x45, 25, 0, 2, 0x4000, 1},
	{"Samsung K9K1216Q0C",    NAND_MFR_SAMSUNG, 0x46, 26, 0, 3, 0x4000, 1},
	{"Samsung K9F1G08U0M",    NAND_MFR_SAMSUNG, 0xf1, 27, 0, 2, 0, 0},
	{NULL,}
};

#endif /* __LINUX_MTD_NAND_IDS_H */
