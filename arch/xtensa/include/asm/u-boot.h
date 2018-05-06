/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007, Tensilica Inc.
 *
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef _XTENSA_U_BOOT_H
#define _XTENSA_U_BOOT_H

#ifdef CONFIG_SYS_GENERIC_BOARD
/* Use the generic board which requires a unified bd_info */
#include <asm-generic/u-boot.h>
#else

#ifndef __ASSEMBLY__
typedef struct bd_info {
	int		bi_baudrate;	/* serial console baudrate */
	unsigned long	bi_ip_addr;	/* IP Address */
	unsigned char	bi_enetaddr[6];	/* Ethernet adress */
	unsigned long	bi_boot_params;	/* where this board expects params */
	unsigned long	bi_memstart;	/* start of DRAM memory VA */
	unsigned long	bi_memsize;	/* size	 of DRAM memory in bytes */
	unsigned long	bi_flashstart;	/* start of FLASH memory */
	unsigned long	bi_flashsize;	/* size  of FLASH memory */
	unsigned long	bi_flashoffset;	/* offset to skip UBoot image */
} bd_t;
#endif	/* __ ASSEMBLY__ */

#endif	/* CONFIG_SYS_GENERIC_BOARD */

/* For image.h:image_check_target_arch() */
#define IH_ARCH_DEFAULT IH_ARCH_XTENSA

#endif	/* _XTENSA_U_BOOT_H */
