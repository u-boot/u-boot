// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <ram.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>

#define TIMER_LOAD_COUNT_L	0x00
#define TIMER_LOAD_COUNT_H	0x04
#define TIMER_CONTROL_REG	0x10
#define TIMER_EN	0x1
#define	TIMER_FMODE	BIT(0)
#define	TIMER_RMODE	BIT(1)

__weak void rockchip_stimer_init(void)
{
	writel(0, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_CONTROL_REG);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 4);
	writel(TIMER_EN | TIMER_FMODE, CONFIG_ROCKCHIP_STIMER_BASE +
	       TIMER_CONTROL_REG);
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

#ifdef CONFIG_DEBUG_UART
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	debug_uart_init();
	printascii("U-Boot TPL board init\n");
#endif

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	/* Init secure timer */
	rockchip_stimer_init();
	/* Init ARM arch timer in arch/arm/cpu/ */
	timer_init();

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
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
