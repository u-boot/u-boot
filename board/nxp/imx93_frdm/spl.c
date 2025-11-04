// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 NXP
 */

#include "lpddr4_timing.h"

#include <init.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/mu.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/trdc.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <dm/uclass-internal.h>
#include <linux/delay.h>
#include <power/pca9450.h>
#include <power/pmic.h>

DECLARE_GLOBAL_DATA_PTR;

#define SRC_DDRC_SW_CTRL		(0x44461020)
#define SRC_DDRPHY_SINGLE_RESET_SW_CTRL	(0x44461424)

static struct _drams {
	u8 mr8;
	struct dram_timing_info *pdram_timing;
	char *name;
} frdm_drams[2] = {
	{0x10, &dram_timing_1GB, "1GB DRAM" },
	{0x18, &dram_timing_2GB, "2GB DRAM" },
};

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_board_init(void)
{
	int ret;

	ret = ele_start_rng();
	if (ret)
		printf("Fail to start RNG: %d\n", ret);

	puts("Normal Boot\n");
}

void spl_dram_init(void)
{
	int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(frdm_drams); i++) {
		struct dram_timing_info *ptiming = frdm_drams[i].pdram_timing;

		printf("DDR: %uMTS\n", ptiming->fsp_msg[0].drate);
		ret = ddr_init(ptiming);
		if (ret == 0) {
			if (lpddr4_mr_read(1, 8) == frdm_drams[i].mr8) {
				printf("found DRAM %s matched\n", frdm_drams[i].name);
				break;
			}

			/* Power down and Power up DDR Mixer */

			/* Clear PwrOkIn via DDRMIX register */
			setbits_32(SRC_DDRPHY_SINGLE_RESET_SW_CTRL, BIT(0));
			/* Power off the DDRMIX */
			setbits_32(SRC_DDRC_SW_CTRL, BIT(31));

			udelay(50);

			/* Power up the DDRMIX */
			clrbits_32(SRC_DDRC_SW_CTRL, BIT(31));
			setbits_32(SRC_DDRC_SW_CTRL, BIT(0));
			udelay(10);
			clrbits_32(SRC_DDRC_SW_CTRL, BIT(0));
			udelay(10);
		}
	}
}

int power_init_board(void)
{
	struct udevice *dev;
	int ret;
	unsigned int val = 0, buck_val;

	ret = pmic_get("pmic@25", &dev);
	if (ret == -ENODEV) {
		puts("No pca9450@25\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/* Enable DVS control through PMIC_STBY_REQ */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	ret = pmic_reg_read(dev, PCA9450_PWR_CTRL);
	if (ret < 0)
		return ret;

	val = ret;

	if (is_voltage_mode(VOLT_LOW_DRIVE)) {
		buck_val = 0x0c; /* 0.8V for Low drive mode */
		printf("PMIC: Low Drive Voltage Mode\n");
	} else if (is_voltage_mode(VOLT_NOMINAL_DRIVE)) {
		buck_val = 0x10; /* 0.85V for Nominal drive mode */
		printf("PMIC: Nominal Voltage Mode\n");
	} else {
		buck_val = 0x14; /* 0.9V for Over drive mode */
		printf("PMIC: Over Drive Voltage Mode\n");
	}

	if (val & PCA9450_REG_PWRCTRL_TOFF_DEB) {
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, buck_val);
		pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, buck_val);
	} else {
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, buck_val + 0x4);
		pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, buck_val + 0x4);
	}

	/* Set standby voltage to 0.65V */
	if (val & PCA9450_REG_PWRCTRL_TOFF_DEB)
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x0);
	else
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x4);

	/* I2C_LT_EN*/
	pmic_reg_write(dev, 0xa, 0x3);
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	timer_init();

	arch_cpu_init();

	board_early_init_f();

	spl_early_init();

	preloader_console_init();

	ret = imx9_probe_mu();
	if (ret) {
		printf("Fail to init Sentinel API\n");
	} else {
		debug("SOC: 0x%x\n", gd->arch.soc_rev);
		debug("LC: 0x%x\n", gd->arch.lifecycle);
	}

	clock_init_late();

	power_init_board();

	if (!is_voltage_mode(VOLT_LOW_DRIVE))
		set_arm_clk(get_cpu_speed_grade_hz());

	/* Init power of mix */
	soc_power_init();

	/* Setup TRDC for DDR access */
	trdc_init();

	/* DDR initialization */
	spl_dram_init();

	/* Put M33 into CPUWAIT for following kick */
	ret = m33_prepare();
	if (!ret)
		printf("M33 prepare ok\n");

	board_init_r(NULL, 0);
}
