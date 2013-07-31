/*
 * (C) Copyright 2007
 * Markus Klotzbuecher, DENX Software Engineering <mk@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_SYS_USB_OHCI_CPU_INIT)

#ifdef CONFIG_4xx_DCACHE
#include <asm/mmu.h>
DECLARE_GLOBAL_DATA_PTR;
#endif

int usb_cpu_init(void)
{
#ifdef CONFIG_4xx_DCACHE
	/* disable cache */
	change_tlb(gd->bd->bi_memstart, gd->bd->bi_memsize, TLB_WORD2_I_ENABLE);
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
