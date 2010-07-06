/*
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2004
 * BEC Systems <http://bec-systems.com>
 * Cliff Brake <cliff.brake@gmail.com>
 * Support for Accelent/Vibren PXA255 IDP
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
#include <command.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	/* arch number of Lubbock-Board */
	gd->bd->bi_arch_number = MACH_TYPE_PXA_IDP;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	/* turn on serial ports */
	*(volatile unsigned int *)(PXA_CS5_PHYS + 0x03C0002c) = 0x13;

	/* set PWM for LCD */
	/* a value that works is 60Hz, 77% duty cycle */
	CKEN |= CKEN0_PWM0;
	PWM_CTRL0 = 0x3f;
	PWM_PERVAL0 = 0x3ff;
	PWM_PWDUTY0 = 792;

	/* clear reset to AC97 codec */
	CKEN |= CKEN2_AC97;
	GCR = GCR_COLD_RST;

	/* enable LCD backlight */
	/* *(volatile unsigned int *)(PXA_CS5_PHYS + 0x03C00030) = 0x7; */

	/* test display */
	/* lcd_puts("This is a test\nTest #2\n"); */

	return 0;
}

int board_late_init(void)
{
	setenv("stdout", "serial");
	setenv("stderr", "serial");
	return 0;
}


int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = PHYS_SDRAM_4_SIZE;

	return 0;
}


#ifdef DEBUG_BLINKC_ENABLE

void delay_c(void)
{
	/* reset OSCR to 0 */
	OSCR = 0;
	while(OSCR > 0x10000)
		;

	while(OSCR < 0xd4000)
		;
}

void blink_c(void)
{
	int led_bit = (1<<10);

	GPDR0 = led_bit;
	GPCR0 = led_bit;
	delay_c();
	GPSR0 = led_bit;
	delay_c();
	GPCR0 = led_bit;
}

int do_idpcmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("IDPCMD started\n");
	return 0;
}

U_BOOT_CMD(idpcmd, CONFIG_SYS_MAXARGS, 0, do_idpcmd,
	   "custom IDP command",
	   "no args at this time"
);

#endif

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
