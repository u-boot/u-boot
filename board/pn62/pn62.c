/*
 * (C) Copyright 2002 Wolfgang Grandegger <wg@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc824x.h>
#include <net.h>
#include <pci.h>
#include <netdev.h>

#include "pn62.h"

DECLARE_GLOBAL_DATA_PTR;

static int get_serial_number (char *string, int size);
static void get_mac_address(int id, u8 *mac);

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

phys_size_t initdram (int board_type)
{
	long size;
	long new_bank0_end;
	long mear1;
	long emear1;

	show_startup_phase (2);

	size = get_ram_size(CONFIG_SYS_SDRAM_BASE, CONFIG_SYS_MAX_RAM_SIZE);

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

	if (!eth_getenv_enetaddr("ethaddr", mac)) {
		get_mac_address(0, mac);
		eth_setenv_enetaddr("ethaddr", mac);
	}
	show_startup_phase (10);

#ifdef CONFIG_HAS_ETH1
	if (!eth_getenv_enetaddr("eth1addr", mac)) {
		get_mac_address(1, mac);
		eth_setenv_enetaddr("eth1addr", mac);
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

static void get_mac_address(int id, u8 *mac)
{
	i2155x_read_vpd (I2155X_VPD_MAC0_START + 6 * id, 6, mac);
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
