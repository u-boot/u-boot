/*
 * (C) Copyright 2004-2008 Texas Instruments, <www.ti.com>
 * Rohit Choraria <rohitkc@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_ARCH_OMAP_GPMC_H
#define __ASM_ARCH_OMAP_GPMC_H

/*
 * These GPMC_NAND_HW_BCHx_ECC_LAYOUT defines using the BCH library.
 * The OOB layout was first defined by linx kernel in commit
 * 0e618ef0a6a33cf7ef96c2c824402088dd8ef48c, we have to reuse it here cause
 * we want to be compatible.
 */
#define GPMC_NAND_HW_BCH8_ECC_LAYOUT {\
	.eccbytes = 56,\
	.eccpos = {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,\
			23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,\
			37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,\
			51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63},\
	.oobfree = {\
		{.offset = 2,\
		 .length = 10 } } \
}

/* GPMC CS configuration for an SMSC LAN9221 ethernet controller */
#define NET_LAN9221_GPMC_CONFIG1    0x00001000
#define NET_LAN9221_GPMC_CONFIG2    0x00060700
#define NET_LAN9221_GPMC_CONFIG3    0x00020201
#define NET_LAN9221_GPMC_CONFIG4    0x06000700
#define NET_LAN9221_GPMC_CONFIG5    0x0006090A
#define NET_LAN9221_GPMC_CONFIG6    0x87030000
#define NET_LAN9221_GPMC_CONFIG7    0x00000f6c

#endif /* __ASM_ARCH_OMAP_GPMC_H */
