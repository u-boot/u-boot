/*
 * (c) 2009 Magnus Lilja <lilja.magnus@gmail.com>
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

#ifndef __FSL_NFC_H
#define __FSL_NFC_H

/*
 * TODO: Use same register defs for nand_spl mxc nand driver
 * and mtd mxc nand driver.
 *
 * Register map and bit definitions for the Freescale NAND Flash
 * Controller present in various i.MX devices.
 *
 * MX31 and MX27 have version 1 which has
 * 	4 512 byte main buffers and
 * 	4 16 byte spare buffers
 * 	to support up to 2K byte pagesize nand.
 * 	Reading or writing a 2K page requires 4 FDI/FDO cycles.
 *
 * MX25 has version 1.1 which has
 * 	8 512 byte main buffers and
 * 	8 64 byte spare buffers
 * 	to support up to 4K byte pagesize nand.
 * 	Reading or writing a 2K or 4K page requires only 1 FDI/FDO cycle.
 *      Also some of registers are moved and/or changed meaning as seen below.
 */
#if defined(CONFIG_MX31) || defined(CONFIG_MX27)
#define MXC_NFC_V1
#elif defined(CONFIG_MX25)
#define MXC_NFC_V1_1
#else
#warning "MXC NFC version not defined"
#endif

#if defined(MXC_NFC_V1)
#define NAND_MXC_NR_BUFS		4
#define NAND_MXC_SPARE_BUF_SIZE		16
#define NAND_MXC_REG_OFFSET		0xe00
#define NAND_MXC_2K_MULTI_CYCLE		1
#elif defined(MXC_NFC_V1_1)
#define NAND_MXC_NR_BUFS		8
#define NAND_MXC_SPARE_BUF_SIZE		64
#define NAND_MXC_REG_OFFSET		0x1e00
#else
#error "define CONFIG_NAND_MXC_VXXX to use the mxc spl_nand driver"
#endif

struct fsl_nfc_regs {
	u32 main_area[NAND_MXC_NR_BUFS][512/4];
	u32 spare_area[NAND_MXC_NR_BUFS][NAND_MXC_SPARE_BUF_SIZE/4];
	/*
	 * reserved size is offset of nfc registers
	 * minus total main and spare sizes
	 */
	u8 reserved1[NAND_MXC_REG_OFFSET
		- NAND_MXC_NR_BUFS * (512 + NAND_MXC_SPARE_BUF_SIZE)];
#if defined(MXC_NFC_V1)
	u16 bufsiz;
	u16 reserved2;
	u16 buffer_address;
	u16 flash_add;
	u16 flash_cmd;
	u16 configuration;
	u16 ecc_status_result;
	u16 ecc_rslt_main_area;
	u16 ecc_rslt_spare_area;
	u16 nf_wr_prot;
	u16 unlock_start_blk_add;
	u16 unlock_end_blk_add;
	u16 nand_flash_wr_pr_st;
	u16 nand_flash_config1;
	u16 nand_flash_config2;
#elif defined(MXC_NFC_V1_1)
	u16 reserved2[2];
	u16 buffer_address;
	u16 flash_add;
	u16 flash_cmd;
	u16 configuration;
	u16 ecc_status_result;
	u16 ecc_status_result2;
	u16 spare_area_size;
	u16 nf_wr_prot;
	u16 reserved3[2];
	u16 nand_flash_wr_pr_st;
	u16 nand_flash_config1;
	u16 nand_flash_config2;
	u16 reserved4;
	u16 unlock_start_blk_add0;
	u16 unlock_end_blk_add0;
	u16 unlock_start_blk_add1;
	u16 unlock_end_blk_add1;
	u16 unlock_start_blk_add2;
	u16 unlock_end_blk_add2;
	u16 unlock_start_blk_add3;
	u16 unlock_end_blk_add3;
#endif
};

/*
 * Set INT to 0, FCMD to 1, rest to 0 in NFC_CONFIG2 Register for Command
 * operation
 */
#define NFC_CMD		0x1

/*
 * Set INT to 0, FADD to 1, rest to 0 in NFC_CONFIG2 Register for Address
 * operation
 */
#define NFC_ADDR	0x2

/*
 * Set INT to 0, FDI to 1, rest to 0 in NFC_CONFIG2 Register for Input
 * operation
 */
#define NFC_INPUT	0x4

/*
 * Set INT to 0, FDO to 001, rest to 0 in NFC_CONFIG2 Register for Data
 * Output operation
 */
#define NFC_OUTPUT	0x8

/*
 * Set INT to 0, FD0 to 010, rest to 0 in NFC_CONFIG2 Register for Read ID
 * operation
 */
#define NFC_ID		0x10

/*
 * Set INT to 0, FDO to 100, rest to 0 in NFC_CONFIG2 Register for Read
 * Status operation
 */
#define NFC_STATUS	0x20

/*
 * Set INT to 1, rest to 0 in NFC_CONFIG2 Register for Read Status
 * operation
 */
#define NFC_INT		0x8000

#ifdef MXC_NFC_V1_1
#define NFC_4_8N_ECC	(1 << 0)
#endif
#define NFC_SP_EN	(1 << 2)
#define NFC_ECC_EN	(1 << 3)
#define NFC_INT_MSK	(1 << 4)
#define NFC_BIG		(1 << 5)
#define NFC_RST		(1 << 6)
#define NFC_CE		(1 << 7)
#define NFC_ONE_CYCLE	(1 << 8)

#endif /* __FSL_NFC_H */
