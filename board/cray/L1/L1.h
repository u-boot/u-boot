/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/****************************************************************************
 * FLASH Memory Map as used by CRAY L1, 4MB AMD29F032B flash chip
 *
 *                          Start Address    Length
 * +++++++++++++++++++++++++ 0xFFC0_0000     Start of Flash -----------------
 * | Failsafe Linux Image  |	(1M)
 * +=======================+ 0xFFD0_0000
 * | (Reserved FlashFiles) |	(1M)
 * +=======================+ 0xFFE0_0000
 * | Failsafe RootFS       |	(1M)
 * +=======================+ 0xFFF0_0000
 * |                       |
 * | U N U S E D           |
 * |                       |
 * +-----------------------+ 0xFFFD_0000	U-Boot image header (64 bytes)
 * | environment settings  |	(64k)
 * +-----------------------+ 0xFFFE_0000	U-Boot image header (64 bytes)
 * | U-Boot                | 0xFFFE_0040    _start of U-Boot
 * |                       | 0xFFFE_FFFC    reset vector - branch to _start
 * +++++++++++++++++++++++++ 0xFFFF_FFFF     End of Flash -----------------
 *****************************************************************************/
