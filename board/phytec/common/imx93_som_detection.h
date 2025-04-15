/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 * Author: Primoz Fiser <primoz.fiser@norik.com>
 */

#ifndef _PHYTEC_IMX93_SOM_DETECTION_H
#define _PHYTEC_IMX93_SOM_DETECTION_H

#include "phytec_som_detection.h"

#define PHYTEC_IMX93_SOM	77

enum phytec_imx93_option_index {
	PHYTEC_IMX93_OPT_DDR = 0,
	PHYTEC_IMX93_OPT_EMMC = 1,
	PHYTEC_IMX93_OPT_CPU = 2,
	PHYTEC_IMX93_OPT_FREQ = 3,
	PHYTEC_IMX93_OPT_NPU = 4,
	PHYTEC_IMX93_OPT_DISP = 5,
	PHYTEC_IMX93_OPT_ETH = 6,
	PHYTEC_IMX93_OPT_FEAT = 7,
	PHYTEC_IMX93_OPT_TEMP = 8,
	PHYTEC_IMX93_OPT_BOOT = 9,
	PHYTEC_IMX93_OPT_LED = 10,
	PHYTEC_IMX93_OPT_EEPROM = 11,
};

enum phytec_imx93_voltage {
	PHYTEC_IMX93_VOLTAGE_INVALID = PHYTEC_EEPROM_INVAL,
	PHYTEC_IMX93_VOLTAGE_3V3 = 0,
	PHYTEC_IMX93_VOLTAGE_1V8 = 1,
};

enum phytec_imx93_ddr_eeprom_code {
	PHYTEC_IMX93_DDR_INVALID = PHYTEC_EEPROM_INVAL,
	PHYTEC_IMX93_LPDDR4X_512MB = 0,
	PHYTEC_IMX93_LPDDR4X_1GB = 1,
	PHYTEC_IMX93_LPDDR4X_2GB = 2,
	PHYTEC_IMX93_LPDDR4_512MB = 3,
	PHYTEC_IMX93_LPDDR4_1GB = 4,
	PHYTEC_IMX93_LPDDR4_2GB = 5,
};

u8 __maybe_unused phytec_imx93_detect(struct phytec_eeprom_data *data);
u8 __maybe_unused phytec_imx93_get_opt(struct phytec_eeprom_data *data,
				       enum phytec_imx93_option_index idx);
enum phytec_imx93_voltage __maybe_unused phytec_imx93_get_voltage
	(struct phytec_eeprom_data *data);

#endif /* _PHYTEC_IMX93_SOM_DETECTION_H */
