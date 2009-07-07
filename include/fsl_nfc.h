/*
 *
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
 * Register map and bit definitions for the Freescale NAND Flash
 * Controller present in i.MX31 and other devices.
 */

struct fsl_nfc_regs {
	u32 main_area0[128]; /* @0x000 */
	u32 main_area1[128];
	u32 main_area2[128];
	u32 main_area3[128];
	u32 spare_area0[4];
	u32 spare_area1[4];
	u32 spare_area2[4];
	u32 spare_area3[4];
	u32 reserved1[64 - 16 + 64 * 5];
	u16 bufsiz; /* @ 0xe00 */
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

#define NFC_SP_EN	(1 << 2)
#define NFC_ECC_EN	(1 << 3)
#define NFC_INT_MSK	(1 << 4)
#define NFC_BIG		(1 << 5)
#define NFC_RST		(1 << 6)
#define NFC_CE		(1 << 7)
#define NFC_ONE_CYCLE	(1 << 8)

#endif /* __FSL_NFC_H */
