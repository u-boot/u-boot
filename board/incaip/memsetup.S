/*
 *  Memory sub-system initialization code for INCA-IP development board.
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
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

#include <config.h>
#include <version.h>
#include <asm/regdef.h>


#define EBU_MODUL_BASE          0xB8000200
#define EBU_CLC(value)          0x0000(value)
#define EBU_CON(value)          0x0010(value)
#define EBU_ADDSEL0(value)      0x0020(value)
#define EBU_ADDSEL1(value)      0x0024(value)
#define EBU_ADDSEL2(value)      0x0028(value)
#define EBU_BUSCON0(value)      0x0060(value)
#define EBU_BUSCON1(value)      0x0064(value)
#define EBU_BUSCON2(value)      0x0068(value)

#define MC_MODUL_BASE           0xBF800000
#define MC_ERRCAUSE(value)      0x0100(value)
#define MC_ERRADDR(value)       0x0108(value)
#define MC_IOGP(value)          0x0800(value)
#define MC_SELFRFSH(value)      0x0A00(value)
#define MC_CTRLENA(value)       0x1000(value)
#define MC_MRSCODE(value)       0x1008(value)
#define MC_CFGDW(value)         0x1010(value)
#define MC_CFGPB0(value)        0x1018(value)
#define MC_LATENCY(value)       0x1038(value)
#define MC_TREFRESH(value)      0x1040(value)

#define CGU_MODUL_BASE          0xBF107000
#define CGU_PLL1CR(value)       0x0008(value)
#define CGU_DIVCR(value)        0x0010(value)
#define CGU_MUXCR(value)        0x0014(value)
#define CGU_PLL1SR(value)       0x000C(value)

	.globl	memsetup
memsetup:

	/* EBU Initialization for the Flash CS0 and CS2.
	 */
	li	t0, EBU_MODUL_BASE

	li	t1, 0xA0000041
	sw	t1, EBU_ADDSEL0(t0)

#if CPU_CLOCK_RATE==100000000	/* 100 MHz clock for the MIPS core */
	lw	t1, EBU_BUSCON0(t0)  /* value set up by magic flash word */
	sw	t1, EBU_BUSCON2(t0)
#else /* 150 MHz or 133 MHz */
	li	t1, 0x8841417E
	sw	t1, EBU_BUSCON0(t0)
	sw	t1, EBU_BUSCON2(t0)
#endif

	li	t1, 0xA0800041
	sw	t1, EBU_ADDSEL2(t0)

	/* Need to initialize CS1 too, so as to to prevent overlapping with
	 * Flash bank 1.
	 */
	li	t1, 0xBE0000F1
 	sw	t1, EBU_ADDSEL1(t0)

#if CPU_CLOCK_RATE==100000000   /* 100 MHz clock for the MIPS core */
	li	t1, 0x684142BD
#else /* 150 MHz or 133 MHz */
	li	t1, 0x684143FD
#endif
 	sw	t1, EBU_BUSCON1(t0)

#if CPU_CLOCK_RATE==150000000   /* 150 MHz clock for the MIPS core */
	li      t0, CGU_MODUL_BASE
	li      t1, 0x80000017
	sw      t1, CGU_DIVCR(t0)
	li      t1, 0xC00B0001
	sw      t1, CGU_PLL1CR(t0)
	lui     t2, 0x8000
b1:
	lw      t1, CGU_PLL1SR(t0)
	and     t1, t1, t2
	beq     t1, zero, b1
	li      t1, 0x80000001
	sw      t1, CGU_MUXCR(t0)
#elif CPU_CLOCK_RATE==133000000	/* 133 MHz clock for the MIPS core */
	li	t0, CGU_MODUL_BASE
	li	t1, 0x80000054
	sw	t1, CGU_DIVCR(t0)
	li	t1, 0x80000000
	sw	t1, CGU_MUXCR(t0)
	li	t1, 0x800B0001
	sw	t1, CGU_PLL1CR(t0)
#endif

	/* SDRAM Initialization.
	 */
	li	t0, MC_MODUL_BASE

	/* Clear Error log registers */
	sw	zero, MC_ERRCAUSE(t0)
	sw	zero, MC_ERRADDR(t0)

	/* Set clock ratio to 1:1 */
	li	t1, 0x03		/* clkrat=1:1, rddel=3 */
	sw	t1, MC_IOGP(t0)

	/* Clear Power-down registers */
	sw	zero, MC_SELFRFSH(t0)

	/* Set CAS Latency */
	li	t1, 0x00000020		/* CL = 2 */
	sw	t1, MC_MRSCODE(t0)

	/* Set word width to 16 bit */
	li	t1, 0x2  
	sw	t1, MC_CFGDW(t0)

	/* Set CS0 to SDRAM parameters */
	li	t1, 0x000014C9
	sw	t1, MC_CFGPB0(t0)

	/* Set SDRAM latency parameters */
	li	t1, 0x00026325		/* BC PC100 */
	sw	t1, MC_LATENCY(t0)

	/* Set SDRAM refresh rate */
	li	t1, 0x00000C30		/* 4K/64ms @ 100MHz */
	sw	t1, MC_TREFRESH(t0)

	/* Finally enable the controller */
	li	t1, 1
	sw	t1, MC_CTRLENA(t0)

	j	ra
	nop

