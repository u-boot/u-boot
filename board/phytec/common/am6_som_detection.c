// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 * Author: Daniel Schultz <d.schultz@phytec.de>
 */

#include <asm/arch/hardware.h>

#include "am6_som_detection.h"

extern struct phytec_eeprom_data eeprom_data;

#if IS_ENABLED(CONFIG_PHYTEC_AM62_SOM_DETECTION) || \
	IS_ENABLED(CONFIG_PHYTEC_AM62A_SOM_DETECTION) || \
	IS_ENABLED(CONFIG_PHYTEC_AM64_SOM_DETECTION)

/* Check if the SoM is actually one of the following products:
 * - phyCORE-AM62x
 * - phyCORE-AM62Ax
 * - phyCORE-AM64x
 *
 * Returns 0 in case it's a known SoM. Otherwise, returns -1.
 */
int phytec_am6_detect(struct phytec_eeprom_data *data)
{
	char *opt;
	u8 som;

	if (!data)
		data = &eeprom_data;

	/* We cannot do the check for early API revisions */
	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return -1;

	som = data->payload.data.data_api2.som_no;
	debug("%s: som id: %u\n", __func__, som);

	opt = phytec_get_opt(data);
	if (!opt)
		return -1;

	if (som == PHYTEC_AM62X_SOM && soc_is_am62x())
		return 0;

	if (som == PHYTEC_AM62AX_SOM && soc_is_am62ax())
		return 0;

	if (som == PHYTEC_AM64X_SOM && soc_is_am64x())
		return 0;

	return -1;
}

static u8 phytec_check_opt(struct phytec_eeprom_data *data, u8 option)
{
	char *opt;

	if (!data)
		data = &eeprom_data;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return PHYTEC_EEPROM_INVAL;

	if (option > 8)
		return PHYTEC_EEPROM_INVAL;

	opt = phytec_get_opt(data);
	if (opt)
		return PHYTEC_GET_OPTION(opt[option]);
	return PHYTEC_EEPROM_INVAL;
}

/*
 * Reads LPDDR4 ram size from EEPROM.
 *
 * returns:
 *  - The size
 *  - PHYTEC_EEPROM_INVAL when the data is invalid.
 */
u8 __maybe_unused phytec_get_am6_ddr_size(struct phytec_eeprom_data *data)
{
	u8 ddr_id = phytec_check_opt(data, 3);

	pr_debug("%s: ddr id: %u\n", __func__, ddr_id);
	return ddr_id;
}

/*
 * Reads SPI-NOR flash size and type from EEPROM.
 *
 * returns:
 *  - PHYTEC_EEPROM_VALUE_X if no SPI is poulated.
 *  - Otherwise a board depended code for the size.
 *  - PHYTEC_EEPROM_INVAL when the data is invalid.
 */
u8 __maybe_unused phytec_get_am6_spi(struct phytec_eeprom_data *data)
{
	u8 spi = phytec_check_opt(data, 5);

	pr_debug("%s: spi: %u\n", __func__, spi);
	return spi;
}

/*
 * Reads Ethernet phy information from EEPROM.
 *
 * returns:
 *  - 0x0 no ethernet phy is populated.
 *  - 0x1 if 10/100/1000 MBit Phy is populated.
 *  - PHYTEC_EEPROM_INVAL when the data is invalid.
 */
u8 __maybe_unused phytec_get_am6_eth(struct phytec_eeprom_data *data)
{
	u8 eth = phytec_check_opt(data, 6);

	pr_debug("%s: eth: %u\n", __func__, eth);
	return eth;
}

/*
 * Reads RTC information from EEPROM.
 *
 * returns:
 *  - 0 if no RTC is poulated.
 *  - 1 if it is populated.
 *  - PHYTEC_EEPROM_INVAL when the data is invalid.
 */
u8 __maybe_unused phytec_get_am6_rtc(struct phytec_eeprom_data *data)
{
	u8 rtc = phytec_check_opt(data, 7);

	pr_debug("%s: rtc: %u\n", __func__, rtc);
	return rtc;
}

#else

inline int __maybe_unused phytec_am6_detect(struct phytec_eeprom_data *data)
{
	return -1;
}

inline u8 __maybe_unused
phytec_get_am6_ddr_size(struct phytec_eeprom_data *data)
{
	return PHYTEC_EEPROM_INVAL;
}

inline u8 __maybe_unused phytec_get_am6_spi(struct phytec_eeprom_data *data)
{
	return PHYTEC_EEPROM_INVAL;
}

inline u8 __maybe_unused phytec_get_am6_eth(struct phytec_eeprom_data *data)
{
	return PHYTEC_EEPROM_INVAL;
}

inline u8 __maybe_unused phytec_get_am6_rtc(struct phytec_eeprom_data *data)
{
	return PHYTEC_EEPROM_INVAL;
}
#endif
