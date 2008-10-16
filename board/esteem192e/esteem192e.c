/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 *
 *
 * Modified By Conn Clark to work with Esteem 192E 7/31/00
 *
 */

#include <common.h>
#include <mpc8xx.h>

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 *
	 * active, NOP, read, precharge, NOP */
	0x0F27CC04, 0x0EAECC04, 0x00B98C04, 0x00F74C00,
	0x11FFCC05,		/* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 * NOP, Program
	 */
	0x0F0A8C34, 0x1F354C37,	/* last */

	_NOT_USED_,		/* Not used */

	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 * active, NOP, read, NOP, NOP, NOP, NOP, NOP */
	0x0F37CC04, 0x0EFECC04, 0x00FDCC04, 0x00FFCC00,
	0x00FFCC00, 0x01FFCC00, 0x0FFFCC00, 0x1FFFCC05,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 * active, NOP, write, NOP, precharge, NOP */
	0x0F27CC04, 0x0EAE8C00, 0x01BD4C04, 0x0FFB8C04,
	0x0FF74C04, 0x1FFFCC05,	/* last */
	_NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 * active, NOP, write, NOP, NOP, NOP, NOP, NOP */
	0x0F37CC04, 0x0EFE8C00, 0x00FD4C00, 0x00FFCC00,
	0x00FFCC00, 0x01FFCC04, 0x0FFFCC04, 0x1FFFCC05,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 * precharge, NOP, auto_ref, NOP, NOP, NOP */
	0x0FF74C34, 0x0FFACCB4, 0x0FF5CC34, 0x0FFFCC34,
	0x0FFFCCB4, 0x1FFFCC35,	/* last */
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x0FFB8C00, 0x1FF74C03,	/* last */
	_NOT_USED_, _NOT_USED_
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 */

int checkboard (void)
{
	puts ("Board: Esteem 192E\n");
	return (0);
}

/* ------------------------------------------------------------------------- */


phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size_b0, size_b1;

	/*
	 * Explain frequency of refresh here
	 */

	memctl->memc_mptpr = 0x0200;	/* divide by 32 */

	memctl->memc_mamr = 0x18003112;	/*CONFIG_SYS_MAMR_8COL; */ /* 0x18005112 TODO: explain here */

	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Map cs 2 and 3 to the SDRAM banks 0 and 1 at
	 * preliminary addresses - these have to be modified after the
	 * SDRAM size has been determined.
	 */

	memctl->memc_or2 = CONFIG_SYS_OR2_PRELIM;	/* not defined yet */
	memctl->memc_br2 = CONFIG_SYS_BR2_PRELIM;

	memctl->memc_or3 = CONFIG_SYS_OR3_PRELIM;
	memctl->memc_br3 = CONFIG_SYS_BR3_PRELIM;


	/* perform SDRAM initializsation sequence */
	memctl->memc_mar = 0x00000088;
	memctl->memc_mcr = 0x80004830;	/* SDRAM bank 0 execute 8 refresh */
	memctl->memc_mcr = 0x80004105;	/* SDRAM bank 0 */

	memctl->memc_mcr = 0x80006830;	/* SDRAM bank 1 execute 8 refresh */
	memctl->memc_mcr = 0x80006105;	/* SDRAM bank 1 */

	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL;	/* 0x18803112  start refresh timer TODO: explain here */

/* printf ("banks 0 and 1 are programed\n"); */

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 */
	size_b0 = get_ram_size ( (long *)SDRAM_BASE2_PRELIM, SDRAM_MAX_SIZE);
	size_b1 = get_ram_size ( (long *)SDRAM_BASE3_PRELIM, SDRAM_MAX_SIZE);

	printf ("\nbank 0 size %lu\nbank 1 size %lu\n", size_b0, size_b1);

/* printf ("bank 1 size %u\n",size_b1); */

	if (size_b1 == 0) {
		/*
		 * Adjust refresh rate if bank 0 isn't stuffed
		 */
		memctl->memc_mptpr = 0x0400;	/* divide by 64 */
		memctl->memc_br3 &= 0x0FFFFFFFE;

		/*
		 * Adjust OR2 for size of bank 0
		 */
		memctl->memc_or2 |= 7 * size_b0;
	} else {
		if (size_b0 < size_b1) {
			memctl->memc_br2 &= 0x00007FFE;
			memctl->memc_br3 &= 0x00007FFF;

			/*
			 * Adjust OR3 for size of bank 1
			 */
			memctl->memc_or3 |= 15 * size_b1;

			/*
			 * Adjust OR2 for size of bank 0
			 */
			memctl->memc_or2 |= 15 * size_b0;
			memctl->memc_br2 += (size_b1 + 1);
		} else {
			memctl->memc_br3 &= 0x00007FFE;

			/*
			 * Adjust OR2 for size of bank 0
			 */
			memctl->memc_or2 |= 15 * size_b0;

			/*
			 * Adjust OR3 for size of bank 1
			 */
			memctl->memc_or3 |= 15 * size_b1;
			memctl->memc_br3 += (size_b0 + 1);
		}
	}

	/* before leaving set all unused i/o pins to outputs */

	/*
	 *      --*Unused Pin List*--
	 *
	 * group/port           bit number
	 * IP_B                 0,1,3,4,5  Taken care of in pcmcia-cs-x.x.xx
	 * PA                   5,7,8,9,14,15
	 * PB                   22,23,31
	 * PC                   4,5,6,7,10,11,12,13,14,15
	 * PD                   5,6,7
	 *
	 */

	/*
	 *   --*Pin Used for I/O List*--
	 *
	 * port     input bit number    output bit number    either
	 * PB                           18,26,27
	 * PD       3,4                                      8,9,10,11,12,13,14,15
	 *
	 */

	immap->im_ioport.iop_papar &= ~0x05C3;	/* set pins as io */
	immap->im_ioport.iop_padir |= 0x05C3;	/* set pins as output */
	immap->im_ioport.iop_paodr &= 0x0008;	/* config pins 9 & 14 as normal outputs */
	immap->im_ioport.iop_padat |= 0x05C3;	/* set unused pins as high */

	immap->im_cpm.cp_pbpar &= ~0x00001331;	/* set unused port b pins as io */
	immap->im_cpm.cp_pbdir |= 0x00001331;	/* set unused port b pins as output */
	immap->im_cpm.cp_pbodr &= ~0x00001331;	/* config bits 18,22,23,26,27 & 31 as normal outputs */
	immap->im_cpm.cp_pbdat |= 0x00001331;	/* set T/E LED, /NV_CS, & /POWER_ADJ_CS and the rest to a high */

	immap->im_ioport.iop_pcpar &= ~0x0F3F;	/* set unused port c pins as io */
	immap->im_ioport.iop_pcdir |= 0x0F3F;	/* set unused port c pins as output */
	immap->im_ioport.iop_pcso &= ~0x0F3F;	/* clear special purpose bit for unused port c pins for clarity */
	immap->im_ioport.iop_pcdat |= 0x0F3F;	/* set unused port c pins high */

	immap->im_ioport.iop_pdpar &= 0xE000;	/* set pins as io */
	immap->im_ioport.iop_pddir &= 0xE000;	/* set bit 3 & 4 as inputs */
	immap->im_ioport.iop_pddir |= 0x07FF;	/* set bits 5 - 15 as outputs */
	immap->im_ioport.iop_pddat = 0x0055;	/* set alternating pattern on test port */

	return (size_b0 + size_b1);
}
