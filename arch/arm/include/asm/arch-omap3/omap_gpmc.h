/*
 * (C) Copyright 2004-2008 Texas Instruments, <www.ti.com>
 * Rohit Choraria <rohitkc@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
