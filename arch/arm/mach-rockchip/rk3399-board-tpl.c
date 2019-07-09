// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <ram.h>
#include <spl.h>
#include <version.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>

__weak void rockchip_stimer_init(void)
{
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	printascii("\nU-Boot TPL "  PLAIN_VERSION " (" U_BOOT_DATE " - "
	     U_BOOT_TIME " " U_BOOT_TZ ")\n");
#endif
	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	rockchip_stimer_init();

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		pr_err("DRAM init failed: %d\n", ret);
		return;
	}
}

void board_return_to_bootrom(void)
{
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_BOOTROM;
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
