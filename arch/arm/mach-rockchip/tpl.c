// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <bootstage.h>
#include <debug_uart.h>
#include <dm.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <spl.h>
#include <version.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/timer.h>
#include <linux/bitops.h>

#if CONFIG_IS_ENABLED(BANNER_PRINT)
#include <timestamp.h>
#endif

__weak void tpl_board_init(void)
{
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

#if defined(CONFIG_DEBUG_UART) && defined(CONFIG_TPL_SERIAL)
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	debug_uart_init();
#ifdef CONFIG_TPL_BANNER_PRINT
	printascii("\nU-Boot TPL " PLAIN_VERSION " (" U_BOOT_DATE " - " \
				U_BOOT_TIME ")\n");
#endif
#endif
	/* Init secure timer */
	rockchip_stimer_init();

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	/* Init ARM arch timer */
	if (IS_ENABLED(CONFIG_SYS_ARCH_TIMER))
		timer_init();

	tpl_board_init();

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		printf("DRAM init failed: %d\n", ret);
		return;
	}
}

int board_return_to_bootrom(struct spl_image_info *spl_image,
			    struct spl_boot_device *bootdev)
{
	int ret;

	bootstage_mark_name(BOOTSTAGE_ID_END_TPL, "end tpl");
	ret = bootstage_stash_default();
	if (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	back_to_bootrom(BROM_BOOT_NEXTSTAGE);

	return 0;
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_BOOTROM;
}
