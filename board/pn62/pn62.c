/*
 * (C) Copyright 2002 Wolfgang Grandegger <wg@denx.de>
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
#include <mpc824x.h>
#include <pci.h>

#include "pn62.h"


static int get_serial_number (char *string, int size);
static int get_mac_address (int id, u8 * mac, char *string, int size);

#ifdef CONFIG_SHOW_BOOT_PROGRESS
void show_boot_progress (int phase)
{
	/*
	 * Show phases of the bootm command on the front panel
	 * LEDs and the scratchpad register #3 as well. We use
	 * blinking LEDs for logical "1".
	 */
	if (phase > 0) {
		set_led (8, (phase & 0x1) ? LED_SLOW_CLOCK : LED_0);
		set_led (9, (phase & 0x2) ? LED_SLOW_CLOCK : LED_0);
		set_led (10, (phase & 0x4) ? LED_SLOW_CLOCK : LED_0);
		set_led (11, (phase & 0x8) ? LED_SLOW_CLOCK : LED_0);
	}
	i2155x_write_scrapad (BOOT_STATUS, phase);
	if (phase < 0)
		i2155x_write_scrapad (BOOT_DONE, BOOT_DONE_ERROR);
}
#endif

void show_startup_phase (int phase)
{
	/*
	 * Show the phase of U-Boot startup on the front panel
	 * LEDs and the scratchpad register #3 as well.
	 */
	if (phase > 0) {
		set_led (8, (phase & 0x1) ? LED_1 : LED_0);
		set_led (9, (phase & 0x2) ? LED_1 : LED_0);
		set_led (10, (phase & 0x4) ? LED_1 : LED_0);
		set_led (11, (phase & 0x8) ? LED_1 : LED_0);
	}
	i2155x_write_scrapad (BOOT_STATUS, phase);
	if (phase < 0)
		i2155x_write_scrapad (BOOT_DONE, BOOT_DONE_ERROR);
}

int checkboard (void)
{
	show_startup_phase (1);
	puts ("Board: PN62\n");
	return 0;
}

long int initdram (int board_type)
{
	long size;
	long new_bank0_end;
	long mear1;
	long emear1;

	show_startup_phase (2);

	size = get_ram_size(CFG_SDRAM_BASE, CFG_MAX_RAM_SIZE);

	new_bank0_end = size - 1;
	mear1 = mpc824x_mpc107_getreg (MEAR1);
	emear1 = mpc824x_mpc107_getreg (EMEAR1);
	mear1 = (mear1 & 0xFFFFFF00) |
		((new_bank0_end & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT);
	emear1 = (emear1 & 0xFFFFFF00) |
		((new_bank0_end & MICR_ADDR_MASK) >> MICR_EADDR_SHIFT);
	mpc824x_mpc107_setreg (MEAR1, mear1);
	mpc824x_mpc107_setreg (EMEAR1, emear1);

	return (size);
}

/*
 * Initialize PCI Devices. We rely on auto-configuration.
 */
#ifndef CONFIG_PCI_PNP
#error "CONFIG_PCI_PNP is not defined, please correct!"
#endif

struct pci_controller hose = {
};

void pci_init_board (void)
{
	show_startup_phase (4);
	pci_mpc824x_init (&hose);

	show_startup_phase (5);
	i2155x_init ();
	show_startup_phase (6);
	am79c95x_init ();
	show_startup_phase (7);
}

int misc_init_r (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	char str[20];
	u8 mac[6];

	show_startup_phase (8);
	/*
	 * Get serial number and ethernet addresses if not already defined
	 * and update the board info structure and the environment.
	 */
	if (getenv ("serial#") == NULL &&
		get_serial_number (str, strlen (str)) > 0) {
		setenv ("serial#", str);
	}
	show_startup_phase (9);

	if (getenv ("ethaddr") == NULL &&
		get_mac_address (0, mac, str, sizeof (str)) > 0) {
		setenv ("ethaddr", str);
		memcpy (gd->bd->bi_enetaddr, mac, 6);
	}
	show_startup_phase (10);

#ifdef CONFIG_HAS_ETH1
	if (getenv ("eth1addr") == NULL &&
		get_mac_address (1, mac, str, sizeof (str)) > 0) {
		setenv ("eth1addr", str);
		memcpy (gd->bd->bi_enet1addr, mac, 6);
	}
#endif /* CONFIG_HAS_ETH1 */
	show_startup_phase (11);

	/* Tell everybody that U-Boot is up and runnig */
	i2155x_write_scrapad (0, 0x12345678);
	return (0);
}

static int get_serial_number (char *string, int size)
{
	int i;
	char c;

	if (size < I2155X_VPD_SN_SIZE)
		size = I2155X_VPD_SN_SIZE;
	for (i = 0; i < (size - 1); i++) {
		i2155x_read_vpd (I2155X_VPD_SN_START + i, 1, (uchar *)&c);
		if (c == '\0')
			break;
		string[i] = c;
	}
	string[i] = '\0';			/* make sure it's terminated */

	return i;
}

static int get_mac_address (int id, u8 * mac, char *string, int size)
{
	if (size < 6 * 3)
		return -1;

	i2155x_read_vpd (I2155X_VPD_MAC0_START + 6 * id, 6, mac);
	return sprintf (string, "%02x:%02x:%02x:%02x:%02x:%02x",
				mac[0], mac[1], mac[2],
				mac[3], mac[4], mac[5]);
}
