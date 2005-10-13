/*
 * (C) Copyright 2000-2004
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
 * The KUP Hardware Information Block is defined as
 * follows:
 * - located in first flash bank
 * - starts at offset CFG_HWINFO_OFFSET
 * - size CFG_HWINFO_SIZE
 *
 * Internal structure:
 * - sequence of ASCII character lines
 * - fields separated by <CR><LF>
 * - last field terminated by NUL character (0x00)
 *
 * Fields in Hardware Information Block:
 * 1) Module Type
 * 2) MAC Address
 * 3) ....
 */


#define ETHADDR_TOKEN "ethaddr="
#define LCD_TOKEN "lcd="

void load_sernum_ethaddr (void)
{
	unsigned char *hwi;
	char *var;
	unsigned char hwi_stack[CFG_HWINFO_SIZE];
	char *p;

	hwi = (unsigned char *) (CFG_FLASH_BASE + CFG_HWINFO_OFFSET);
	if (*((unsigned long *) hwi) != (unsigned long) CFG_HWINFO_MAGIC) {
		printf ("HardwareInfo not found!\n");
		return;
	}
	memcpy (hwi_stack, hwi, CFG_HWINFO_SIZE);

	/*
	 ** ethaddr
	 */
	var = strstr ((char *)hwi_stack, ETHADDR_TOKEN);
	if (var) {
		var += sizeof (ETHADDR_TOKEN) - 1;
		p = strchr (var, '\r');
		if ((unsigned char *)p < hwi + CFG_HWINFO_SIZE) {
			*p = '\0';
			setenv ("ethaddr", var);
			*p = '\r';
		}
	}
	/*
	 ** lcd
	 */
	var = strstr ((char *)hwi_stack, LCD_TOKEN);
	if (var) {
		var += sizeof (LCD_TOKEN) - 1;
		p = strchr (var, '\r');
		if ((unsigned char *)p < hwi + CFG_HWINFO_SIZE) {
			*p = '\0';
			setenv ("lcd", var);
			*p = '\r';
		}
	}
}
