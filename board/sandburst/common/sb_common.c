/*
 *  Copyright (C) 2005 Sandburst Corporation
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <config.h>
#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <spd_sdram.h>
#include <i2c.h>
#include "ppc440gx_i2c.h"
#include "sb_common.h"

DECLARE_GLOBAL_DATA_PTR;

long int fixed_sdram (void);

/*************************************************************************
 *  metrobox_get_master
 *
 *  PRI_N - active low signal.	If the GPIO pin is low we are the master
 *
 ************************************************************************/
int sbcommon_get_master(void)
{
	ppc440_gpio_regs_t *gpio_regs;

	gpio_regs = (ppc440_gpio_regs_t *)CONFIG_SYS_GPIO_BASE;

	if (gpio_regs->in & SBCOMMON_GPIO_PRI_N) {
		return 0;
	}
	else {
		return 1;
	}
}

/*************************************************************************
 *  metrobox_secondary_present
 *
 *  Figure out if secondary/slave board is present
 *
 ************************************************************************/
int sbcommon_secondary_present(void)
{
	ppc440_gpio_regs_t *gpio_regs;

	gpio_regs = (ppc440_gpio_regs_t *)CONFIG_SYS_GPIO_BASE;

	if (gpio_regs->in & SBCOMMON_GPIO_SEC_PRES)
		return 0;
	else
		return 1;
}

/*************************************************************************
 *  sbcommon_get_serial_number
 *
 *  Retrieve the board serial number via the mac address in eeprom
 *
 ************************************************************************/
unsigned short sbcommon_get_serial_number(void)
{
	unsigned char buff[0x100];
	unsigned short sernum;

	/* Get the board serial number from eeprom */
	/* Initialize I2C */
	i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	/* Read 256 bytes in EEPROM */
	i2c_read (0x50, 0, 1, buff, 0x100);

	memcpy(&sernum, &buff[0xF4], 2);
	sernum /= 32;

	return (sernum);
}

/*************************************************************************
 *  sbcommon_fans
 *
 *  Spin up fans 2 & 3 to get some air moving.	OS will take care
 *  of the rest.  This is mostly a precaution...
 *
 *  Assumes i2c bus 1 is ready.
 *
 ************************************************************************/
void sbcommon_fans(void)
{
	/*
	 * Attempt to turn on 2 of the fans...
	 * Need to go through the bridge
	 */
	puts ("FANS:  ");

	/* select fan4 through the bridge */
	i2c_reg_write1(0x73, /* addr */
		       0x00, /* reg */
		       0x08); /* val = bus 4 */

	/* Turn on FAN 4 */
	i2c_reg_write1(0x2e,
		       1,
		       0x80);

	i2c_reg_write1(0x2e,
		       0,
		       0x19);

	/* Deselect bus 4 on the bridge */
	i2c_reg_write1(0x73,
		       0x00,
		       0x00);

	/* select fan3 through the bridge */
	i2c_reg_write1(0x73, /* addr */
		       0x00, /* reg */
		       0x04); /* val = bus 3 */

	/* Turn on FAN 3 */
	i2c_reg_write1(0x2e,
		       1,
		       0x80);

	i2c_reg_write1(0x2e,
		       0,
		       0x19);

	/* Deselect bus 3 on the bridge */
	i2c_reg_write1(0x73,
		       0x00,
		       0x00);

	/* select fan2 through the bridge */
	i2c_reg_write1(0x73, /* addr */
		       0x00, /* reg */
		       0x02); /* val = bus 4 */

	/* Turn on FAN 2 */
	i2c_reg_write1(0x2e,
		       1,
		       0x80);

	i2c_reg_write1(0x2e,
		       0,
		       0x19);

	/* Deselect bus 2 on the bridge */
	i2c_reg_write1(0x73,
		       0x00,
		       0x00);

	/* select fan1 through the bridge */
	i2c_reg_write1(0x73, /* addr */
		       0x00, /* reg */
		       0x01); /* val = bus 0 */

	/* Turn on FAN 1 */
	i2c_reg_write1(0x2e,
		       1,
		       0x80);

	i2c_reg_write1(0x2e,
		       0,
		       0x19);

	/* Deselect bus 1 on the bridge */
	i2c_reg_write1(0x73,
		       0x00,
		       0x00);

	puts ("on\n");

	return;

}

/*************************************************************************
 *  initdram
 *
 *  Initialize sdram
 *
 ************************************************************************/
phys_size_t initdram (int board_type)
{
	long dram_size = 0;

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram ();
#else
	dram_size = fixed_sdram ();
#endif
	return dram_size;
}


/*************************************************************************
 *  testdram
 *
 *
 ************************************************************************/
#if defined(CONFIG_SYS_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CONFIG_SYS_MEMTEST_START;
	uint *pend = (uint *) CONFIG_SYS_MEMTEST_END;
	uint *p;

	printf("Testing SDRAM: ");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("OK\n");
	return 0;
}
#endif

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 *
 *  Assumes:	128 MB, non-ECC, non-registered
 *		PLB @ 133 MHz
 *
 ************************************************************************/
long int fixed_sdram (void)
{
	uint reg;

	/*--------------------------------------------------------------------
	 * Setup some default
	 *------------------------------------------------------------------*/
	mtsdram (SDRAM0_UABBA, 0x00000000);	/* ubba=0 (default)		*/
	mtsdram (SDRAM0_SLIO, 0x00000000);		/* rdre=0 wrre=0 rarw=0		*/
	mtsdram (SDRAM0_DEVOPT, 0x00000000);	/* dll=0 ds=0 (normal)		*/
	mtsdram (SDRAM0_WDDCTR, 0x00000000);	/* wrcp=0 dcd=0			*/
	mtsdram (SDRAM0_CLKTR, 0x40000000);	/* clkp=1 (90 deg wr) dcdt=0	*/

	/*--------------------------------------------------------------------
	 * Setup for board-specific specific mem
	 *------------------------------------------------------------------*/
	/*
	 * Following for CAS Latency = 2.5 @ 133 MHz PLB
	 */
	mtsdram (SDRAM0_B0CR, 0x000a4001); /* SDBA=0x000 128MB, Mode 3, enabled */
	mtsdram (SDRAM0_TR0, 0x410a4012);	/* WR=2	 WD=1 CL=2.5 PA=3 CP=4 LD=2 */
	/* RA=10 RD=3			    */
	mtsdram (SDRAM0_TR1, 0x8080082f);	/* SS=T2 SL=STAGE 3 CD=1 CT=0x02f   */
	mtsdram (SDRAM0_RTR, 0x08200000);	/* Rate 15.625 ns @ 133 MHz PLB	    */
	mtsdram (SDRAM0_CFG1, 0x00000000); /* Self-refresh exit, disable PM    */
	udelay (400);			/* Delay 200 usecs (min)	    */

	/*--------------------------------------------------------------------
	 * Enable the controller, then wait for DCEN to complete
	 *------------------------------------------------------------------*/
	mtsdram (SDRAM0_CFG0, 0x86000000); /* DCEN=1, PMUD=1, 64-bit	    */
	for (;;) {
		mfsdram (SDRAM0_MCSTS, reg);
		if (reg & 0x80000000)
			break;
	}

	return (128 * 1024 * 1024);	/* 128 MB			    */
}
#endif	/* !defined(CONFIG_SPD_EEPROM) */

/*************************************************************************
 *  board_get_enetaddr
 *
 *  Get the ethernet MAC address for the management ethernet from the
 *  strap EEPROM.  Note that is the BASE address for the range of
 *  external ethernet MACs on the board.  The base + 31 is the actual
 *  mgmt mac address.
 *
 ************************************************************************/

void board_get_enetaddr(int macaddr_idx, uchar *enet)
{
	int i;
	unsigned short tmp;
	unsigned char buff[0x100], *cp;

	if (0 == macaddr_idx) {

		/* Initialize I2C */
		i2c_init (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

		/* Read 256 bytes in EEPROM */
		i2c_read (0x50, 0, 1, buff, 0x100);

		cp = &buff[0xF0];

		for (i = 0; i < 6; i++,cp++)
			enet[i] = *cp;

		memcpy(&tmp, &enet[4], 2);
		tmp += 31;
		memcpy(&enet[4], &tmp, 2);

	} else {
		enet[0] = 0x02;
		enet[1] = 0x00;
		enet[2] = 0x00;
		enet[3] = 0x00;
		enet[4] = 0x00;
		if (1 == sbcommon_get_master() ) {
			/* Master/Primary card */
			enet[5] = 0x01;
		} else {
			/* Slave/Secondary card */
			enet [5] = 0x02;
		}
	}

	return;
}

#ifdef CONFIG_POST
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{

	return (ctrlc());
}
#endif
