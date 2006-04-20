/*
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <i2c.h>
#include <da9030.h>
#include <asm/arch/pxa-regs.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

static void init_DA9030(void);

/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	/* arch number of Lubbock-Board mk@tbd: fix this! */
	gd->bd->bi_arch_number = MACH_TYPE_LUBBOCK;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0xa0000100;

	return 0;
}

int board_late_init(void)
{
	setenv("stdout", "serial");
	setenv("stderr", "serial");
	init_DA9030();
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

void i2c_init_board()
{
	CKENB |= (CKENB_4_I2C);

	/* setup I2C GPIO's */
	GPIO32 = 0x801;		/* SCL = Alt. Fkt. 1 */
	GPIO33 = 0x801;		/* SDA = Alt. Fkt. 1 */
}

/* initialize the DA9030 Power Controller */
static void init_DA9030()
{
	uchar addr = (uchar) DA9030_I2C_ADDR, val = 0;

	CKENB |= CKENB_7_GPIO;
	udelay(100);

	/* Rising Edge on EXTON to reset DA9030 */
	GPIO17 = 0x8800;	/* configure GPIO17, no pullup, -down */
	GPDR0 |= (1<<17);	/* GPIO17 is output */
	GSDR0 = (1<<17);
	GPCR0 = (1<<17);	/* drive GPIO17 low */
	GPSR0 = (1<<17);	/* drive GPIO17 high */

#if CFG_DA9030_EXTON_DELAY
	udelay((unsigned long) CFG_DA9030_EXTON_DELAY);	/* wait for DA9030 */
#endif
	GPCR0 = (1<<17);	/* drive GPIO17 low */

	/* reset the watchdog and go active (0xec) */
	val = (SYS_CONTROL_A_HWRES_ENABLE |
	       (0x6<<4) |
	       SYS_CONTROL_A_WDOG_ACTION |
	       SYS_CONTROL_A_WATCHDOG);
	if(i2c_write(addr, SYS_CONTROL_A, 1, &val, 1)) {
		printf("Error accessing DA9030 via i2c.\n");
		return;
	}

	i2c_reg_write(addr, REG_CONTROL_1_97, 0xfd); /* disable LDO1, enable LDO6 */
	i2c_reg_write(addr, LDO2_3, 0xd1);	/* LDO2 =1,9V, LDO3=3,1V */
	i2c_reg_write(addr, LDO4_5, 0xcc);	/* LDO2 =1,9V, LDO3=3,1V */
	i2c_reg_write(addr, LDO6_SIMCP, 0x3e);	/* LDO6=3,2V, SIMCP = 5V support */
	i2c_reg_write(addr, LDO7_8, 0xc9);	/* LDO7=2,7V, LDO8=3,0V */
	i2c_reg_write(addr, LDO9_12, 0xec);	/* LDO9=3,0V, LDO12=3,2V */
	i2c_reg_write(addr, BUCK, 0x0c);	/* Buck=1.2V */
	i2c_reg_write(addr, REG_CONTROL_2_98, 0x7f); /* All LDO'S on 8,9,10,11,12,14 */
	i2c_reg_write(addr, LDO_10_11, 0xcc);	/* LDO10=3.0V  LDO11=3.0V */
	i2c_reg_write(addr, LDO_15, 0xae);	/* LDO15=1.8V, dislock first 3bit */
	i2c_reg_write(addr, LDO_14_16, 0x05);	/* LDO14=2.8V, LDO16=NB */
	i2c_reg_write(addr, LDO_18_19, 0x9c);	/* LDO18=3.0V, LDO19=2.7V */
	i2c_reg_write(addr, LDO_17_SIMCP0, 0x2c); /* LDO17=3.0V, SIMCP=3V support */
	i2c_reg_write(addr, BUCK2_DVC1, 0x9a);	/* Buck2=1.5V plus Update support of 520 MHz */
	i2c_reg_write(addr, REG_CONTROL_2_18, 0x43); /* Ball on */
	i2c_reg_write(addr, MISC_CONTROLB, 0x08); /* session valid enable */
	i2c_reg_write(addr, USBPUMP, 0xc1);	/* start pump, ignore HW signals */

	val = i2c_reg_read(addr, STATUS);
	if(val & STATUS_CHDET)
		printf("Charger detected, turning on LED.\n");
	else {
		printf("No charger detetected.\n");
		/* undervoltage? print error and power down */
	}
}


#if 0
/* reset the DA9030 watchdog */
void hw_watchdog_reset(void)
{
	uchar addr = (uchar) DA9030_I2C_ADDR, val = 0;
	val = i2c_reg_read(addr, SYS_CONTROL_A);
	val |= SYS_CONTROL_A_WATCHDOG;
	i2c_reg_write(addr, SYS_CONTROL_A, val);
}
#endif
