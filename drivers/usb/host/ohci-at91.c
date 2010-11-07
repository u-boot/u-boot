/*
 * (C) Copyright 2006
 * DENX Software Engineering <mk@denx.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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

#if defined(CONFIG_AT91CAP9) || defined(CONFIG_AT91SAM9260) || \
    defined(CONFIG_AT91SAM9263) || defined(CONFIG_AT91SAM9G20) || \
    defined(CONFIG_AT91SAM9261)
	/* Enable PLLB */
	writel(get_pllb_init(), &pmc->pllbr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKB) != AT91_PMC_LOCKB)
		;
#elif defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
	/* Enable UPLL */
	writel(readl(&pmc->uckr) | AT91_PMC_UPLLEN | AT91_PMC_BIASEN,
		&pmc->uckr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKU) != AT91_PMC_LOCKU)
		;

	/* Select PLLA as input clock of OHCI */
	writel(AT91_PMC_USBS_USB_UPLL | AT91_PMC_USBDIV_10, &pmc->usb);
#endif

	/* Enable USB host clock. */
	writel(1 << ATMEL_ID_UHP, &pmc->pcer);
#ifdef CONFIG_AT91SAM9261
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
	writel(1 << ATMEL_ID_UHP, &pmc->pcdr);
#ifdef CONFIG_AT91SAM9261
	writel(ATMEL_PMC_UHP | AT91_PMC_HCK0, &pmc->scdr);
#else
	writel(ATMEL_PMC_UHP, &pmc->scdr);
#endif

#if defined(CONFIG_AT91CAP9) || defined(CONFIG_AT91SAM9260) || \
    defined(CONFIG_AT91SAM9263) || defined(CONFIG_AT91SAM9G20)
	/* Disable PLLB */
	writel(0, &pmc->pllbr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKB) != 0)
		;
#elif defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45)
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
