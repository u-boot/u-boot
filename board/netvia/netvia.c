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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Pantelis Antoniou, Intracom S.A., panto@intracom.gr
 * U-Boot port on NetVia board
 */

#include <common.h>
#include "mpc8xx.h"

/* some sane bit macros */
#define _BD(_b)				(1U << (31-(_b)))
#define _BDR(_l, _h)			(((((1U << (31-(_l))) - 1) << 1) | 1) & ~((1U << (31-(_h))) - 1))

#define _BW(_b)				(1U << (15-(_b)))
#define _BWR(_l, _h)			(((((1U << (15-(_l))) - 1) << 1) | 1) & ~((1U << (15-(_h))) - 1))

#define _BB(_b)				(1U << (7-(_b)))
#define _BBR(_l, _h)			(((((1U << (7-(_l))) - 1) << 1) | 1) & ~((1U << (7-(_h))) - 1))

#define _B(_b)				_BD(_b)
#define _BR(_l, _h)			_BDR(_l, _h)

/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */

#define _NOT_USED_	0xFFFFFFFF

/* ------------------------------------------------------------------------- */

#define CS_0000		0x00000000
#define CS_0001		0x10000000
#define CS_0010		0x20000000
#define CS_0011		0x30000000
#define CS_0100		0x40000000
#define CS_0101		0x50000000
#define CS_0110		0x60000000
#define CS_0111		0x70000000
#define CS_1000		0x80000000
#define CS_1001		0x90000000
#define CS_1010		0xA0000000
#define CS_1011		0xB0000000
#define CS_1100		0xC0000000
#define CS_1101		0xD0000000
#define CS_1110		0xE0000000
#define CS_1111		0xF0000000

#define BS_0000		0x00000000
#define BS_0001		0x01000000
#define BS_0010		0x02000000
#define BS_0011		0x03000000
#define BS_0100		0x04000000
#define BS_0101		0x05000000
#define BS_0110		0x06000000
#define BS_0111		0x07000000
#define BS_1000		0x08000000
#define BS_1001		0x09000000
#define BS_1010		0x0A000000
#define BS_1011		0x0B000000
#define BS_1100		0x0C000000
#define BS_1101		0x0D000000
#define BS_1110		0x0E000000
#define BS_1111		0x0F000000

#define A10_AAAA	0x00000000
#define A10_AAA0	0x00200000
#define A10_AAA1	0x00300000
#define A10_000A	0x00800000
#define A10_0000	0x00A00000
#define A10_0001	0x00B00000
#define A10_111A	0x00C00000
#define A10_1110	0x00E00000
#define A10_1111	0x00F00000

#define RAS_0000	0x00000000
#define RAS_0001	0x00040000
#define RAS_1110	0x00080000
#define RAS_1111	0x000C0000

#define CAS_0000	0x00000000
#define CAS_0001	0x00010000
#define CAS_1110	0x00020000
#define CAS_1111	0x00030000

#define WE_0000		0x00000000
#define WE_0001		0x00004000
#define WE_1110		0x00008000
#define WE_1111		0x0000C000

#define GPL4_0000	0x00000000
#define GPL4_0001	0x00001000
#define GPL4_1110	0x00002000
#define GPL4_1111	0x00003000

#define GPL5_0000	0x00000000
#define GPL5_0001	0x00000400
#define GPL5_1110	0x00000800
#define GPL5_1111	0x00000C00
#define LOOP		0x00000080

#define EXEN		0x00000040

#define AMX_COL		0x00000000
#define AMX_ROW		0x00000020
#define AMX_MAR		0x00000030

#define NA		0x00000008

#define UTA		0x00000004

#define TODT		0x00000002

#define LAST		0x00000001

const uint sdram_table[0x40] = {
	/* RSS */
	CS_0001 | BS_1111 | A10_AAAA | RAS_0001 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* ACT   */
	CS_1111 | BS_1111 | A10_0000 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* NOP   */
	CS_0000 | BS_1111 | A10_0001 | RAS_1111 | CAS_0001 | WE_1111 | AMX_COL | UTA,			/* READ  */
	CS_0001 | BS_0001 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL | UTA,			/* PALL  */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP   */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA | TODT | LAST,	/* NOP   */
	_NOT_USED_, _NOT_USED_,

	/* RBS */
	CS_0001 | BS_1111 | A10_AAAA | RAS_0001 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* ACT   */
	CS_1111 | BS_1111 | A10_0000 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* NOP   */
	CS_0001 | BS_1111 | A10_0001 | RAS_1111 | CAS_0001 | WE_1111 | AMX_COL | UTA,			/* READ  */
	CS_1111 | BS_0000 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* NOP   */
	CS_1111 | BS_0000 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP	 */
	CS_1111 | BS_0000 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP	 */
	CS_0001 | BS_0001 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL,				/* PALL  */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | TODT | LAST,		/* NOP	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* WSS */
	CS_0001 | BS_1111 | A10_AAAA | RAS_0001 | CAS_1111 | WE_1111 | AMX_COL | UTA,
	CS_1111 | BS_1111 | A10_0000 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,
	CS_0000 | BS_0001 | A10_0000 | RAS_1111 | CAS_0001 | WE_0000 | AMX_COL | UTA,
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL | UTA | TODT | LAST,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* WBS */
	CS_0001 | BS_1111 | A10_AAAA | RAS_0001 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* ACT   */
	CS_1111 | BS_1111 | A10_0000 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP   */
	CS_0001 | BS_0000 | A10_0000 | RAS_1111 | CAS_0001 | WE_0000 | AMX_COL,				/* WRITE */
	CS_1111 | BS_0000 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP   */
	CS_1111 | BS_0000 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP   */
	CS_1111 | BS_0001 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* NOP   */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* NOP   */
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL | UTA,			/* PALL  */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA | TODT | LAST,	/* NOP   */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* UPT */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP   */
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_0001 | WE_1111 | AMX_COL | LOOP,
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | LOOP,
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | LAST,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_,

	/* EXC */
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL,
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | TODT | LAST,

	/* REG */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1110 | AMX_MAR,
	CS_0001 | BS_1111 | A10_0001 | RAS_0001 | CAS_0001 | WE_0001 | AMX_MAR | TODT | LAST,
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 * Test ETX ID string (ETX_xxx...)
 *
 * Return 1 always.
 */

int checkboard(void)
{
	printf ("NETVIA\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

/* 0xC8 = 0b11001000 , CAS3, >> 2 = 0b00 11 0 010 */
#define MAR_SDRAM_INIT		0x000000C8LU

#define MCR_OP(x)		((unsigned long)((x) & 3) << (31-1))
#define MCR_OP_MASK		MCR_OP(3)

#define MCR_UM(x)		((unsigned long)((x) & 1) << (31 - 8))
#define MCR_UM_MASK		MCR_UM(1)
#define MCR_UM_UPMA		MCR_UM(0)
#define MCR_UM_UPMB		MCR_UM(1)

#define MCR_MB(x)		((unsigned long)((x) & 7) << (31 - 18))
#define MCR_MB_MASK		MCR_MB(7)
#define MCR_MB_CS(x)		MCR_MB(x)

#define MCR_MCLF(x)		((unsigned long)((x) & 15) << (31 - 23))
#define MCR_MCLF_MASK		MCR_MCLF(15)

long int initdram(int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size;

	upmconfig(UPMA, (uint *) sdram_table, sizeof(sdram_table) / sizeof(uint));

	/*
	 * Preliminary prescaler for refresh
	 */
	memctl->memc_mptpr = CFG_MPTPR_1BK_8K;

	memctl->memc_mar = MAR_SDRAM_INIT;	/* 32-bit address to be output on the address bus if AMX = 0b11 */

    /*
     * Map controller bank 3 to the SDRAM bank at preliminary address.
     */
	memctl->memc_or3 = CFG_OR3_PRELIM;
	memctl->memc_br3 = CFG_BR3_PRELIM;

	memctl->memc_mamr = CFG_MAMR_9COL & ~MAMR_PTAE;	/* no refresh yet */

	udelay(200);

	/* perform SDRAM initialisation sequence */
	memctl->memc_mcr = MCR_OP_RUN | MCR_UM_UPMA | MCR_MB_CS3 | MCR_MCLF(1) | MCR_MAD(0x3C);	/* precharge all			*/
	udelay(1);
	memctl->memc_mcr = MCR_OP_RUN | MCR_UM_UPMA | MCR_MB_CS3 | MCR_MCLF(0) | MCR_MAD(0x30); /* refresh 16 times(0)		*/
	udelay(1);
	memctl->memc_mcr = MCR_OP_RUN | MCR_UM_UPMA | MCR_MB_CS3 | MCR_MCLF(1) | MCR_MAD(0x3E);	/* exception program (write mar) */
	udelay(1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay(1000);

	memctl->memc_mamr = CFG_MAMR_9COL;

	size = SDRAM_MAX_SIZE;

	udelay(10000);

	/* do the ram test */
	{
		register unsigned long *rp;
		register unsigned long v;

		/* first fill */
		for (rp = (unsigned long *)0; rp < (unsigned long *)SDRAM_MAX_SIZE; )
		    *rp++ = (unsigned long)rp;

		/* now check */
		for (rp = (unsigned long *)0; rp < (unsigned long *)SDRAM_MAX_SIZE; rp++) {
		    if ((v = *rp) != (unsigned long)rp) {
			printf("ERROR at 0x%lx (0x%lx)\n", (unsigned long)rp, v);
			return -1;
		    }
		}

	}

	return (size);
}

/* ------------------------------------------------------------------------- */

int misc_init_r(void)
{
	return(0);
}

/* ------------------------------------------------------------------------- */

/* bits that can have a special purpose or can be configured as inputs/outputs */
#define PA_MASK		(_BWR(4, 9) | _BWR(12, 15))
#define PA_ODR_MASK	(_BW(9) | _BW(12) | _BW(14))
#define PA_ODR_VAL	0
#define PA_GP_INMASK	0
#define PA_GP_OUTMASK	(_BW(5) | _BW(14) | _BW(15))
#define PA_SP_OUTMASK	0
#define PA_GP_OUTVAL	_BW(5)
#define PA_SP_OUTVAL	0

#define PB_MASK		(_BR(16, 19) | _BR(22, 31))
#define PB_ODR_MASK	PB_MASK
#define PB_ODR_VAL	0
#define PB_GP_INMASK	0
#define PB_GP_OUTMASK	(_BR(16, 19) | _BR(26, 27) | _B(31))
#define PB_SP_OUTMASK	_BR(28, 30)
#define PB_SP_OUTVAL	_BR(28, 30)
#define PB_GP_OUTVAL	(_BR(16, 19) | _BR(26, 27) | _B(31))

#define PC_MASK		_BWR(4, 15)
#define PC_SO_MASK	(_BWR(6, 11) | _BWR(14, 15))
#define PC_SO_VAL	0
#define PC_INT_MASK	PC_MASK
#define PC_INT_VAL	0
#define PC_GP_INMASK	(_BWR(5, 7) | _BWR(9, 10) | _BW(13))
#define PC_GP_OUTMASK	_BW(12)
#define PC_SP_OUTMASK	0
#define PC_SP_OUTVAL	_BW(12)
#define PC_GP_OUTVAL	0

#define PD_MASK		_BWR(0, 15)
#define PD_GP_INMASK	0
#define PD_GP_OUTMASK	_BWR(3, 15)
#define PD_SP_OUTMASK	0
#define PD_GP_OUTVAL	(_BW(3) | _BW(5) | _BW(7) | _BWR(8, 15))
#define PD_SP_OUTVAL	0

int board_pre_init(void)
{
	register volatile immap_t *immap = (immap_t *) CFG_IMMR;
	register volatile iop8xx_t *ioport = &immap->im_ioport;
	register volatile cpm8xx_t *cpm = &immap->im_cpm;

	ioport->iop_padat = (ioport->iop_padat & ~PA_MASK)     | PA_SP_OUTVAL | PA_GP_OUTVAL;
	ioport->iop_paodr = (ioport->iop_paodr & ~PA_ODR_MASK) | PA_ODR_VAL;
	ioport->iop_padir = (ioport->iop_padir & ~PA_GP_INMASK)| PA_SP_OUTMASK | PA_GP_OUTMASK;
	ioport->iop_papar = (ioport->iop_papar & ~(PA_GP_INMASK & PA_GP_OUTMASK));

	cpm->cp_pbdat = (ioport->iop_padat & ~PB_MASK)     | PB_SP_OUTVAL | PB_GP_OUTVAL;
	cpm->cp_pbodr = (ioport->iop_paodr & ~PB_ODR_MASK) | PB_ODR_VAL;
	cpm->cp_pbdir = (ioport->iop_padir & ~PB_GP_INMASK)| PB_SP_OUTMASK | PB_GP_OUTMASK;
	cpm->cp_pbpar = (ioport->iop_papar & ~(PB_GP_INMASK & PB_GP_OUTMASK));

	ioport->iop_pcdat = (ioport->iop_pcdat & ~PC_MASK)     | PC_SP_OUTVAL | PC_GP_OUTVAL;
	ioport->iop_pcdir = (ioport->iop_pcdir & ~PC_GP_INMASK)| PC_SP_OUTMASK | PC_GP_OUTMASK;
	ioport->iop_pcso  = (ioport->iop_pcso  & ~PC_SO_MASK)  | PC_SO_VAL;
	ioport->iop_pcint = (ioport->iop_pcint & ~PC_INT_MASK) | PC_INT_VAL;
	ioport->iop_pcpar = (ioport->iop_pcpar & ~(PC_GP_INMASK & PC_GP_OUTMASK));

	ioport->iop_pddat = (ioport->iop_pddat & ~PD_MASK)     | PD_SP_OUTVAL | PD_GP_OUTVAL;
	ioport->iop_pddir = (ioport->iop_pddir & ~PD_GP_INMASK)| PD_SP_OUTMASK | PD_GP_OUTMASK;
	ioport->iop_pdpar = (ioport->iop_pdpar & ~(PD_GP_INMASK & PD_GP_OUTMASK));

	return 0;
}

