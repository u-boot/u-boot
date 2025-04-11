// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright (C) 2024 Toradex */

#include <hang.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/mach-imx/boot_mode.h>
#include <dm/device.h>
#include <power/pmic.h>
#include <power/pca9450.h>

#include "lpddr4_timing.h"

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_dram_init(void)
{
	/*
	 * Try configuring for dual rank memory falling back to single rank
	 */
	if (!ddr_init(&dram_timing)) {
		puts("DDR configured as dual rank\n");
		return;
	}

	lpddr4_single_rank_training_patch();
	if (!ddr_init(&dram_timing)) {
		puts("DDR configured as single rank\n");
		return;
	}
	puts("DDR configuration failed\n");
}

void spl_board_init(void)
{
	arch_misc_init();

	/*
	 * Set GIC clock to 500Mhz for OD VDD_SOC. Kernel driver does
	 * not allow to change it. Should set the clock after PMIC
	 * setting done. Default is 400Mhz (system_pll1_800m with div = 2)
	 * set by ROM for ND VDD_SOC
	 */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);

	puts("Normal Boot\n");
}

int power_init_board(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@25", &dev);
	if (ret == -ENODEV) {
		puts("No pmic@25\n");
		return 0;
	}
	if (ret < 0)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/*
	 * Increase VDD_SOC to typical value 0.95V before first
	 * DRAM access, set DVS1 to 0.85V for suspend.
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H)
	 */
	if (IS_ENABLED(CONFIG_IMX8M_VDD_SOC_850MV))
		/* set DVS0 to 0.85v for special case */
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x14);
	else
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1c);

	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/*
	 * Kernel uses OD/OD freq for SOC.
	 * To avoid timing risk from SOC to ARM,increase VDD_ARM to OD
	 * voltage 0.95V.
	 */
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1c);

	/* set LDO4 and CONFIG2 to enable the I2C level translator */
	pmic_reg_write(dev, PCA9450_LDO4CTRL, 0x59);
	pmic_reg_write(dev, PCA9450_CONFIG2, 0x1);

	return 0;
}

/* Do not use BSS area in this phase */
void board_init_f(ulong dummy)
{
	int ret;

	arch_cpu_init();

	init_uart_clk(3);

	ret = spl_early_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	/* PMIC initialization */
	power_init_board();

	/* DDR initialization */
	spl_dram_init();
}
