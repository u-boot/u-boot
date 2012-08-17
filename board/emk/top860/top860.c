/*
 * (C) Copyright 2003
 * EMK Elektronik GmbH <www.emk-elektronik.de>
 * Reinhard Meyer <r.meyer@emk-elektronik.de>
 *
 * Board specific routines for the TOP860
 *
 * - initialisation
 * - interface to VPD data (mac address, clock speeds)
 * - memory controller
 * - serial io initialisation
 * - ethernet io initialisation
 *
 * -----------------------------------------------------------------
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
#include <commproc.h>
#include <mpc8xx.h>
#include <asm/io.h>

/*****************************************************************************
 * UPM table for 60ns EDO RAM at 25 MHz bus/external clock
 *****************************************************************************/
static const uint edo_60ns_25MHz_tbl[] = {

/* single read   (offset 0x00 in upm ram) */
    0x0ff3fc04,0x08f3fc04,0x00f3fc04,0x00f3fc00,
    0x33f7fc07,0xfffffc05,0xfffffc05,0xfffffc05,
/* burst read    (offset 0x08 in upm ram) */
    0x0ff3fc04,0x08f3fc04,0x00f3fc0c,0x0ff3fc40,
    0x0cf3fc04,0x03f3fc48,0x0cf3fc04,0x03f3fc48,
    0x0cf3fc04,0x03f3fc00,0x3ff7fc07,0xfffffc05,
    0xfffffc05,0xfffffc05,0xfffffc05,0xfffffc05,
/* single write  (offset 0x18 in upm ram) */
    0x0ffffc04,0x08fffc04,0x30fffc00,0xf1fffc07,
    0xfffffc05,0xfffffc05,0xfffffc05,0xfffffc05,
/* burst write   (offset 0x20 in upm ram) */
    0x0ffffc04,0x08fffc00,0x00fffc04,0x03fffc4c,
    0x00fffc00,0x07fffc4c,0x00fffc00,0x0ffffc4c,
    0x00fffc00,0x3ffffc07,0xfffffc05,0xfffffc05,
    0xfffffc05,0xfffffc05,0xfffffc05,0xfffffc05,
/* refresh       (offset 0x30 in upm ram) */
    0xc0fffc04,0x07fffc04,0x0ffffc04,0x0ffffc04,
    0xfffffc05,0xfffffc05,0xfffffc05,0xfffffc05,
    0xfffffc05,0xfffffc05,0xfffffc05,0xfffffc05,
/* exception     (offset 0x3C in upm ram) */
    0xfffffc07,0xfffffc03,0xfffffc05,0xfffffc05,
};

/*****************************************************************************
 * Print Board Identity
 *****************************************************************************/
int checkboard (void)
{
	puts ("Board:"CONFIG_IDENT_STRING"\n");
	return (0);
}

/*****************************************************************************
 * Initialize DRAM controller
 *****************************************************************************/
phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	/*
	 * Only initialize memory controller when running from FLASH.
	 * When running from RAM, don't touch it.
	 */
	if ((ulong) initdram & 0xff000000) {
		volatile uint *addr1, *addr2;
		uint i;

		upmconfig (UPMA, (uint *) edo_60ns_25MHz_tbl,
			   sizeof (edo_60ns_25MHz_tbl) / sizeof (uint));
		memctl->memc_mptpr = 0x0200;
		memctl->memc_mamr = 0x0ca20330;
		memctl->memc_or2 = -CONFIG_SYS_DRAM_MAX | OR_CSNT_SAM;
		memctl->memc_br2 = CONFIG_SYS_DRAM_BASE | BR_MS_UPMA | BR_V;
		/*
		 * Do 8 read accesses to DRAM
		 */
		addr1 = (volatile uint *) 0;
		addr2 = (volatile uint *) 0x00400000;
		for (i = 0; i < 8; i++)
			in_be32(addr1);

		/*
		 * Now check whether we got 4MB or 16MB populated
		 */
		addr1[0] = 0x12345678;
		addr1[1] = 0x9abcdef0;
		addr2[0] = 0xfeedc0de;
		addr2[1] = 0x47110815;
		if (addr1[0] == 0xfeedc0de && addr1[1] == 0x47110815) {
			/* only 4MB populated */
			memctl->memc_or2 = -(CONFIG_SYS_DRAM_MAX / 4) | OR_CSNT_SAM;
		}
	}

	return -(memctl->memc_or2 & 0xffff0000);
}

/*****************************************************************************
 * prepare for FLASH detection
 *****************************************************************************/
void flash_preinit(void)
{
}

/*****************************************************************************
 * finalize FLASH setup
 *****************************************************************************/
void flash_afterinit(uint bank, ulong start, ulong size)
{
}

/*****************************************************************************
 * otherinits after RAM is there and we are relocated to RAM
 * note: though this is an int function, nobody cares for the result!
 *****************************************************************************/
int misc_init_r (void)
{
	/* read 'factory' part of EEPROM */
	extern void read_factory_r (void);
	read_factory_r ();

	return (0);
}
