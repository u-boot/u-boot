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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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


#define EBU_MODUL_BASE		0xB8000200
#define EBU_CLC(value)		0x0000(value)
#define EBU_CON(value)		0x0010(value)
#define EBU_ADDSEL0(value)	0x0020(value)
#define EBU_ADDSEL1(value)	0x0024(value)
#define EBU_ADDSEL2(value)	0x0028(value)
#define EBU_BUSCON0(value)	0x0060(value)
#define EBU_BUSCON1(value)	0x0064(value)
#define EBU_BUSCON2(value)	0x0068(value)

#define MC_MODUL_BASE		0xBF800000
#define MC_ERRCAUSE(value)	0x0100(value)
#define MC_ERRADDR(value)	0x0108(value)
#define MC_IOGP(value)		0x0800(value)
#define MC_SELFRFSH(value)	0x0A00(value)
#define MC_CTRLENA(value)	0x1000(value)
#define MC_MRSCODE(value)	0x1008(value)
#define MC_CFGDW(value)		0x1010(value)
#define MC_CFGPB0(value)	0x1018(value)
#define MC_LATENCY(value)	0x1038(value)
#define MC_TREFRESH(value)	0x1040(value)

#define CGU_MODUL_BASE		0xBF107000
#define CGU_PLL1CR(value)	0x0008(value)
#define CGU_DIVCR(value)	0x0010(value)
#define CGU_MUXCR(value)	0x0014(value)
#define CGU_PLL1SR(value)	0x000C(value)

	.set	noreorder


/*
 * void ebu_init(long)
 *
 * a0 has the clock value we are going to run at
 */
	.globl	ebu_init
	.ent	ebu_init
ebu_init:
__ebu_init:

	li	t1, EBU_MODUL_BASE
	li	t2, 0xA0000041
	sw	t2, EBU_ADDSEL0(t1)
	li	t2, 0xA0800041
	sw	t2, EBU_ADDSEL2(t1)
	li	t2, 0xBE0000F1
	sw	t2, EBU_ADDSEL1(t1)

	li	t3, 100000000
	beq	a0, t3, 1f
	nop
	li	t3, 133000000
	beq	a0, t3, 2f
	nop
	li	t3, 150000000
	beq	a0, t3, 2f
	nop
	b	3f
	nop

	/* 100 MHz */
1:
	li	t2, 0x8841417D
	sw	t2, EBU_BUSCON0(t1)
	sw	t2, EBU_BUSCON2(t1)
	li	t2, 0x684142BD
	b	3f
	sw	t2, EBU_BUSCON1(t1)	/* delay slot */

	/* 133 or 150 MHz */
2:
	li	t2, 0x8841417E
	sw	t2, EBU_BUSCON0(t1)
	sw	t2, EBU_BUSCON2(t1)
	li	t2, 0x684143FD
	sw	t2, EBU_BUSCON1(t1)
3:
	j	ra
	nop

	.end	ebu_init


/*
 * void cgu_init(long)
 *
 * a0 has the clock value
 */
	.globl	cgu_init
	.ent	cgu_init
cgu_init:
__cgu_init:

	li	t1, CGU_MODUL_BASE

	li	t3, 100000000
	beq	a0, t3, 1f
	nop
	li	t3, 133000000
	beq	a0, t3, 2f
	nop
	li	t3, 150000000
	beq	a0, t3, 3f
	nop
	b	5f
	nop

	/* 100 MHz clock */
1:
	li	t2, 0x80000014
	sw	t2, CGU_DIVCR(t1)
	li	t2, 0x80000000
	sw	t2, CGU_MUXCR(t1)
	li	t2, 0x800B0001
	b	5f
	sw	t2, CGU_PLL1CR(t1)	/* delay slot */

	/* 133 MHz clock */
2:
	li	t2, 0x80000054
	sw	t2, CGU_DIVCR(t1)
	li	t2, 0x80000000
	sw	t2, CGU_MUXCR(t1)
	li	t2, 0x800B0001
	b	5f
	sw	t2, CGU_PLL1CR(t1)	/* delay slot */

	/* 150 MHz clock */
3:
	li	t2, 0x80000017
	sw	t2, CGU_DIVCR(t1)
	li	t2, 0xC00B0001
	sw	t2, CGU_PLL1CR(t1)
	li	t3, 0x80000000
4:
	lw	t2, CGU_PLL1SR(t1)
	and	t2, t2, t3
	beq	t2, zero, 4b
	nop
	li	t2, 0x80000001
	sw	t2, CGU_MUXCR(t1)
5:
	j	ra
	nop

	.end	cgu_init


/*
 * void sdram_init(long)
 *
 * a0 has the clock value
 */
	.globl	sdram_init
	.ent	sdram_init
sdram_init:
__sdram_init:

	li	t1, MC_MODUL_BASE

#if 0
	/* Disable memory controller before changing any of its registers */
	sw	zero, MC_CTRLENA(t1)
#endif

	li	t2, 100000000
	beq	a0, t2, 1f
	nop
	li	t2, 133000000
	beq	a0, t2, 2f
	nop
	li	t2, 150000000
	beq	a0, t2, 3f
	nop
	b	5f
	nop

	/* 100 MHz clock */
1:
	/* Set clock ratio (clkrat=1:1, rddel=3) */
	li	t2, 0x00000003
	sw	t2, MC_IOGP(t1)

	/* Set sdram refresh rate (4K/64ms @ 100MHz) */
	li	t2, 0x0000061A
	b	4f
	sw	t2, MC_TREFRESH(t1)

	/* 133 MHz clock */
2:
	/* Set clock ratio (clkrat=1:1, rddel=3) */
	li	t2, 0x00000003
	sw	t2, MC_IOGP(t1)

	/* Set sdram refresh rate (4K/64ms @ 133MHz) */
	li	t2, 0x00000822
	b	4f
	sw	t2, MC_TREFRESH(t1)

	/* 150 MHz clock */
3:
	/* Set clock ratio (clkrat=3:2, rddel=4) */
	li	t2, 0x00000014
	sw	t2, MC_IOGP(t1)

	/* Set sdram refresh rate (4K/64ms @ 150MHz) */
	li	t2, 0x00000927
	sw	t2, MC_TREFRESH(t1)

4:
	/* Clear Error log registers */
	sw	zero, MC_ERRCAUSE(t1)
	sw	zero, MC_ERRADDR(t1)

	/* Clear Power-down registers */
	sw	zero, MC_SELFRFSH(t1)

	/* Set CAS Latency */
	li	t2, 0x00000020		/* CL = 2 */
	sw	t2, MC_MRSCODE(t1)

	/* Set word width to 16 bit */
	li	t2, 0x2
	sw	t2, MC_CFGDW(t1)

	/* Set CS0 to SDRAM parameters */
	li	t2, 0x000014C9
	sw	t2, MC_CFGPB0(t1)

	/* Set SDRAM latency parameters */
	li	t2, 0x00026325		/* BC PC100 */
	sw	t2, MC_LATENCY(t1)

5:
	/* Finally enable the controller */
	li	t2, 0x00000001
	sw	t2, MC_CTRLENA(t1)

	j	ra
	nop

	.end	sdram_init


	.globl	lowlevel_init
	.ent	lowlevel_init
lowlevel_init:

	/* EBU, CGU and SDRAM Initialization.
	 */
	li	a0, CPU_CLOCK_RATE
	move	t0, ra

	/* We rely on the fact that neither ebu_init() nor cgu_init() nor sdram_init()
	 * modify t0 and a0.
	 */
	bal	__cgu_init
	nop
	bal	__ebu_init
	nop
	bal	__sdram_init
	nop
	move	ra, t0

	j	ra
	nop

	.end	lowlevel_init
