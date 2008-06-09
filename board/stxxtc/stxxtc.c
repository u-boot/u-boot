/*
 * (C) Copyright 2000-2004
 * Pantelis Antoniou, Intracom S.A., panto@intracom.gr
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * (C) Copyright 2005
 * Dan Malek, Embedded Edge, LLC, dan@embeddededge.com
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
 * U-Boot port on STx XTc board
 * Mostly copied from Netta
 */

#include <common.h>
#include <miiphy.h>

#include "mpc8xx.h"

#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>
#endif

/****************************************************************/

/* some sane bit macros */
#define _BD(_b)				(1U << (31-(_b)))
#define _BDR(_l, _h)			(((((1U << (31-(_l))) - 1) << 1) | 1) & ~((1U << (31-(_h))) - 1))

#define _BW(_b)				(1U << (15-(_b)))
#define _BWR(_l, _h)			(((((1U << (15-(_l))) - 1) << 1) | 1) & ~((1U << (15-(_h))) - 1))

#define _BB(_b)				(1U << (7-(_b)))
#define _BBR(_l, _h)			(((((1U << (7-(_l))) - 1) << 1) | 1) & ~((1U << (7-(_h))) - 1))

#define _B(_b)				_BD(_b)
#define _BR(_l, _h)			_BDR(_l, _h)

/****************************************************************/

/*
 * Check Board Identity:
 *
 * Return 1 always.
 */

int checkboard(void)
{
	printf ("Silicon Turnkey eXpress XTc\n");
	return (0);
}

/****************************************************************/

#define _NOT_USED_	0xFFFFFFFF

/****************************************************************/

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

#define GPL0_AAAA	0x00000000
#define GPL0_AAA0	0x00200000
#define GPL0_AAA1	0x00300000
#define GPL0_000A	0x00800000
#define GPL0_0000	0x00A00000
#define GPL0_0001	0x00B00000
#define GPL0_111A	0x00C00000
#define GPL0_1110	0x00E00000
#define GPL0_1111	0x00F00000

#define GPL1_0000	0x00000000
#define GPL1_0001	0x00040000
#define GPL1_1110	0x00080000
#define GPL1_1111	0x000C0000

#define GPL2_0000	0x00000000
#define GPL2_0001	0x00010000
#define GPL2_1110	0x00020000
#define GPL2_1111	0x00030000

#define GPL3_0000	0x00000000
#define GPL3_0001	0x00004000
#define GPL3_1110	0x00008000
#define GPL3_1111	0x0000C000

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

#define A10_AAAA	GPL0_AAAA
#define A10_AAA0	GPL0_AAA0
#define A10_AAA1	GPL0_AAA1
#define A10_000A	GPL0_000A
#define A10_0000	GPL0_0000
#define A10_0001	GPL0_0001
#define A10_111A	GPL0_111A
#define A10_1110	GPL0_1110
#define A10_1111	GPL0_1111

#define RAS_0000	GPL1_0000
#define RAS_0001	GPL1_0001
#define RAS_1110	GPL1_1110
#define RAS_1111	GPL1_1111

#define CAS_0000	GPL2_0000
#define CAS_0001	GPL2_0001
#define CAS_1110	GPL2_1110
#define CAS_1111	GPL2_1111

#define WE_0000		GPL3_0000
#define WE_0001		GPL3_0001
#define WE_1110		GPL3_1110
#define WE_1111		GPL3_1111

/* #define CAS_LATENCY	3  */
#define CAS_LATENCY	2

const uint sdram_table[0x40] = {

#if CAS_LATENCY == 3
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
	CS_0001 | BS_1111 | A10_AAAA | RAS_0001 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* ACT   */
	CS_1111 | BS_1111 | A10_0000 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP	 */
	CS_0000 | BS_0001 | A10_0000 | RAS_1111 | CAS_0001 | WE_0000 | AMX_COL | UTA,			/* WRITE */
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL | UTA,			/* PALL  */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA | TODT | LAST,	/* NOP   */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

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
#endif

#if CAS_LATENCY == 2
	/* RSS */
	CS_0001 | BS_1111 | A10_AAAA | RAS_0001 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* ACT   */
	CS_1110 | BS_1110 | A10_0000 | RAS_1111 | CAS_1110 | WE_1111 | AMX_COL | UTA,			/* NOP   */
	CS_0001 | BS_0001 | A10_0000 | RAS_1111 | CAS_0001 | WE_1111 | AMX_COL | UTA,			/* READ  */
	CS_1110 | BS_1111 | A10_0001 | RAS_1110 | CAS_1111 | WE_1110 | AMX_COL,				/* NOP   */
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL | UTA | TODT | LAST,	/* PALL  */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_,

	/* RBS */
	CS_0001 | BS_1111 | A10_AAAA | RAS_0001 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* ACT   */
	CS_1110 | BS_1110 | A10_0000 | RAS_1111 | CAS_1110 | WE_1111 | AMX_COL | UTA,			/* NOP   */
	CS_0001 | BS_0000 | A10_0000 | RAS_1111 | CAS_0001 | WE_1111 | AMX_COL | UTA,			/* READ  */
	CS_1111 | BS_0000 | A10_0000 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP   */
	CS_1111 | BS_0000 | A10_0000 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP   */
	CS_1111 | BS_0001 | A10_0000 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP   */
	CS_1110 | BS_1111 | A10_0001 | RAS_1110 | CAS_1111 | WE_1110 | AMX_COL,				/* NOP   */
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL | UTA | TODT | LAST,	/* PALL  */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* WSS */
	CS_0001 | BS_1111 | A10_AAA0 | RAS_0001 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* ACT   */
	CS_1110 | BS_1110 | A10_0000 | RAS_1111 | CAS_1110 | WE_1110 | AMX_COL,				/* NOP   */
	CS_0000 | BS_0001 | A10_0001 | RAS_1110 | CAS_0001 | WE_0000 | AMX_COL | UTA,			/* WRITE */
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL | UTA | TODT | LAST,	/* PALL  */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_,

	/* WBS */
	CS_0001 | BS_1111 | A10_AAAA | RAS_0001 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* ACT   */
	CS_1110 | BS_1110 | A10_0000 | RAS_1111 | CAS_1110 | WE_1110 | AMX_COL,				/* NOP   */
	CS_0001 | BS_0000 | A10_0000 | RAS_1111 | CAS_0001 | WE_0001 | AMX_COL,				/* WRITE */
	CS_1111 | BS_0000 | A10_0000 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP   */
	CS_1111 | BS_0000 | A10_0000 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL,				/* NOP   */
	CS_1110 | BS_0001 | A10_0001 | RAS_1110 | CAS_1111 | WE_1110 | AMX_COL | UTA,			/* NOP   */
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL | UTA | TODT | LAST,	/* PALL  */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_,

#endif

	/* UPT */
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_0001 | WE_1111 | AMX_COL | UTA | LOOP,		/* ATRFR */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* NOP   */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* NOP   */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA,			/* NOP   */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA | LOOP,		/* NOP   */
	CS_1111 | BS_1111 | A10_1111 | RAS_1111 | CAS_1111 | WE_1111 | AMX_COL | UTA | TODT | LAST,	/* NOP   */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_,

	/* EXC */
	CS_0001 | BS_1111 | A10_1111 | RAS_0001 | CAS_1111 | WE_0001 | AMX_COL | UTA | LAST,
	_NOT_USED_,

	/* REG */
	CS_1110 | BS_1111 | A10_1110 | RAS_1110 | CAS_1110 | WE_1110 | AMX_MAR | UTA,
	CS_0001 | BS_1111 | A10_0001 | RAS_0001 | CAS_0001 | WE_0001 | AMX_MAR | UTA | LAST,
};

static const uint nandcs_table[0x40] = {
	/* RSS */
	CS_1000 | GPL4_1111 | GPL5_1111 | UTA,
	CS_0000 | GPL4_1110 | GPL5_1111 | UTA,
	CS_0000 | GPL4_0000 | GPL5_1111 | UTA,
	CS_0000 | GPL4_0000 | GPL5_1111 | UTA,
	CS_0000 | GPL4_0000 | GPL5_1111,
	CS_0000 | GPL4_0001 | GPL5_1111 | UTA,
	CS_0000 | GPL4_1111 | GPL5_1111 | UTA,
	CS_0011 | GPL4_1111 | GPL5_1111 | UTA | LAST,	/* NOP   */

	/* RBS */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* WSS */
	CS_1000 | GPL4_1111 | GPL5_1110 | UTA,
	CS_0000 | GPL4_1111 | GPL5_0000 | UTA,
	CS_0000 | GPL4_1111 | GPL5_0000 | UTA,
	CS_0000 | GPL4_1111 | GPL5_0000 | UTA,
	CS_0000 | GPL4_1111 | GPL5_0001 | UTA,
	CS_0000 | GPL4_1111 | GPL5_1111 | UTA,
	CS_0000 | GPL4_1111 | GPL5_1111,
	CS_0011 | GPL4_1111 | GPL5_1111 | UTA | LAST,

	/* WBS */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* UPT */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_,	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_,	_NOT_USED_, _NOT_USED_,

	/* EXC */
	CS_0001 | LAST,
	_NOT_USED_,

	/* REG */
	CS_1110 ,
	CS_0001 | LAST,
};

/* 0xC8 = 0b11001000 , CAS3, >> 2 = 0b00 11 0 010 */
/* 0x88 = 0b10001000 , CAS2, >> 2 = 0b00 10 0 010 */
#define MAR_SDRAM_INIT		((CAS_LATENCY << 6) | 0x00000008LU)

/* 9 */
#define CFG_MAMR	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)

void check_ram(unsigned int addr, unsigned int size)
{
	unsigned int i, j, v, vv;
	volatile unsigned int *p;
	unsigned int pv;

	p = (unsigned int *)addr;
	pv = (unsigned int)p;
	for (i = 0; i < size / sizeof(unsigned int); i++, pv += sizeof(unsigned int))
		*p++ = pv;

	p = (unsigned int *)addr;
	for (i = 0; i < size / sizeof(unsigned int); i++) {
		v = (unsigned int)p;
		vv = *p;
		if (vv != v) {
			printf("%p: read %08x instead of %08x\n", p, vv, v);
			hang();
		}
		p++;
	}

	for (j = 0; j < 5; j++) {
		switch (j) {
			case 0: v = 0x00000000; break;
			case 1: v = 0xffffffff; break;
			case 2: v = 0x55555555; break;
			case 3: v = 0xaaaaaaaa; break;
			default:v = 0xdeadbeef; break;
		}
		p = (unsigned int *)addr;
		for (i = 0; i < size / sizeof(unsigned int); i++) {
			*p = v;
			vv = *p;
			if (vv != v) {
				printf("%p: read %08x instead of %08x\n", p, vv, v);
				hang();
			}
			*p = ~v;
			p++;
		}
	}
}

#define DO_LOOP do { for (;;) asm volatile ("nop" : : : "memory"); } while(0)

phys_size_t initdram(int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size;
	u32 d1, d2;

	upmconfig(UPMA, (uint *) sdram_table, sizeof(sdram_table) / sizeof(sdram_table[0]));

	/*
	 * Preliminary prescaler for refresh
	 */
	memctl->memc_mptpr = MPTPR_PTP_DIV8;

	memctl->memc_mar = MAR_SDRAM_INIT;	/* 32-bit address to be output on the address bus if AMX = 0b11 */

	/*
	 * Map controller bank 3 to the SDRAM bank at preliminary address.
	 */
	memctl->memc_or4 = CFG_OR4_PRELIM;
	memctl->memc_br4 = CFG_BR4_PRELIM;

	memctl->memc_mamr = CFG_MAMR & ~MAMR_PTAE;	/* no refresh yet */

	udelay(200);

	/* perform SDRAM initialisation sequence */
	memctl->memc_mcr = MCR_OP_RUN | MCR_UPM_A | MCR_MB_CS4 | MCR_MLCF(1) | MCR_MAD(0x3C);	/* precharge all		*/
	udelay(1);

	memctl->memc_mcr = MCR_OP_RUN | MCR_UPM_A | MCR_MB_CS4 | MCR_MLCF(2) | MCR_MAD(0x30);	/* refresh 2 times(0)		*/
	udelay(1);

	memctl->memc_mcr = MCR_OP_RUN | MCR_UPM_A | MCR_MB_CS4 | MCR_MLCF(1) | MCR_MAD(0x3E);	/* exception program (write mar)*/
	udelay(1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay(10000);


	d1 = 0xAA55AA55;
	*(volatile u32 *)0 = d1;
	d2 = *(volatile u32 *)0;
	if (d1 != d2) {
		printf("DRAM fails: wrote 0x%08x read 0x%08x\n", d1, d2);
		DO_LOOP;
	}

	d1 = 0x55AA55AA;
	*(volatile u32 *)0 = d1;
	d2 = *(volatile u32 *)0;
	if (d1 != d2) {
		printf("DRAM fails: wrote 0x%08x read 0x%08x\n", d1, d2);
		DO_LOOP;
	}

	d1 = 0x12345678;
	*(volatile u32 *)0 = d1;
	d2 = *(volatile u32 *)0;
	if (d1 != d2) {
		printf("DRAM fails: wrote 0x%08x read 0x%08x\n", d1, d2);
		DO_LOOP;
	}

	size = get_ram_size((long *)0, SDRAM_MAX_SIZE);

	return size;
}

/* ------------------------------------------------------------------------- */

void reset_phys(void)
{
	int phyno;
	unsigned short v;

	udelay(10000);
	/* reset the damn phys */
	mii_init();

	for (phyno = 0; phyno < 32; ++phyno) {
		miiphy_read("FEC ETHERNET", phyno, PHY_PHYIDR1, &v);
		if (v == 0xFFFF)
			continue;
		miiphy_write("FEC ETHERNET", phyno, PHY_BMCR, PHY_BMCR_POWD);
		udelay(10000);
		miiphy_write("FEC ETHERNET", phyno, PHY_BMCR, PHY_BMCR_RESET | PHY_BMCR_AUTON);
		udelay(10000);
	}
}

/* ------------------------------------------------------------------------- */

/* GP = general purpose, SP = special purpose (on chip peripheral) */

/* bits that can have a special purpose or can be configured as inputs/outputs */
#define PA_GP_INMASK	_BW(6)
#define PA_GP_OUTMASK	(_BW(7))
#define PA_SP_MASK	0
#define PA_ODR_VAL	0
#define PA_GP_OUTVAL	(_BW(7))
#define PA_SP_DIRVAL	0

#define PB_GP_INMASK	0
#define PB_GP_OUTMASK	(_B(23))
#define PB_SP_MASK	0
#define PB_ODR_VAL	0
#define PB_GP_OUTVAL	(_B(23))
#define PB_SP_DIRVAL	0

#define PC_GP_INMASK	0
#define PC_GP_OUTMASK	(_BW(15))

#define PC_SP_MASK	0
#define PC_SOVAL	0
#define PC_INTVAL	0
#define PC_GP_OUTVAL	0
#define PC_SP_DIRVAL	0

#define PE_GP_INMASK	0
#define PE_GP_OUTMASK	0
#define PE_GP_OUTVAL	0

#define PE_SP_MASK	0
#define PE_ODR_VAL	0
#define PE_SP_DIRVAL	0

int board_early_init_f(void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile iop8xx_t *ioport = &immap->im_ioport;
	volatile cpm8xx_t *cpm = &immap->im_cpm;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	(void)ioport;
	(void)cpm;
#if 1
	/* NAND chip select */
	upmconfig(UPMB, (uint *) nandcs_table, sizeof(nandcs_table) / sizeof(nandcs_table[0]));
	memctl->memc_or2 = ((0xFFFFFFFFLU & ~(NAND_SIZE - 1)) | OR_BI | OR_G5LS);
	memctl->memc_br2 = ((NAND_BASE & BR_BA_MSK) | BR_PS_8 | BR_V | BR_MS_UPMB);
	memctl->memc_mbmr = 0;	/* all clear */
#endif

	memctl->memc_br5 &= ~BR_V;
	memctl->memc_br6 &= ~BR_V;
	memctl->memc_br7 &= ~BR_V;

#if 1
	ioport->iop_padat	= PA_GP_OUTVAL;
	ioport->iop_paodr	= PA_ODR_VAL;
	ioport->iop_padir	= PA_GP_OUTMASK | PA_SP_DIRVAL;
	ioport->iop_papar	= PA_SP_MASK;

	cpm->cp_pbdat		= PB_GP_OUTVAL;
	cpm->cp_pbodr		= PB_ODR_VAL;
	cpm->cp_pbdir		= PB_GP_OUTMASK | PB_SP_DIRVAL;
	cpm->cp_pbpar		= PB_SP_MASK;

	ioport->iop_pcdat	= PC_GP_OUTVAL;
	ioport->iop_pcdir	= PC_GP_OUTMASK | PC_SP_DIRVAL;
	ioport->iop_pcso	= PC_SOVAL;
	ioport->iop_pcint	= PC_INTVAL;
	ioport->iop_pcpar	= PC_SP_MASK;

	cpm->cp_pedat		= PE_GP_OUTVAL;
	cpm->cp_peodr		= PE_ODR_VAL;
	cpm->cp_pedir		= PE_GP_OUTMASK | PE_SP_DIRVAL;
	cpm->cp_pepar		= PE_SP_MASK;
#endif

	return 0;
}

#if defined(CONFIG_CMD_NAND)

#include <linux/mtd/nand_legacy.h>

extern ulong nand_probe(ulong physadr);
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];

void nand_init(void)
{
	unsigned long totlen;

	totlen = nand_probe(CFG_NAND_BASE);
	printf ("%4lu MB\n", totlen >> 20);
}
#endif

#ifdef CONFIG_HW_WATCHDOG

void hw_watchdog_reset(void)
{
	/* XXX add here the really funky stuff */
}

#endif

#ifdef CONFIG_SHOW_ACTIVITY

/* called from timer interrupt every 1/CFG_HZ sec */
void board_show_activity(ulong timestamp)
{
}

/* called when looping */
void show_activity(int arg)
{
}

#endif

#if defined(CFG_CONSOLE_IS_IN_ENV) && defined(CFG_CONSOLE_OVERWRITE_ROUTINE)
int overwrite_console(void)
{
	/* printf("overwrite_console called\n"); */
	return 0;
}
#endif

extern int drv_phone_init(void);
extern int drv_phone_use_me(void);
extern int drv_phone_is_idle(void);

int misc_init_r(void)
{
	return 0;
}

int last_stage_init(void)
{
	reset_phys();

	return 0;
}
