// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <spl.h>

#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/mach-imx/boot_mode.h>

#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#include <power/pmic.h>
#include <power/pca9450.h>

#include "lpddr4_timing.h"

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

int data_modul_imx_edm_sbc_board_power_init(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@25", &dev);
	if (ret == -ENODEV) {
		puts("Failed to get PMIC\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output. */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/* Increase VDD_SOC to typical value 0.95V before first DRAM access. */
	if (IS_ENABLED(CONFIG_IMX8M_VDD_SOC_850MV))
		/* Set DVS0 to 0.85V for special case. */
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x14);
	else
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1c);

	/* Set DVS1 to 0.85v for suspend. */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);

	/*
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H).
	 */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Kernel uses OD/OD frequency for SoC. */

	/* To avoid timing risk from SoC to ARM, increase VDD_ARM to OD voltage 0.95V */
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1c);

	/* Set LDO4 and CONFIG2 to enable the I2C level translator. */
	pmic_reg_write(dev, PCA9450_LDO4CTRL, 0x59);
	pmic_reg_write(dev, PCA9450_CONFIG2, 0x1);

	return 0;
}

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	if (boot_dev_spl == SPI_NOR_BOOT)	/* SPI NOR */
		return BOOT_DEVICE_SPI;

	if (boot_dev_spl == MMC3_BOOT)		/* eMMC */
		return BOOT_DEVICE_MMC2;

	return BOOT_DEVICE_MMC1;		/* SD */
}

void board_boot_order(u32 *spl_boot_list)
{
	int boot_device = spl_boot_device();

	spl_boot_list[0] = boot_device;		/* 1:SD 2:eMMC 8:SPI NOR */

	if (boot_device == BOOT_DEVICE_SPI) {		/* SPI, eMMC, SD */
		spl_boot_list[1] = BOOT_DEVICE_MMC2;	/* eMMC */
		spl_boot_list[2] = BOOT_DEVICE_MMC1;	/* SD */
	} else if (boot_device == BOOT_DEVICE_MMC1) {	/* SD, eMMC, SPI */
		spl_boot_list[1] = BOOT_DEVICE_MMC2;	/* eMMC */
		spl_boot_list[2] = BOOT_DEVICE_SPI;	/* SPI */
	} else {					/* eMMC, SPI, SD */
		spl_boot_list[1] = BOOT_DEVICE_SPI;	/* SPI */
		spl_boot_list[2] = BOOT_DEVICE_MMC1;	/* SD */
	}

	spl_boot_list[3] = BOOT_DEVICE_UART;	/* YModem */
	spl_boot_list[4] = BOOT_DEVICE_NONE;
}

static struct dram_timing_info *dram_timing_info[8] = {
	&dmo_imx8mp_sbc_dram_timing_32_32,	/* 32 Gbit x32 */
	NULL,					/* 32 Gbit x16 */
	NULL,					/* 16 Gbit x32 */
	NULL,					/* 16 Gbit x16 */
	NULL,					/* 8 Gbit x32 */
	NULL,					/* 8 Gbit x16 */
	NULL,					/* INVALID */
	NULL,					/* INVALID */
};

void board_init_f(ulong dummy)
{
	dmo_board_init_f(MX8MP_PAD_GPIO1_IO02__WDOG1_WDOG_B, dram_timing_info);
}
