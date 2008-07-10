/*
 * (C) Copyright 2008 Wolfgang Grandegger <wg@denx.de>
 *
 * (C) Copyright 2006
 * Thomas Waehner, TQ-System GmbH, thomas.waehner@tqs.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/fsl_upm.h>
#include <ioports.h>

#include <nand.h>

DECLARE_GLOBAL_DATA_PTR;

extern uint get_lbc_clock (void);

/* index of UPM RAM array run pattern for NAND command cycle */
#define	CFG_NAN_UPM_WRITE_CMD_OFS	0x08

/* index of UPM RAM array run pattern for NAND address cycle */
#define	CFG_NAND_UPM_WRITE_ADDR_OFS	0x10

/* Structure for table with supported UPM timings */
struct upm_freq {
	ulong freq;
	const u32 *upm_patt;
	uchar gpl4_disable;
	uchar ehtr;
	uchar ead;
};

/* NAND-FLASH UPM tables for TQM85XX according to TQM8548.pq.timing.101.doc */

/* UPM pattern for bus clock = 25 MHz */
static const u32 upm_patt_25[] = {
	/* Offset */ /* UPM Read Single RAM array entry -> NAND Read Data */
	/* 0x00 */ 0x0ff32000, 0x0fa32000, 0x3fb32005, 0xfffffc00,
	/* 0x04 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write CMD */
	/* 0x08 */ 0x00ff2c30, 0x00ff2c30, 0x0fff2c35, 0xfffffc00,
	/* 0x0C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write ADDR */
	/* 0x10 */ 0x00f3ec30, 0x00f3ec30, 0x0ff3ec35, 0xfffffc00,
	/* 0x14 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Single RAM array entry -> NAND Write Data */
	/* 0x18 */ 0x00f32c00, 0x00f32c00, 0x0ff32c05, 0xfffffc00,
	/* 0x1C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Burst RAM array entry -> unused */
	/* 0x20 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x24 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x28 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x2C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Refresh Timer RAM array entry -> unused */
	/* 0x30 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x34 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x38 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Exception RAM array entry -> unsused */
	/* 0x3C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for bus clock = 33.3 MHz */
static const u32 upm_patt_33[] = {
	/* Offset */ /* UPM Read Single RAM array entry -> NAND Read Data */
	/* 0x00 */ 0x0ff32000, 0x0fa32100, 0x3fb32005, 0xfffffc00,
	/* 0x04 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write CMD */
	/* 0x08 */ 0x00ff2c30, 0x00ff2c30, 0x0fff2c35, 0xfffffc00,
	/* 0x0C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write ADDR */
	/* 0x10 */ 0x00f3ec30, 0x00f3ec30, 0x0ff3ec35, 0xfffffc00,
	/* 0x14 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Single RAM array entry -> NAND Write Data */
	/* 0x18 */ 0x00f32c00, 0x00f32c00, 0x0ff32c05, 0xfffffc00,
	/* 0x1C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Burst RAM array entry -> unused */
	/* 0x20 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x24 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x28 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x2C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Refresh Timer RAM array entry -> unused */
	/* 0x30 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x34 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x38 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Exception RAM array entry -> unsused */
	/* 0x3C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for bus clock = 41.7 MHz */
static const u32 upm_patt_42[] = {
	/* Offset */ /* UPM Read Single RAM array entry -> NAND Read Data */
	/* 0x00 */ 0x0ff32000, 0x0fa32100, 0x3fb32005, 0xfffffc00,
	/* 0x04 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write CMD */
	/* 0x08 */ 0x00ff2c30, 0x00ff2c30, 0x0fff2c35, 0xfffffc00,
	/* 0x0C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write ADDR */
	/* 0x10 */ 0x00f3ec30, 0x00f3ec30, 0x0ff3ec35, 0xfffffc00,
	/* 0x14 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Single RAM array entry -> NAND Write Data */
	/* 0x18 */ 0x00f32c00, 0x00f32c00, 0x0ff32c05, 0xfffffc00,
	/* 0x1C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Burst RAM array entry -> unused */
	/* 0x20 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x24 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x28 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x2C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Refresh Timer RAM array entry -> unused */
	/* 0x30 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x34 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x38 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Exception RAM array entry -> unsused */
	/* 0x3C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for bus clock = 50 MHz */
static const u32 upm_patt_50[] = {
	/* Offset */ /* UPM Read Single RAM array entry -> NAND Read Data */
	/* 0x00 */ 0x0ff33000, 0x0fa33100, 0x0fa33005, 0xfffffc00,
	/* 0x04 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write CMD */
	/* 0x08 */ 0x00ff3d30, 0x00ff3c30, 0x0fff3c35, 0xfffffc00,
	/* 0x0C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write ADDR */
	/* 0x10 */ 0x00f3fd30, 0x00f3fc30, 0x0ff3fc35, 0xfffffc00,
	/* 0x14 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Single RAM array entry -> NAND Write Data */
	/* 0x18 */ 0x00f33d00, 0x00f33c00, 0x0ff33c05, 0xfffffc00,
	/* 0x1C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Burst RAM array entry -> unused */
	/* 0x20 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x24 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x28 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x2C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Refresh Timer RAM array entry -> unused */
	/* 0x30 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x34 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x38 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Exception RAM array entry -> unsused */
	/* 0x3C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for bus clock = 66.7 MHz */
static const u32 upm_patt_67[] = {
	/* Offset */ /* UPM Read Single RAM array entry -> NAND Read Data */
	/* 0x00 */ 0x0ff33000, 0x0fe33000, 0x0fa33100, 0x0fa33000,
	/* 0x04 */ 0x0fa33005, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write CMD */
	/* 0x08 */ 0x00ff3d30, 0x00ff3c30, 0x0fff3c30, 0x0fff3c35,
	/* 0x0C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write ADDR */
	/* 0x10 */ 0x00f3fd30, 0x00f3fc30, 0x0ff3fc30, 0x0ff3fc35,
	/* 0x14 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Single RAM array entry -> NAND Write Data */
	/* 0x18 */ 0x00f33d00, 0x00f33c00, 0x0ff33c00, 0x0ff33c05,
	/* 0x1C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Burst RAM array entry -> unused */
	/* 0x20 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x24 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x28 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x2C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Refresh Timer RAM array entry -> unused */
	/* 0x30 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x34 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x38 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Exception RAM array entry -> unsused */
	/* 0x3C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for bus clock = 83.3 MHz */
static const u32 upm_patt_83[] = {
	/* Offset */ /* UPM Read Single RAM array entry -> NAND Read Data */
	/* 0x00 */ 0x0ff33000, 0x0fe33000, 0x0fa33100, 0x0fa33000,
	/* 0x04 */ 0x0fa33005, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write CMD */
	/* 0x08 */ 0x00ff3e30, 0x00ff3c30, 0x0fff3c30, 0x0fff3c35,
	/* 0x0C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write ADDR */
	/* 0x10 */ 0x00f3fe30, 0x00f3fc30, 0x0ff3fc30, 0x0ff3fc35,
	/* 0x14 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Single RAM array entry -> NAND Write Data */
	/* 0x18 */ 0x00f33e00, 0x00f33c00, 0x0ff33c00, 0x0ff33c05,
	/* 0x1C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Burst RAM array entry -> unused */
	/* 0x20 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x24 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x28 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x2C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Refresh Timer RAM array entry -> unused */
	/* 0x30 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x34 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x38 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Exception RAM array entry -> unsused */
	/* 0x3C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for bus clock = 100 MHz */
static const u32 upm_patt_100[] = {
	/* Offset */ /* UPM Read Single RAM array entry -> NAND Read Data */
	/* 0x00 */ 0x0ff33100, 0x0fe33000, 0x0fa33200, 0x0fa33000,
	/* 0x04 */ 0x0fa33005, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write CMD */
	/* 0x08 */ 0x00ff3f30, 0x00ff3c30, 0x0fff3c30, 0x0fff3c35,
	/* 0x0C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write ADDR */
	/* 0x10 */ 0x00f3ff30, 0x00f3fc30, 0x0ff3fc30, 0x0ff3fc35,
	/* 0x14 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Single RAM array entry -> NAND Write Data */
	/* 0x18 */ 0x00f33f00, 0x00f33c00, 0x0ff33c00, 0x0ff33c05,
	/* 0x1C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Burst RAM array entry -> unused */
	/* 0x20 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x24 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x28 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x2C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Refresh Timer RAM array entry -> unused */
	/* 0x30 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x34 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x38 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Exception RAM array entry -> unsused */
	/* 0x3C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for bus clock = 133.3 MHz */
static const u32 upm_patt_133[] = {
	/* Offset */ /* UPM Read Single RAM array entry -> NAND Read Data */
	/* 0x00 */ 0x0ff33100, 0x0fe33000, 0x0fa33300, 0x0fa33000,
	/* 0x04 */ 0x0fa33000, 0x0fa33005, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write CMD */
	/* 0x08 */ 0x00ff3f30, 0x00ff3d30, 0x0fff3d30, 0x0fff3c35,
	/* 0x0C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write ADDR */
	/* 0x10 */ 0x00f3ff30, 0x00f3fd30, 0x0ff3fd30, 0x0ff3fc35,
	/* 0x14 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Single RAM array entry -> NAND Write Data */
	/* 0x18 */ 0x00f33f00, 0x00f33d00, 0x0ff33d00, 0x0ff33c05,
	/* 0x1C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Burst RAM array entry -> unused */
	/* 0x20 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x24 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x28 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x2C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Refresh Timer RAM array entry -> unused */
	/* 0x30 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x34 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x38 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Exception RAM array entry -> unsused */
	/* 0x3C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* UPM pattern for bus clock = 166.7 MHz */
static const u32 upm_patt_167[] = {
	/* Offset */ /* UPM Read Single RAM array entry -> NAND Read Data */
	/* 0x00 */ 0x0ff33200, 0x0fe33000, 0x0fa33300, 0x0fa33300,
	/* 0x04 */ 0x0fa33005, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write CMD */
	/* 0x08 */ 0x00ff3f30, 0x00ff3f30, 0x0fff3e30, 0xffff3c35,
	/* 0x0C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Read Burst RAM array entry -> NAND Write ADDR */
	/* 0x10 */ 0x00f3ff30, 0x00f3ff30, 0x0ff3fe30, 0x0ff3fc35,
	/* 0x14 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Single RAM array entry -> NAND Write Data */
	/* 0x18 */ 0x00f33f00, 0x00f33f00, 0x0ff33e00, 0x0ff33c05,
	/* 0x1C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,

	/* UPM Write Burst RAM array entry -> unused */
	/* 0x20 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x24 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x28 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x2C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Refresh Timer RAM array entry -> unused */
	/* 0x30 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x34 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc00,
	/* 0x38 */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,

	/* UPM Exception RAM array entry -> unsused */
	/* 0x3C */ 0xfffffc00, 0xfffffc00, 0xfffffc00, 0xfffffc01,
};

/* Supported UPM timings */
struct upm_freq upm_freq_table[] = {
	/* nominal freq. | ptr to table | GPL4 dis. | EHTR  | EAD */
	{25000000, upm_patt_25, 1, 0, 0},
	{33333333, upm_patt_33, 1, 0, 0},
	{41666666, upm_patt_42, 1, 0, 0},
	{50000000, upm_patt_50, 0, 0, 0},
	{66666666, upm_patt_67, 0, 0, 0},
	{83333333, upm_patt_83, 0, 0, 0},
	{100000000, upm_patt_100, 0, 1, 1},
	{133333333, upm_patt_133, 0, 1, 1},
	{166666666, upm_patt_167, 0, 1, 1},
};

#define UPM_FREQS (sizeof(upm_freq_table) / sizeof(struct upm_freq))

volatile const u32 *nand_upm_patt;

/*
 * write into UPMB ram
 */
static void upmb_write (u_char addr, ulong val)
{
	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);

	out_be32 (&lbc->mdr, val);

	clrsetbits_be32(&lbc->mbmr, MxMR_MAD_MSK,
			MxMR_OP_WARR | (addr & MxMR_MAD_MSK));

	/* dummy access to perform write */
	out_8 ((void __iomem *)CFG_NAND0_BASE, 0);

	clrbits_be32(&lbc->mbmr, MxMR_OP_WARR);
}

/*
 * Initialize UPM for NAND flash access.
 */
static void nand_upm_setup (volatile ccsr_lbc_t *lbc)
{
	uint i;
	uint or3 = CFG_OR3_PRELIM;
	uint clock = get_lbc_clock ();

	out_be32 (&lbc->br3, 0);	/* disable bank and reset all bits */
	out_be32 (&lbc->br3, CFG_BR3_PRELIM);

	/*
	 * Search appropriate UPM table for bus clock.
	 * If the bus clock exceeds a tolerated value, take the UPM timing for
	 * the next higher supported frequency to ensure that access works
	 * (even the access may be slower then).
	 */
	for (i = 0; (i < UPM_FREQS) && (clock > upm_freq_table[i].freq); i++)
		;

	if (i >= UPM_FREQS)
	/* no valid entry found */
		/* take last entry with configuration for max. bus clock */
		i--;

	if (upm_freq_table[i].ehtr) {
		/* EHTR must be set due to TQM8548 timing specification */
		or3 |= OR_UPM_EHTR;
	}
	if (upm_freq_table[i].ead)
		/* EAD must be set due to TQM8548 timing specification */
		or3 |= OR_UPM_EAD;

	out_be32 (&lbc->or3, or3);

	/* Assign address of table */
	nand_upm_patt = upm_freq_table[i].upm_patt;

	for (i = 0; i < 64; i++) {
		upmb_write (i, *nand_upm_patt);
		nand_upm_patt++;
	}

	/* Put UPM back to normal operation mode */
	if (upm_freq_table[i].gpl4_disable)
		/* GPL4 must be disabled according to timing specification */
		out_be32 (&lbc->mbmr, MxMR_OP_NORM | MxMR_GPL_x4DIS);

	return;
}

static struct fsl_upm_nand fun = {
	.width = 8,
	.upm_cmd_offset = 0x08,
	.upm_addr_offset = 0x10,
	.chip_delay = NAND_BIG_DELAY_US,
};

void board_nand_select_device (struct nand_chip *nand, int chip)
{
}

int board_nand_init (struct nand_chip *nand)
{
	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);

	if (!nand_upm_patt)
		nand_upm_setup (lbc);

	fun.upm.io_addr = nand->IO_ADDR_R;
	fun.upm.mxmr = (void __iomem *)&lbc->mbmr;
	fun.upm.mdr = (void __iomem *)&lbc->mdr;
	fun.upm.mar = (void __iomem *)&lbc->mar;

	return fsl_upm_nand_init (nand, &fun);
}
