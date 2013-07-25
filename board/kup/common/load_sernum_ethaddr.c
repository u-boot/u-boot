/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
 * - starts at offset CONFIG_SYS_HWINFO_OFFSET
 * - size CONFIG_SYS_HWINFO_SIZE
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
	unsigned char hwi_stack[CONFIG_SYS_HWINFO_SIZE];
	char *p;

	hwi = (unsigned char *) (CONFIG_SYS_FLASH_BASE + CONFIG_SYS_HWINFO_OFFSET);
	if (*((unsigned long *) hwi) != (unsigned long) CONFIG_SYS_HWINFO_MAGIC) {
		printf ("HardwareInfo not found!\n");
		return;
	}
	memcpy (hwi_stack, hwi, CONFIG_SYS_HWINFO_SIZE);

	/*
	 ** ethaddr
	 */
	var = strstr ((char *)hwi_stack, ETHADDR_TOKEN);
	if (var) {
		var += sizeof (ETHADDR_TOKEN) - 1;
		p = strchr (var, '\r');
		if ((unsigned char *)p < hwi + CONFIG_SYS_HWINFO_SIZE) {
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
		if ((unsigned char *)p < hwi + CONFIG_SYS_HWINFO_SIZE) {
			*p = '\0';
			setenv ("lcd", var);
			*p = '\r';
		}
	}
}
