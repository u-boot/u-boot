/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Some init for sunxi platform.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <miiphy.h>
#include <serial.h>
#ifdef CONFIG_SPL_BUILD
#include <spl.h>
#endif
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/timer.h>

#ifdef CONFIG_SPL_BUILD
/* Pointer to the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;

/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nand flash, mmc2.
 * Unfortunately we can't check how SPL was loaded so assume
 * it's always the first SD/MMC controller
 */
u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}

/* No confirmation data available in SPL yet. Hardcode bootmode */
u32 spl_boot_mode(void)
{
	return MMCSD_MODE_RAW;
}
#endif

int gpio_init(void)
{
	sunxi_gpio_set_cfgpin(SUNXI_GPB(22), SUN4I_GPB22_UART0_TX);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(23), SUN4I_GPB23_UART0_RX);
	sunxi_gpio_set_pull(SUNXI_GPB(23), 1);

	return 0;
}

void reset_cpu(ulong addr)
{
}

/* do some early init */
void s_init(void)
{
#if !defined CONFIG_SPL_BUILD && (defined CONFIG_SUN7I || defined CONFIG_SUN6I)
	/* Enable SMP mode for CPU0, by setting bit 6 of Auxiliary Ctl reg */
	asm volatile(
		"mrc p15, 0, r0, c1, c0, 1\n"
		"orr r0, r0, #1 << 6\n"
		"mcr p15, 0, r0, c1, c0, 1\n");
#endif

	clock_init();
	timer_init();
	gpio_init();

#ifdef CONFIG_SPL_BUILD
	gd = &gdata;
	preloader_console_init();

	sunxi_board_init();
#endif
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#ifdef CONFIG_CMD_NET
/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
	int rc;

#ifdef CONFIG_SUNXI_GMAC
	rc = sunxi_gmac_initialize(bis);
	if (rc < 0) {
		printf("sunxi: failed to initialize gmac\n");
		return rc;
	}
#endif

	return 0;
}
#endif
