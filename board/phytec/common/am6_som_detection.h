/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 * Author: Daniel Schultz <d.schultz@phytec.de>
 */

#ifndef _PHYTEC_AM6_SOM_DETECTION_H
#define _PHYTEC_AM6_SOM_DETECTION_H

#include "phytec_som_detection.h"

#define EEPROM_ADDR				0x50
#define PHYTEC_AM62X_SOM			71
#define PHYTEC_AM62AX_SOM			75
#define PHYTEC_AM64X_SOM			72
#define PHYTEC_EEPROM_VALUE_X			0x21
#define PHYTEC_EEPROM_NOR_FLASH_64MB_QSPI	0xC

enum {
	EEPROM_RAM_SIZE_512MB = 0,
	EEPROM_RAM_SIZE_1GB = 1,
	EEPROM_RAM_SIZE_2GB = 2,
	EEPROM_RAM_SIZE_4GB = 4
};

int __maybe_unused phytec_am6_detect(struct phytec_eeprom_data *data);
u8 __maybe_unused phytec_get_am6_ddr_size(struct phytec_eeprom_data *data);
u8 __maybe_unused phytec_get_am6_spi(struct phytec_eeprom_data *data);
u8 __maybe_unused phytec_get_am6_eth(struct phytec_eeprom_data *data);
u8 __maybe_unused phytec_get_am6_rtc(struct phytec_eeprom_data *data);

static inline int phytec_am6_is_qspi(struct phytec_eeprom_data *data)
{
	u8 spi = phytec_get_am6_spi(data);

	if (spi == PHYTEC_EEPROM_VALUE_X)
		return 0;
	return spi <= PHYTEC_EEPROM_NOR_FLASH_64MB_QSPI;
}

static inline int phytec_am6_is_ospi(struct phytec_eeprom_data *data)
{
	return phytec_get_am6_spi(data) > PHYTEC_EEPROM_NOR_FLASH_64MB_QSPI;
}
#endif /* _PHYTEC_AM6_SOM_DETECTION_H */
