/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ns16550.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#ifdef CONFIG_LCD
#include <asm/arch/display.h>
#endif
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/pmu.h>
#ifdef CONFIG_PWM_TEGRA
#include <asm/arch/pwm.h>
#endif
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/board.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/sys_proto.h>
#include <asm/arch-tegra/uart.h>
#include <asm/arch-tegra/warmboot.h>
#ifdef CONFIG_TEGRA_CLOCK_SCALING
#include <asm/arch/emc.h>
#endif
#ifdef CONFIG_USB_EHCI_TEGRA
#include <asm/arch-tegra/usb.h>
#include <usb.h>
#endif
#ifdef CONFIG_TEGRA_MMC
#include <asm/arch-tegra/tegra_mmc.h>
#include <asm/arch-tegra/mmc.h>
#endif
#include <i2c.h>
#include <spi.h>
#include "emc.h"

DECLARE_GLOBAL_DATA_PTR;

const struct tegra_sysinfo sysinfo = {
	CONFIG_TEGRA_BOARD_STRING
};

void __pinmux_init(void)
{
}

void pinmux_init(void) __attribute__((weak, alias("__pinmux_init")));

void __pin_mux_usb(void)
{
}

void pin_mux_usb(void) __attribute__((weak, alias("__pin_mux_usb")));

void __pin_mux_spi(void)
{
}

void pin_mux_spi(void) __attribute__((weak, alias("__pin_mux_spi")));

void __gpio_early_init_uart(void)
{
}

void gpio_early_init_uart(void)
__attribute__((weak, alias("__gpio_early_init_uart")));

#if defined(CONFIG_TEGRA_NAND)
void __pin_mux_nand(void)
{
	funcmux_select(PERIPH_ID_NDFLASH, FUNCMUX_DEFAULT);
}

void pin_mux_nand(void) __attribute__((weak, alias("__pin_mux_nand")));
#endif

void __pin_mux_display(void)
{
}

void pin_mux_display(void) __attribute__((weak, alias("__pin_mux_display")));

/*
 * Routine: power_det_init
 * Description: turn off power detects
 */
static void power_det_init(void)
{
#if defined(CONFIG_TEGRA20)
	struct pmc_ctlr *const pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;

	/* turn off power detects */
	writel(0, &pmc->pmc_pwr_det_latch);
	writel(0, &pmc->pmc_pwr_det);
#endif
}

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	__maybe_unused int err;

	/* Do clocks and UART first so that printf() works */
	clock_init();
	clock_verify();

#ifdef CONFIG_FDT_SPI
	pin_mux_spi();
	spi_init();
#endif

#ifdef CONFIG_PWM_TEGRA
	if (pwm_init(gd->fdt_blob))
		debug("%s: Failed to init pwm\n", __func__);
#endif
#ifdef CONFIG_LCD
	pin_mux_display();
	tegra_lcd_check_next_stage(gd->fdt_blob, 0);
#endif
	/* boot param addr */
	gd->bd->bi_boot_params = (NV_PA_SDRAM_BASE + 0x100);

	power_det_init();

#ifdef CONFIG_SYS_I2C_TEGRA
#ifndef CONFIG_SYS_I2C_INIT_BOARD
#error "You must define CONFIG_SYS_I2C_INIT_BOARD to use i2c on Nvidia boards"
#endif
	i2c_init_board();
# ifdef CONFIG_TEGRA_PMU
	if (pmu_set_nominal())
		debug("Failed to select nominal voltages\n");
#  ifdef CONFIG_TEGRA_CLOCK_SCALING
	err = board_emc_init();
	if (err)
		debug("Memory controller init failed: %d\n", err);
#  endif
# endif /* CONFIG_TEGRA_PMU */
#endif /* CONFIG_SYS_I2C_TEGRA */

#ifdef CONFIG_USB_EHCI_TEGRA
	pin_mux_usb();
	usb_process_devicetree(gd->fdt_blob);
#endif

#ifdef CONFIG_LCD
	tegra_lcd_check_next_stage(gd->fdt_blob, 0);
#endif

#ifdef CONFIG_TEGRA_NAND
	pin_mux_nand();
#endif

#ifdef CONFIG_TEGRA_LP0
	/* save Sdram params to PMC 2, 4, and 24 for WB0 */
	warmboot_save_sdram_params();

	/* prepare the WB code to LP0 location */
	warmboot_prepare_code(TEGRA_LP0_ADDR, TEGRA_LP0_SIZE);
#endif

	return 0;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
static void __gpio_early_init(void)
{
}

void gpio_early_init(void) __attribute__((weak, alias("__gpio_early_init")));

int board_early_init_f(void)
{
	pinmux_init();
	board_init_uart_f();

	/* Initialize periph GPIOs */
	gpio_early_init();
	gpio_early_init_uart();
#ifdef CONFIG_LCD
	tegra_lcd_early_init(gd->fdt_blob);
#endif

	return 0;
}
#endif	/* EARLY_INIT */

int board_late_init(void)
{
#ifdef CONFIG_LCD
	/* Make sure we finish initing the LCD */
	tegra_lcd_check_next_stage(gd->fdt_blob, 1);
#endif
	return 0;
}

#if defined(CONFIG_TEGRA_MMC)
void __pin_mux_mmc(void)
{
}

void pin_mux_mmc(void) __attribute__((weak, alias("__pin_mux_mmc")));

/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	debug("%s called\n", __func__);

	/* Enable muxes, etc. for SDMMC controllers */
	pin_mux_mmc();

	debug("%s: init MMC\n", __func__);
	tegra_mmc_init();

	return 0;
}

void pad_init_mmc(struct mmc_host *host)
{
#if defined(CONFIG_TEGRA30)
	enum periph_id id = host->mmc_id;
	u32 val;

	debug("%s: sdmmc address = %08x, id = %d\n", __func__,
		(unsigned int)host->reg, id);

	/* Set the pad drive strength for SDMMC1 or 3 only */
	if (id != PERIPH_ID_SDMMC1 && id != PERIPH_ID_SDMMC3) {
		debug("%s: settings are only valid for SDMMC1/SDMMC3!\n",
			__func__);
		return;
	}

	val = readl(&host->reg->sdmemcmppadctl);
	val &= 0xFFFFFFF0;
	val |= MEMCOMP_PADCTRL_VREF;
	writel(val, &host->reg->sdmemcmppadctl);

	val = readl(&host->reg->autocalcfg);
	val &= 0xFFFF0000;
	val |= AUTO_CAL_PU_OFFSET | AUTO_CAL_PD_OFFSET | AUTO_CAL_ENABLED;
	writel(val, &host->reg->autocalcfg);
#endif	/* T30 */
}
#endif	/* MMC */
