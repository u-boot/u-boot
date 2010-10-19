/*
 * (C) Copyright 2006
 * Markus Klotzbuecher, DENX Software Engineering <mk@denx.de>
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

#include <common.h>

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT)
# if defined(CONFIG_CPU_MONAHANS) || defined(CONFIG_PXA27X)

#include <asm/arch/pxa-regs.h>
#include <asm/io.h>
#include <usb.h>

int usb_cpu_init(void)
{
#if defined(CONFIG_CPU_MONAHANS)
	/* Enable USB host clock. */
	writel(readl(CKENA) | CKENA_2_USBHOST | CKENA_20_UDC, CKENA);
	udelay(100);
#endif
#if defined(CONFIG_PXA27X)
	/* Enable USB host clock. */
	writel(readl(CKEN) | CKEN10_USBHOST, CKEN);
#endif

#if defined(CONFIG_CPU_MONAHANS)
	/* Configure Port 2 for Host (USB Client Registers) */
	writel(0x3000c, UP2OCR);
#endif

	writel(readl(UHCHR) | UHCHR_FHR, UHCHR);
	wait_ms(11);
	writel(readl(UHCHR) & ~UHCHR_FHR, UHCHR);

	writel(readl(UHCHR) | UHCHR_FSBIR, UHCHR);
	while (readl(UHCHR) & UHCHR_FSBIR)
		udelay(1);

#if defined(CONFIG_CPU_MONAHANS)
	writel(readl(UHCHR) & ~UHCHR_SSEP0, UHCHR);
#endif
#if defined(CONFIG_PXA27X)
	writel(readl(UHCHR) & ~UHCHR_SSEP2, UHCHR);
#endif
	writel(readl(UHCHR) & ~(UHCHR_SSEP1 | UHCHR_SSE), UHCHR);

	return 0;
}

int usb_cpu_stop(void)
{
	writel(readl(UHCHR) | UHCHR_FHR, UHCHR);
	udelay(11);
	writel(readl(UHCHR) & ~UHCHR_FHR, UHCHR);

	writel(readl(UHCCOMS) | UHCHR_FHR, UHCCOMS);
	udelay(10);

#if defined(CONFIG_CPU_MONAHANS)
	writel(readl(UHCHR) | UHCHR_SSEP0, UHCHR);
#endif
#if defined(CONFIG_PXA27X)
	writel(readl(UHCHR) | UHCHR_SSEP2, UHCHR);
#endif
	writel(readl(UHCHR) | UHCHR_SSEP1 | UHCHR_SSE, UHCHR);

#if defined(CONFIG_CPU_MONAHANS)
	/* Disable USB host clock. */
	writel(readl(CKENA) & ~(CKENA_2_USBHOST | CKENA_20_UDC), CKENA);
	udelay(100);
#endif
#if defined(CONFIG_PXA27X)
	/* Disable USB host clock. */
	writel(readl(CKEN) & ~CKEN10_USBHOST, CKEN);
#endif

	return 0;
}

int usb_cpu_init_fail(void)
{
	return usb_cpu_stop();
}

# endif /* defined(CONFIG_CPU_MONAHANS) || defined(CONFIG_PXA27X) */
#endif /* defined(CONFIG_USB_OHCI) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT) */
