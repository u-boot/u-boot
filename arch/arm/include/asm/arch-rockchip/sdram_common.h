/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 */

#ifndef _ASM_ARCH_SDRAM_COMMON_H
#define _ASM_ARCH_SDRAM_COMMON_H

enum {
	DDR4 = 0,
	DDR3 = 0x3,
	LPDDR2 = 0x5,
	LPDDR3 = 0x6,
	LPDDR4 = 0x7,
	UNUSED = 0xFF
};

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
	unsigned int ddrconfig;
};

struct sdram_base_params {
	unsigned int ddr_freq;
	unsigned int dramtype;
	unsigned int num_channels;
	unsigned int stride;
	unsigned int odt;
};

/*
 * sys_reg bitfield struct
 * [31]		row_3_4_ch1
 * [30]		row_3_4_ch0
 * [29:28]	chinfo
 * [27]		rank_ch1
 * [26:25]	col_ch1
 * [24]		bk_ch1
 * [23:22]	cs0_row_ch1
 * [21:20]	cs1_row_ch1
 * [19:18]	bw_ch1
 * [17:16]	dbw_ch1;
 * [15:13]	ddrtype
 * [12]		channelnum
 * [11]		rank_ch0
 * [10:9]	col_ch0
 * [8]		bk_ch0
 * [7:6]	cs0_row_ch0
 * [5:4]	cs1_row_ch0
 * [3:2]	bw_ch0
 * [1:0]	dbw_ch0
*/
#define SYS_REG_DDRTYPE_SHIFT		13
#define SYS_REG_DDRTYPE_MASK		7
#define SYS_REG_NUM_CH_SHIFT		12
#define SYS_REG_NUM_CH_MASK		1
#define SYS_REG_ROW_3_4_SHIFT(ch)	(30 + (ch))
#define SYS_REG_ROW_3_4_MASK		1
#define SYS_REG_CHINFO_SHIFT(ch)	(28 + (ch))
#define SYS_REG_RANK_SHIFT(ch)		(11 + (ch) * 16)
#define SYS_REG_RANK_MASK		1
#define SYS_REG_COL_SHIFT(ch)		(9 + (ch) * 16)
#define SYS_REG_COL_MASK		3
#define SYS_REG_BK_SHIFT(ch)		(8 + (ch) * 16)
#define SYS_REG_BK_MASK			1
#define SYS_REG_CS0_ROW_SHIFT(ch)	(6 + (ch) * 16)
#define SYS_REG_CS0_ROW_MASK		3
#define SYS_REG_CS1_ROW_SHIFT(ch)	(4 + (ch) * 16)
#define SYS_REG_CS1_ROW_MASK		3
#define SYS_REG_BW_SHIFT(ch)		(2 + (ch) * 16)
#define SYS_REG_BW_MASK			3
#define SYS_REG_DBW_SHIFT(ch)		((ch) * 16)
#define SYS_REG_DBW_MASK		3

/* Get sdram size decode from reg */
size_t rockchip_sdram_size(phys_addr_t reg);

/* Called by U-Boot board_init_r for Rockchip SoCs */
int dram_init(void);

#if !defined(CONFIG_RAM_ROCKCHIP_DEBUG)
inline void sdram_print_dram_type(unsigned char dramtype)
{
}

inline void sdram_print_ddr_info(struct sdram_cap_info *cap_info,
				 struct sdram_base_params *base)
{
}
#else
void sdram_print_dram_type(unsigned char dramtype);
void sdram_print_ddr_info(struct sdram_cap_info *cap_info,
			  struct sdram_base_params *base);
#endif /* CONFIG_RAM_ROCKCHIP_DEBUG */

#endif
