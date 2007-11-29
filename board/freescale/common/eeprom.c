/*
 * Copyright 2004 Freescale Semiconductor.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>
#include <i2c.h>

#include "eeprom.h"


typedef struct {
	char idee_pcbid[4];		/* "CCID" for CDC v1.X */
	u8 idee_major;
	u8 idee_minor;
	char idee_serial[10];
	char idee_errata[2];
	char idee_date[8];		/* yyyymmdd */
	/* The rest of the EEPROM space is reserved */
} id_eeprom_t;


unsigned int
get_cpu_board_revision(void)
{
	uint major = 0;
	uint minor = 0;

	id_eeprom_t id_eeprom;

	i2c_read(CFG_I2C_EEPROM_ADDR, 0, 2,
		 (uchar *) &id_eeprom, sizeof(id_eeprom));

	major = id_eeprom.idee_major;
	minor = id_eeprom.idee_minor;

	if (major == 0xff && minor == 0xff) {
		major = minor = 0;
	}

	return MPC85XX_CPU_BOARD_REV(major,minor);
}
