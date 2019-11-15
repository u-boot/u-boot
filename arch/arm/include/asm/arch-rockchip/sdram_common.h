/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright (C) 2018 Rockchip Electronics Co., Ltd
 */

#ifndef _ASM_ARCH_SDRAM_COMMON_H
#define _ASM_ARCH_SDRAM_COMMON_H

struct sdram_cap_info {
	unsigned int rank;
	/* dram column number, 0 means this channel is invalid */
	unsigned int col;
	/* dram bank number, 3:8bank, 2:4bank */
	unsigned int bk;
	/* channel buswidth, 2:32bit, 1:16bit, 0:8bit */
	unsigned int bw;
	/* die buswidth, 2:32bit, 1:16bit, 0:8bit */
	unsigned int dbw;
	/*
	 * row_3_4 = 1: 6Gb or 12Gb die
	 * row_3_4 = 0: normal die, power of 2
	 */
	unsigned int row_3_4;
	unsigned int cs0_row;
	unsigned int cs1_row;
	unsigned int cs0_high16bit_row;
	unsigned int cs1_high16bit_row;
	unsigned int ddrconfig;
};

struct sdram_base_params {
	unsigned int ddr_freq;
	unsigned int dramtype;
	unsigned int num_channels;
	unsigned int stride;
	unsigned int odt;
};

#if !defined(CONFIG_RAM_ROCKCHIP_DEBUG)
inline void sdram_print_dram_type(unsigned char dramtype)
{
}

inline void sdram_print_ddr_info(struct sdram_cap_info *cap_info,
				 struct sdram_base_params *base)
{
}

inline void sdram_print_stride(unsigned int stride)
{
}
#else
void sdram_print_dram_type(unsigned char dramtype);
void sdram_print_ddr_info(struct sdram_cap_info *cap_info,
			  struct sdram_base_params *base);
void sdram_print_stride(unsigned int stride);
#endif /* CONFIG_RAM_ROCKCHIP_DEBUG */

#endif
