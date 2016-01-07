/*
 * Copyright (C) 2014-2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <debug_uart.h>
#include <fdtdec.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

static u32 get_boot_device(void)
{
	u32 val;
	u32 boot_device;

	/*
	 * First check, if UART boot-mode is active. This can only
	 * be done, via the bootrom error register. Here the
	 * MSB marks if the UART mode is active.
	 */
	val = readl(CONFIG_BOOTROM_ERR_REG);
	boot_device = (val & BOOTROM_ERR_MODE_MASK) >> BOOTROM_ERR_MODE_OFFS;
	debug("BOOTROM_REG=0x%08x boot_device=0x%x\n", val, boot_device);
	if (boot_device == BOOTROM_ERR_MODE_UART)
		return BOOT_DEVICE_UART;

	/*
	 * Now check the SAR register for the strapped boot-device
	 */
	val = readl(CONFIG_SAR_REG);	/* SAR - Sample At Reset */
	boot_device = (val & BOOT_DEV_SEL_MASK) >> BOOT_DEV_SEL_OFFS;
	debug("SAR_REG=0x%08x boot_device=0x%x\n", val, boot_device);
	switch (boot_device) {
#ifdef CONFIG_SPL_MMC_SUPPORT
	case BOOT_FROM_MMC:
	case BOOT_FROM_MMC_ALT:
		return BOOT_DEVICE_MMC1;
#endif
	case BOOT_FROM_UART:
		return BOOT_DEVICE_UART;
	case BOOT_FROM_SPI:
	default:
		return BOOT_DEVICE_SPI;
	};
}

u32 spl_boot_device(void)
{
	return get_boot_device();
}

#ifdef CONFIG_SPL_MMC_SUPPORT
u32 spl_boot_mode(void)
{
	return MMCSD_MODE_RAW;
}
#endif

void board_init_f(ulong dummy)
{
	int ret;

	/*
	 * Pin muxing needs to be done before UART output, since
	 * on A38x the UART pins need some re-muxing for output
	 * to work.
	 */
	board_early_init_f();

	/* Example code showing how to enable the debug UART on MVEBU */
#ifdef EARLY_UART
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
#endif

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	/* Use special translation offset for SPL */
	dm_set_translation_offset(0xd0000000 - 0xf1000000);

	preloader_console_init();

	timer_init();

	/* First init the serdes PHY's */
	serdes_phy_config();

	/* Setup DDR */
	ddr3_init();

	/*
	 * Return to the BootROM to continue the Marvell xmodem
	 * UART boot protocol. As initiated by the kwboot tool.
	 *
	 * This can only be done by the BootROM and not by the
	 * U-Boot SPL infrastructure, since the beginning of the
	 * image is already read and interpreted by the BootROM.
	 * SPL has no chance to receive this information. So we
	 * need to return to the BootROM to enable this xmodem
	 * UART download.
	 */
	if (get_boot_device() == BOOT_DEVICE_UART)
		return_to_bootrom();
}
