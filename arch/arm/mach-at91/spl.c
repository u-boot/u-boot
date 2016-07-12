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

#if defined(CONFIG_SAMA5D2) || defined(CONFIG_SAMA5D3) || \
    defined(CONFIG_SAMA5D4)
#include <asm/arch/sama5_boot.h>
struct {
	u32	r4;
} bootrom_stash __attribute__((section(".data")));

u32 spl_boot_device(void)
{
	u32 dev = (bootrom_stash.r4 >> ATMEL_SAMA5_BOOT_FROM_OFF) &
		  ATMEL_SAMA5_BOOT_FROM_MASK;
	u32 off = (bootrom_stash.r4 >> ATMEL_SAMA5_BOOT_DEV_ID_OFF) &
		  ATMEL_SAMA5_BOOT_DEV_ID_MASK;

#if defined(CONFIG_SYS_USE_MMC)
	if (dev == ATMEL_SAMA5_BOOT_FROM_MCI) {
		if (off == 0)
			return BOOT_DEVICE_MMC1;
		if (off == 1)
			return BOOT_DEVICE_MMC2;
		printf("ERROR: MMC controller %i not present!\n", dev);
		hang();
	}
#endif

#if defined(CONFIG_SYS_USE_SERIALFLASH) || defined(CONFIG_SYS_USE_SPIFLASH)
	if (dev == ATMEL_SAMA5_BOOT_FROM_SPI)
		return BOOT_DEVICE_SPI;
#endif

	if (dev == ATMEL_SAMA5_BOOT_FROM_SAMBA)
		return BOOT_DEVICE_USB;

	printf("ERROR: SMC/TWI/QSPI boot device not supported!\n"
	       "       Boot device %i, controller number %i\n", dev, off);

	return BOOT_DEVICE_NONE;
}
#else
u32 spl_boot_device(void)
{
#ifdef CONFIG_SYS_USE_MMC
	return BOOT_DEVICE_MMC1;
#elif CONFIG_SYS_USE_NANDFLASH
	return BOOT_DEVICE_NAND;
#elif CONFIG_SYS_USE_SERIALFLASH || CONFIG_SYS_USE_SPIFLASH
	return BOOT_DEVICE_SPI;
#endif
	return BOOT_DEVICE_NONE;
}
#endif

u32 spl_boot_mode(const u32 boot_device)
{
	switch (boot_device) {
#ifdef CONFIG_SYS_USE_MMC
	case BOOT_DEVICE_MMC1:
	case BOOT_DEVICE_MMC2:
		return MMCSD_MODE_FS;
		break;
#endif
	case BOOT_DEVICE_NONE:
	default:
		hang();
	}
}
