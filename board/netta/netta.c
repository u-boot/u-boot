/*
 * (C) Copyright 2000-2004
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
 * U-Boot port on NetTA4 board
 */

#include <common.h>
#include <miiphy.h>

#include "mpc8xx.h"

#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>
#endif

int fec8xx_miiphy_read(char *devname, unsigned char addr,
		unsigned char  reg, unsigned short *value);
int fec8xx_miiphy_write(char *devname, unsigned char  addr,
		unsigned char  reg, unsigned short value);

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
	printf ("Intracom NETTA"
#if defined(CONFIG_NETTA_ISDN)
			" with ISDN support"
#endif
#if defined(CONFIG_NETTA_6412)
			" (DSP:TI6412)"
#else
			" (DSP:TI6711)"
#endif
			"\n"
			);
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

/* #define CAS_LATENCY	3 */
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

/* 0xC8 = 0b11001000 , CAS3, >> 2 = 0b00 11 0 010 */
/* 0x88 = 0b10001000 , CAS2, >> 2 = 0b00 10 0 010 */
#define MAR_SDRAM_INIT		((CAS_LATENCY << 6) | 0x00000008LU)

/* 8 */
#define CFG_MAMR	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_0 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A11 |	\
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

long int initdram(int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size;

	upmconfig(UPMB, (uint *) sdram_table, sizeof(sdram_table) / sizeof(uint));

	/*
	 * Preliminary prescaler for refresh
	 */
	memctl->memc_mptpr = MPTPR_PTP_DIV8;

	memctl->memc_mar = MAR_SDRAM_INIT;	/* 32-bit address to be output on the address bus if AMX = 0b11 */

	/*
	 * Map controller bank 3 to the SDRAM bank at preliminary address.
	 */
	memctl->memc_or3 = CFG_OR3_PRELIM;
	memctl->memc_br3 = CFG_BR3_PRELIM;

	memctl->memc_mbmr = CFG_MAMR & ~MAMR_PTAE;	/* no refresh yet */

	udelay(200);

	/* perform SDRAM initialisation sequence */
	memctl->memc_mcr = MCR_OP_RUN | MCR_UPM_B | MCR_MB_CS3 | MCR_MLCF(1) | MCR_MAD(0x3C);	/* precharge all		*/
	udelay(1);

	memctl->memc_mcr = MCR_OP_RUN | MCR_UPM_B | MCR_MB_CS3 | MCR_MLCF(2) | MCR_MAD(0x30);	/* refresh 2 times(0)		*/
	udelay(1);

	memctl->memc_mcr = MCR_OP_RUN | MCR_UPM_B | MCR_MB_CS3 | MCR_MLCF(1) | MCR_MAD(0x3E);	/* exception program (write mar)*/
	udelay(1);

	memctl->memc_mbmr |= MAMR_PTAE;	/* enable refresh */

	udelay(10000);

	{
		u32 d1, d2;

		d1 = 0xAA55AA55;
		*(volatile u32 *)0 = d1;
		d2 = *(volatile u32 *)0;
		if (d1 != d2) {
			printf("DRAM fails: wrote 0x%08x read 0x%08x\n", d1, d2);
			hang();
		}

		d1 = 0x55AA55AA;
		*(volatile u32 *)0 = d1;
		d2 = *(volatile u32 *)0;
		if (d1 != d2) {
			printf("DRAM fails: wrote 0x%08x read 0x%08x\n", d1, d2);
			hang();
		}
	}

	size = get_ram_size((long *)0, SDRAM_MAX_SIZE);

#if 0
	printf("check 0\n");
	check_ram(( 0 << 20), (2 << 20));
	printf("check 16\n");
	check_ram((16 << 20), (2 << 20));
	printf("check 32\n");
	check_ram((32 << 20), (2 << 20));
	printf("check 48\n");
	check_ram((48 << 20), (2 << 20));
#endif

	if (size == 0) {
		printf("SIZE is zero: LOOP on 0\n");
		for (;;) {
			*(volatile u32 *)0 = 0;
			(void)*(volatile u32 *)0;
		}
	}

	return size;
}

/* ------------------------------------------------------------------------- */

int misc_init_r(void)
{
	return(0);
}

void reset_phys(void)
{
	int phyno;
	unsigned short v;

	/* reset the damn phys */
	mii_init();

	for (phyno = 0; phyno < 32; ++phyno) {
		fec8xx_miiphy_read(NULL, phyno, PHY_PHYIDR1, &v);
		if (v == 0xFFFF)
			continue;
		fec8xx_miiphy_write(NULL, phyno, PHY_BMCR, PHY_BMCR_POWD);
		udelay(10000);
		fec8xx_miiphy_write(NULL, phyno, PHY_BMCR,
				PHY_BMCR_RESET | PHY_BMCR_AUTON);
		udelay(10000);
	}
}

extern int board_dsp_reset(void);

int last_stage_init(void)
{
	int r;

	reset_phys();
	r = board_dsp_reset();
	if (r < 0)
		printf("*** WARNING *** DSP reset failed (run diagnostics)\n");
	return 0;
}

/* ------------------------------------------------------------------------- */

/* GP = general purpose, SP = special purpose (on chip peripheral) */

/* bits that can have a special purpose or can be configured as inputs/outputs */
#define PA_GP_INMASK	(_BWR(3) | _BWR(7, 9) | _BW(11))
#define PA_GP_OUTMASK	(_BW(6) | _BW(10) | _BWR(12, 15))
#define PA_SP_MASK	(_BWR(0, 2) | _BWR(4, 5))
#define PA_ODR_VAL	0
#define PA_GP_OUTVAL	(_BW(13) | _BWR(14, 15))
#define PA_SP_DIRVAL	0

#define PB_GP_INMASK	(_B(28) | _B(31))
#define PB_GP_OUTMASK	(_BR(15, 19) | _BR(26, 27) | _BR(29, 30))
#define PB_SP_MASK	(_BR(22, 25))
#define PB_ODR_VAL	0
#define PB_GP_OUTVAL	(_BR(15, 19) | _BR(26, 27) | _BR(29, 31))
#define PB_SP_DIRVAL	0

#define PC_GP_INMASK	(_BW(5) | _BW(7) | _BW(8) | _BWR(9, 11) | _BWR(13, 15))
#define PC_GP_OUTMASK	(_BW(6) | _BW(12))
#define PC_SP_MASK	(_BW(4) | _BW(8))
#define PC_SOVAL	0
#define PC_INTVAL	_BW(7)
#define PC_GP_OUTVAL	(_BW(6) | _BW(12))
#define PC_SP_DIRVAL	0

#define PD_GP_INMASK	0
#define PD_GP_OUTMASK	_BWR(3, 15)
#define PD_SP_MASK	0

#if defined(CONFIG_NETTA_6412)

#define PD_GP_OUTVAL	(_BWR(5, 7) | _BW(9) | _BW(11) | _BW(15))

#else

#define PD_GP_OUTVAL	(_BWR(5, 7) | _BW(9) | _BW(11))

#endif

#define PD_SP_DIRVAL	0

int board_early_init_f(void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile iop8xx_t *ioport = &immap->im_ioport;
	volatile cpm8xx_t *cpm = &immap->im_cpm;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	/* CS1: NAND chip select */
	memctl->memc_or1 = ((0xFFFFFFFFLU & ~(NAND_SIZE - 1)) | OR_BI | OR_SCY_2_CLK | OR_TRLX | OR_ACS_DIV2) ;
	memctl->memc_br1 = ((NAND_BASE & BR_BA_MSK) | BR_PS_8 | BR_V);
#if !defined(CONFIG_NETTA_6412)
	/* CS2: DSP */
	memctl->memc_or2 = ((0xFFFFFFFFLU & ~(DSP_SIZE - 1)) | OR_CSNT_SAM | OR_BI | OR_SCY_7_CLK | OR_ACS_DIV2);
	memctl->memc_br2 = ((DSP_BASE & BR_BA_MSK) | BR_PS_16 | BR_V);
#else
	/* CS6: DSP */
	memctl->memc_or6 = ((0xFFFFFFFFLU & ~(DSP_SIZE - 1)) | OR_CSNT_SAM | OR_BI | OR_SCY_7_CLK | OR_ACS_DIV2);
	memctl->memc_br6 = ((DSP_BASE & BR_BA_MSK) | BR_PS_16 | BR_V);
#endif
	/* CS4: External register chip select */
	memctl->memc_or4 = ((0xFFFFFFFFLU & ~(ER_SIZE - 1)) | OR_BI | OR_SCY_4_CLK);
	memctl->memc_br4 = ((ER_BASE & BR_BA_MSK) | BR_PS_32 | BR_V);

	/* CS5: dummy for accurate delay */
	memctl->memc_or5 = ((0xFFFFFFFFLU & ~(DUMMY_SIZE - 1)) | OR_CSNT_SAM | OR_BI | OR_SCY_0_CLK | OR_ACS_DIV2);
	memctl->memc_br5 = ((DUMMY_BASE & BR_BA_MSK) | BR_PS_32 | BR_V);

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

	ioport->iop_pddat	= PD_GP_OUTVAL;
	ioport->iop_pddir	= PD_GP_OUTMASK | PD_SP_DIRVAL;
	ioport->iop_pdpar	= PD_SP_MASK;

	/* ioport->iop_pddat |=  (1 << (15 - 6)) | (1 << (15 - 7)); */

	return 0;
}

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && defined(CFG_NAND_LEGACY)

#include <linux/mtd/nand_legacy.h>

extern ulong nand_probe(ulong physadr);
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];

void nand_init(void)
{
	unsigned long totlen = nand_probe(CFG_NAND_BASE);

	printf ("%4lu MB\n", totlen >> 20);
}
#endif

#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA)

int pcmcia_init(void)
{
	return 0;
}

#endif

#ifdef CONFIG_POST
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{
	return 0;	/* No hotkeys supported */
}
#endif

#ifdef CONFIG_HW_WATCHDOG

void hw_watchdog_reset(void)
{
	/* XXX add here the really funky stuff */
}

#endif
