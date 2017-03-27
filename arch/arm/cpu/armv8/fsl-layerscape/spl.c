/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <fsl_ifc.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
#ifdef CONFIG_SPL_MMC_SUPPORT
	return BOOT_DEVICE_MMC1;
#endif
#ifdef CONFIG_SPL_NAND_SUPPORT
	return BOOT_DEVICE_NAND;
#endif
	return 0;
}

u32 spl_boot_mode(const u32 boot_device)
{
	switch (spl_boot_device()) {
	case BOOT_DEVICE_MMC1:
#ifdef CONFIG_SPL_FAT_SUPPORT
		return MMCSD_MODE_FS;
#else
		return MMCSD_MODE_RAW;
#endif
	case BOOT_DEVICE_NAND:
		return 0;
	default:
		puts("spl: error: unsupported device\n");
		hang();
	}
}

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
#if defined(CONFIG_SECURE_BOOT) && defined(CONFIG_FSL_LSCH2)
	/*
	 * In case of Secure Boot, the IBR configures the SMMU
	 * to allow only Secure transactions.
	 * SMMU must be reset in bypass mode.
	 * Set the ClientPD bit and Clear the USFCFG Bit
	*/
	u32 val;
	val = (in_le32(SMMU_SCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_SCR0, val);
	val = (in_le32(SMMU_NSCR0) | SCR0_CLIENTPD_MASK) & ~(SCR0_USFCFG_MASK);
	out_le32(SMMU_NSCR0, val);
#endif
}

void board_init_f(ulong dummy)
{
	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));
	board_early_init_f();
	timer_init();
#ifdef CONFIG_ARCH_LS2080A
	env_init();
#endif
	get_clocks();

	preloader_console_init();

#ifdef CONFIG_SPL_I2C_SUPPORT
	i2c_init_all();
#endif
	dram_init();
}
#endif
