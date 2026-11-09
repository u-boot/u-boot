// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <spi.h>
#include <spi_flash.h>

#include "sf_internal.h"

const struct spi_nor_manufacturer *manufacturers[] = {
#if (IS_ENABLED(CONFIG_SPI_FLASH_ATMEL))
	&spi_nor_atmel,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_DOSILICON))
	&spi_nor_dosilicon,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_EON))
	&spi_nor_eon,
#endif
#if (IS_ENABLED(CONFIG_SPI_FRAM_FUJITSU))
	&spi_nor_fujitsu,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_GIGADEVICE))
	&spi_nor_gigadevice,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_ISSI))
	&spi_nor_issi,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_MACRONIX))
	&spi_nor_macronix,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_PUYA))
	&spi_nor_puya,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_SILICONKAISER))
	&spi_nor_siliconkaiser,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_STMICRO))
	&spi_nor_micron,
	&spi_nor_st,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_SPANSION))
	&spi_nor_spansion,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_SST))
	&spi_nor_sst,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_WINBOND))
	&spi_nor_winbond,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_XMC))
	&spi_nor_xmc,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_XTX))
	&spi_nor_xtx,
#endif
#if (IS_ENABLED(CONFIG_SPI_FLASH_ZBIT))
	&spi_nor_zbit,
#endif
	NULL /* sentinel */
};

const struct flash_info *spi_nor_match_id(struct spi_nor *nor, const u8 *id)
{
	const struct flash_info *part;
	unsigned int i, j;

	for (i = 0; manufacturers[i]; i++) {
		for (j = 0; j < manufacturers[i]->nparts; j++) {
			part = &manufacturers[i]->parts[j];
			if (!memcmp(part->id, id, part->id_len)) {
				nor->manufacturer = manufacturers[i];
				return part;
			}
		}
	}

	return NULL;
}
