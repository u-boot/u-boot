// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Common board functions for AM33XX based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - https://www.ti.com/
 */

#include <config.h>
#include <dm.h>
#include <debug_uart.h>
#include <errno.h>
#include <event.h>
#include <init.h>
#include <net.h>
#include <ns16550.h>
#include <omap3_spi.h>
#include <spl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/i2c.h>
#if IS_ENABLED(CONFIG_TARGET_AM335X_GUARDIAN)
#include <asm/arch/mem-guardian.h>
#else
#include <asm/arch/mem.h>
#endif
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <asm/omap_common.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/compiler.h>
#include <linux/printk.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/musb.h>
#include <asm/omap_musb.h>
#include <asm/davinci_rtc.h>

#define AM43XX_EMIF_BASE				0x4C000000
#define AM43XX_SDRAM_CONFIG_OFFSET			0x8
#define AM43XX_SDRAM_TYPE_MASK				0xE0000000
#define AM43XX_SDRAM_TYPE_SHIFT				29
#define AM43XX_SDRAM_TYPE_DDR3				3
#define AM43XX_READ_WRITE_LEVELING_CTRL_OFFSET		0xDC
#define AM43XX_RDWRLVLFULL_START			0x80000000

/* SPI flash. */
#if CONFIG_IS_ENABLED(DM_SPI) && !CONFIG_IS_ENABLED(OF_CONTROL)
#define AM33XX_SPI0_BASE	0x48030000
#define AM33XX_SPI0_OFFSET	(AM33XX_SPI0_BASE + OMAP4_MCSPI_REG_OFFSET)
#endif

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
	sdram_init();
#endif

	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size(
			(void *)CFG_SYS_SDRAM_BASE,
			CFG_MAX_RAM_BANK_SIZE);
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct ns16550_plat am33xx_serial[] = {
	{ .base = CFG_SYS_NS16550_COM1, .reg_shift = 2,
	  .clock = CFG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
# ifdef CFG_SYS_NS16550_COM2
	{ .base = CFG_SYS_NS16550_COM2, .reg_shift = 2,
	  .clock = CFG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
#  ifdef CFG_SYS_NS16550_COM3
	{ .base = CFG_SYS_NS16550_COM3, .reg_shift = 2,
	  .clock = CFG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = CFG_SYS_NS16550_COM4, .reg_shift = 2,
	  .clock = CFG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = CFG_SYS_NS16550_COM5, .reg_shift = 2,
	  .clock = CFG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
	{ .base = CFG_SYS_NS16550_COM6, .reg_shift = 2,
	  .clock = CFG_SYS_NS16550_CLK, .fcr = UART_FCR_DEFVAL, },
#  endif
# endif
};

U_BOOT_DRVINFOS(am33xx_uarts) = {
	{ "ns16550_serial", &am33xx_serial[0] },
#  ifdef CFG_SYS_NS16550_COM2
	{ "ns16550_serial", &am33xx_serial[1] },
#   ifdef CFG_SYS_NS16550_COM3
	{ "ns16550_serial", &am33xx_serial[2] },
	{ "ns16550_serial", &am33xx_serial[3] },
	{ "ns16550_serial", &am33xx_serial[4] },
	{ "ns16550_serial", &am33xx_serial[5] },
#   endif
#  endif
};

#if CONFIG_IS_ENABLED(DM_I2C)
static const struct omap_i2c_plat am33xx_i2c[] = {
	{ I2C_BASE1, 100000, OMAP_I2C_REV_V2},
	{ I2C_BASE2, 100000, OMAP_I2C_REV_V2},
	{ I2C_BASE3, 100000, OMAP_I2C_REV_V2},
};

U_BOOT_DRVINFOS(am33xx_i2c) = {
	{ "i2c_omap", &am33xx_i2c[0] },
	{ "i2c_omap", &am33xx_i2c[1] },
	{ "i2c_omap", &am33xx_i2c[2] },
};
#endif

#if CONFIG_IS_ENABLED(DM_GPIO)
static const struct omap_gpio_plat am33xx_gpio[] = {
	{ 0, AM33XX_GPIO0_BASE },
	{ 1, AM33XX_GPIO1_BASE },
	{ 2, AM33XX_GPIO2_BASE },
	{ 3, AM33XX_GPIO3_BASE },
#ifdef CONFIG_AM43XX
	{ 4, AM33XX_GPIO4_BASE },
	{ 5, AM33XX_GPIO5_BASE },
#endif
};

U_BOOT_DRVINFOS(am33xx_gpios) = {
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
#if CONFIG_IS_ENABLED(DM_SPI) && !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct omap3_spi_plat omap3_spi_pdata = {
	.regs = (struct mcspi *)AM33XX_SPI0_OFFSET,
	.pin_dir = MCSPI_PINDIR_D0_IN_D1_OUT,
};

U_BOOT_DRVINFO(am33xx_spi) = {
	.name = "omap3_spi",
	.plat = &omap3_spi_pdata,
};
#endif
#endif

#if !CONFIG_IS_ENABLED(DM_GPIO)
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
int cpu_mmc_init(struct bd_info *bis)
{
	int ret;

	ret = omap_mmc_init(0, 0, 0, -1, -1);
	if (ret)
		return ret;

	return omap_mmc_init(1, 0, 0, -1, -1);
}
#endif

/*
 * RTC only with DDR in self-refresh mode magic value, checked against during
 * boot to see if we have a valid config. This should be in sync with the value
 * that will be in drivers/soc/ti/pm33xx.c.
 */
#define RTC_MAGIC_VAL		0x8cd0

/* Board type field bit shift for RTC only with DDR in self-refresh mode */
#define RTC_BOARD_TYPE_SHIFT	16

/* AM33XX has two MUSB controllers which can be host or gadget */
#if (defined(CONFIG_AM335X_USB0) || defined(CONFIG_AM335X_USB1)) && \
	defined(CONFIG_XPL_BUILD)

static struct musb_hdrc_config musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 16,
	.ram_bits       = 12,
};

#ifdef CONFIG_AM335X_USB0
static struct ti_musb_plat usb0 = {
	.base = (void *)USB0_OTG_BASE,
	.ctrl_mod_base = &((struct ctrl_dev *)CTRL_DEVICE_BASE)->usb_ctrl0,
	.plat = {
		.config         = &musb_config,
		.power          = 50,
		.platform_ops	= &musb_dsps_ops,
		},
};
#endif

#ifdef CONFIG_AM335X_USB1
static struct ti_musb_plat usb1 = {
	.base = (void *)USB1_OTG_BASE,
	.ctrl_mod_base = &((struct ctrl_dev *)CTRL_DEVICE_BASE)->usb_ctrl1,
	.plat = {
		.config         = &musb_config,
		.power          = 50,
		.platform_ops	= &musb_dsps_ops,
		},
};
#endif

U_BOOT_DRVINFOS(am33xx_usbs) = {
#ifdef CONFIG_AM335X_USB0_PERIPHERAL
	{ "ti-musb-peripheral", &usb0 },
#elif defined(CONFIG_AM335X_USB0_HOST)
	{ "ti-musb-host", &usb0 },
#endif
#ifdef CONFIG_AM335X_USB1_PERIPHERAL
	{ "ti-musb-peripheral", &usb1 },
#elif defined(CONFIG_AM335X_USB1_HOST)
	{ "ti-musb-host", &usb1 },
#endif
};

int arch_misc_init(void)
{
	return 0;
}
#else	/* CONFIG_USB_MUSB_* && CONFIG_AM335X_USB* && !CONFIG_DM_USB */

int arch_misc_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_MISC, &dev);
	if (ret)
		return ret;

#if defined(CONFIG_DM_ETH) && defined(CONFIG_USB_ETHER)
	usb_ether_init();
#endif

	return 0;
}

#endif /* CONFIG_USB_MUSB_* && CONFIG_AM335X_USB* && !CONFIG_DM_USB */

#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)

#if defined(CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC) || \
	(defined(CONFIG_XPL_BUILD) && defined(CONFIG_SPL_RTC_DDR_SUPPORT))
static void rtc32k_unlock(struct davinci_rtc *rtc)
{
	/*
	 * Unlock the RTC's registers.  For more details please see the
	 * RTC_SS section of the TRM.  In order to unlock we need to
	 * write these specific values (keys) in this order.
	 */
	writel(RTC_KICK0R_WE, &rtc->kick0r);
	writel(RTC_KICK1R_WE, &rtc->kick1r);
}
#endif

#if defined(CONFIG_XPL_BUILD) && defined(CONFIG_SPL_RTC_DDR_SUPPORT)
/*
 * Write contents of the RTC_SCRATCH1 register based on board type
 * Two things are passed
 * on. First 16 bits (0:15) are written with RTC_MAGIC value. Once the
 * control gets to kernel, kernel reads the scratchpad register and gets to
 * know that bootloader has rtc_only support.
 *
 * Second important thing is the board type  (16:31). This is needed in the
 * rtc_only boot where in we want to avoid costly i2c reads to eeprom to
 * identify the board type and we go ahead and copy the board strings to
 * am43xx_board_name.
 */
void update_rtc_magic(void)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;
	u32 magic = RTC_MAGIC_VAL;

	magic |= (rtc_only_get_board_type() << RTC_BOARD_TYPE_SHIFT);

	rtc32k_unlock(rtc);

	/* write magic */
	writel(magic, &rtc->scratch1);
}
#endif

/*
 * In the case of non-SPL based booting we'll want to call these
 * functions a tiny bit later as it will require gd to be set and cleared
 * and that's not true in s_init in this case so we cannot do it there.
 */
int board_early_init_f(void)
{
	set_mux_conf_regs();
	prcm_init();
#if defined(CONFIG_XPL_BUILD) && defined(CONFIG_SPL_RTC_DDR_SUPPORT)
	update_rtc_magic();
#endif
	return 0;
}

#if defined(CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC)
static void rtc32k_enable(void)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;

	rtc32k_unlock(rtc);

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

#if defined(CONFIG_XPL_BUILD) && defined(CONFIG_SPL_RTC_DDR_SUPPORT)
/*
 * Check if we are executing rtc-only + DDR mode, and resume from it if needed
 */
static void rtc_only(void)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;
	struct prm_device_inst *prm_device =
				(struct prm_device_inst *)PRM_DEVICE_INST;

	u32 scratch1, sdrc;
	void (*resume_func)(void);

	scratch1 = readl(&rtc->scratch1);

	/*
	 * Check RTC scratch against RTC_MAGIC_VAL, RTC_MAGIC_VAL is only
	 * written to this register when we want to wake up from RTC only
	 * with DDR in self-refresh mode. Contents of the RTC_SCRATCH1:
	 * bits 0-15:  RTC_MAGIC_VAL
	 * bits 16-31: board type (needed for sdram_init)
	 */
	if ((scratch1 & 0xffff) != RTC_MAGIC_VAL)
		return;

	rtc32k_unlock(rtc);

	/* Clear RTC magic */
	writel(0, &rtc->scratch1);

	/*
	 * Update board type based on value stored on RTC_SCRATCH1, this
	 * is done so that we don't need to read the board type from eeprom
	 * over i2c bus which is expensive
	 */
	rtc_only_update_board_type(scratch1 >> RTC_BOARD_TYPE_SHIFT);

	/*
	 * Enable EMIF_DEVOFF in PRCM_PRM_EMIF_CTRL to indicate to EMIF we
	 * are resuming from self-refresh. This avoids an unnecessary re-init
	 * of the DDR. The re-init takes time and we would need to wait for
	 * it to complete before accessing DDR to avoid L3 NOC errors.
	 */
	writel(EMIF_CTRL_DEVOFF, &prm_device->emif_ctrl);

	rtc_only_prcm_init();
	sdram_init();

	/* Check EMIF4D_SDRAM_CONFIG[31:29] SDRAM_TYPE */
	/* Only perform leveling if SDRAM_TYPE = 3 (DDR3) */
	sdrc = readl(AM43XX_EMIF_BASE + AM43XX_SDRAM_CONFIG_OFFSET);

	sdrc &= AM43XX_SDRAM_TYPE_MASK;
	sdrc >>= AM43XX_SDRAM_TYPE_SHIFT;

	if (sdrc == AM43XX_SDRAM_TYPE_DDR3) {
		writel(AM43XX_RDWRLVLFULL_START,
		       AM43XX_EMIF_BASE +
		       AM43XX_READ_WRITE_LEVELING_CTRL_OFFSET);
		mdelay(1);

am43xx_wait:
		sdrc = readl(AM43XX_EMIF_BASE +
			     AM43XX_READ_WRITE_LEVELING_CTRL_OFFSET);
		if (sdrc == AM43XX_RDWRLVLFULL_START)
			goto am43xx_wait;
	}

	resume_func = (void *)readl(&rtc->scratch0);
	if (resume_func)
		resume_func();
}
#endif

void s_init(void)
{
#if defined(CONFIG_XPL_BUILD) && defined(CONFIG_SPL_RTC_DDR_SUPPORT)
	rtc_only();
#endif
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
#ifdef CONFIG_XPL_BUILD
	/*
	 * Save the boot parameters passed from romcode.
	 * We cannot delay the saving further than this,
	 * to prevent overwrites.
	 */
	save_omap_boot_params();
#endif

#ifdef CONFIG_XPL_BUILD
	spl_early_init();
#endif

#ifdef CONFIG_TI_I2C_BOARD_DETECT
	do_board_detect();
#endif

#if defined(CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC)
	/* Enable RTC32K clock */
	rtc32k_enable();
#endif
}

#ifdef CONFIG_XPL_BUILD
void board_init_f(ulong dummy)
{
	hw_data_init();
	early_system_init();
	board_early_init_f();
	sdram_init();
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size(
			(void *)CFG_SYS_SDRAM_BASE,
			CFG_MAX_RAM_BANK_SIZE);
}
#endif

#endif

static int am33xx_dm_post_init(void)
{
	hw_data_init();
#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
	early_system_init();
#endif
	return 0;
}
EVENT_SPY_SIMPLE(EVT_DM_POST_INIT_F, am33xx_dm_post_init);

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	if (xpl_is_first_phase()) {
		hw_data_init();
		set_uart_mux_conf();
		setup_early_clocks();
		uart_soft_reset();

		/* avoid uart gibberish by allowing the clocks to settle */
		mdelay(50);
	}
}
#endif
