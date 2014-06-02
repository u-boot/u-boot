/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <i2c.h>

#include "gsc.h"
#include "ventana_eeprom.h"

/* read ventana EEPROM, check for validity, and return baseboard type */
int
read_eeprom(int bus, struct ventana_board_info *info)
{
	int i;
	int chksum;
	char baseboard;
	int type;
	unsigned char *buf = (unsigned char *)info;

	memset(info, 0, sizeof(*info));

	/*
	 * On a board with a missing/depleted backup battery for GSC, the
	 * board may be ready to probe the GSC before its firmware is
	 * running.  We will wait here indefinately for the GSC/EEPROM.
	 */
	while (1) {
		if (0 == i2c_set_bus_num(bus) &&
		    0 == i2c_probe(GSC_EEPROM_ADDR))
			break;
		mdelay(1);
	}

	/* read eeprom config section */
	if (gsc_i2c_read(GSC_EEPROM_ADDR, 0x00, 1, buf, sizeof(*info))) {
		puts("EEPROM: Failed to read EEPROM\n");
		info->model[0] = 0;
		return GW_UNKNOWN;
	}

	/* sanity checks */
	if (info->model[0] != 'G' || info->model[1] != 'W') {
		puts("EEPROM: Invalid Model in EEPROM\n");
		info->model[0] = 0;
		return GW_UNKNOWN;
	}

	/* validate checksum */
	for (chksum = 0, i = 0; i < sizeof(*info)-2; i++)
		chksum += buf[i];
	if ((info->chksum[0] != chksum>>8) ||
	    (info->chksum[1] != (chksum&0xff))) {
		puts("EEPROM: Failed EEPROM checksum\n");
		info->model[0] = 0;
		return GW_UNKNOWN;
	}

	/* original GW5400-A prototype */
	baseboard = info->model[3];
	if (strncasecmp((const char *)info->model, "GW5400-A", 8) == 0)
		baseboard = '0';

	switch (baseboard) {
	case '0': /* original GW5400-A prototype */
		type = GW54proto;
		break;
	case '1':
		type = GW51xx;
		break;
	case '2':
		type = GW52xx;
		break;
	case '3':
		type = GW53xx;
		break;
	case '4':
		type = GW54xx;
		break;
	default:
		printf("EEPROM: Unknown model in EEPROM: %s\n", info->model);
		type = GW_UNKNOWN;
		break;
	}
	return type;
}
