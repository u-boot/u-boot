/*
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *     Tom Rini <trini@ti.com>
 *
 * Initial Code from:
 *     Richard Woodruff <r-woodruff2@ti.com>
 *     Jian Zhang <jzhang@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/mtd/nand.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>

static struct gpmc *gpmc_config = (struct gpmc *)GPMC_BASE;

/*
 * Many boards will want to know the results of the NAND_CMD_READID command
 * in order to decide what to do about DDR initialization.  This function
 * allows us to do that very early and to pass those results back to the
 * board so it can make whatever decisions need to be made.
 */
int identify_nand_chip(int *mfr, int *id)
{
	int loops = 1000;

	/* Make sure that we have setup GPMC for NAND correctly. */
	writel(M_NAND_GPMC_CONFIG1, &gpmc_config->cs[0].config1);
	writel(M_NAND_GPMC_CONFIG2, &gpmc_config->cs[0].config2);
	writel(M_NAND_GPMC_CONFIG3, &gpmc_config->cs[0].config3);
	writel(M_NAND_GPMC_CONFIG4, &gpmc_config->cs[0].config4);
	writel(M_NAND_GPMC_CONFIG5, &gpmc_config->cs[0].config5);
	writel(M_NAND_GPMC_CONFIG6, &gpmc_config->cs[0].config6);

	/*
	 * Enable the config.  The CS size goes in bits 11:8.  We set
	 * bit 6 to enable the CS and the base address goes into bits 5:0.
	 */
	writel((GPMC_SIZE_128M << 8) | (GPMC_CS_ENABLE << 6) |
				((NAND_BASE >> 24) & GPMC_BASEADDR_MASK),
			&gpmc_config->cs[0].config7);

	sdelay(2000);

	/* Issue a RESET and then READID */
	writeb(NAND_CMD_RESET, &gpmc_config->cs[0].nand_cmd);
	writeb(NAND_CMD_STATUS, &gpmc_config->cs[0].nand_cmd);
	while ((readl(&gpmc_config->cs[0].nand_dat) & NAND_STATUS_READY)
	                                           != NAND_STATUS_READY) {
		sdelay(100);
		if (--loops == 0)
			return 1;
	}
	writeb(NAND_CMD_READID, &gpmc_config->cs[0].nand_cmd);

	/* Set the address to read to 0x0 */
	writeb(0x0, &gpmc_config->cs[0].nand_adr);

	/* Read off the manufacturer and device id. */
	*mfr = readb(&gpmc_config->cs[0].nand_dat);
	*id = readb(&gpmc_config->cs[0].nand_dat);

	return 0;
}
