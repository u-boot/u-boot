// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2023 PHYTEC Messtechnik GmbH
 * Author: Christoph Stoidner <c.stoidner@phytec.de>
 * Copyright (C) 2024 Mathieu Othacehe <m.othacehe@gmail.com>
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 */

#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/mach-imx/boot_mode.h>
#include <env.h>
#include <fdt_support.h>

#include "../common/imx93_som_detection.h"

DECLARE_GLOBAL_DATA_PTR;

#define EEPROM_ADDR            0x50

int board_init(void)
{
	int ret = phytec_eeprom_data_setup(NULL, 2, EEPROM_ADDR);

	if (ret)
		printf("%s: EEPROM data init failed\n", __func__);

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int board_late_init(void)
{
	switch (get_boot_device()) {
	case SD2_BOOT:
		env_set_ulong("mmcdev", 1);
		break;
	case MMC1_BOOT:
		env_set_ulong("mmcdev", 0);
		break;
	default:
		break;
	}

	return 0;
}

static void emmc_fixup(void *blob, struct phytec_eeprom_data *data)
{
	enum phytec_imx93_voltage voltage = phytec_imx93_get_voltage(data);
	int offset;

	if (voltage == PHYTEC_IMX93_VOLTAGE_INVALID)
		goto err;

	if (voltage == PHYTEC_IMX93_VOLTAGE_1V8) {
		offset = fdt_node_offset_by_compat_reg(blob, "fsl,imx93-usdhc",
						       0x42850000);
		if (offset)
			fdt_delprop(blob, offset, "no-1-8-v");
		else
			goto err;
	}

	return;
err:
	printf("Could not detect eMMC VDD-IO. Fall back to default.\n");
}

int board_fix_fdt(void *blob)
{
	struct phytec_eeprom_data data;

	phytec_eeprom_data_setup(&data, 2, EEPROM_ADDR);

	emmc_fixup(blob, &data);

	/* Update dtb clocks for low drive mode */
	if (is_voltage_mode(VOLT_LOW_DRIVE))
		low_drive_freq_update(blob);

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	emmc_fixup(blob, NULL);

	/**
	 * NOTE: VOLT_LOW_DRIVE fixup is done by the ft_system_setup()
	 * in arch/arm/mach-imx/imx9/soc.c for Linux device-tree.
	 */

	return 0;
}
