/*
 * (C) Copyright 2005
 * Ladislav Michl, 2N Telekomunikace, michl@2n.cz
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
 *
 * Some code shamelessly stolen back from Robin Getz.
 */

#define DEBUG

#include <common.h>
#include <exports.h>
#include "../drivers/smc91111.h"

#define SMC_BASE_ADDRESS CONFIG_SMC91111_BASE

static int verify_macaddr(char *);
static int set_mac(char *);

int eeprom(int argc, char *argv[])
{
	app_startup(argv);
	if (get_version() != XF_VERSION) {
		printf("Wrong XF_VERSION.\n");
		printf("Application expects ABI version %d\n", XF_VERSION);
		printf("Actual U-Boot ABI version %d\n", (int)get_version());
		return 1;
	}

	if ((SMC_inw (BANK_SELECT) & 0xFF00) != 0x3300) {
		printf("SMSC91111 not found.\n");
		return 2;
	}

	if (argc != 2) {
		printf("VoiceBlue EEPROM writer\n");
		printf("Built: %s at %s\n", __DATE__ , __TIME__ );
		printf("Usage:\n\t<mac_address>");
		return 3;
	}

	set_mac(argv[1]);
	if (verify_macaddr(argv[1])) {
		printf("*** ERROR ***\n");
		return 4;
	}

	return 0;
}

static u16 read_eeprom_reg(u16 reg)
{
	int timeout;

	SMC_SELECT_BANK(2);
	SMC_outw(reg, PTR_REG);

	SMC_SELECT_BANK(1);
	SMC_outw(SMC_inw (CTL_REG) | CTL_EEPROM_SELECT | CTL_RELOAD,
		 CTL_REG);
	timeout = 100;
	while((SMC_inw (CTL_REG) & CTL_RELOAD) && --timeout)
		udelay(100);
	if (timeout == 0) {
		printf("Timeout Reading EEPROM register %02x\n", reg);
		return 0;
	}

	return SMC_inw (GP_REG);
}

static int write_eeprom_reg(u16 value, u16 reg)
{
	int timeout;

	SMC_SELECT_BANK(2);
	SMC_outw(reg, PTR_REG);

	SMC_SELECT_BANK(1);
	SMC_outw(value, GP_REG);
	SMC_outw(SMC_inw (CTL_REG) | CTL_EEPROM_SELECT | CTL_STORE, CTL_REG);
	timeout = 100;
	while ((SMC_inw(CTL_REG) & CTL_STORE) && --timeout)
		udelay (100);
	if (timeout == 0) {
		printf("Timeout Writing EEPROM register %02x\n", reg);
		return 0;
	}

	return 1;
}

static int verify_macaddr(char *s)
{
	u16 reg;
	int i, err = 0;

	printf("Verifying MAC Address: ");
	err = i = 0;
	for (i = 0; i < 3; i++) {
		reg = read_eeprom_reg(0x20 + i);
		printf("%02x:%02x%c", reg & 0xff, reg >> 8, i != 2 ? ':' : '\n');
		err |= reg != ((u16 *)s)[i];
	}

	return err ? 0 : 1;
}

static int set_mac(char *s)
{
	int i;
	char *e, eaddr[6];

	/* turn string into mac value */
	for (i = 0; i < 6; i++) {
		eaddr[i] = simple_strtoul(s, &e, 16);
		s = (*e) ? e+1 : e;
	}

	for (i = 0; i < 3; i++)
		write_eeprom_reg(*(((u16 *)eaddr) + i), 0x20 + i);

	return 0;
}
