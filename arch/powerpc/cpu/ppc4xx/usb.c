/*
 * (C) Copyright 2007
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

#ifdef CONFIG_4xx_DCACHE
#include <asm/mmu.h>
DECLARE_GLOBAL_DATA_PTR;
#endif

#include "usbdev.h"

int usb_cpu_init(void)
{
#ifdef CONFIG_4xx_DCACHE
	/* disable cache */
	change_tlb(gd->bd->bi_memstart, gd->bd->bi_memsize, TLB_WORD2_I_ENABLE);
#endif

#if defined(CONFIG_440EP) || defined(CONFIG_440EPX)
	usb_dev_init();
#endif
	return 0;
}

int usb_cpu_stop(void)
{
#ifdef CONFIG_4xx_DCACHE
	/* enable cache */
	change_tlb(gd->bd->bi_memstart, gd->bd->bi_memsize, 0);
#endif
	return 0;
}

int usb_cpu_init_fail(void)
{
#ifdef CONFIG_4xx_DCACHE
	/* enable cache */
	change_tlb(gd->bd->bi_memstart, gd->bd->bi_memsize, 0);
#endif
	return 0;
}

#endif /* defined(CONFIG_USB_OHCI) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT) */
