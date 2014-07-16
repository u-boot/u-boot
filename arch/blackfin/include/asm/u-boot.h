/*
 * U-boot - u-boot.h Structure declarations for board specific data
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _U_BOOT_H_
#define _U_BOOT_H_	1

typedef struct bd_info {
	unsigned long bi_boot_params;	/* where this board expects params */
	unsigned long bi_memstart;	/* start of DRAM memory */
	phys_size_t bi_memsize;		/* size  of DRAM memory in bytes */
	unsigned long bi_flashstart;	/* start of FLASH memory */
	unsigned long bi_flashsize;	/* size  of FLASH memory */
	unsigned long bi_flashoffset;	/* reserved area for startup monitor */
	const char *bi_r_version;
	const char *bi_cpu;
	const char *bi_board_name;
	unsigned long bi_vco;
	unsigned long bi_cclk;
	unsigned long bi_sclk;
} bd_t;

/* For image.h:image_check_target_arch() */
#define IH_ARCH_DEFAULT IH_ARCH_BLACKFIN

#endif	/* _U_BOOT_H_ */
