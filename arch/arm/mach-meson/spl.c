// SPDX-License-Identifier: GPL-2.0+
/*
 * Portions Copyright (C) 2015, Amlogic, Inc. All rights reserved.
 * Copyright (C) 2023, Ferass El Hafidi <funderscore@postmarketos.org>
 */
#include <spl.h>
#include <hang.h>
#include <asm/io.h>
#include <asm/spl.h>
#include <asm/arch/boot.h>
#include <vsprintf.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <atf_common.h>
#include <image.h>
#include <asm/arch/gx.h>
#include <linux/delay.h>
#include <asm/arch/clock-gx.h>

u32 spl_boot_device(void)
{
	int boot_device = meson_get_boot_device();

	switch (boot_device) {
	case BOOT_DEVICE_EMMC:
		return BOOT_DEVICE_MMC2;
	case BOOT_DEVICE_SD:
		return BOOT_DEVICE_MMC1;
	/*
	 * TODO: Get USB DFU to work
	 * Right now we just panic when booted from USB.
	 */
	case BOOT_DEVICE_USB:
		if (CONFIG_IS_ENABLED(YMODEM_SUPPORT))
			return BOOT_DEVICE_UART;
		else
			return BOOT_DEVICE_DFU;
	}

	panic("Unknown device %d\n", boot_device);
	return BOOT_DEVICE_NONE; /* Never reached */
}

__weak struct legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return (void *)CONFIG_TEXT_BASE + 0x4000000;
}

__weak void *board_spl_fit_buffer_addr(ulong fit_size, int sectors, int bl_len)
{
	/* HACK: use same fit load buffer address as for mmc raw */
	return spl_get_load_buffer(0, fit_size);
}

__weak bool spl_load_simple_fit_skip_processing(void)
{
	return false;
}

/* To be defined in dram-${GENERATION}.c */
__weak int dram_init(void)
{
	return 0;
}

/* Placeholder functions to be defined in SoC-specific spl-... file */
__weak void meson_power_init(void)
{
}

__weak int meson_pll_init(void)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	/* Restart execution at EL3 */
	if (current_el() != 3) {
		struct pt_regs regs = {0};
		static struct entry_point_info spl_ep_info;

		SET_PARAM_HEAD(&spl_ep_info, ATF_PARAM_BL31, ATF_VERSION_1, 0);
		spl_ep_info.pc = CONFIG_SPL_TEXT_BASE;
		spl_ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX, DISABLE_ALL_EXECPTIONS);

		regs.regs[0] = 0xc0000000;
		regs.regs[1] = (unsigned long)&spl_ep_info;
		smc_call(&regs);
	}

	meson_power_init();
	ret = meson_pll_init();
	if (ret) {
		debug("meson_pll_init() failed: %d\n", ret);
		return;
	}

	ret = dram_init();
	if (ret) {
		debug("dram_init() failed: %d\n", ret);
		hang();
	}

	if (CONFIG_IS_ENABLED(OF_CONTROL)) {
		ret = spl_early_init();
		if (ret) {
			debug("spl_early_init() failed: %d\n", ret);
			hang();
		}
	}

	spl_init();
	icache_enable();
	preloader_console_init();

#if !CONFIG_IS_ENABLED(WDT_MESON_GXBB)
	/* Disable watchdog */
	clrbits_32(GX_WDT_CTRL_REG, (1 << 18) | (1 << 25));
#endif
}
