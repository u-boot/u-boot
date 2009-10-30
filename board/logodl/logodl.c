/*
 * (C) 2002 Kyle Harris <kharris@nexus-tech.net>, Nexus Technologies, Inc.
 * (C) 2002 Marius Groeger <mgroeger@sysgo.de>, Sysgo GmbH
 * (C) 2003 Robert Schwebel <r.schwebel@pengutronix.de>, Pengutronix
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
#include <netdev.h>
#include <asm/arch/pxa-regs.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * board_init: - setup some data structures
 *
 * @return: 0 in case of success
 */

int board_init (void)
{
	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	gd->bd->bi_arch_number = MACH_TYPE_LOGODL;
	gd->bd->bi_boot_params = 0x08000100;
	gd->bd->bi_baudrate = CONFIG_BAUDRATE;

	(*((volatile short*)0x14800000)) = 0xff; /* power on eth0 */
	(*((volatile short*)0x14000000)) = 0xff; /* power on uart */

	return 0;
}


/**
 * dram_init: - setup dynamic RAM
 *
 * @return: 0 in case of success
 */

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}


/**
 * logodl_set_led: - switch LEDs on or off
 *
 * @param led:   LED to switch (0,1)
 * @param state: switch on (1) or off (0)
 */

void logodl_set_led(int led, int state)
{
	switch(led) {

	case 0:
		if (state==1) {
			CONFIG_SYS_LED_A_CR = CONFIG_SYS_LED_A_BIT;
		} else if (state==0) {
			CONFIG_SYS_LED_A_SR = CONFIG_SYS_LED_A_BIT;
		}
		break;

	case 1:
		if (state==1) {
			CONFIG_SYS_LED_B_CR = CONFIG_SYS_LED_B_BIT;
		} else if (state==0) {
			CONFIG_SYS_LED_B_SR = CONFIG_SYS_LED_B_BIT;
		}
		break;
	}

	return;
}


/**
 * show_boot_progress: - indicate state of the boot process
 *
 * @param status: Status number - see README for details.
 *
 * The LOGOTRONIC does only have 2 LEDs, so we switch them on at the most
 * important states (1, 5, 15).
 */

void show_boot_progress (int status)
{
	if (status < -32) status = -1;  /* let things compatible */
	/*
	  switch(status) {
	  case  1: logodl_set_led(0,1); break;
	  case  5: logodl_set_led(1,1); break;
	  case 15: logodl_set_led(2,1); break;
	  }
	*/
	logodl_set_led(0, (status & 1)==1);
	logodl_set_led(1, (status & 2)==2);

	return;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC91111
	rc = smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif
	return rc;
}
#endif
