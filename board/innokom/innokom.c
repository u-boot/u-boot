/*
 * (C) Copyright 2002
 * Robert Schwebel, Pengutronix, r.schwebel@pengutronix.de
 * Kyle Harris, Nexus Technologies, Inc., kharris@nexus-tech.net
 * Marius Groeger, Sysgo Real-Time Solutions GmbH, mgroeger@sysgo.de
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
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SHOW_BOOT_PROGRESS
# define SHOW_BOOT_PROGRESS(arg)        show_boot_progress(arg)
#else
# define SHOW_BOOT_PROGRESS(arg)
#endif

/**
 * i2c_init_board - reset i2c bus. When the board is powercycled during a
 * bus transfer it might hang; for details see doc/I2C_Edge_Conditions.
 * The Innokom board has GPIO70 connected to SCLK which can be toggled
 * until all chips think that their current cycles are finished.
 */
int i2c_init_board(void)
{
	int i, icr;

	/* disable I2C controller first, otherwhise it thinks we want to    */
	/* talk to the slave port...                                        */
	icr = ICR; ICR &= ~(ICR_SCLE | ICR_IUE);

	/* set gpio pin low _before_ we change direction to output          */
	GPCR(70) = GPIO_bit(70);

	/* now toggle between output=low and high-impedance                 */
	for (i = 0; i < 20; i++) {
		GPDR(70) |= GPIO_bit(70);  /* output */
		udelay(10);
		GPDR(70) &= ~GPIO_bit(70); /* input  */
		udelay(10);
	}

	ICR = icr;

	return 0;
}


/**
 * misc_init_r: - misc initialisation routines
 */

int misc_init_r(void)
{
	char *str;

	/* determine if the software update key is pressed during startup   */
	if (GPLR0 & 0x00000800) {
		printf("using bootcmd_normal (sw-update button not pressed)\n");
		str = getenv("bootcmd_normal");
	} else {
		printf("using bootcmd_update (sw-update button pressed)\n");
		str = getenv("bootcmd_update");
	}

	setenv("bootcmd",str);

	return 0;
}


/**
 * board_init: - setup some data structures
 *
 * @return: 0 in case of success
 */

int board_init (void)
{
	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	gd->bd->bi_arch_number = MACH_TYPE_INNOKOM;
	gd->bd->bi_boot_params = 0xa0000100;
	gd->bd->bi_baudrate = CONFIG_BAUDRATE;

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
 * innokom_set_led: - switch LEDs on or off
 *
 * @param led:   LED to switch (0,1,2)
 * @param state: switch on (1) or off (0)
 */

void innokom_set_led(int led, int state)
{
	switch(led) {
/*
		case 0: if (state==1) {
				GPCR0 |= CSB226_USER_LED0;
			} else if (state==0) {
				GPSR0 |= CSB226_USER_LED0;
			}
			break;

		case 1: if (state==1) {
				GPCR0 |= CSB226_USER_LED1;
			} else if (state==0) {
				GPSR0 |= CSB226_USER_LED1;
			}
			break;

		case 2: if (state==1) {
				GPCR0 |= CSB226_USER_LED2;
			} else if (state==0) {
				GPSR0 |= CSB226_USER_LED2;
			}
			break;
*/
	}

	return;
}


/**
 * show_boot_progress: - indicate state of the boot process
 *
 * @param status: Status number - see README for details.
 *
 * The CSB226 does only have 3 LEDs, so we switch them on at the most
 * important states (1, 5, 15).
 */

void show_boot_progress (int status)
{
	switch(status) {
/*
		case  1: csb226_set_led(0,1); break;
		case  5: csb226_set_led(1,1); break;
		case 15: csb226_set_led(2,1); break;
*/
	}

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
