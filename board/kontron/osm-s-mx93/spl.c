// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Kontron Electronics GmbH
 */

#include <asm/arch/imx93_pins.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <power/pca9450.h>
#include <power/pmic.h>
#include <spl.h>
#include <asm/sections.h>
#include <asm/arch/trdc.h>
#include <asm/arch/ccm_regs.h>

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	if (IS_ENABLED(CONFIG_SPL_BOOTROM_SUPPORT))
		return BOOT_DEVICE_BOOTROM;

	switch (boot_dev_spl) {
	case SD1_BOOT:
	case MMC1_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC2;
	default:
		return BOOT_DEVICE_NONE;
	}
}

bool check_ram_available(long size)
{
	long sz = get_ram_size((long *)PHYS_SDRAM, size);

	if (sz == size)
		return true;

	return false;
}

static void spl_dram_init(void)
{
	if (ddr_init(&dram_timing) || !check_ram_available(SZ_2G))
		printf("Failed to initialize DDR RAM!\n");

	printf("DDR:   LPDDR4 initialized (2GB)\n");
}

void spl_board_init(void)
{
	puts("Normal Boot\n");
}

static int power_init_board(void)
{
	struct udevice *dev;
	int ret  = pmic_get("pmic@25", &dev);

	if (ret == -ENODEV)
		puts("No pmic found\n");

	if (ret)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/* enable DVS control through PMIC_STBY_REQ */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* 0.9v for Over drive mode */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x18);
	pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, 0x18);

	/* set standby voltage to 0.65v */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x4);

	/* I2C_LT_EN*/
	pmic_reg_write(dev, 0xa, 0x3);

	return 0;
}

const char *spl_board_loader_name(u32 boot_device)
{
	static char name[16];
	struct mmc *mmc;

	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		mmc_init_device(0);
		mmc = find_mmc_device(0);
		mmc_init(mmc);
		snprintf(name, sizeof(name), "eMMC %s",
			 emmc_hwpart_names[EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config)]);
		return name;
	case BOOT_DEVICE_MMC2:
		sprintf(name, "SD card");
		return name;
	}

	return NULL;
}

extern int imx9_probe_mu(void);
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

	power_init_board();

	if (!IS_ENABLED(CONFIG_IMX9_LOW_DRIVE_MODE))
		set_arm_clk(get_cpu_speed_grade_hz());

	/* Init power of mix */
	soc_power_init();

	/* Setup TRDC for DDR access */
	trdc_init();

	/* DDR initialization */
	spl_dram_init();

	/* Put M33 into CPUWAIT for following kick */
	ret = m33_prepare();
	if (ret)
		printf("M33 prepare failed!\n");

	board_init_r(NULL, 0);
}
