/*
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sama5_matrix.h>

void matrix_init(void)
{
	struct atmel_matrix *h64mx = (struct atmel_matrix *)ATMEL_BASE_MATRIX0;
	struct atmel_matrix *h32mx = (struct atmel_matrix *)ATMEL_BASE_MATRIX1;
	int i;

	/* Disable the write protect */
	writel(ATMEL_MATRIX_WPMR_WPKEY & ~ATMEL_MATRIX_WPMR_WPEN, &h64mx->wpmr);
	writel(ATMEL_MATRIX_WPMR_WPKEY & ~ATMEL_MATRIX_WPMR_WPEN, &h32mx->wpmr);

	/* DDR port 1 ~ poart 7, slave number is: 4 ~ 10 */
	for (i = 4; i <= 10; i++) {
		writel(0x000f0f0f, &h64mx->ssr[i]);
		writel(0x0000ffff, &h64mx->sassr[i]);
		writel(0x0000000f, &h64mx->srtsr[i]);
	}

	/* CS3 */
	writel(0x00c0c0c0, &h32mx->ssr[3]);
	writel(0xff000000, &h32mx->sassr[3]);
	writel(0xff000000, &h32mx->srtsr[3]);

	/* NFC SRAM */
	writel(0x00010101, &h32mx->ssr[4]);
	writel(0x00000001, &h32mx->sassr[4]);
	writel(0x00000001, &h32mx->srtsr[4]);

	/* Configure Programmable Security peripherals on matrix 64 */
	writel(readl(&h64mx->spselr[0]) | 0x00080000, &h64mx->spselr[0]);
	writel(readl(&h64mx->spselr[1]) | 0x00180000, &h64mx->spselr[1]);
	writel(readl(&h64mx->spselr[2]) | 0x00000008, &h64mx->spselr[2]);

	/* Configure Programmable Security peripherals on matrix 32 */
	writel(readl(&h32mx->spselr[0]) | 0xFFC00000, &h32mx->spselr[0]);
	writel(readl(&h32mx->spselr[1]) | 0x60E3FFFF, &h32mx->spselr[1]);

	/* Enable the write protect */
	writel(ATMEL_MATRIX_WPMR_WPKEY | ATMEL_MATRIX_WPMR_WPEN, &h64mx->wpmr);
	writel(ATMEL_MATRIX_WPMR_WPKEY | ATMEL_MATRIX_WPMR_WPEN, &h32mx->wpmr);
}
