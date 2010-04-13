/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
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

#ifndef MMC_H
#define MMC_H

#include "mmc_host_def.h"

/* Responses */
#define RSP_TYPE_NONE	(RSP_TYPE_NORSP   | CCCE_NOCHECK | CICE_NOCHECK)
#define RSP_TYPE_R1	(RSP_TYPE_LGHT48  | CCCE_CHECK   | CICE_CHECK)
#define RSP_TYPE_R1B	(RSP_TYPE_LGHT48B | CCCE_CHECK   | CICE_CHECK)
#define RSP_TYPE_R2	(RSP_TYPE_LGHT136 | CCCE_CHECK   | CICE_NOCHECK)
#define RSP_TYPE_R3	(RSP_TYPE_LGHT48  | CCCE_NOCHECK | CICE_NOCHECK)
#define RSP_TYPE_R4	(RSP_TYPE_LGHT48  | CCCE_NOCHECK | CICE_NOCHECK)
#define RSP_TYPE_R5	(RSP_TYPE_LGHT48  | CCCE_CHECK   | CICE_CHECK)
#define RSP_TYPE_R6	(RSP_TYPE_LGHT48  | CCCE_CHECK   | CICE_CHECK)
#define RSP_TYPE_R7	(RSP_TYPE_LGHT48  | CCCE_CHECK   | CICE_CHECK)

/* All supported commands */
#define MMC_CMD0	(INDEX(0)  | RSP_TYPE_NONE | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD1	(INDEX(1)  | RSP_TYPE_R3   | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD2	(INDEX(2)  | RSP_TYPE_R2   | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD3	(INDEX(3)  | RSP_TYPE_R1   | DP_NO_DATA | DDIR_WRITE)
#define MMC_SDCMD3	(INDEX(3)  | RSP_TYPE_R6   | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD4	(INDEX(4)  | RSP_TYPE_NONE | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD6	(INDEX(6)  | RSP_TYPE_R1B  | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD7_SELECT	(INDEX(7)  | RSP_TYPE_R1B  | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD7_DESELECT (INDEX(7)| RSP_TYPE_NONE | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD8	(INDEX(8)  | RSP_TYPE_R1   | DP_DATA    | DDIR_READ)
#define MMC_SDCMD8	(INDEX(8)  | RSP_TYPE_R7   | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD9	(INDEX(9)  | RSP_TYPE_R2   | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD12	(INDEX(12) | RSP_TYPE_R1B  | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD13	(INDEX(13) | RSP_TYPE_R1   | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD15	(INDEX(15) | RSP_TYPE_NONE | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD16	(INDEX(16) | RSP_TYPE_R1   | DP_NO_DATA | DDIR_WRITE)
#define MMC_CMD17	(INDEX(17) | RSP_TYPE_R1   | DP_DATA    | DDIR_READ)
#define MMC_CMD24	(INDEX(24) | RSP_TYPE_R1   | DP_DATA    | DDIR_WRITE)
#define MMC_ACMD6	(INDEX(6)  | RSP_TYPE_R1   | DP_NO_DATA | DDIR_WRITE)
#define MMC_ACMD41	(INDEX(41) | RSP_TYPE_R3   | DP_NO_DATA | DDIR_WRITE)
#define MMC_ACMD51	(INDEX(51) | RSP_TYPE_R1   | DP_DATA    | DDIR_READ)
#define MMC_CMD55	(INDEX(55) | RSP_TYPE_R1   | DP_NO_DATA | DDIR_WRITE)

#define MMC_AC_CMD_RCA_MASK	(unsigned int)(0xFFFF << 16)
#define MMC_BC_CMD_DSR_MASK	(unsigned int)(0xFFFF << 16)
#define MMC_DSR_DEFAULT		0x0404
#define SD_CMD8_CHECK_PATTERN	0xAA
#define SD_CMD8_2_7_3_6_V_RANGE	(0x01 << 8)

/* Clock Configurations and Macros */

#define MMC_CLOCK_REFERENCE		96
#define MMC_RELATIVE_CARD_ADDRESS	0x1234
#define MMC_INIT_SEQ_CLK		(MMC_CLOCK_REFERENCE * 1000 / 80)
#define MMC_400kHz_CLK			(MMC_CLOCK_REFERENCE * 1000 / 400)
#define CLKDR(r, f, u)			((((r)*100) / ((f)*(u))) + 1)
#define CLKD(f, u)			(CLKDR(MMC_CLOCK_REFERENCE, f, u))

#define MMC_OCR_REG_ACCESS_MODE_MASK	(0x3 << 29)
#define MMC_OCR_REG_ACCESS_MODE_BYTE 	(0x0 << 29)
#define MMC_OCR_REG_ACCESS_MODE_SECTOR	(0x2 << 29)

#define MMC_OCR_REG_HOST_CAPACITY_SUPPORT_MASK		(0x1 << 30)
#define MMC_OCR_REG_HOST_CAPACITY_SUPPORT_BYTE		(0x0 << 30)
#define MMC_OCR_REG_HOST_CAPACITY_SUPPORT_SECTOR	(0x1 << 30)

#define MMC_SD2_CSD_C_SIZE_LSB_MASK	0xFFFF
#define MMC_SD2_CSD_C_SIZE_MSB_MASK	0x003F
#define MMC_SD2_CSD_C_SIZE_MSB_OFFSET	16
#define MMC_CSD_C_SIZE_LSB_MASK		0x0003
#define MMC_CSD_C_SIZE_MSB_MASK		0x03FF
#define MMC_CSD_C_SIZE_MSB_OFFSET	2

#define MMC_CSD_TRAN_SPEED_UNIT_MASK	(0x07 << 0)
#define MMC_CSD_TRAN_SPEED_FACTOR_MASK	(0x0F << 3)
#define MMC_CSD_TRAN_SPEED_UNIT_100MHZ	(0x3 << 0)
#define MMC_CSD_TRAN_SPEED_FACTOR_1_0	(0x01 << 3)
#define MMC_CSD_TRAN_SPEED_FACTOR_8_0	(0x0F << 3)

typedef struct {
	unsigned not_used:1;
	unsigned crc:7;
	unsigned ecc:2;
	unsigned file_format:2;
	unsigned tmp_write_protect:1;
	unsigned perm_write_protect:1;
	unsigned copy:1;
	unsigned file_format_grp:1;
	unsigned content_prot_app:1;
	unsigned reserved_1:4;
	unsigned write_bl_partial:1;
	unsigned write_bl_len:4;
	unsigned r2w_factor:3;
	unsigned default_ecc:2;
	unsigned wp_grp_enable:1;
	unsigned wp_grp_size:5;
	unsigned erase_grp_mult:5;
	unsigned erase_grp_size:5;
	unsigned c_size_mult:3;
	unsigned vdd_w_curr_max:3;
	unsigned vdd_w_curr_min:3;
	unsigned vdd_r_curr_max:3;
	unsigned vdd_r_curr_min:3;
	unsigned c_size_lsb:2;
	unsigned c_size_msb:10;
	unsigned reserved_2:2;
	unsigned dsr_imp:1;
	unsigned read_blk_misalign:1;
	unsigned write_blk_misalign:1;
	unsigned read_bl_partial:1;
	unsigned read_bl_len:4;
	unsigned ccc:12;
	unsigned tran_speed:8;
	unsigned nsac:8;
	unsigned taac:8;
	unsigned reserved_3:2;
	unsigned spec_vers:4;
	unsigned csd_structure:2;
} mmc_csd_reg_t;

/* csd for sd2.0 */
typedef struct {
	unsigned not_used:1;
	unsigned crc:7;
	unsigned reserved_1:2;
	unsigned file_format:2;
	unsigned tmp_write_protect:1;
	unsigned perm_write_protect:1;
	unsigned copy:1;
	unsigned file_format_grp:1;
	unsigned reserved_2:5;
	unsigned write_bl_partial:1;
	unsigned write_bl_len:4;
	unsigned r2w_factor:3;
	unsigned reserved_3:2;
	unsigned wp_grp_enable:1;
	unsigned wp_grp_size:7;
	unsigned sector_size:7;
	unsigned erase_blk_len:1;
	unsigned reserved_4:1;
	unsigned c_size_lsb:16;
	unsigned c_size_msb:6;
	unsigned reserved_5:6;
	unsigned dsr_imp:1;
	unsigned read_blk_misalign:1;
	unsigned write_blk_misalign:1;
	unsigned read_bl_partial:1;
	unsigned read_bl_len:4;
	unsigned ccc:12;
	unsigned tran_speed:8;
	unsigned nsac:8;
	unsigned taac:8;
	unsigned reserved_6:6;
	unsigned csd_structure:2;
} mmc_sd2_csd_reg_t;

/* extended csd - 512 bytes long */
typedef struct {
	unsigned char reserved_1[181];
	unsigned char erasedmemorycontent;
	unsigned char reserved_2;
	unsigned char buswidthmode;
	unsigned char reserved_3;
	unsigned char highspeedinterfacetiming;
	unsigned char reserved_4;
	unsigned char powerclass;
	unsigned char reserved_5;
	unsigned char commandsetrevision;
	unsigned char reserved_6;
	unsigned char commandset;
	unsigned char extendedcsdrevision;
	unsigned char reserved_7;
	unsigned char csdstructureversion;
	unsigned char reserved_8;
	unsigned char cardtype;
	unsigned char reserved_9[3];
	unsigned char powerclass_52mhz_1_95v;
	unsigned char powerclass_26mhz_1_95v;
	unsigned char powerclass_52mhz_3_6v;
	unsigned char powerclass_26mhz_3_6v;
	unsigned char reserved_10;
	unsigned char minreadperf_4b_26mhz;
	unsigned char minwriteperf_4b_26mhz;
	unsigned char minreadperf_8b_26mhz_4b_52mhz;
	unsigned char minwriteperf_8b_26mhz_4b_52mhz;
	unsigned char minreadperf_8b_52mhz;
	unsigned char minwriteperf_8b_52mhz;
	unsigned char reserved_11;
	unsigned int sectorcount;
	unsigned char reserved_12[288];
	unsigned char supportedcommandsets;
	unsigned char reserved_13[7];
} mmc_extended_csd_reg_t;

/* mmc sd responce */
typedef struct {
	unsigned int ocr;
} mmc_resp_r3;

typedef struct {
	unsigned short cardstatus;
	unsigned short newpublishedrca;
} mmc_resp_r6;

typedef union {
	unsigned int resp[4];
	mmc_resp_r3 r3;
	mmc_resp_r6 r6;
	mmc_csd_reg_t Card_CSD;
} mmc_resp_t;

extern mmc_card_data mmc_dev;

unsigned char mmc_lowlevel_init(void);
unsigned char mmc_send_command(unsigned int cmd, unsigned int arg,
			       unsigned int *response);
unsigned char mmc_setup_clock(unsigned int iclk, unsigned short clkd);
unsigned char mmc_set_opendrain(unsigned char state);
unsigned char mmc_read_data(unsigned int *output_buf);

#endif /* MMC_H */
