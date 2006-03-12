/*
 * U-boot - start.S Startup file of u-boot for BF533
 *
 * Copyright (c) 2005 blackfin.uclinux.org
 *
 * This file is based on head.S
 * Copyright (c) 2003  Metrowerks/Motorola
 * Copyright (C) 1998  D. Jeff Dionne <jeff@ryeham.ee.ryerson.ca>,
 *                     Kenneth Albanowski <kjahds@kjahds.com>,
 *                     The Silver Hammer Group, Ltd.
 * (c) 1995, Dionne & Associates
 * (c) 1995, DKG Display Tech.
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

/*
 * Note: A change in this file subsequently requires a change in
 *       board/$(board_name)/config.mk for a valid u-boot.bin
 */

#define ASSEMBLY

#include <linux/config.h>
#include <asm/blackfin.h>
#include <config.h>
#include <asm/mem_init.h>

#if (CONFIG_CCLK_DIV == 1)
#define CONFIG_CCLK_ACT_DIV   CCLK_DIV1
#endif
#if (CONFIG_CCLK_DIV == 2)
#define CONFIG_CCLK_ACT_DIV   CCLK_DIV2
#endif
#if (CONFIG_CCLK_DIV == 4)
#define CONFIG_CCLK_ACT_DIV   CCLK_DIV4
#endif
#if (CONFIG_CCLK_DIV == 8)
#define CONFIG_CCLK_ACT_DIV   CCLK_DIV8
#endif
#ifndef CONFIG_CCLK_ACT_DIV
#define CONFIG_CCLK_ACT_DIV   CONFIG_CCLK_DIV_not_defined_properly
#endif

.global _stext;
.global __bss_start;
.global start;
.global _start;
.global _rambase;
.global _ramstart;
.global _ramend;
.global _bf533_data_dest;
.global _bf533_data_size;
.global edata;
.global _initialize;
.global _exit;
.global flashdataend;

.text
_start:
start:
_stext:

	R0 = 0x30;
	SYSCFG = R0;
	SSYNC;

	/* As per HW reference manual DAG registers,
	 * DATA and Address resgister shall be zero'd
	 * in initialization, after a reset state
	 */
	r1 = 0;	/* Data registers zero'd */
	r2 = 0;
	r3 = 0;
	r4 = 0;
	r5 = 0;
	r6 = 0;
	r7 = 0;

	p0 = 0; /* Address registers zero'd */
	p1 = 0;
	p2 = 0;
	p3 = 0;
	p4 = 0;
	p5 = 0;

	i0 = 0; /* DAG Registers zero'd */
	i1 = 0;
	i2 = 0;
	i3 = 0;
	m0 = 0;
	m1 = 0;
	m3 = 0;
	m3 = 0;
	l0 = 0;
	l1 = 0;
	l2 = 0;
	l3 = 0;
	b0 = 0;
	b1 = 0;
	b2 = 0;
	b3 = 0;

	/* Set loop counters to zero, to make sure that
	 * hw loops are disabled.
	 */
	lc0 = 0;
	lc1 = 0;

	SSYNC;

	/* Check soft reset status */
	p0.h = SWRST >> 16;
	p0.l = SWRST & 0xFFFF;
	r0.l = w[p0];

	cc = bittst(r0, 15);
	if !cc jump no_soft_reset;

	/* Clear Soft reset */
	r0 = 0x0000;
	w[p0] = r0;
	ssync;

no_soft_reset:
	nop;

	/* Clear EVT registers */
	p0.h = (EVT_EMULATION_ADDR >> 16);
	p0.l = (EVT_EMULATION_ADDR & 0xFFFF);
	p0 += 8;
	p1 = 14;
	r1 = 0;
	LSETUP(4,4) lc0 = p1;
	[ p0 ++ ] = r1;

	/*
	 *  Set PLL_CTL
	 *   - [14:09] = MSEL[5:0] : CLKIN / VCO multiplication factors
	 *   - [8]     = BYPASS    : BYPASS the PLL, run CLKIN into CCLK/SCLK
	 *   - [7]     = output delay (add 200ps of delay to mem signals)
	 *   - [6]     = input delay (add 200ps of input delay to mem signals)
	 *   - [5]     = PDWN      : 1=All Clocks off
	 *   - [3]     = STOPCK    : 1=Core Clock off
	 *   - [1]     = PLL_OFF   : 1=Disable Power to PLL
	 *   - [0]     = DF        : 1=Pass CLKIN/2 to PLL / 0=Pass CLKIN to PLL
	 *   all other bits set to zero
	 */

	r0 = CONFIG_VCO_MULT;	/* Load the VCO multiplier */
	r0 = r0 << 9;		/* Shift it over */
	r1 =  CONFIG_CLKIN_HALF;	/* Do we need to divide CLKIN by 2? */
	r0 = r1 | r0;
	r1 = CONFIG_PLL_BYPASS;	/* Bypass the PLL?                 */
	r1 = r1 << 8;	/* Shift it over */
	r0 = r1 | r0;	/* add them all together */

	p0.h = (PLL_CTL >> 16);
	p0.l = (PLL_CTL & 0xFFFF);	/* Load the address */
	cli r2;				/* Disable interrupts */
	w[p0] = r0;			/* Set the value */
	idle;				/* Wait for the PLL to stablize */
	sti r2;				/* Enable interrupts */
	ssync;

	/*
	 * Turn on the CYCLES COUNTER
	 */
	r2 = SYSCFG;
	BITSET (r2,1);
	SYSCFG = r2;

	/* Configure SCLK & CCLK Dividers */
	r0 = CONFIG_CCLK_ACT_DIV | CONFIG_SCLK_DIV;
	p0.h = (PLL_DIV >> 16);
	p0.l = (PLL_DIV & 0xFFFF);
	w[p0] = r0;
	ssync;

wait_for_pll_stab:
	p0.h = (PLL_STAT >> 16);
	p0.l = (PLL_STAT & 0xFFFF);
	r0.l = w[p0];
	cc = bittst(r0,5);
	if !cc jump wait_for_pll_stab;

	/* Configure SDRAM if SDRAM is already not enabled */
	p0.l = (EBIU_SDSTAT & 0xFFFF);
	p0.h = (EBIU_SDSTAT >> 16);
	r0.l = w[p0];
	cc = bittst(r0, 3);
	if !cc jump skip_sdram_enable;

	/* SDRAM initialization */
	p0.l = (EBIU_SDGCTL & 0xFFFF);
	p0.h = (EBIU_SDGCTL >> 16);	/* SDRAM Memory Global Control Register */
	r0.h = (mem_SDGCTL >> 16);
	r0.l = (mem_SDGCTL & 0xFFFF);
	[p0] = r0;
	ssync;

	p0.l = (EBIU_SDBCTL & 0xFFFF);
	p0.h = (EBIU_SDBCTL >> 16);	/* SDRAM Memory Bank Control Register */
	r0 = mem_SDBCTL;
	w[p0] = r0.l;
	ssync;

	p0.l = (EBIU_SDRRC & 0xFFFF);
	p0.h = (EBIU_SDRRC >> 16);	/* SDRAM Refresh Rate Control Register */
	r0 = mem_SDRRC;
	w[p0] = r0.l;
	ssync;

skip_sdram_enable:
	nop;

#ifndef	CFG_NO_FLASH
	/* relocate into to RAM */
	p1.l = (CFG_FLASH_BASE & 0xffff);
	p1.h = (CFG_FLASH_BASE >> 16);
	p2.l = (CFG_MONITOR_BASE & 0xffff);
	p2.h = (CFG_MONITOR_BASE >> 16);
	r0.l = (CFG_MONITOR_LEN & 0xffff);
	r0.h = (CFG_MONITOR_LEN >> 16);
loop1:
	r1 = [p1];
	[p2] = r1;
	p3=0x4;
	p1=p1+p3;
	p2=p2+p3;
	r2=0x4;
	r0=r0-r2;
	cc=r0==0x0;
	if !cc jump loop1;
#endif
	/*
	 * configure STACK
	 */
	r0.h = (CONFIG_STACKBASE >> 16);
	r0.l = (CONFIG_STACKBASE & 0xFFFF);
	sp = r0;
	fp = sp;

	/*
	 * This next section keeps the processor in supervisor mode
	 * during kernel boot.  Switches to user mode at end of boot.
	 * See page 3-9 of Hardware Reference manual for documentation.
	 */

	/* To keep ourselves in the supervisor mode */
	p0.l = (EVT_IVG15_ADDR & 0xFFFF);
	p0.h = (EVT_IVG15_ADDR >> 16);

	p1.l = _real_start;
	p1.h = _real_start;
	[p0] = p1;

	p0.l = (IMASK & 0xFFFF);
	p0.h = (IMASK >> 16);
	r0 = IVG15_POS;
	[p0] = r0;
	raise 15;
	p0.l = WAIT_HERE;
	p0.h = WAIT_HERE;
	reti = p0;
	rti;

WAIT_HERE:
	jump WAIT_HERE;

.global _real_start;
_real_start:
	[ -- sp ] = reti;

#ifdef CONFIG_EZKIT533
	p0.l = (WDOG_CTL & 0xFFFF);
	p0.h = (WDOG_CTL >> 16);
	r0 = WATCHDOG_DISABLE(z);
	w[p0] = r0;
#endif

	/* Code for initializing Async mem banks */
	p2.h = (EBIU_AMBCTL1 >> 16);
	p2.l = (EBIU_AMBCTL1 & 0xFFFF);
	r0.h = (AMBCTL1VAL >> 16);
	r0.l = (AMBCTL1VAL & 0xFFFF);
	[p2] = r0;
	ssync;

	p2.h = (EBIU_AMBCTL0 >> 16);
	p2.l = (EBIU_AMBCTL0 & 0xFFFF);
	r0.h = (AMBCTL0VAL >> 16);
	r0.l = (AMBCTL0VAL & 0xFFFF);
	[p2] = r0;
	ssync;

	p2.h = (EBIU_AMGCTL >> 16);
	p2.l = (EBIU_AMGCTL & 0xffff);
	r0 = AMGCTLVAL;
	w[p2] = r0;
	ssync;

	/* DMA reset code to Hi of L1 SRAM */
copy:
	P1.H = hi(SYSMMR_BASE);	/* P1 Points to the beginning of SYSTEM MMR Space */
	P1.L = lo(SYSMMR_BASE);

	R0.H = reset_start;	/* Source Address (high) */
	R0.L = reset_start;	/* Source Address (low) */
	R1.H = reset_end;
	R1.L = reset_end;
	R2 = R1 - R0;		/* Count */
	R1.H = hi(L1_ISRAM);	/* Destination Address (high) */
	R1.L = lo(L1_ISRAM);	/* Destination Address (low) */
	R3.L = DMAEN;		/* Source DMAConfig Value (8-bit words) */
	R4.L = (DI_EN | WNR | DMAEN);	/* Destination DMAConfig Value (8-bit words) */

DMA:
	R6 = 0x1 (Z);
	W[P1+OFFSET_(MDMA_S0_X_MODIFY)] = R6;	/* Source Modify = 1 */
	W[P1+OFFSET_(MDMA_D0_X_MODIFY)] = R6;	/* Destination Modify = 1 */

	[P1+OFFSET_(MDMA_S0_START_ADDR)] = R0;	/* Set Source Base Address */
	W[P1+OFFSET_(MDMA_S0_X_COUNT)] = R2;	/* Set Source Count */
	/* Set Source  DMAConfig = DMA Enable,
	Memory Read,  8-Bit Transfers, 1-D DMA, Flow - Stop */
	W[P1+OFFSET_(MDMA_S0_CONFIG)] = R3;

	[P1+OFFSET_(MDMA_D0_START_ADDR)] = R1;	/* Set Destination Base Address */
	W[P1+OFFSET_(MDMA_D0_X_COUNT)] = R2;	/* Set Destination Count */
	/* Set Destination DMAConfig = DMA Enable,
	Memory Write, 8-Bit Transfers, 1-D DMA, Flow - Stop, IOC */
	W[P1+OFFSET_(MDMA_D0_CONFIG)] = R4;

	IDLE;	/* Wait for DMA to Complete */

	R0 = 0x1;
	W[P1+OFFSET_(MDMA_D0_IRQ_STATUS)] = R0;	/* Write 1 to clear DMA interrupt */

	/* DMA reset code to DATA BANK A which uses this port
	 * to avoid following problem
	 * " Data from a Data Cache fill can be corrupoted after or during
	 *   instruction DMA if certain core stalls exist"
	 */

copy_as_data:
	R0.H = reset_start;	/* Source Address (high) */
	R0.L = reset_start;	/* Source Address (low) */
	R1.H = reset_end;
	R1.L = reset_end;
	R2 = R1 - R0;	/* Count */
	R1.H = hi(DATA_BANKA_SRAM);	/* Destination Address (high) */
	R1.L = lo(DATA_BANKA_SRAM);	/* Destination Address (low) */
	R3.L = DMAEN;	/* Source DMAConfig Value (8-bit words) */
	R4.L = (DI_EN | WNR | DMAEN);	/* Destination DMAConfig Value (8-bit words) */

DMA_DATA:
	R6 = 0x1 (Z);
	W[P1+OFFSET_(MDMA_S0_X_MODIFY)] = R6;	/* Source Modify = 1 */
	W[P1+OFFSET_(MDMA_D0_X_MODIFY)] = R6;	/* Destination Modify = 1 */

 	[P1+OFFSET_(MDMA_S0_START_ADDR)] = R0;	/* Set Source Base Address */
	W[P1+OFFSET_(MDMA_S0_X_COUNT)] = R2;	/* Set Source Count */
	/* Set Source      DMAConfig = DMA Enable,
	Memory Read,  8-Bit Transfers, 1-D DMA, Flow - Stop */
	W[P1+OFFSET_(MDMA_S0_CONFIG)] = R3;

	[P1+OFFSET_(MDMA_D0_START_ADDR)] = R1;	/* Set Destination Base Address */
	W[P1+OFFSET_(MDMA_D0_X_COUNT)] = R2;	/* Set Destination Count */
	/* Set Destination DMAConfig = DMA Enable,
	Memory Write, 8-Bit Transfers, 1-D DMA, Flow - Stop, IOC */
	W[P1+OFFSET_(MDMA_D0_CONFIG)] = R4;

	IDLE;	/* Wait for DMA to Complete */

	R0 = 0x1;
	W[P1+OFFSET_(MDMA_D0_IRQ_STATUS)] = R0;	/* Write 1 to clear DMA interrupt */

copy_end: nop;

	/* Initialize BSS Section with 0 s */
	p1.l = __bss_start;
	p1.h = __bss_start;
	p2.l = _end;
	p2.h = _end;
	r1 = p1;
	r2 = p2;
	r3 = r2 - r1;
	r3 = r3 >> 2;
	p3 = r3;
	lsetup (_clear_bss, _clear_bss_end ) lc1 = p3;
	CC = p2<=p1;
	if CC jump _clear_bss_skip;
	r0 = 0;
_clear_bss:
_clear_bss_end:
 	[p1++] = r0;
_clear_bss_skip:

	p0.l = _start1;
	p0.h = _start1;
	jump (p0);

reset_start:
	p0.h = WDOG_CNT >> 16;
	p0.l = WDOG_CNT & 0xffff;
	r0 = 0x0010;
	w[p0] = r0;
	p0.h = WDOG_CTL >> 16;
	p0.l = WDOG_CTL & 0xffff;
	r0 = 0x0000;
	w[p0] = r0;
reset_wait:
	jump reset_wait;

reset_end: nop;

_exit:
	jump.s	_exit;
