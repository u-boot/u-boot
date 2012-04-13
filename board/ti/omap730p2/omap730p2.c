/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
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

#include <common.h>
#include <netdev.h>
#if defined(CONFIG_OMAP730)
#include <./configs/omap730.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int test_boot_mode(void);
void spin_up_leds(void);
void flash__init (void);
void ether__init (void);
void set_muxconf_regs (void);
void peripheral_power_enable (void);

#define FLASH_ON_CS0	1
#define FLASH_ON_CS3	0

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
			  "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0" (loops));
}

int test_boot_mode(void)
{
	/* Check for CS0 and CS3 address decode swapping */
	if (*((volatile int *)EMIFS_CONFIG) & 0x00000002)
		return(FLASH_ON_CS3);
	else
		return(FLASH_ON_CS0);
}

/* Toggle backup LED indication */
void toggle_backup_led(void)
{
	static int  backupLEDState = 0;	 /* Init variable so that the LED will be ON the first time */
	volatile unsigned int *IOConfReg;


	IOConfReg = (volatile unsigned int *) ((unsigned int) OMAP730_GPIO_BASE_5 + GPIO_DATA_OUTPUT);

	if (backupLEDState != 0) {
		*IOConfReg &= (0xFFFFEFFF);
		backupLEDState = 0;
	} else {
		*IOConfReg |= (0x00001000);
		backupLEDState = 1;
	}
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	/* arch number of OMAP 730 P2 Board - Same as the Innovator! */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP_PERSEUS2;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x10000100;

	/* Configure MUX settings */
	set_muxconf_regs ();

	peripheral_power_enable ();

	/* Backup LED indication via GPIO_140 -> Red led if MUX correctly setup */
	toggle_backup_led();

	/* Hold GSM in reset until needed */
	*((volatile unsigned short *)M_CTL) &= ~1;

	/*
	 *  CSx timings, GPIO Mux ... setup
	 */

	/* Flash: CS0 timings setup */
	*((volatile unsigned int *) FLASH_CFG_0) = 0x0000fff3;
	*((volatile unsigned int *) FLASH_ACFG_0_1) = 0x00000088;

	/* Ethernet support trough the debug board */
	/* CS1 timings setup */
	*((volatile unsigned int *) FLASH_CFG_1) = 0x0000fff3;
	*((volatile unsigned int *) FLASH_ACFG_0_1) = 0x00000000;

	/* this speeds up your boot a quite a bit.  However to make it
	 *  work, you need make sure your kernel startup flush bug is fixed.
	 *  ... rkw ...
	 */
	icache_enable ();

	flash__init ();
	ether__init ();

	return 0;
}

/******************************
 Routine:
 Description:
******************************/
void flash__init (void)
{
	unsigned int regval;

	regval = *((volatile unsigned int *) EMIFS_CONFIG);
	/* Turn off write protection for flash devices. */
	regval = regval | 0x0001;
	*((volatile unsigned int *) EMIFS_CONFIG) = regval;
}

/*************************************************************
 Routine:ether__init
 Description: take the Ethernet controller out of reset and wait
			   for the EEPROM load to complete.
*************************************************************/
void ether__init (void)
{
#define LAN_RESET_REGISTER 0x0400001c

	*((volatile unsigned short *) LAN_RESET_REGISTER) = 0x0000;
	do {
		*((volatile unsigned short *) LAN_RESET_REGISTER) = 0x0001;
		udelay (100);
	} while (*((volatile unsigned short *) LAN_RESET_REGISTER) != 0x0001);

	do {
		*((volatile unsigned short *) LAN_RESET_REGISTER) = 0x0000;
		udelay (100);
	} while (*((volatile unsigned short *) LAN_RESET_REGISTER) != 0x0000);

#define ETH_CONTROL_REG 0x0400030b

	*((volatile unsigned char *) ETH_CONTROL_REG) &= ~0x01;
	udelay (100);
}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

/******************************************************
 Routine: set_muxconf_regs
 Description: Setting up the configuration Mux registers
			  specific to the hardware
*******************************************************/
void set_muxconf_regs (void)
{
	volatile unsigned int *MuxConfReg;
	/* set each registers to its reset value; */

	/*
	 *  Backup LED Indication
	 */

	/* Configure MUXed pin. Mode 6: GPIO_140 */
	MuxConfReg = (volatile unsigned int *) (PERSEUS2_IO_CONF10);
	*MuxConfReg &= (0xFFFFFF1F);	     /* Clear D_MPU_LPG1 */
	*MuxConfReg |= 0x000000C0;	     /* Set D_MPU_LPG1 to 0x6 */

	/* Configure GPIO_140 as output */
	MuxConfReg = (volatile unsigned int *) ((unsigned int) OMAP730_GPIO_BASE_5 + GPIO_DIRECTION_CONTROL);
	*MuxConfReg &= (0xFFFFEFFF);	     /* Clear direction (output) for GPIO 140 */

	/*
	 * Configure GPIOs for battery charge & feedback
	 */

	/* Configure MUXed pin. Mode 6: GPIO_35 */
	MuxConfReg = (volatile unsigned int *) (PERSEUS2_IO_CONF3);
	*MuxConfReg &= 0xFFFFFFF1;	     /* Clear M_CLK_OUT */
	*MuxConfReg |= 0x0000000C;	     /* Set M_CLK_OUT = 0x6 (GPIOs) */

	/* Configure MUXed pin. Mode 6: GPIO_72,73,74 */
	MuxConfReg = (volatile unsigned int *) (PERSEUS2_IO_CONF5);
	*MuxConfReg &= 0xFFFF1FFF;	     /* Clear D_DDR */
	*MuxConfReg |= 0x0000C000;	     /* Set D_DDR = 0x6 (GPIOs) */

	MuxConfReg = (volatile unsigned int *) ((unsigned int) OMAP730_GPIO_BASE_3 + GPIO_DIRECTION_CONTROL);
	*MuxConfReg |= 0x00000100;	     /* Configure GPIO_72 as input */
	*MuxConfReg &= 0xFFFFFDFF;	     /* Configure GPIO_73 as output	*/

	/*
	 * Allow battery charge
	 */

	MuxConfReg = (volatile unsigned int *) ((unsigned int) OMAP730_GPIO_BASE_3 + GPIO_DATA_OUTPUT);
	*MuxConfReg &= (0xFFFFFDFF);	     /* Clear GPIO_73 pin */

	/*
	 * Configure MPU_EXT_NIRQ IO in IO_CONF9 register,
	 * It is used as the Ethernet controller interrupt
	 */
	MuxConfReg = (volatile unsigned int *) (PERSEUS2_IO_CONF9);
	*MuxConfReg &= 0x1FFFFFFF;
}

/******************************************************
 Routine: peripheral_power_enable
 Description: Enable the power for UART1
*******************************************************/
void peripheral_power_enable (void)
{
	volatile unsigned int *MuxConfReg;


	/* Set up pins used by UART */

	/* Start UART clock (48MHz) */
	MuxConfReg = (volatile unsigned int *) (PERSEUS_PCC_CONF_REG);
	*MuxConfReg &= (0xFFFFFFF7);
	*MuxConfReg |= (0x00000008);

	/* Get the UART pin in mode0  */
	MuxConfReg = (volatile unsigned int *) (PERSEUS2_IO_CONF3);
	*MuxConfReg &= (0xFF1FFFFF);
	*MuxConfReg &= (0xF1FFFFFF);
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_LAN91C96
	rc = lan91c96_initialize(0, CONFIG_LAN91C96_BASE);
#endif
	return rc;
}
#endif
