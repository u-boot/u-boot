/*
 * (C) Copyright 2000, 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2005
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>

#define I2C_CHIP	0x50	/* I2C bus address of onboard EEPROM */
#define I2C_ALEN	1	/* length of EEPROM addresses in bytes */
#define I2C_OFFSET	0x0	/* start address of manufacturere data block
				 * in EEPROM */

/* 64 Byte manufacturer data block in EEPROM */
struct manufacturer_data {
	unsigned int	serial_number;	/* serial number (0...999999) */
	unsigned short	hardware;	/* hardware version (e.g. V1.02) */
	unsigned short	manuf_date;	/* manufacture date (e.g. 25/02) */
	unsigned char	name[20];	/* device name (in CHIP.INI) */
	unsigned char	macadr[6];	/* MAC address */
	signed char	a_kal[4];	/* calibration value for U */
	signed char	i_kal[4];	/* calibration value for I */
	unsigned char	reserve[18];	/* reserved */
	unsigned short	save_nr;	/* save count */
	unsigned short	chksum;		/* checksum */
};


int i2c_read (unsigned char chip, unsigned int addr, int alen,
	      unsigned char *buffer, int len);

/*-----------------------------------------------------------------------
 * Process manufacturer data block in EEPROM:
 *
 * If we boot on a system fresh from factory, check if the manufacturer data
 * in the EEPROM is valid and save some information it contains.
 *
 * CMC manufacturer data is defined as follows:
 *
 * - located in the onboard EEPROM
 * - starts at offset 0x0
 * - size 0x00000040
 *
 * Internal structure: see struct definition
 */

void load_sernum_ethaddr (void)
{
	struct manufacturer_data data;
	char  ethaddr[18];
	char  serial [9];
	unsigned short chksum;
	unsigned char *p;
	unsigned short i, is, id;

#if !defined(CONFIG_HARD_I2C) && !defined(CONFIG_SOFT_I2C)
#error you must define some I2C support (CONFIG_HARD_I2C or CONFIG_SOFT_I2C)
#endif
	if (i2c_read(I2C_CHIP, I2C_OFFSET, I2C_ALEN, (unsigned char *)&data,
		     sizeof(data)) != 0) {
		puts ("Error reading manufacturer data from EEPROM\n");
		return;
	}

	/* check if manufacturer data block is valid  */
	p = (unsigned char *)&data;
	chksum = 0;
	for (i = 0; i < (sizeof(data) - sizeof(data.chksum)); i++)
		chksum += *p++;

	debug ("checksum of manufacturer data block: %#.4x\n", chksum);

	if (chksum != data.chksum) {
		puts ("Error: manufacturer data block has invalid checksum\n");
		return;
	}

	/* copy MAC address */
	is = 0;
	id = 0;
	for (i = 0; i < 6; i++) {
		sprintf (&ethaddr[id], "%02x", data.macadr[is++]);
		id += 2;
		if (is < 6)
			ethaddr[id++] = ':';
	}
	ethaddr[id] = '\0';	/* just to be sure */

	/* copy serial number */
	sprintf (serial, "%d", data.serial_number);

	/* set serial# and ethaddr if not yet defined */
	if (getenv("serial#") == NULL) {
		setenv ("serial#", serial);
	}

	if (getenv("ethaddr") == NULL) {
		setenv ("ethaddr", ethaddr);
	}
}
