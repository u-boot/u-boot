/*
 * (C) Copyright 2006
 * DENX Software Engineering <mk@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT)

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/clk.h>

int usb_cpu_init(void)
{
	at91_pmc_t *pmc	= (at91_pmc_t *)ATMEL_BASE_PMC;

#ifdef CONFIG_USB_ATMEL_CLK_SEL_PLLB
	/* Enable PLLB */
	writel(get_pllb_init(), &pmc->pllbr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKB) != AT91_PMC_LOCKB)
		;
#ifdef CONFIG_AT91SAM9N12
	writel(AT91_PMC_USBS_USB_PLLB | AT91_PMC_USB_DIV_2, &pmc->usb);
#endif
#elif defined(CONFIG_USB_ATMEL_CLK_SEL_UPLL)
	/* Enable UPLL */
	writel(readl(&pmc->uckr) | AT91_PMC_UPLLEN | AT91_PMC_BIASEN,
		&pmc->uckr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKU) != AT91_PMC_LOCKU)
		;

	/* Select PLLA as input clock of OHCI */
	writel(AT91_PMC_USBS_USB_UPLL | AT91_PMC_USBDIV_10, &pmc->usb);
#endif

	/* Enable USB host clock. */
#ifdef CPU_HAS_PCR
	at91_periph_clk_enable(ATMEL_ID_UHP);
#else
	writel(1 << ATMEL_ID_UHP, &pmc->pcer);
#endif

#if defined(CONFIG_AT91SAM9261) || defined(CONFIG_AT91SAM9G10)
	writel(ATMEL_PMC_UHP | AT91_PMC_HCK0, &pmc->scer);
#else
	writel(ATMEL_PMC_UHP, &pmc->scer);
#endif

	return 0;
}

int usb_cpu_stop(void)
{
	at91_pmc_t *pmc	= (at91_pmc_t *)ATMEL_BASE_PMC;

	/* Disable USB host clock. */
#ifdef CPU_HAS_PCR
	at91_periph_clk_disable(ATMEL_ID_UHP);
#else
	writel(1 << ATMEL_ID_UHP, &pmc->pcdr);
#endif

#if defined(CONFIG_AT91SAM9261) || defined(CONFIG_AT91SAM9G10)
	writel(ATMEL_PMC_UHP | AT91_PMC_HCK0, &pmc->scdr);
#else
	writel(ATMEL_PMC_UHP, &pmc->scdr);
#endif

#ifdef CONFIG_USB_ATMEL_CLK_SEL_PLLB
#ifdef CONFIG_AT91SAM9N12
	writel(0, &pmc->usb);
#endif
	/* Disable PLLB */
	writel(0, &pmc->pllbr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKB) != 0)
		;
#elif defined(CONFIG_USB_ATMEL_CLK_SEL_UPLL)
	/* Disable UPLL */
	writel(readl(&pmc->uckr) & (~AT91_PMC_UPLLEN), &pmc->uckr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKU) == AT91_PMC_LOCKU)
		;
#endif

	return 0;
}

int usb_cpu_init_fail(void)
{
	return usb_cpu_stop();
}

#endif /* defined(CONFIG_USB_OHCI) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT) */
