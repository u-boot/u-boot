/*
 * Copyright (C) 2012 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2012 Xilinx, Inc. All rights reserved.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

/* Control regsiter bitfield definitions */
#define ZYNQ_DDRC_CTRLREG_BUSWIDTH_MASK		0xC
#define ZYNQ_DDRC_CTRLREG_BUSWIDTH_SHIFT	2
#define ZYNQ_DDRC_CTRLREG_BUSWIDTH_16BIT	1

/* ECC scrub regsiter definitions */
#define ZYNQ_DDRC_ECC_SCRUBREG_ECC_MODE_MASK	0x7
#define ZYNQ_DDRC_ECC_SCRUBREG_ECCMODE_SECDED	0x4

void zynq_ddrc_init(void)
{
	u32 width, ecctype;

	width = readl(&ddrc_base->ddrc_ctrl);
	width = (width & ZYNQ_DDRC_CTRLREG_BUSWIDTH_MASK) >>
					ZYNQ_DDRC_CTRLREG_BUSWIDTH_SHIFT;
	ecctype = (readl(&ddrc_base->ecc_scrub) &
		ZYNQ_DDRC_ECC_SCRUBREG_ECC_MODE_MASK);

	/* ECC is enabled when memory is in 16bit mode and it is enabled */
	if ((ecctype == ZYNQ_DDRC_ECC_SCRUBREG_ECCMODE_SECDED) &&
	    (width == ZYNQ_DDRC_CTRLREG_BUSWIDTH_16BIT)) {
		puts("Memory: ECC enabled\n");
		/*
		 * Clear the first 1MB because it is not initialized from
		 * first stage bootloader. To get ECC to work all memory has
		 * been initialized by writing any value.
		 */
		memset(0, 0, 1 * 1024 * 1024);

		gd->ram_size /= 2;
	} else {
		puts("Memory: ECC disabled\n");
	}
}
