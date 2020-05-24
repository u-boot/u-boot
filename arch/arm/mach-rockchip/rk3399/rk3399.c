// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2016 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <fdt_support.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <spl_gpio.h>
#include <syscon.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/gpio.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/hardware.h>
#include <linux/bitops.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

#define GRF_EMMCCORE_CON11 0xff77f02c
#define GRF_BASE	0xff770000

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/sdhci@fe330000",
	[BROM_BOOTSOURCE_SPINOR] = "/spi@ff1d0000",
	[BROM_BOOTSOURCE_SD] = "/mmc@fe320000",
};

static struct mm_region rk3399_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xf8000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xf8000000UL,
		.phys = 0xf8000000UL,
		.size = 0x08000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3399_mem_map;

#ifdef CONFIG_SPL_BUILD

#define TIMER_END_COUNT_L	0x00
#define TIMER_END_COUNT_H	0x04
#define TIMER_INIT_COUNT_L	0x10
#define TIMER_INIT_COUNT_H	0x14
#define TIMER_CONTROL_REG	0x1c

#define TIMER_EN	0x1
#define TIMER_FMODE	BIT(0)
#define TIMER_RMODE	BIT(1)

void rockchip_stimer_init(void)
{
	/* If Timer already enabled, don't re-init it */
	u32 reg = readl(CONFIG_ROCKCHIP_STIMER_BASE + TIMER_CONTROL_REG);

	if (reg & TIMER_EN)
		return;

	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_END_COUNT_L);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_END_COUNT_H);
	writel(0, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_INIT_COUNT_L);
	writel(0, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_INIT_COUNT_H);
	writel(TIMER_EN | TIMER_FMODE, CONFIG_ROCKCHIP_STIMER_BASE + \
	       TIMER_CONTROL_REG);
}
#endif

int arch_cpu_init(void)
{

#ifdef CONFIG_SPL_BUILD
	struct rk3399_pmusgrf_regs *sgrf;
	struct rk3399_grf_regs *grf;

	/*
	 * Disable DDR and SRAM security regions.
	 *
	 * As we are entered from the BootROM, the region from
	 * 0x0 through 0xfffff (i.e. the first MB of memory) will
	 * be protected. This will cause issues with the DW_MMC
	 * driver, which tries to DMA from/to the stack (likely)
	 * located in this range.
	 */
	sgrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUSGRF);
	rk_clrsetreg(&sgrf->ddr_rgn_con[16], 0x1ff, 0);
	rk_clrreg(&sgrf->slv_secure_con4, 0x2000);

	/*  eMMC clock generator: disable the clock multipilier */
	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrreg(&grf->emmccore_con[11], 0x0ff);
#endif

	return 0;
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
#define GRF_BASE	0xff770000
#define GPIO0_BASE	0xff720000
#define PMUGRF_BASE	0xff320000
	struct rk3399_grf_regs * const grf = (void *)GRF_BASE;
#ifdef CONFIG_TARGET_CHROMEBOOK_BOB
	struct rk3399_pmugrf_regs * const pmugrf = (void *)PMUGRF_BASE;
	struct rockchip_gpio_regs * const gpio = (void *)GPIO0_BASE;
#endif

#if defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff180000)
	/* Enable early UART0 on the RK3399 */
	rk_clrsetreg(&grf->gpio2c_iomux,
		     GRF_GPIO2C0_SEL_MASK,
		     GRF_UART0BT_SIN << GRF_GPIO2C0_SEL_SHIFT);
	rk_clrsetreg(&grf->gpio2c_iomux,
		     GRF_GPIO2C1_SEL_MASK,
		     GRF_UART0BT_SOUT << GRF_GPIO2C1_SEL_SHIFT);
#elif defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff1B0000)
	/* Enable early UART3 on the RK3399 */
	rk_clrsetreg(&grf->gpio3b_iomux,
		     GRF_GPIO3B6_SEL_MASK,
		     GRF_UART3_SIN << GRF_GPIO3B6_SEL_SHIFT);
	rk_clrsetreg(&grf->gpio3b_iomux,
		     GRF_GPIO3B7_SEL_MASK,
		     GRF_UART3_SOUT << GRF_GPIO3B7_SEL_SHIFT);
#else
# ifdef CONFIG_TARGET_CHROMEBOOK_BOB
	rk_setreg(&grf->io_vsel, 1 << 0);

	/*
	 * Let's enable these power rails here, we are already running the SPI
	 * Flash based code.
	 */
	spl_gpio_output(gpio, GPIO(BANK_B, 2), 1);  /* PP1500_EN */
	spl_gpio_set_pull(&pmugrf->gpio0_p, GPIO(BANK_B, 2), GPIO_PULL_NORMAL);

	spl_gpio_output(gpio, GPIO(BANK_B, 4), 1);  /* PP3000_EN */
	spl_gpio_set_pull(&pmugrf->gpio0_p, GPIO(BANK_B, 4), GPIO_PULL_NORMAL);
#endif /* CONFIG_TARGET_CHROMEBOOK_BOB */

	/* Enable early UART2 channel C on the RK3399 */
	rk_clrsetreg(&grf->gpio4c_iomux,
		     GRF_GPIO4C3_SEL_MASK,
		     GRF_UART2DGBC_SIN << GRF_GPIO4C3_SEL_SHIFT);
	rk_clrsetreg(&grf->gpio4c_iomux,
		     GRF_GPIO4C4_SEL_MASK,
		     GRF_UART2DBGC_SOUT << GRF_GPIO4C4_SEL_SHIFT);
	/* Set channel C as UART2 input */
	rk_clrsetreg(&grf->soc_con7,
		     GRF_UART_DBG_SEL_MASK,
		     GRF_UART_DBG_SEL_C << GRF_UART_DBG_SEL_SHIFT);
#endif
}
#endif

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
const char *spl_decode_boot_device(u32 boot_device)
{
	int i;
	static const struct {
		u32 boot_device;
		const char *ofpath;
	} spl_boot_devices_tbl[] = {
		{ BOOT_DEVICE_MMC1, "/mmc@fe320000" },
		{ BOOT_DEVICE_MMC2, "/sdhci@fe330000" },
		{ BOOT_DEVICE_SPI, "/spi@ff1d0000" },
	};

	for (i = 0; i < ARRAY_SIZE(spl_boot_devices_tbl); ++i)
		if (spl_boot_devices_tbl[i].boot_device == boot_device)
			return spl_boot_devices_tbl[i].ofpath;

	return NULL;
}

void spl_perform_fixups(struct spl_image_info *spl_image)
{
	void *blob = spl_image->fdt_addr;
	const char *boot_ofpath;
	int chosen;

	/*
	 * Inject the ofpath of the device the full U-Boot (or Linux in
	 * Falcon-mode) was booted from into the FDT, if a FDT has been
	 * loaded at the same time.
	 */
	if (!blob)
		return;

	boot_ofpath = spl_decode_boot_device(spl_image->boot_device);
	if (!boot_ofpath) {
		pr_err("%s: could not map boot_device to ofpath\n", __func__);
		return;
	}

	chosen = fdt_find_or_add_subnode(blob, 0, "chosen");
	if (chosen < 0) {
		pr_err("%s: could not find/create '/chosen'\n", __func__);
		return;
	}
	fdt_setprop_string(blob, chosen,
			   "u-boot,spl-boot-device", boot_ofpath);
}

#if defined(SPL_GPIO_SUPPORT)
static void rk3399_force_power_on_reset(void)
{
	ofnode node;
	struct gpio_desc sysreset_gpio;

	debug("%s: trying to force a power-on reset\n", __func__);

	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		debug("%s: no /config node?\n", __func__);
		return;
	}

	if (gpio_request_by_name_nodev(node, "sysreset-gpio", 0,
				       &sysreset_gpio, GPIOD_IS_OUT)) {
		debug("%s: could not find a /config/sysreset-gpio\n", __func__);
		return;
	}

	dm_gpio_set_value(&sysreset_gpio, 1);
}
#endif

void spl_board_init(void)
{
#if defined(SPL_GPIO_SUPPORT)
	struct rockchip_cru *cru = rockchip_get_cru();

	/*
	 * The RK3399 resets only 'almost all logic' (see also in the TRM
	 * "3.9.4 Global software reset"), when issuing a software reset.
	 * This may cause issues during boot-up for some configurations of
	 * the application software stack.
	 *
	 * To work around this, we test whether the last reset reason was
	 * a power-on reset and (if not) issue an overtemp-reset to reset
	 * the entire module.
	 *
	 * While this was previously fixed by modifying the various places
	 * that could generate a software reset (e.g. U-Boot's sysreset
	 * driver, the ATF or Linux), we now have it here to ensure that
	 * we no longer have to track this through the various components.
	 */
	if (cru->glb_rst_st != 0)
		rk3399_force_power_on_reset();
#endif

#if defined(SPL_DM_REGULATOR)
	/*
	 * Turning the eMMC and SPI back on (if disabled via the Qseven
	 * BIOS_ENABLE) signal is done through a always-on regulator).
	 */
	if (regulators_enable_boot_on(false))
		debug("%s: Cannot enable boot on regulator\n", __func__);
#endif
}
#endif
