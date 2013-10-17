/*
 * NAND boot for Freescale Enhanced Local Bus Controller, Flash Control Machine
 *
 * (C) Copyright 2006-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Copyright (c) 2008 Freescale Semiconductor, Inc.
 * Author: Scott Wood <scottwood@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/fsl_lbc.h>
#include <linux/mtd/nand.h>

#define WINDOW_SIZE 8192

static void nand_wait(void)
{
	fsl_lbc_t *regs = LBC_BASE_ADDR;

	for (;;) {
		uint32_t status = in_be32(&regs->ltesr);

		if (status == 1)
			return;

		if (status & 1) {
			puts("read failed (ltesr)\n");
			for (;;);
		}
	}
}

static void nand_load(unsigned int offs, int uboot_size, uchar *dst)
{
	fsl_lbc_t *regs = LBC_BASE_ADDR;
	uchar *buf = (uchar *)CONFIG_SYS_NAND_BASE;
	const int large = CONFIG_SYS_NAND_OR_PRELIM & OR_FCM_PGS;
	const int block_shift = large ? 17 : 14;
	const int block_size = 1 << block_shift;
	const int page_size = large ? 2048 : 512;
	const int bad_marker = large ? page_size + 0 : page_size + 5;
	int fmr = (15 << FMR_CWTO_SHIFT) | (2 << FMR_AL_SHIFT) | 2;
	int pos = 0;

	if (offs & (block_size - 1)) {
		puts("bad offset\n");
		for (;;);
	}

	if (large) {
		fmr |= FMR_ECCM;
		__raw_writel((NAND_CMD_READ0 << FCR_CMD0_SHIFT) |
			(NAND_CMD_READSTART << FCR_CMD1_SHIFT),
			&regs->fcr);
		__raw_writel(
			(FIR_OP_CW0 << FIR_OP0_SHIFT) |
			(FIR_OP_CA  << FIR_OP1_SHIFT) |
			(FIR_OP_PA  << FIR_OP2_SHIFT) |
			(FIR_OP_CW1 << FIR_OP3_SHIFT) |
			(FIR_OP_RBW << FIR_OP4_SHIFT),
			&regs->fir);
	} else {
		__raw_writel(NAND_CMD_READ0 << FCR_CMD0_SHIFT, &regs->fcr);
		__raw_writel(
			(FIR_OP_CW0 << FIR_OP0_SHIFT) |
			(FIR_OP_CA  << FIR_OP1_SHIFT) |
			(FIR_OP_PA  << FIR_OP2_SHIFT) |
			(FIR_OP_RBW << FIR_OP3_SHIFT),
			&regs->fir);
	}

	__raw_writel(0, &regs->fbcr);

	while (pos < uboot_size) {
		int i = 0;
		__raw_writel(offs >> block_shift, &regs->fbar);

		do {
			int j;
			unsigned int page_offs = (offs & (block_size - 1)) << 1;

			__raw_writel(~0, &regs->ltesr);
			__raw_writel(0, &regs->lteatr);
			__raw_writel(page_offs, &regs->fpar);
			__raw_writel(fmr, &regs->fmr);
			sync();
			__raw_writel(0, &regs->lsor);
			nand_wait();

			page_offs %= WINDOW_SIZE;

			/*
			 * If either of the first two pages are marked bad,
			 * continue to the next block.
			 */
			if (i++ < 2 && buf[page_offs + bad_marker] != 0xff) {
				puts("skipping\n");
				offs = (offs + block_size) & ~(block_size - 1);
				pos &= ~(block_size - 1);
				break;
			}

			for (j = 0; j < page_size; j++)
				dst[pos + j] = buf[page_offs + j];

			pos += page_size;
			offs += page_size;
		} while ((offs & (block_size - 1)) && (pos < uboot_size));
	}
}

/*
 * The main entry for NAND booting. It's necessary that SDRAM is already
 * configured and available since this code loads the main U-Boot image
 * from NAND into SDRAM and starts it from there.
 */
void nand_boot(void)
{
	__attribute__((noreturn)) void (*uboot)(void);

	/*
	 * Load U-Boot image from NAND into RAM
	 */
	nand_load(CONFIG_SYS_NAND_U_BOOT_OFFS, CONFIG_SYS_NAND_U_BOOT_SIZE,
		  (uchar *)CONFIG_SYS_NAND_U_BOOT_DST);

	/*
	 * Jump to U-Boot image
	 */
	puts("transfering control\n");
	/*
	 * Clean d-cache and invalidate i-cache, to
	 * make sure that no stale data is executed.
	 */
	flush_cache(CONFIG_SYS_NAND_U_BOOT_DST, CONFIG_SYS_NAND_U_BOOT_SIZE);
	uboot = (void *)CONFIG_SYS_NAND_U_BOOT_START;
	uboot();
}
