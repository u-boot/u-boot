/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright (C) 2018 Rockchip Electronics Co., Ltd
 */

#ifndef _ASM_ARCH_SDRAM_COMMON_H
#define _ASM_ARCH_SDRAM_COMMON_H

#ifndef MHZ
#define MHZ		(1000 * 1000)
#endif

#define PATTERN		(0x5aa5f00f)

#define MIN(a, b)	(((a) > (b)) ? (b) : (a))
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))

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

#define DDR_SYS_REG_VERSION		(0x2)
/*
 * sys_reg2 bitfield struct
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
#define SYS_REG_ENC_ROW_3_4(n, ch)	((n) << (30 + (ch)))
#define SYS_REG_DEC_ROW_3_4(n, ch)	(((n) >> (30 + (ch))) & 0x1)
#define SYS_REG_ENC_CHINFO(ch)		(1 << (28 + (ch)))
#define SYS_REG_ENC_DDRTYPE(n)		((n) << 13)
#define SYS_REG_DEC_DDRTYPE(n)		(((n) >> 13) & 0x7)
#define SYS_REG_ENC_NUM_CH(n)		(((n) - 1) << 12)
#define SYS_REG_DEC_NUM_CH(n)		(1 + (((n) >> 12) & 0x1))
#define SYS_REG_ENC_RANK(n, ch)		(((n) - 1) << (11 + ((ch) * 16)))
#define SYS_REG_DEC_RANK(n, ch)		(1 + (((n) >> (11 + 16 * (ch))) & 0x1))
#define SYS_REG_ENC_COL(n, ch)		(((n) - 9) << (9 + ((ch) * 16)))
#define SYS_REG_DEC_COL(n, ch)		(9 + (((n) >> (9 + 16 * (ch))) & 0x3))
#define SYS_REG_ENC_BK(n, ch)		(((n) == 3 ? 0 : 1) << \
						(8 + ((ch) * 16)))
#define SYS_REG_DEC_BK(n, ch)		(3 - (((n) >> (8 + 16 * (ch))) & 0x1))
#define SYS_REG_ENC_BW(n, ch)		((2 >> (n)) << (2 + ((ch) * 16)))
#define SYS_REG_DEC_BW(n, ch)		(2 >> (((n) >> (2 + 16 * (ch))) & 0x3))
#define SYS_REG_ENC_DBW(n, ch)		((2 >> (n)) << (0 + ((ch) * 16)))
#define SYS_REG_DEC_DBW(n, ch)		(2 >> (((n) >> (0 + 16 * (ch))) & 0x3))
/* sys reg 3 */
#define SYS_REG_ENC_VERSION(n)		((n) << 28)
#define SYS_REG_DEC_VERSION(n)		(((n) >> 28) & 0xf)
#define SYS_REG_ENC_CS0_ROW(n, os_reg2, os_reg3, ch) do { \
			(os_reg2) |= (((n) - 13) & 0x3) << (6 + 16 * (ch)); \
			(os_reg3) |= ((((n) - 13) & 0x4) >> 2) << \
				     (5 + 2 * (ch)); \
		} while (0)

#define SYS_REG_DEC_CS0_ROW(os_reg2, os_reg3, ch)	\
		((((((os_reg2) >> (6 + 16 * (ch)) & 0x3) | \
		 ((((os_reg3) >> (5 + 2 * (ch))) & 0x1) << 2)) + 1) & 0x7) + 12)

#define SYS_REG_ENC_CS1_ROW(n, os_reg2, os_reg3, ch) do { \
			(os_reg2) &= (~(0x3 << (4 + 16 * (ch)))); \
			(os_reg3) &= (~(0x1 << (4 + 2 * (ch)))); \
			(os_reg2) |= (((n) - 13) & 0x3) << (4 + 16 * (ch)); \
			(os_reg3) |= ((((n) - 13) & 0x4) >> 2) << \
				     (4 + 2 * (ch)); \
		} while (0)

#define SYS_REG_DEC_CS1_ROW(os_reg2, os_reg3, ch) \
		((((((os_reg2) >> (4 + 16 * (ch)) & 0x3) | \
		 ((((os_reg3) >> (4 + 2 * (ch))) & 0x1) << 2)) + 1) & 0x7) + 12)

#define SYS_REG_ENC_CS1_COL(n, ch)	(((n) - 9) << (0 + 2 * (ch)))
#define SYS_REG_DEC_CS1_COL(n, ch)	(9 + (((n) >> (0 + 2 * (ch))) & 0x3))

void sdram_print_dram_type(unsigned char dramtype);
void sdram_print_ddr_info(struct sdram_cap_info *cap_info,
			  struct sdram_base_params *base);
void sdram_print_stride(unsigned int stride);

void sdram_org_config(struct sdram_cap_info *cap_info,
		      struct sdram_base_params *base,
		      u32 *p_os_reg2, u32 *p_os_reg3, u32 channel);

int sdram_detect_bw(struct sdram_cap_info *cap_info);
int sdram_detect_cs(struct sdram_cap_info *cap_info);
int sdram_detect_col(struct sdram_cap_info *cap_info,
		     u32 coltmp);
int sdram_detect_bank(struct sdram_cap_info *cap_info,
		      u32 coltmp, u32 bktmp);
int sdram_detect_bg(struct sdram_cap_info *cap_info,
		    u32 coltmp);
int sdram_detect_dbw(struct sdram_cap_info *cap_info, u32 dram_type);
int sdram_detect_row(struct sdram_cap_info *cap_info,
		     u32 coltmp, u32 bktmp, u32 rowtmp);
int sdram_detect_row_3_4(struct sdram_cap_info *cap_info,
			 u32 coltmp, u32 bktmp);
int sdram_detect_high_row(struct sdram_cap_info *cap_info);
int sdram_detect_cs1_row(struct sdram_cap_info *cap_info, u32 dram_type);
u64 sdram_get_cs_cap(struct sdram_cap_info *cap_info, u32 cs, u32 dram_type);
void sdram_copy_to_reg(u32 *dest, const u32 *src, u32 n);

#endif
