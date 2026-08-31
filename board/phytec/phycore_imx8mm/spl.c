// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019-2020 PHYTEC Messtechnik GmbH
 * Author: Teresa Remmet <t.remmet@phytec.de>
 */

#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mm_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/sections.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <spl.h>

#include "lpddr4_timing.h"
#include "../common/imx8m_som_detection.h"

#define EEPROM_ADDR		0x51
#define EEPROM_ADDR_FALLBACK	0x59

enum phytec_imx8mm_ddr_eeprom_code {
	INVALID = PHYTEC_EEPROM_INVAL,
	PHYTEC_IMX8MM_DDR_1GB = 1,
	PHYTEC_IMX8MM_DDR_2GB = 3,
	PHYTEC_IMX8MM_DDR_4GB = 5,
};

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	switch (boot_dev_spl) {
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD3_BOOT:
	case MMC3_BOOT:
		return BOOT_DEVICE_MMC2;
	case QSPI_BOOT:
		return BOOT_DEVICE_NOR;
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	default:
		return BOOT_DEVICE_NONE;
	}
}

static void spl_dram_init(void)
{
	int ret;
	enum phytec_imx8mm_ddr_eeprom_code size = PHYTEC_EEPROM_INVAL;
	u8 rev = PHYTEC_EEPROM_INVAL;

	ret = phytec_eeprom_data_setup_fallback(NULL, 0, EEPROM_ADDR,
			EEPROM_ADDR_FALLBACK);
	if (ret && !IS_ENABLED(CONFIG_PHYCORE_IMX8MM_RAM_SIZE_FIX))
		goto out;

	ret = phytec_imx8m_detect(NULL);
	if (!ret)
		phytec_print_som_info(NULL);

	if (IS_ENABLED(CONFIG_PHYCORE_IMX8MM_RAM_SIZE_FIX)) {
		if (IS_ENABLED(CONFIG_PHYCORE_IMX8MM_RAM_SIZE_1GB))
			size = PHYTEC_IMX8MM_DDR_1GB;
		else if (IS_ENABLED(CONFIG_PHYCORE_IMX8MM_RAM_SIZE_2GB))
			size = PHYTEC_IMX8MM_DDR_2GB;
		else if (IS_ENABLED(CONFIG_PHYCORE_IMX8MM_RAM_SIZE_4GB))
			size = PHYTEC_IMX8MM_DDR_4GB;
	} else {
		size = phytec_get_imx8m_ddr_size(NULL);
	}

	if (IS_ENABLED(CONFIG_PHYCORE_IMX8MM_RAM_STATIC_SOM_REV)) {
		if (IS_ENABLED(CONFIG_PHYCORE_IMX8MM_USE_SOM_REV_6))
			rev = 6;
		else if (IS_ENABLED(CONFIG_PHYCORE_IMX8MM_USE_SOM_REV_7))
			rev = 7;
	} else {
		rev = phytec_get_rev(NULL);
	}

	if (rev >= 7 || rev == PHYTEC_EEPROM_INVAL) {
		debug("%s: Using rev7 RAM timings.\n", __func__);
		set_dram_timings_rev7();
	} else {
		debug("%s: Using rev6 RAM timings.\n", __func__);
	}

	switch (size) {
	case PHYTEC_IMX8MM_DDR_1GB:
		set_dram_timings_1gb();
		break;
	case PHYTEC_IMX8MM_DDR_2GB:
		break;
	case PHYTEC_IMX8MM_DDR_4GB:
		set_dram_timings_4gb();
		break;
	default:
		goto out;
	}
	ddr_init(&dram_timing);

	return;
out:
	puts("Could not detect correct RAM size. Fall back to default.\n");
	set_dram_timings_rev7();
	ddr_init(&dram_timing);
}

int board_fit_config_name_match(const char *name)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	arch_cpu_init();

	init_uart_clk(2);

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
