// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 */

#include <stdio.h>
#include <malloc.h>
#include <i2c.h>
#include <extension_board.h>
#include <vsprintf.h>
#include <linux/delay.h>

#include "cape_detect.h"

static void sanitize_field(char *text, size_t size)
{
	char *c = NULL;

	for (c = text; c < text + (int)size; c++) {
		if (*c == 0xFF)
			*c = 0;
	}
}

static int ti_extension_board_scan(struct udevice *dev,
				   struct alist *extension_list)
{
	unsigned char addr;
	int num_capes = 0;

	for (addr = CAPE_EEPROM_FIRST_ADDR; addr <= CAPE_EEPROM_LAST_ADDR; addr++) {
		struct am335x_cape_eeprom_id eeprom_header;
		char process_cape_part_number[17] = {'0'};
		char process_cape_version[5] = {'0'};
		struct extension cape = {0};
		struct udevice *dev;
		u8 cursor = 0;
		int ret, i;

		ret = i2c_get_chip_for_busnum(CONFIG_CAPE_EEPROM_BUS_NUM, addr, 1, &dev);
		if (ret)
			continue;

		/* Move the read cursor to the beginning of the EEPROM */
		dm_i2c_write(dev, 0, &cursor, 1);
		/* Need 5ms (tWR) to complete internal write */
		mdelay(6);
		ret = dm_i2c_read(dev, 0, (uint8_t *)&eeprom_header,
				  sizeof(struct am335x_cape_eeprom_id));
		if (ret) {
			printf("Cannot read i2c EEPROM\n");
			continue;
		}

		if (eeprom_header.header != CAPE_MAGIC)
			continue;

		sanitize_field(eeprom_header.board_name, sizeof(eeprom_header.board_name));
		sanitize_field(eeprom_header.version, sizeof(eeprom_header.version));
		sanitize_field(eeprom_header.manufacturer, sizeof(eeprom_header.manufacturer));
		sanitize_field(eeprom_header.part_number, sizeof(eeprom_header.part_number));

		/* Process cape part_number */
		strlcpy(process_cape_part_number, eeprom_header.part_number,
			sizeof(process_cape_part_number));
		/* Some capes end with '.' */
		for (i = 15; i >= 0; i--) {
			if (process_cape_part_number[i] == '.')
				process_cape_part_number[i] = '\0';
			else
				break;
		}

		/* Process cape version */
		strlcpy(process_cape_version, eeprom_header.version,
			sizeof(process_cape_version));
		for (i = 0; i < 4; i++) {
			if (process_cape_version[i] == 0)
				process_cape_version[i] = '0';
		}

		printf("BeagleBone Cape: %s (0x%x)\n", eeprom_header.board_name, addr);

		snprintf(cape.overlay, sizeof(cape.overlay), "%s-%s.dtbo",
			 process_cape_part_number, process_cape_version);
		strlcpy(cape.name, eeprom_header.board_name,
			sizeof(eeprom_header.board_name));
		strlcpy(cape.version, process_cape_version,
			sizeof(process_cape_version));
		strlcpy(cape.owner, eeprom_header.manufacturer,
			sizeof(eeprom_header.manufacturer) + 1);
		if (!alist_add(extension_list, cape))
			return -ENOMEM;
		num_capes++;
	}
	return num_capes;
}

U_BOOT_EXTENSION(cape, ti_extension_board_scan);

U_BOOT_DRVINFO(cape) = {
	.name	= "cape",
};
