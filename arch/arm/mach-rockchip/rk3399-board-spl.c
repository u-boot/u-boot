// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <ram.h>
#include <spl.h>
#include <spl_gpio.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/sys_proto.h>
#include <dm/pinctrl.h>

void board_return_to_bootrom(void)
{
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);
}

static const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/sdhci@fe330000",
	[BROM_BOOTSOURCE_SPINOR] = "/spi@ff1d0000",
	[BROM_BOOTSOURCE_SD] = "/dwmmc@fe320000",
};

const char *board_spl_was_booted_from(void)
{
	u32  bootdevice_brom_id = readl(RK3399_BROM_BOOTSOURCE_ID_ADDR);
	const char *bootdevice_ofpath = NULL;

	if (bootdevice_brom_id < ARRAY_SIZE(boot_devices))
		bootdevice_ofpath = boot_devices[bootdevice_brom_id];

	if (bootdevice_ofpath)
		debug("%s: brom_bootdevice_id %x maps to '%s'\n",
		      __func__, bootdevice_brom_id, bootdevice_ofpath);
	else
		debug("%s: failed to resolve brom_bootdevice_id %x\n",
		      __func__, bootdevice_brom_id);

	return bootdevice_ofpath;
}

u32 spl_boot_device(void)
{
	u32 boot_device = BOOT_DEVICE_MMC1;

	if (CONFIG_IS_ENABLED(ROCKCHIP_BACK_TO_BROM))
		return BOOT_DEVICE_BOOTROM;

	return boot_device;
}

const char *spl_decode_boot_device(u32 boot_device)
{
	int i;
	static const struct {
		u32 boot_device;
		const char *ofpath;
	} spl_boot_devices_tbl[] = {
		{ BOOT_DEVICE_MMC1, "/dwmmc@fe320000" },
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


void board_init_f(ulong dummy)
{
	struct udevice *pinctrl;
	struct udevice *dev;
	struct rk3399_pmusgrf_regs *sgrf;
	struct rk3399_grf_regs *grf;
	int ret;

#ifdef CONFIG_DEBUG_UART
	debug_uart_init();

# ifdef CONFIG_TARGET_CHROMEBOOK_BOB
	int sum, i;

	/*
	 * Add a delay and ensure that the compiler does not optimise this out.
	 * This is needed since the power rails tail a while to turn on, and
	 * we get garbage serial output otherwise.
	 */
	sum = 0;
	for (i = 0; i < 150000; i++)
		sum += i;
	gru_dummy_function(sum);
#endif /* CONFIG_TARGET_CHROMEBOOK_BOB */

	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	printascii("U-Boot SPL board init\n");
#endif

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
	sgrf = syscon_get_first_range(ROCKCHIP_SYSCON_PMUSGRF);
	rk_clrsetreg(&sgrf->ddr_rgn_con[16], 0x1ff, 0);
	rk_clrreg(&sgrf->slv_secure_con4, 0x2000);

	/*  eMMC clock generator: disable the clock multipilier */
	grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	rk_clrreg(&grf->emmccore_con[11], 0x0ff);

	secure_timer_init();

	ret = uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl);
	if (ret) {
		pr_err("Pinctrl init failed: %d\n", ret);
		return;
	}

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		pr_err("DRAM init failed: %d\n", ret);
		return;
	}
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
