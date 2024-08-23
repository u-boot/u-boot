// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <cpu_func.h>
#include <debug_uart.h>
#include <dm.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <spl.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/timer.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

int board_return_to_bootrom(struct spl_image_info *spl_image,
			    struct spl_boot_device *bootdev)
{
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);

	return 0;
}

__weak const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
};

const char *board_spl_was_booted_from(void)
{
	static u32 brom_bootsource_id_cache = BROM_BOOTSOURCE_UNKNOWN;
	u32 bootdevice_brom_id;
	const char *bootdevice_ofpath = NULL;

	if (brom_bootsource_id_cache != BROM_BOOTSOURCE_UNKNOWN)
		bootdevice_brom_id = brom_bootsource_id_cache;
	else
		bootdevice_brom_id = readl(BROM_BOOTSOURCE_ID_ADDR);

	if (bootdevice_brom_id < ARRAY_SIZE(boot_devices))
		bootdevice_ofpath = boot_devices[bootdevice_brom_id];

	if (bootdevice_ofpath) {
		brom_bootsource_id_cache = bootdevice_brom_id;
		debug("%s: brom_bootdevice_id %x maps to '%s'\n",
		      __func__, bootdevice_brom_id, bootdevice_ofpath);
	} else {
		debug("%s: failed to resolve brom_bootdevice_id %x\n",
		      __func__, bootdevice_brom_id);
	}

	return bootdevice_ofpath;
}

u32 spl_boot_device(void)
{
	u32 boot_device = BOOT_DEVICE_MMC1;

#if defined(CONFIG_TARGET_CHROMEBOOK_JERRY) || \
		defined(CONFIG_TARGET_CHROMEBIT_MICKEY) || \
		defined(CONFIG_TARGET_CHROMEBOOK_MINNIE) || \
		defined(CONFIG_TARGET_CHROMEBOOK_SPEEDY) || \
		defined(CONFIG_TARGET_CHROMEBOOK_BOB) || \
		defined(CONFIG_TARGET_CHROMEBOOK_KEVIN)
	return BOOT_DEVICE_SPI;
#endif
	if (CONFIG_IS_ENABLED(ROCKCHIP_BACK_TO_BROM))
		return BOOT_DEVICE_BOOTROM;

	return boot_device;
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	return MMCSD_MODE_RAW;
}

__weak int board_early_init_f(void)
{
	return 0;
}

__weak int arch_cpu_init(void)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	board_early_init_f();

	ret = spl_early_init();
	if (ret) {
		printf("spl_early_init() failed: %d\n", ret);
		hang();
	}
	arch_cpu_init();

	rockchip_stimer_init();

#ifdef CONFIG_SYS_ARCH_TIMER
	/* Init ARM arch timer in arch/arm/cpu/armv7/arch_timer.c */
	timer_init();
#endif
#if !defined(CONFIG_TPL) || defined(CONFIG_SPL_RAM)
	debug("\nspl:init dram\n");
	ret = dram_init();
	if (ret) {
		printf("DRAM init failed: %d\n", ret);
		return;
	}
	gd->ram_top = gd->ram_base + get_effective_memsize();
	gd->ram_top = board_get_usable_ram_top(gd->ram_size);

	if (IS_ENABLED(CONFIG_ARM64) && !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)) {
		gd->relocaddr = gd->ram_top;
		arch_reserve_mmu();
		enable_caches();
	}
#endif
	preloader_console_init();
}

void spl_board_prepare_for_boot(void)
{
	if (!IS_ENABLED(CONFIG_ARM64) || CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		return;

	cleanup_before_linux();
}
