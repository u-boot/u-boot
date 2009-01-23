/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006-2007
 * Copyright (C) Sheldon Instruments, Inc. 2008
 *
 * Author: Ron Madrid <info@sheldoninst.com>
 *
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS for A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <mpc83xx.h>
#include <spd_sdram.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

static long fixed_sdram(void);

#if defined(CONFIG_NAND_SPL)
void si_wait_i2c(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;

	while (!(__raw_readb(&im->i2c[0].sr) & 0x02))
		;

	__raw_writeb(0x00, &im->i2c[0].sr);

	sync();

	return;
}

void si_read_i2c(u32 lbyte, int count, u8 *buffer)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 i;
	u8 chip = 0x50 << 1; /* boot sequencer I2C */
	u32 ubyte = (lbyte & 0xff00) >> 8;

	lbyte &= 0xff;

	/*
	 * Set up controller
	 */
	__raw_writeb(0x3f, &im->i2c[0].fdr);
	__raw_writeb(0x00, &im->i2c[0].adr);
	__raw_writeb(0x00, &im->i2c[0].sr);
	__raw_writeb(0x00, &im->i2c[0].dr);

	while (__raw_readb(&im->i2c[0].sr) & 0x20)
		;

	/*
	 * Writing address to device
	 */
	__raw_writeb(0xb0, &im->i2c[0].cr);
	sync();
	__raw_writeb(chip, &im->i2c[0].dr);
	si_wait_i2c();

	__raw_writeb(0xb0, &im->i2c[0].cr);
	sync();
	__raw_writeb(ubyte, &im->i2c[0].dr);
	si_wait_i2c();

	__raw_writeb(lbyte, &im->i2c[0].dr);
	si_wait_i2c();

	__raw_writeb(0xb4, &im->i2c[0].cr);
	sync();
	__raw_writeb(chip + 1, &im->i2c[0].dr);
	si_wait_i2c();

	__raw_writeb(0xa0, &im->i2c[0].cr);
	sync();

	/*
	 * Dummy read
	 */
	__raw_readb(&im->i2c[0].dr);

	si_wait_i2c();

	/*
	 * Read actual data
	 */
	for (i = 0; i < count; i++)
	{
		if (i == (count - 2))	/* Reached next to last byte, No ACK */
			__raw_writeb(0xa8, &im->i2c[0].cr);
		if (i == (count - 1))	/* Reached last byte, STOP */
			__raw_writeb(0x88, &im->i2c[0].cr);

		/* Read byte of data */
		buffer[i] = __raw_readb(&im->i2c[0].dr);

		if (i == (count - 1))
			break;
		si_wait_i2c();
	}

	return;
}
#endif /* CONFIG_NAND_SPL */

phys_size_t initdram(int board_type)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile fsl_lbus_t *lbc= &im->lbus;
	u32 msize;

	if ((__raw_readl(&im->sysconf.immrbar) & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	__raw_writel(CONFIG_SYS_DDR_BASE & LAWBAR_BAR, &im->sysconf.ddrlaw[0].bar);

	msize = fixed_sdram();

	/* Local Bus setup lbcr and mrtpr */
	__raw_writel(CONFIG_SYS_LBC_LBCR, &lbc->lbcr);
	__raw_writel(CONFIG_SYS_LBC_MRTPR, &lbc->mrtpr);
	sync();

	/* return total bus SDRAM size(bytes)  -- DDR */
	return (msize * 1024 * 1024);
}

/*************************************************************************
 *  fixed sdram init -- reads values from boot sequencer I2C
 ************************************************************************/
static long fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 msizelog2, msize = 1;
#if defined(CONFIG_NAND_SPL)
	u32 i;
	const u8 bytecount = 135;
	u8 buffer[bytecount];
	u32 addr, data;

	si_read_i2c(0, bytecount, buffer);

	for (i = 18; i < bytecount; i += 7){
		addr = (u32)buffer[i];
		addr <<= 8;
		addr |= (u32)buffer[i + 1];
		addr <<= 2;
		data = (u32)buffer[i + 2];
		data <<= 8;
		data |= (u32)buffer[i + 3];
		data <<= 8;
		data |= (u32)buffer[i + 4];
		data <<= 8;
		data |= (u32)buffer[i + 5];

		__raw_writel(data, (u32 *)(CONFIG_SYS_IMMR + addr));
	}

	sync();

	/* enable DDR controller */
	__raw_writel((__raw_readl(&im->ddr.sdram_cfg) | SDRAM_CFG_MEM_EN), &im->ddr.sdram_cfg);
#endif /* (CONFIG_NAND_SPL) */

	msizelog2 = ((__raw_readl(&im->sysconf.ddrlaw[0].ar) & LAWAR_SIZE) + 1);
	msize <<= (msizelog2 - 20);

	return msize;
}
