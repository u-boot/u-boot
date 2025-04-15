// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 * Author: Primoz Fiser <primoz.fiser@norik.com>
 */

#include <asm/arch/sys_proto.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <i2c.h>
#include <u-boot/crc.h>

#include "imx93_som_detection.h"

extern struct phytec_eeprom_data eeprom_data;

#if IS_ENABLED(CONFIG_PHYTEC_IMX93_SOM_DETECTION)

/* Check if the SoM is actually one of the following products:
 * - i.MX93
 *
 * Returns 0 in case it's a known SoM. Otherwise, returns 1.
 */
u8 __maybe_unused phytec_imx93_detect(struct phytec_eeprom_data *data)
{
	u8 som;

	if (!data)
		data = &eeprom_data;

	/* Early API revisions are not supported */
	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return 1;

	som = data->payload.data.data_api2.som_no;
	debug("%s: som id: %u\n", __func__, som);

	if (som == PHYTEC_IMX93_SOM && is_imx93())
		return 0;

	pr_err("%s: SoM ID does not match. Wrong EEPROM data?\n", __func__);
	return 1;
}

/*
 * Filter PHYTEC i.MX93 SoM options by option index
 *
 * Returns:
 *  - option value
 *  - PHYTEC_EEPROM_INVAL when the data is invalid
 *
 */
u8 __maybe_unused phytec_imx93_get_opt(struct phytec_eeprom_data *data,
				       enum phytec_imx93_option_index idx)
{
	char *opt;
	u8 opt_id;

	if (!data)
		data = &eeprom_data;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return PHYTEC_EEPROM_INVAL;

	opt = phytec_get_opt(data);
	if (opt)
		opt_id = PHYTEC_GET_OPTION(opt[idx]);
	else
		opt_id = PHYTEC_EEPROM_INVAL;

	debug("%s: opt[%d] id: %u\n", __func__, idx, opt_id);
	return opt_id;
}

/*
 * Filter PHYTEC i.MX93 SoM voltage
 *
 * Returns:
 *  - PHYTEC_IMX93_VOLTAGE_1V8 or PHYTEC_IMX93_VOLTAGE_3V3
 *  - PHYTEC_EEPROM_INVAL when the data is invalid
 *
 */
enum phytec_imx93_voltage __maybe_unused phytec_imx93_get_voltage(struct phytec_eeprom_data *data)
{
	u8 option = phytec_imx93_get_opt(data, PHYTEC_IMX93_OPT_FEAT);

	if (option == PHYTEC_EEPROM_INVAL)
		return PHYTEC_IMX93_VOLTAGE_INVALID;
	return (option & 0x01) ? PHYTEC_IMX93_VOLTAGE_1V8 : PHYTEC_IMX93_VOLTAGE_3V3;
}

#else

inline u8 __maybe_unused phytec_imx93_detect(struct phytec_eeprom_data *data)
{
	return 1;
}

inline u8 __maybe_unused phytec_imx93_get_opt(struct phytec_eeprom_data *data,
					      enum phytec_imx93_option_index idx)
{
	return PHYTEC_EEPROM_INVAL;
}

inline enum phytec_imx93_voltage __maybe_unused phytec_imx93_get_voltage
	(struct phytec_eeprom_data *data)
{
	return PHYTEC_EEPROM_INVAL;
}

#endif /* IS_ENABLED(CONFIG_PHYTEC_IMX93_SOM_DETECTION) */
