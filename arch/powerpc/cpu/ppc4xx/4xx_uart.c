/*
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * This source code is dual-licensed.  You may use it under the terms of the
 * GNU General Public License version 2, or under the license below.
 *
 * This source code has been made available to you by IBM on an AS-IS
 * basis.  Anyone receiving this source is licensed under IBM
 * copyrights to use it in any way he or she deems fit, including
 * copying it, modifying it, compiling it, and redistributing it either
 * with or without modifications.  No license under IBM patents or
 * patent applications is to be implied by the copyright license.
 *
 * Any user of this software should understand that IBM cannot provide
 * technical support for this software and will not be responsible for
 * any consequences resulting from the use of this software.
 *
 * Any person who transfers this source code or any derivative work
 * must include the IBM copyright notice, this paragraph, and the
 * preceding two paragraphs in the transferred software.
 *
 * COPYRIGHT   I B M   CORPORATION 1995
 * LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 */

#include <common.h>
#include <commproc.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <watchdog.h>
#include <asm/ppc4xx.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || \
    defined(CONFIG_405EP) || defined(CONFIG_405EZ) || \
    defined(CONFIG_405EX) || defined(CONFIG_440)

#if defined(CONFIG_440)

#if defined(CONFIG_440GP)
#define CR0_MASK        0x3fff0000
#define CR0_EXTCLK_ENA  0x00600000
#define CR0_UDIV_POS    16
#define UDIV_SUBTRACT	1
#define UART0_SDR	CPC0_CR0
#define MFREG(a, d)	d = mfdcr(a)
#define MTREG(a, d)	mtdcr(a, d)
#else /* #if defined(CONFIG_440GP) */
/* all other 440 PPC's access clock divider via sdr register */
#define CR0_MASK        0xdfffffff
#define CR0_EXTCLK_ENA  0x00800000
#define CR0_UDIV_POS    0
#define UDIV_SUBTRACT	0
#define UART0_SDR	SDR0_UART0
#define UART1_SDR	SDR0_UART1
#if defined(CONFIG_440EP) || defined(CONFIG_440EPX) || \
    defined(CONFIG_440GR) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define UART2_SDR	SDR0_UART2
#endif
#if defined(CONFIG_440EP) || defined(CONFIG_440EPX) || \
    defined(CONFIG_440GR) || defined(CONFIG_440GRX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define UART3_SDR	SDR0_UART3
#endif
#define MFREG(a, d)	mfsdr(a, d)
#define MTREG(a, d)	mtsdr(a, d)
#endif /* #if defined(CONFIG_440GP) */
#elif defined(CONFIG_405EP) || defined(CONFIG_405EZ)
#define UCR0_MASK       0x0000007f
#define UCR1_MASK       0x00007f00
#define UCR0_UDIV_POS   0
#define UCR1_UDIV_POS   8
#define UDIV_MAX        127
#elif defined(CONFIG_405EX)
#define MFREG(a, d)	mfsdr(a, d)
#define MTREG(a, d)	mtsdr(a, d)
#define CR0_MASK	0x000000ff
#define CR0_EXTCLK_ENA	0x00800000
#define CR0_UDIV_POS	0
#define UDIV_SUBTRACT	0
#define UART0_SDR	SDR0_UART0
#define UART1_SDR	SDR0_UART1
#else /* CONFIG_405GP || CONFIG_405CR */
#define CR0_MASK        0x00001fff
#define CR0_EXTCLK_ENA  0x000000c0
#define CR0_UDIV_POS    1
#define UDIV_MAX        32
#endif

#if defined(CONFIG_405EP) && defined(CONFIG_SYS_EXT_SERIAL_CLOCK)
#error "External serial clock not supported on AMCC PPC405EP!"
#endif

#if (defined(CONFIG_405EX) || defined(CONFIG_405EZ) ||	\
     defined(CONFIG_440)) && !defined(CONFIG_SYS_EXT_SERIAL_CLOCK)
/*
 * For some SoC's, the cpu clock is on divider chain A, UART on
 * divider chain B ... so cpu clock is irrelevant. Get the
 * "optimized" values that are subject to the 1/2 opb clock
 * constraint.
 */
static u16 serial_bdiv(int baudrate, u32 *udiv)
{
	sys_info_t sysinfo;
	u32 div;		/* total divisor udiv * bdiv */
	u32 umin;		/* minimum udiv	*/
	u16 diff;		/* smallest diff */
	u16 idiff;		/* current diff */
	u16 ibdiv;		/* current bdiv */
	u32 i;
	u32 est;		/* current estimate */
	u32 max;
#if defined(CONFIG_405EZ)
	u32 cpr_pllc;
	u32 plloutb;
	u32 reg;
#endif

	get_sys_info(&sysinfo);

#if defined(CONFIG_405EZ)
	/* check the pll feedback source */
	mfcpr(CPR0_PLLC, cpr_pllc);
	plloutb = ((CONFIG_SYS_CLK_FREQ * ((cpr_pllc & PLLC_SRC_MASK) ?
					   sysinfo.pllFwdDivB : sysinfo.pllFwdDiv) *
		    sysinfo.pllFbkDiv) / sysinfo.pllFwdDivB);
	div = plloutb / (16 * baudrate); /* total divisor */
	umin = (plloutb / get_OPB_freq()) << 1;	/* 2 x OPB divisor */
	max = 256;			/* highest possible */
#else /* 405EZ */
	div = sysinfo.freqPLB / (16 * baudrate); /* total divisor */
	umin = sysinfo.pllOpbDiv << 1;	/* 2 x OPB divisor */
	max = 32;			/* highest possible */
#endif /* 405EZ */

	*udiv = diff = max;

	/*
	 * i is the test udiv value -- start with the largest
	 * possible (max) to minimize serial clock and constrain
	 * search to umin.
	 */
	for (i = max; i > umin; i--) {
		ibdiv = div / i;
		est = i * ibdiv;
		idiff = (est > div) ? (est - div) : (div - est);
		if (idiff == 0) {
			*udiv = i;
			break;		/* can't do better */
		} else if (idiff < diff) {
			*udiv = i;	/* best so far */
			diff = idiff;	/* update lowest diff*/
		}
	}

#if defined(CONFIG_405EZ)
	mfcpr(CPR0_PERD0, reg);
	reg &= ~0x0000ffff;
	reg |= ((*udiv - 0) << 8) | (*udiv - 0);
	mtcpr(CPR0_PERD0, reg);
#endif

	return div / *udiv;
}
#endif /* #if (defined(CONFIG_405EP) ... */

/*
 * This function returns the UART clock used by the common
 * NS16550 driver. Additionally the SoC internal divisors for
 * optimal UART baudrate are configured.
 */
int get_serial_clock(void)
{
	u32 clk;
	u32 udiv;
#if defined(CONFIG_405CR) || defined(CONFIG_405EP) || defined(CONFIG_405GP)
	u32 tmp;
#endif
#if !defined(CONFIG_405EZ)
	u32 reg;
#endif
#if !defined(CONFIG_SYS_EXT_SERIAL_CLOCK)
	PPC4xx_SYS_INFO sys_info;
#endif

	/*
	 * Programming of the internal divisors is SoC specific.
	 * Let's handle this in some #ifdef's for the SoC's.
	 */

#if defined(CONFIG_405CR) || defined(CONFIG_405GP)
	tmp = 0;
	reg = mfdcr(CPC0_CR0) & ~CR0_MASK;
#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
	udiv = 1;
	reg |= CR0_EXTCLK_ENA;
#else /* CONFIG_SYS_EXT_SERIAL_CLOCK */
	clk = gd->cpu_clk;
#ifdef CONFIG_SYS_405_UART_ERRATA_59
	udiv = 31;			/* Errata 59: stuck at 31 */
#else /* CONFIG_SYS_405_UART_ERRATA_59 */
	tmp = CONFIG_SYS_BASE_BAUD * 16;
	udiv = (clk + tmp / 2) / tmp;
	if (udiv > UDIV_MAX)                    /* max. n bits for udiv */
		udiv = UDIV_MAX;
#endif /* CONFIG_SYS_405_UART_ERRATA_59 */
#endif /* CONFIG_SYS_EXT_SERIAL_CLOCK */
	reg |= (udiv - 1) << CR0_UDIV_POS;	/* set the UART divisor */
	mtdcr (CPC0_CR0, reg);
#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
#else
	clk = CONFIG_SYS_BASE_BAUD * 16;
#endif
#endif /* CONFIG_405CR */

#if defined(CONFIG_405EP)
	reg = mfdcr(CPC0_UCR) & ~(UCR0_MASK | UCR1_MASK);
	clk = gd->cpu_clk;
	tmp = CONFIG_SYS_BASE_BAUD * 16;
	udiv = (clk + tmp / 2) / tmp;
	if (udiv > UDIV_MAX)                    /* max. n bits for udiv */
		udiv = UDIV_MAX;
	reg |= udiv << UCR0_UDIV_POS;	        /* set the UART divisor */
	reg |= udiv << UCR1_UDIV_POS;	        /* set the UART divisor */
	mtdcr(CPC0_UCR, reg);
	clk = CONFIG_SYS_BASE_BAUD * 16;
#endif /* CONFIG_405EP */

#if defined(CONFIG_405EX) || defined(CONFIG_440)
	MFREG(UART0_SDR, reg);
	reg &= ~CR0_MASK;

#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	reg |= CR0_EXTCLK_ENA;
	udiv = 1;
	clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
#else /* CONFIG_SYS_EXT_SERIAL_CLOCK */
	clk = gd->baudrate * serial_bdiv(gd->baudrate, &udiv) * 16;
#endif /* CONFIG_SYS_EXT_SERIAL_CLOCK */

	reg |= (udiv - UDIV_SUBTRACT) << CR0_UDIV_POS;	/* set the UART divisor */

	/*
	 * Configure input clock to baudrate generator for all
	 * available serial ports here
	 */
	MTREG(UART0_SDR, reg);
#if defined(UART1_SDR)
	MTREG(UART1_SDR, reg);
#endif
#if defined(UART2_SDR)
	MTREG(UART2_SDR, reg);
#endif
#if defined(UART3_SDR)
	MTREG(UART3_SDR, reg);
#endif
#endif /* CONFIG_405EX ... */

#if defined(CONFIG_405EZ)
	clk = gd->baudrate * serial_bdiv(gd->baudrate, &udiv) * 16;
#endif /* CONFIG_405EZ */

	/*
	 * Correct UART frequency in bd-info struct now that
	 * the UART divisor is available
	 */
#ifdef CONFIG_SYS_EXT_SERIAL_CLOCK
	gd->uart_clk = CONFIG_SYS_EXT_SERIAL_CLOCK;
#else
	get_sys_info(&sys_info);
	gd->uart_clk = sys_info.freqUART / udiv;
#endif

	return clk;
}
#endif	/* CONFIG_405GP || CONFIG_405CR */
