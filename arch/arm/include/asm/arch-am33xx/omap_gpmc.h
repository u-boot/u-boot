/*
 * (C) Copyright 2004-2008 Texas Instruments, <www.ti.com>
 * Rohit Choraria <rohitkc@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_ARCH_OMAP_GPMC_H
#define __ASM_ARCH_OMAP_GPMC_H

/* These GPMC_NAND_HW_BCHx_ECC_LAYOUT defines are based on AM33xx ELM */
#define GPMC_NAND_HW_BCH4_ECC_LAYOUT {\
	.eccbytes = 32,\
	.eccpos = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,\
				16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,\
				28, 29, 30, 31, 32, 33},\
	.oobfree = {\
		{.offset = 34,\
		 .length = 30 } } \
}

#define GPMC_NAND_HW_BCH8_ECC_LAYOUT {\
	.eccbytes = 56,\
	.eccpos = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,\
				16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,\
				28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,\
				40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,\
				52, 53, 54, 55, 56, 57},\
	.oobfree = {\
		{.offset = 58,\
		 .length = 6 } } \
}

#define GPMC_NAND_HW_BCH16_ECC_LAYOUT {\
	.eccbytes = 104,\
	.eccpos = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,\
				16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,\
				28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,\
				40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,\
				52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,\
				64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,\
				76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,\
				88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,\
				100, 101, 102, 103, 104, 105},\
	.oobfree = {\
		{.offset = 106,\
		 .length = 8 } } \
}
#endif /* __ASM_ARCH_OMAP_GPMC_H */
