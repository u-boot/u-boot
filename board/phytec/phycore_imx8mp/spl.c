// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 PHYTEC Messtechnik GmbH
 * Author: Teresa Remmet <t.remmet@phytec.de>
 */

#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/iomux-v3.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <power/pmic.h>
#include <power/pca9450.h>
#include <spl.h>

#include "lpddr4_timing.h"
#include "../common/imx8m_som_detection.h"

#define EEPROM_ADDR		0x51
#define EEPROM_ADDR_FALLBACK	0x59

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

enum phytec_imx8mp_ddr_eeprom_code {
	PHYTEC_IMX8MP_DDR_1GB = 2,
	PHYTEC_IMX8MP_DDR_2GB = 3,
	PHYTEC_IMX8MP_DDR_4GB = 5,
	PHYTEC_IMX8MP_DDR_8GB = 7,
	PHYTEC_IMX8MP_DDR_4GB_2GHZ = 8,
};

void spl_dram_init(void)
{
	int ret;
	bool use_2ghz_timings = false;
	enum phytec_imx8mp_ddr_eeprom_code size = PHYTEC_EEPROM_INVAL;

	ret = phytec_eeprom_data_setup_fallback(NULL, 0, EEPROM_ADDR,
						EEPROM_ADDR_FALLBACK);
	if (ret && !IS_ENABLED(CONFIG_PHYCORE_IMX8MP_RAM_SIZE_FIX))
		goto out;

	ret = phytec_imx8m_detect(NULL);
	if (!ret)
		phytec_print_som_info(NULL);

	if (IS_ENABLED(CONFIG_PHYCORE_IMX8MP_RAM_SIZE_FIX)) {
		if (IS_ENABLED(CONFIG_PHYCORE_IMX8MP_RAM_SIZE_1GB))
			size = PHYTEC_IMX8MP_DDR_1GB;
		else if (IS_ENABLED(CONFIG_PHYCORE_IMX8MP_RAM_SIZE_2GB))
			size = PHYTEC_IMX8MP_DDR_2GB;
		else if (IS_ENABLED(CONFIG_PHYCORE_IMX8MP_RAM_SIZE_4GB))
			size = PHYTEC_IMX8MP_DDR_4GB;
		else if (IS_ENABLED(CONFIG_PHYCORE_IMX8MP_RAM_SIZE_8GB))
			size = PHYTEC_IMX8MP_DDR_8GB;
	} else {
		size = phytec_get_imx8m_ddr_size(NULL);
	}

	if (IS_ENABLED(CONFIG_PHYCORE_IMX8MP_RAM_FREQ_FIX)) {
		if (IS_ENABLED(CONFIG_PHYCORE_IMX8MP_USE_2GHZ_RAM_TIMINGS)) {
			if (size == PHYTEC_IMX8MP_DDR_4GB)
				size = PHYTEC_IMX8MP_DDR_4GB_2GHZ;
			else
				use_2ghz_timings = true;
		} else if (IS_ENABLED(CONFIG_PHYCORE_IMX8MP_USE_1_5GHZ_RAM_TIMINGS)) {
			if (size == PHYTEC_IMX8MP_DDR_4GB_2GHZ)
				size = PHYTEC_IMX8MP_DDR_4GB;
			else
				use_2ghz_timings = false;
		}
	} else {
		u8 rev = phytec_get_rev(NULL);
		u8 somtype = phytec_get_som_type(NULL);

		if (rev != PHYTEC_EEPROM_INVAL &&
		    (rev >= 3 || (somtype == SOM_TYPE_PCL && rev >= 1)))
			use_2ghz_timings = true;
	}

	switch (size) {
	case PHYTEC_IMX8MP_DDR_1GB:
		if (use_2ghz_timings)
			set_dram_timings_2ghz_1gb();
		else
			set_dram_timings_1_5ghz_1gb();
		break;
	case PHYTEC_IMX8MP_DDR_2GB:
		if (use_2ghz_timings)
			set_dram_timings_2ghz_2gb();
		break;
	case PHYTEC_IMX8MP_DDR_4GB:
		set_dram_timings_1_5ghz_4gb();
		break;
	case PHYTEC_IMX8MP_DDR_4GB_2GHZ:
		set_dram_timings_2ghz_4gb();
		break;
	case PHYTEC_IMX8MP_DDR_8GB:
		set_dram_timings_2ghz_8gb();
		break;
	default:
		goto out;
	}
	ddr_init(&dram_timing);
	return;
out:
	printf("Could not detect correct RAM size. Fallback to default.\n");
	ddr_init(&dram_timing);
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

	/* Increase VDD_SOC and VDD_ARM to OD voltage 0.95V */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1C);
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1C);

	/* Set BUCK1 DVS1 to suspend controlled through PMIC_STBY_REQ */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Set WDOG_B_CFG to cold reset */
	pmic_reg_write(dev, PCA9450_RESET_CTRL, 0xA1);

	return 0;
}

void spl_board_init(void)
{
	arch_misc_init();

	/* Set GIC clock to 500Mhz for OD VDD_SOC. */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);
}

int board_fit_config_name_match(const char *name)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	arch_cpu_init();

	init_uart_clk(0);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	power_init_board();

	/* DDR initialization */
	spl_dram_init();
}
