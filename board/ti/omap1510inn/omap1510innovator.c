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

DECLARE_GLOBAL_DATA_PTR;

static void flash__init (void);
static void ether__init (void);

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
			  "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	/* arch number of OMAP 1510-Board */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP_INNOVATOR;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x10000100;

/* kk - this speeds up your boot a quite a bit.  However to make it
 *  work, you need make sure your kernel startup flush bug is fixed.
 *  ... rkw ...
 */
	icache_enable ();

	flash__init ();
	ether__init ();
	return 0;
}


int misc_init_r (void)
{
	/* volatile ushort *gdir = (ushort *) (GPIO_DIR_CONTROL_REG); */
	/* volatile ushort *mdir = (ushort *) (MPUIO_DIR_CONTROL_REG); */

	/* setup gpio direction to match board (no floats!) */
	/**gdir = 0xCFF9; */
	/**mdir = 0x103F; */

	return (0);
}

/******************************
 Routine:
 Description:
******************************/
static void flash__init (void)
{
#define CS0_CHIP_SELECT_REG 0xfffecc10
#define CS3_CHIP_SELECT_REG 0xfffecc1c
#define EMIFS_GlB_Config_REG 0xfffecc0c

	{
		unsigned int regval;

		regval = *((volatile unsigned int *) EMIFS_GlB_Config_REG);
		regval = regval | 0x0001;	/* Turn off write protection for flash devices. */
		if (regval & 0x0002) {
			regval = regval & 0xfffd;	/* Swap CS0 and CS3 so that flash is visible at 0x0 and eeprom at 0x0c000000. */
			/* If, instead, you want to reference flash at 0x0c000000, then it seemed the following were necessary. */
			/* *((volatile unsigned int *)CS0_CHIP_SELECT_REG) = 0x202090; / * Overrides head.S setting of 0x212090 */
			/* *((volatile unsigned int *)CS3_CHIP_SELECT_REG) = 0x202090; / * Let's flash chips be fully functional. */
		}
		*((volatile unsigned int *) EMIFS_GlB_Config_REG) = regval;
	}
}


/******************************
 Routine:
 Description:
******************************/
static void ether__init (void)
{
#define ETH_CONTROL_REG 0x0800000b
	/* take the Ethernet controller out of reset and wait
	 * for the EEPROM load to complete.
	 */
	*((volatile unsigned char *) ETH_CONTROL_REG) &= ~0x01;
	udelay (3);
}


int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
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
