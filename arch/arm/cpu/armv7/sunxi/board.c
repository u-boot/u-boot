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
#include <i2c.h>
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

#include <linux/compiler.h>

struct fel_stash {
	uint32_t sp;
	uint32_t lr;
	uint32_t cpsr;
	uint32_t sctlr;
	uint32_t vbar;
	uint32_t cr;
};

struct fel_stash fel_stash __attribute__((section(".data")));

static int gpio_init(void)
{
#if CONFIG_CONS_INDEX == 1 && defined(CONFIG_UART0_PORT_F)
#if defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN7I)
	/* disable GPB22,23 as uart0 tx,rx to avoid conflict */
	sunxi_gpio_set_cfgpin(SUNXI_GPB(22), SUNXI_GPIO_INPUT);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(23), SUNXI_GPIO_INPUT);
#endif
	sunxi_gpio_set_cfgpin(SUNXI_GPF(2), SUNXI_GPF2_UART0_TX);
	sunxi_gpio_set_cfgpin(SUNXI_GPF(4), SUNXI_GPF4_UART0_RX);
	sunxi_gpio_set_pull(SUNXI_GPF(4), 1);
#elif CONFIG_CONS_INDEX == 1 && (defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN7I))
	sunxi_gpio_set_cfgpin(SUNXI_GPB(22), SUN4I_GPB22_UART0_TX);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(23), SUN4I_GPB23_UART0_RX);
	sunxi_gpio_set_pull(SUNXI_GPB(23), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN5I)
	sunxi_gpio_set_cfgpin(SUNXI_GPB(19), SUN5I_GPB19_UART0_TX);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(20), SUN5I_GPB20_UART0_RX);
	sunxi_gpio_set_pull(SUNXI_GPB(20), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 1 && defined(CONFIG_MACH_SUN6I)
	sunxi_gpio_set_cfgpin(SUNXI_GPH(20), SUN6I_GPH20_UART0_TX);
	sunxi_gpio_set_cfgpin(SUNXI_GPH(21), SUN6I_GPH21_UART0_RX);
	sunxi_gpio_set_pull(SUNXI_GPH(21), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 2 && defined(CONFIG_MACH_SUN5I)
	sunxi_gpio_set_cfgpin(SUNXI_GPG(3), SUN5I_GPG3_UART1_TX);
	sunxi_gpio_set_cfgpin(SUNXI_GPG(4), SUN5I_GPG4_UART1_RX);
	sunxi_gpio_set_pull(SUNXI_GPG(4), SUNXI_GPIO_PULL_UP);
#elif CONFIG_CONS_INDEX == 5 && defined(CONFIG_MACH_SUN8I)
	sunxi_gpio_set_cfgpin(SUNXI_GPL(2), SUN8I_GPL2_R_UART_TX);
	sunxi_gpio_set_cfgpin(SUNXI_GPL(3), SUN8I_GPL3_R_UART_RX);
	sunxi_gpio_set_pull(SUNXI_GPL(3), SUNXI_GPIO_PULL_UP);
#else
#error Unsupported console port number. Please fix pin mux settings in board.c
#endif

	return 0;
}

void spl_board_load_image(void)
{
	debug("Returning to FEL sp=%x, lr=%x\n", fel_stash.sp, fel_stash.lr);
	return_to_fel(fel_stash.sp, fel_stash.lr);
}

void s_init(void)
{
#if defined CONFIG_MACH_SUN6I || defined CONFIG_MACH_SUN8I
	/* Magic (undocmented) value taken from boot0, without this DRAM
	 * access gets messed up (seems cache related) */
	setbits_le32(SUNXI_SRAMC_BASE + 0x44, 0x1800);
#endif
#if !defined CONFIG_SPL_BUILD && (defined CONFIG_MACH_SUN7I || \
		defined CONFIG_MACH_SUN6I || defined CONFIG_MACH_SUN8I)
	/* Enable SMP mode for CPU0, by setting bit 6 of Auxiliary Ctl reg */
	asm volatile(
		"mrc p15, 0, r0, c1, c0, 1\n"
		"orr r0, r0, #1 << 6\n"
		"mcr p15, 0, r0, c1, c0, 1\n");
#endif

	clock_init();
	timer_init();
	gpio_init();
	i2c_init_board();
}

#ifdef CONFIG_SPL_BUILD
/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nand flash, mmc2.
 * Unfortunately we can't check how SPL was loaded so assume
 * it's always the first SD/MMC controller
 */
u32 spl_boot_device(void)
{
#ifdef CONFIG_SPL_FEL
	/*
	 * This is the legacy compile time configuration for a special FEL
	 * enabled build. It has many restrictions and can only boot over USB.
	 */
	return BOOT_DEVICE_BOARD;
#else
	/*
	 * When booting from the SD card, the "eGON.BT0" signature is expected
	 * to be found in memory at the address 0x0004 (see the "mksunxiboot"
	 * tool, which generates this header).
	 *
	 * When booting in the FEL mode over USB, this signature is patched in
	 * memory and replaced with something else by the 'fel' tool. This other
	 * signature is selected in such a way, that it can't be present in a
	 * valid bootable SD card image (because the BROM would refuse to
	 * execute the SPL in this case).
	 *
	 * This branch is just making a decision at runtime whether to load
	 * the main u-boot binary from the SD card (if the "eGON.BT0" signature
	 * is found) or return to the FEL code in the BROM to wait and receive
	 * the main u-boot binary over USB.
	 */
	if (readl(4) == 0x4E4F4765 && readl(8) == 0x3054422E) /* eGON.BT0 */
		return BOOT_DEVICE_MMC1;
	else
		return BOOT_DEVICE_BOARD;
#endif
}

/* No confirmation data available in SPL yet. Hardcode bootmode */
u32 spl_boot_mode(void)
{
	return MMCSD_MODE_RAW;
}

void board_init_f(ulong dummy)
{
	preloader_console_init();

#ifdef CONFIG_SPL_I2C_SUPPORT
	/* Needed early by sunxi_board_init if PMU is enabled */
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif
	sunxi_board_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	board_init_r(NULL, 0);
}
#endif

void reset_cpu(ulong addr)
{
#if defined(CONFIG_MACH_SUN4I) || defined(CONFIG_MACH_SUN5I) || defined(CONFIG_MACH_SUN7I)
	static const struct sunxi_wdog *wdog =
		 &((struct sunxi_timer_reg *)SUNXI_TIMER_BASE)->wdog;

	/* Set the watchdog for its shortest interval (.5s) and wait */
	writel(WDT_MODE_RESET_EN | WDT_MODE_EN, &wdog->mode);
	writel(WDT_CTRL_KEY | WDT_CTRL_RESTART, &wdog->ctl);

	while (1) {
		/* sun5i sometimes gets stuck without this */
		writel(WDT_MODE_RESET_EN | WDT_MODE_EN, &wdog->mode);
	}
#else /* CONFIG_MACH_SUN6I || CONFIG_MACH_SUN8I || .. */
	static const struct sunxi_wdog *wdog =
		 ((struct sunxi_timer_reg *)SUNXI_TIMER_BASE)->wdog;

	/* Set the watchdog for its shortest interval (.5s) and wait */
	writel(WDT_CFG_RESET, &wdog->cfg);
	writel(WDT_MODE_EN, &wdog->mode);
	writel(WDT_CTRL_KEY | WDT_CTRL_RESTART, &wdog->ctl);
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
	__maybe_unused int rc;

#ifdef CONFIG_MACPWR
	gpio_direction_output(CONFIG_MACPWR, 1);
	mdelay(200);
#endif

#ifdef CONFIG_SUNXI_EMAC
	rc = sunxi_emac_initialize(bis);
	if (rc < 0) {
		printf("sunxi: failed to initialize emac\n");
		return rc;
	}
#endif

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
