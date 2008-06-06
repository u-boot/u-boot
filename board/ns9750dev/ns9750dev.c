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
 * Copyright (C) 2004 by FS Forth-Systeme GmbH.
 * All rights reserved.
 * Markus Pietrek <mpietrek@fsforth.de>
 * derived from omap1610innovator.c
 * @References: [1] NS9750 Hardware Reference/December 2003
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
#if defined(CONFIG_NS9750DEV)
# include <./configs/ns9750dev.h>
# include <./ns9750_bbus.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

void flash__init( void );
void ether__init( void );

static inline void delay( unsigned long loops )
{
	__asm__ volatile ("1:\n"
		"subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}


/***********************************************************************
 * @Function: board_init
 * @Return: 0
 * @Descr: Enables BBUS modules and other devices
 ***********************************************************************/

int board_init( void )
{
	/* Active BBUS modules */
	*get_bbus_reg_addr( NS9750_BBUS_MASTER_RESET ) = 0;

#warning Please register your machine at http://www.arm.linux.org.uk/developer/machines/?action=new
	/* arch number of OMAP 1510-Board */
	/* to be changed for OMAP 1610 Board */
	gd->bd->bi_arch_number = 234;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x10000100;


/* this speeds up your boot a quite a bit.  However to make it
 *  work, you need make sure your kernel startup flush bug is fixed.
 *  ... rkw ...
 */
	icache_enable();

	flash__init();
	ether__init();
	return 0;
}


int misc_init_r (void)
{
	/* currently empty */
	return (0);
}

/******************************
 Routine:
 Description:
******************************/
void flash__init (void)
{
}
/*************************************************************
 Routine:ether__init
 Description: take the Ethernet controller out of reset and wait
			   for the EEPROM load to complete.
*************************************************************/
void ether__init (void)
{
}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

#if CONFIG_NR_DRAM_BANKS > 1
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
#endif
	return 0;
}
