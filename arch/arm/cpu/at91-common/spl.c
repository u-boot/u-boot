/*
 * Copyright (C) 2013 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_wdt.h>
#include <asm/arch/clk.h>
#include <spl.h>

#if defined(CONFIG_AT91SAM9_WATCHDOG)
void at91_disable_wdt(void) { }
#else
void at91_disable_wdt(void)
{
	struct at91_wdt *wdt = (struct at91_wdt *)ATMEL_BASE_WDT;

	writel(AT91_WDT_MR_WDDIS, &wdt->mr);
}
#endif

u32 spl_boot_device(void)
{
#ifdef CONFIG_SYS_USE_MMC
	return BOOT_DEVICE_MMC1;
#elif CONFIG_SYS_USE_NANDFLASH
	return BOOT_DEVICE_NAND;
#elif CONFIG_SYS_USE_SERIALFLASH
	return BOOT_DEVICE_SPI;
#endif
	return BOOT_DEVICE_NONE;
}

u32 spl_boot_mode(void)
{
	switch (spl_boot_device()) {
#ifdef CONFIG_SYS_USE_MMC
	case BOOT_DEVICE_MMC1:
		return MMCSD_MODE_FS;
		break;
#endif
	case BOOT_DEVICE_NONE:
	default:
		hang();
	}
}
