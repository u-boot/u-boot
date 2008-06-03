/*
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
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
#include <asm/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialization
 */

int board_init (void)
{
	u32 temp;

	/* Configuration Port Control Register*/
	/* Port A */
	PCONA = 0x3ff;

	/* Port B */
	PCONB = 0xff;
	PDATB = 0xFFFF;

	/* Port C */
	/*
	PCONC = 0xff55ff15;
	PDATC = 0x0;
	PUPC = 0xffff;
	*/

	/* Port D */
	/*
	PCOND = 0xaaaa;
	PUPD = 0xff;
	*/

	/* Port E */
	PCONE = 0x0001aaa9;
	PDATE = 0x0;
	PUPE = 0xff;

	/* Port F */
	PCONF = 0x124955;
	PDATF  = 0xff; /* B2-eth_reset tied high level */
	/*
	PUPF = 0x1e3;
	*/

	/* Port G */
	PUPG = 0x1;
	PCONG = 0x3; /*PG0= EINT0= ETH_INT prepared for linux kernel*/

	INTMSK = 0x03fffeff;
	INTCON = 0x05;

    /*
	Configure chip ethernet interrupt as High level
	Port G EINT 0-7 EINT0 -> CHIP ETHERNET
    */
	temp = EXTINT;
	temp &= ~0x7;
    temp |= 0x1; /*LEVEL_HIGH*/
	EXTINT = temp;

    /*
	Reset SMSC LAN91C96 chip
    */
    temp= PCONF;
    temp |= 0x00000040;
    PCONF = temp;

	/* Reset high */
    temp = PDATF;
    temp |= (1 << 3);
    PDATF = temp;

    /* Short delay */
    for (temp=0;temp<10;temp++)
    {
		/* NOP */
    }

    /* Reset low */
    temp = PDATF;
    temp &= ~(1 << 3);
    PDATF = temp;

	/* arch number MACH_TYPE_MBA44B0 */
	gd->bd->bi_arch_number = MACH_TYPE_S3C44B0;

	/* location of boot parameters */
	gd->bd->bi_boot_params = 0x0c000100;

	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return (0);
}
