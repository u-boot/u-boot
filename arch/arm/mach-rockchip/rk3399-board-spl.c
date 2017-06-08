/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <fdtdec.h>
#include <led.h>
#include <malloc.h>
#include <mmc.h>
#include <ram.h>
#include <spl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/periph.h>
#include <asm/arch/sdram.h>
#include <asm/arch/timer.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/util.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_OF_CONTROL)
static int spl_node_to_boot_device(int node)
{
	struct udevice *parent;

	/*
	 * This should eventually move into the SPL code, once SPL becomes
	 * aware of the block-device layer.  Until then (and to avoid unneeded
	 * delays in getting this feature out, it lives at the board-level).
	 */
	if (!uclass_get_device_by_of_offset(UCLASS_MMC, node, &parent)) {
		struct udevice *dev;
		struct blk_desc *desc = NULL;

		for (device_find_first_child(parent, &dev);
		     dev;
		     device_find_next_child(&dev)) {
			if (device_get_uclass_id(dev) == UCLASS_BLK) {
				desc = dev_get_uclass_platdata(dev);
				break;
			}
		}

		if (!desc)
			return -ENOENT;

		switch (desc->devnum) {
		case 0:
			return BOOT_DEVICE_MMC1;
		case 1:
			return BOOT_DEVICE_MMC2;
		default:
			return -ENOSYS;
		}
	}

	/*
	 * SPL doesn't differentiate SPI flashes, so we keep the detection
	 * brief and inaccurate... hopefully, the common SPL layer can be
	 * extended with awareness of the BLK layer (and matching OF_CONTROL)
	 * soon.
	 */
	if (!uclass_get_device_by_of_offset(UCLASS_SPI_FLASH, node, &parent))
		return BOOT_DEVICE_SPI;

	return -1;
}

void board_boot_order(u32 *spl_boot_list)
{
	const void *blob = gd->fdt_blob;
	int chosen_node = fdt_path_offset(blob, "/chosen");
	int idx = 0;
	int elem;
	int boot_device;
	int node;
	const char *conf;

	if (chosen_node < 0) {
		debug("%s: /chosen not found, using spl_boot_device()\n",
		      __func__);
		spl_boot_list[0] = spl_boot_device();
		return;
	}

	for (elem = 0;
	     (conf = fdt_stringlist_get(blob, chosen_node,
					"u-boot,spl-boot-order", elem, NULL));
	     elem++) {
		/* First check if the list element is an alias */
		const char *alias = fdt_get_alias(blob, conf);
		if (alias)
			conf = alias;

		/* Try to resolve the config item (or alias) as a path */
		node = fdt_path_offset(blob, conf);
		if (node < 0) {
			debug("%s: could not find %s in FDT", __func__, conf);
			continue;
		}

		/* Try to map this back onto SPL boot devices */
		boot_device = spl_node_to_boot_device(node);
		if (boot_device < 0) {
			debug("%s: could not map node @%x to a boot-device\n",
			      __func__, node);
			continue;
		}

		spl_boot_list[idx++] = boot_device;
	}

	/* If we had no matches, fall back to spl_boot_device */
	if (idx == 0)
		spl_boot_list[0] = spl_boot_device();
}
#endif

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_MMC1;
}

u32 spl_boot_mode(const u32 boot_device)
{
	return MMCSD_MODE_RAW;
}

#define TIMER_CHN10_BASE	0xff8680a0
#define TIMER_END_COUNT_L	0x00
#define TIMER_END_COUNT_H	0x04
#define TIMER_INIT_COUNT_L	0x10
#define TIMER_INIT_COUNT_H	0x14
#define TIMER_CONTROL_REG	0x1c

#define TIMER_EN	0x1
#define	TIMER_FMODE	(0 << 1)
#define	TIMER_RMODE	(1 << 1)

void secure_timer_init(void)
{
	writel(0xffffffff, TIMER_CHN10_BASE + TIMER_END_COUNT_L);
	writel(0xffffffff, TIMER_CHN10_BASE + TIMER_END_COUNT_H);
	writel(0, TIMER_CHN10_BASE + TIMER_INIT_COUNT_L);
	writel(0, TIMER_CHN10_BASE + TIMER_INIT_COUNT_H);
	writel(TIMER_EN | TIMER_FMODE, TIMER_CHN10_BASE + TIMER_CONTROL_REG);
}

void board_debug_uart_init(void)
{
#include <asm/arch/grf_rk3399.h>
#define GRF_BASE	0xff770000
	struct rk3399_grf_regs * const grf = (void *)GRF_BASE;

#if defined(CONFIG_DEBUG_UART_BASE) && (CONFIG_DEBUG_UART_BASE == 0xff180000)
	/* Enable early UART0 on the RK3399 */
	rk_clrsetreg(&grf->gpio2c_iomux,
		     GRF_GPIO2C0_SEL_MASK,
		     GRF_UART0BT_SIN << GRF_GPIO2C0_SEL_SHIFT);
	rk_clrsetreg(&grf->gpio2c_iomux,
		     GRF_GPIO2C1_SEL_MASK,
		     GRF_UART0BT_SOUT << GRF_GPIO2C1_SEL_SHIFT);
#else
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

#define GRF_EMMCCORE_CON11 0xff77f02c
#define SGRF_DDR_RGN_CON16 0xff330040
#define SGRF_SLV_SECURE_CON4 0xff33e3d0
void board_init_f(ulong dummy)
{
	struct udevice *pinctrl;
	struct udevice *dev;
	int ret;

#define EARLY_UART
#ifdef EARLY_UART
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	debug_uart_init();
	printascii("U-Boot SPL board init");
#endif

	/*  Emmc clock generator: disable the clock multipilier */
	rk_clrreg(GRF_EMMCCORE_CON11, 0x0ff);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	/*
	 * Disable DDR and SRAM security regions.
	 *
	 * As we are entered from the BootROM, the region from
	 * 0x0 through 0xfffff (i.e. the first MB of memory) will
	 * be protected. This will cause issues with the DW_MMC
	 * driver, which tries to DMA from/to the stack (likely)
	 * located in this range.
	 */
	rk_clrsetreg(SGRF_DDR_RGN_CON16, 0x1FF, 0);
	rk_clrreg(SGRF_SLV_SECURE_CON4, 0x2000);

	secure_timer_init();

	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("Pinctrl init failed: %d\n", ret);
		return;
	}

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return;
	}
}

void spl_board_init(void)
{
	struct udevice *pinctrl;
	int ret;

	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		goto err;
	}

	/* Enable debug UART */
	ret = pinctrl_request_noflags(pinctrl, PERIPH_ID_UART_DBG);
	if (ret) {
		debug("%s: Failed to set up console UART\n", __func__);
		goto err;
	}

	preloader_console_init();
#ifdef CONFIG_ROCKCHIP_SPL_BACK_TO_BROM
	back_to_bootrom();
#endif

	return;
err:
	printf("spl_board_init: Error %d\n", ret);

	/* No way to report error here */
	hang();
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
