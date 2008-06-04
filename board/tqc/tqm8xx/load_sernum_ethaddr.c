/*
 * (C) Copyright 2000, 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <mpc8xx.h>

/*-----------------------------------------------------------------------
 * Process Hardware Information Block:
 *
 * If we boot on a system fresh from factory, check if the Hardware
 * Information Block exists and save the information it contains.
 *
 * The TQM8xxL / TQM82xx Hardware Information Block is defined as
 * follows:
 * - located in first flash bank
 * - starts at offset 0x0003FFC0
 * - size 0x00000040
 *
 * Internal structure:
 * - sequence of ASCII character strings
 * - fields separated by a single space character (0x20)
 * - last field terminated by NUL character (0x00)
 * - remaining space filled with NUL characters (0x00)
 *
 * Fields in Hardware Information Block:
 * 1) Module Type
 * 2) Serial Number
 * 3) First MAC Address
 * 4) Number of additional MAC addresses
 */

void load_sernum_ethaddr (void)
{
	unsigned char *hwi;
	unsigned char  serial [CFG_HWINFO_SIZE];
	unsigned char  ethaddr[CFG_HWINFO_SIZE];
	unsigned short ih, is, ie, part;

	hwi = (unsigned char *)(CFG_FLASH_BASE + CFG_HWINFO_OFFSET);
	ih = is = ie = 0;

	if (*((unsigned long *)hwi) != (unsigned long)CFG_HWINFO_MAGIC) {
		return;
	}

	part = 1;

	/* copy serial # / MAC address */
	while ((hwi[ih] != '\0') && (ih < CFG_HWINFO_SIZE)) {
		if (hwi[ih] < ' ' || hwi[ih] > '~') { /* ASCII strings! */
			return;
		}
		switch (part) {
		default:		/* Copy serial # */
			if (hwi[ih] == ' ') {
				++part;
			}
			serial[is++] = hwi[ih];
			break;
		case 3:			/* Copy MAC address */
			if (hwi[ih] == ' ') {
				++part;
				break;
			}
			ethaddr[ie++] = hwi[ih];
			if ((ie % 3) == 2)
				ethaddr[ie++] = ':';
			break;
		}
		++ih;
	}
	serial[is]  = '\0';
	if (ie && ethaddr[ie-1] == ':')
		--ie;
	ethaddr[ie] = '\0';

	/* set serial# and ethaddr if not yet defined */
	if (getenv("serial#") == NULL) {
		setenv ((char *)"serial#", (char *)serial);
	}

	if (getenv("ethaddr") == NULL) {
		setenv ((char *)"ethaddr", (char *)ethaddr);
	}
}
