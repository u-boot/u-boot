/*
 * Copyright (C) 2004-2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_U_BOOT_H__
#define __ASM_U_BOOT_H__ 1

typedef struct bd_info {
	unsigned int		bi_baudrate;
	unsigned char		bi_phy_id[4];
	unsigned long		bi_board_number;
	void			*bi_boot_params;
	struct {
		unsigned long	start;
		unsigned long	size;
	}			bi_dram[CONFIG_NR_DRAM_BANKS];
	unsigned long		bi_flashstart;
	unsigned long		bi_flashsize;
	unsigned long		bi_flashoffset;
} bd_t;

#define bi_memstart bi_dram[0].start
#define bi_memsize bi_dram[0].size

/* For image.h:image_check_target_arch() */
#define IH_ARCH_DEFAULT IH_ARCH_AVR32

#endif /* __ASM_U_BOOT_H__ */
