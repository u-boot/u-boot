/*
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/clk.h>
#include <asm/arch/sama5_matrix.h>
#include <asm/arch/sama5_sfr.h>
#include <asm/arch/sama5d4.h>

char *get_cpu_name()
{
	unsigned int extension_id = get_extension_chip_id();

	if (cpu_is_sama5d4())
		switch (extension_id) {
		case ARCH_EXID_SAMA5D41:
			return "SAMA5D41";
		case ARCH_EXID_SAMA5D42:
			return "SAMA5D42";
		case ARCH_EXID_SAMA5D43:
			return "SAMA5D43";
		case ARCH_EXID_SAMA5D44:
			return "SAMA5D44";
		default:
			return "Unknown CPU type";
		}
	else
		return "Unknown CPU type";
}

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
void at91_udp_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable UPLL clock */
	writel(AT91_PMC_UPLLEN | AT91_PMC_BIASEN, &pmc->uckr);
	/* Enable UDPHS clock */
	at91_periph_clk_enable(ATMEL_ID_UDPHS);
}
#endif

#ifdef CONFIG_SPL_BUILD
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

void redirect_int_from_saic_to_aic(void)
{
	struct atmel_sfr *sfr = (struct atmel_sfr *)ATMEL_BASE_SFR;
	u32 key32;

	if (!(readl(&sfr->aicredir) & ATMEL_SFR_AICREDIR_NSAIC)) {
		key32 = readl(&sfr->sn1) ^ ATMEL_SFR_AICREDIR_KEY;
		writel((key32 | ATMEL_SFR_AICREDIR_NSAIC), &sfr->aicredir);
	}
}
#endif
