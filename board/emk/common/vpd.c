/*
 * (C) Copyright 2003
 * Reinhard Meyer, EMK Elektronik GmbH, r.meyer@emk-elektronik.de
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

#include <common.h>

/*****************************************************************************
 * read "factory" part of EEPROM and set some environment variables
 *****************************************************************************/
void read_factory_r (void)
{
	/* read 'factory' part of EEPROM */
	uchar buf[81];
	uchar *p;
	uint length;
	uint addr;
	uint len;

	/* get length first */
	addr = CFG_FACT_OFFSET;
	if (eeprom_read (CFG_I2C_FACT_ADDR, addr, buf, 2)) {
	  bailout:
		printf ("cannot read factory configuration\n");
		printf ("be sure to set ethaddr	yourself!\n");
		return;
	}
	length = buf[0] + (buf[1] << 8);
	addr += 2;

	/* sanity check */
	if (length < 20 || length > CFG_FACT_SIZE - 2)
		goto bailout;

	/* read lines */
	while (length > 0) {
		/* read one line */
		len = length > 80 ? 80 : length;
		if (eeprom_read (CFG_I2C_FACT_ADDR, addr, buf, len))
			goto bailout;
		/* mark end of buffer */
		buf[len] = 0;
		/* search end of line */
		for (p = buf; *p && *p != 0x0a; p++);
		if (!*p)
			goto bailout;
		*p++ = 0;
		/* advance to next line start */
		length -= p - buf;
		addr += p - buf;
		/*printf ("%s\n", buf); */
		/* search for our specific entry */
		if (!strncmp ((char *) buf, "[RLA/lan/Ethernet] ", 19)) {
			setenv ("ethaddr", (char *)(buf + 19));
		} else if (!strncmp ((char *) buf, "[BOARD/SERIAL] ", 15)) {
			setenv ("serial#", (char *)(buf + 15));
		} else if (!strncmp ((char *) buf, "[BOARD/TYPE] ", 13)) {
			setenv ("board_id", (char *)(buf + 13));
		}
	}
}
