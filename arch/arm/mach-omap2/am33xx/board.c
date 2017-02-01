/*
 * board.c
 *
 * Common board functions for AM33XX based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <ns16550.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <linux/errno.h>
#include <linux/compiler.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/musb.h>
#include <asm/omap_musb.h>
#include <asm/davinci_rtc.h>

DECLARE_GLOBAL_DATA_PTR;

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct ns16550_platdata am33xx_serial[] = {
	{ .base = CONFIG_SYS_NS16550_COM1, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
# ifdef CONFIG_SYS_NS16550_COM2
	{ .base = CONFIG_SYS_NS16550_COM2, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
#  ifdef CONFIG_SYS_NS16550_COM3
	{ .base = CONFIG_SYS_NS16550_COM3, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = CONFIG_SYS_NS16550_COM4, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = CONFIG_SYS_NS16550_COM5, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = CONFIG_SYS_NS16550_COM6, .reg_shift = 2,
	  .clock = CONFIG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
#  endif
# endif
};

U_BOOT_DEVICES(am33xx_uarts) = {
	{ "ns16550_serial", &am33xx_serial[0] },
#  ifdef CONFIG_SYS_NS16550_COM2
	{ "ns16550_serial", &am33xx_serial[1] },
#   ifdef CONFIG_SYS_NS16550_COM3
	{ "ns16550_serial", &am33xx_serial[2] },
	{ "ns16550_serial", &am33xx_serial[3] },
	{ "ns16550_serial", &am33xx_serial[4] },
	{ "ns16550_serial", &am33xx_serial[5] },
#   endif
#  endif
};

#ifdef CONFIG_DM_GPIO
static const struct omap_gpio_platdata am33xx_gpio[] = {
	{ 0, AM33XX_GPIO0_BASE },
	{ 1, AM33XX_GPIO1_BASE },
	{ 2, AM33XX_GPIO2_BASE },
	{ 3, AM33XX_GPIO3_BASE },
#ifdef CONFIG_AM43XX
	{ 4, AM33XX_GPIO4_BASE },
	{ 5, AM33XX_GPIO5_BASE },
#endif
};

U_BOOT_DEVICES(am33xx_gpios) = {
	{ "gpio_omap", &am33xx_gpio[0] },
	{ "gpio_omap", &am33xx_gpio[1] },
	{ "gpio_omap", &am33xx_gpio[2] },
	{ "gpio_omap", &am33xx_gpio[3] },
#ifdef CONFIG_AM43XX
	{ "gpio_omap", &am33xx_gpio[4] },
	{ "gpio_omap", &am33xx_gpio[5] },
#endif
};
#endif
#endif

#ifndef CONFIG_DM_GPIO
static const struct gpio_bank gpio_bank_am33xx[] = {
	{ (void *)AM33XX_GPIO0_BASE },
	{ (void *)AM33XX_GPIO1_BASE },
	{ (void *)AM33XX_GPIO2_BASE },
	{ (void *)AM33XX_GPIO3_BASE },
#ifdef CONFIG_AM43XX
	{ (void *)AM33XX_GPIO4_BASE },
	{ (void *)AM33XX_GPIO5_BASE },
#endif
};

const struct gpio_bank *const omap_gpio_bank = gpio_bank_am33xx;
#endif

#if defined(CONFIG_MMC_OMAP_HS)
int cpu_mmc_init(bd_t *bis)
{
	int ret;

	ret = omap_mmc_init(0, 0, 0, -1, -1);
	if (ret)
		return ret;

	return omap_mmc_init(1, 0, 0, -1, -1);
}
#endif

/* AM33XX has two MUSB controllers which can be host or gadget */
#if (defined(CONFIG_USB_MUSB_GADGET) || defined(CONFIG_USB_MUSB_HOST)) && \
	(defined(CONFIG_AM335X_USB0) || defined(CONFIG_AM335X_USB1)) && \
	(!defined(CONFIG_DM_USB))
static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

/* USB 2.0 PHY Control */
#define CM_PHY_PWRDN			(1 << 0)
#define CM_PHY_OTG_PWRDN		(1 << 1)
#define OTGVDET_EN			(1 << 19)
#define OTGSESSENDEN			(1 << 20)

static void am33xx_usb_set_phy_power(u8 on, u32 *reg_addr)
{
	if (on) {
		clrsetbits_le32(reg_addr, CM_PHY_PWRDN | CM_PHY_OTG_PWRDN,
				OTGVDET_EN | OTGSESSENDEN);
	} else {
		clrsetbits_le32(reg_addr, 0, CM_PHY_PWRDN | CM_PHY_OTG_PWRDN);
	}
}

static struct musb_hdrc_config musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 16,
	.ram_bits       = 12,
};

#ifdef CONFIG_AM335X_USB0
static void am33xx_otg0_set_phy_power(struct udevice *dev, u8 on)
{
	am33xx_usb_set_phy_power(on, &cdev->usb_ctrl0);
}

struct omap_musb_board_data otg0_board_data = {
	.set_phy_power = am33xx_otg0_set_phy_power,
};

static struct musb_hdrc_platform_data otg0_plat = {
	.mode           = CONFIG_AM335X_USB0_MODE,
	.config         = &musb_config,
	.power          = 50,
	.platform_ops	= &musb_dsps_ops,
	.board_data	= &otg0_board_data,
};
#endif

#ifdef CONFIG_AM335X_USB1
static void am33xx_otg1_set_phy_power(struct udevice *dev, u8 on)
{
	am33xx_usb_set_phy_power(on, &cdev->usb_ctrl1);
}

struct omap_musb_board_data otg1_board_data = {
	.set_phy_power = am33xx_otg1_set_phy_power,
};

static struct musb_hdrc_platform_data otg1_plat = {
	.mode           = CONFIG_AM335X_USB1_MODE,
	.config         = &musb_config,
	.power          = 50,
	.platform_ops	= &musb_dsps_ops,
	.board_data	= &otg1_board_data,
};
#endif
#endif

int arch_misc_init(void)
{
#ifndef CONFIG_DM_USB
#ifdef CONFIG_AM335X_USB0
	musb_register(&otg0_plat, &otg0_board_data,
		(void *)USB0_OTG_BASE);
#endif
#ifdef CONFIG_AM335X_USB1
	musb_register(&otg1_plat, &otg1_board_data,
		(void *)USB1_OTG_BASE);
#endif
#else
	struct udevice *dev;
	int ret;

	ret = uclass_first_device(UCLASS_MISC, &dev);
	if (ret || !dev)
		return ret;

#if defined(CONFIG_DM_ETH) && defined(CONFIG_USB_ETHER)
	ret = usb_ether_init();
	if (ret) {
		error("USB ether init failed\n");
		return ret;
	}
#endif
#endif
	return 0;
}

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
/*
 * In the case of non-SPL based booting we'll want to call these
 * functions a tiny bit later as it will require gd to be set and cleared
 * and that's not true in s_init in this case so we cannot do it there.
 */
int board_early_init_f(void)
{
	prcm_init();
	set_mux_conf_regs();

	return 0;
}

/*
 * This function is the place to do per-board things such as ramp up the
 * MPU clock frequency.
 */
__weak void am33xx_spl_board_init(void)
{
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
}

#if defined(CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC)
static void rtc32k_enable(void)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;

	/*
	 * Unlock the RTC's registers.  For more details please see the
	 * RTC_SS section of the TRM.  In order to unlock we need to
	 * write these specific values (keys) in this order.
	 */
	writel(RTC_KICK0R_WE, &rtc->kick0r);
	writel(RTC_KICK1R_WE, &rtc->kick1r);

	/* Enable the RTC 32K OSC by setting bits 3 and 6. */
	writel((1 << 3) | (1 << 6), &rtc->osc);
}
#endif

static void uart_soft_reset(void)
{
	struct uart_sys *uart_base = (struct uart_sys *)DEFAULT_UART_BASE;
	u32 regval;

	regval = readl(&uart_base->uartsyscfg);
	regval |= UART_RESET;
	writel(regval, &uart_base->uartsyscfg);
	while ((readl(&uart_base->uartsyssts) &
		UART_CLK_RUNNING_MASK) != UART_CLK_RUNNING_MASK)
		;

	/* Disable smart idle */
	regval = readl(&uart_base->uartsyscfg);
	regval |= UART_SMART_IDLE_EN;
	writel(regval, &uart_base->uartsyscfg);
}

static void watchdog_disable(void)
{
	struct wd_timer *wdtimer = (struct wd_timer *)WDT_BASE;

	writel(0xAAAA, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
	writel(0x5555, &wdtimer->wdtwspr);
	while (readl(&wdtimer->wdtwwps) != 0x0)
		;
}

void s_init(void)
{
}

void early_system_init(void)
{
	/*
	 * The ROM will only have set up sufficient pinmux to allow for the
	 * first 4KiB NOR to be read, we must finish doing what we know of
	 * the NOR mux in this space in order to continue.
	 */
#ifdef CONFIG_NOR_BOOT
	enable_norboot_pin_mux();
#endif
	watchdog_disable();
	set_uart_mux_conf();
	setup_early_clocks();
	uart_soft_reset();
#ifdef CONFIG_TI_I2C_BOARD_DETECT
	do_board_detect();
#endif
#if defined(CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC)
	/* Enable RTC32K clock */
	rtc32k_enable();
#endif
}

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong dummy)
{
	early_system_init();
	board_early_init_f();
	sdram_init();
}
#endif

#endif

int arch_cpu_init_dm(void)
{
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	early_system_init();
#endif
	return 0;
}
