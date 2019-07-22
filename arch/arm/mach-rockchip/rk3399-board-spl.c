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
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/cru_rk3399.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/periph.h>
#include <asm/arch-rockchip/sys_proto.h>

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
	u32  bootdevice_brom_id = readl(BROM_BOOTSOURCE_ID_ADDR);
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

__weak void rockchip_stimer_init(void)
{
}

__weak int arch_cpu_init(void)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
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
	debug("U-Boot SPL board init\n");
#endif

	arch_cpu_init();

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

	preloader_console_init();
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif
