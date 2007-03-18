#define ASSEMBLY

#include <linux/config.h>
#include <config.h>
#include <asm/blackfin.h>
#include <asm/mem_init.h>
.global init_sdram;

#if (BFIN_BOOT_MODE != BF537_UART_BOOT)
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
#endif

init_sdram:
	[--SP] = ASTAT;
	[--SP] = RETS;
	[--SP] = (R7:0);
	[--SP] = (P5:0);

#if (BFIN_BOOT_MODE == BF537_SPI_MASTER_BOOT)
	p0.h = hi(SIC_IWR);
	p0.l = lo(SIC_IWR);
	r0.l = 0x1;
	w[p0] = r0.l;
	SSYNC;

	p0.h = hi(SPI_BAUD);
	p0.l = lo(SPI_BAUD);
	r0.l = CONFIG_SPI_BAUD;
	w[p0] = r0.l;
	SSYNC;
#endif

#if (BFIN_BOOT_MODE != BF537_UART_BOOT)

#ifdef CONFIG_BF537
	/* Enable PHY CLK buffer output */
	p0.h = hi(VR_CTL);
	p0.l = lo(VR_CTL);
	r0.l = w[p0];
	bitset(r0, 14);
	w[p0] = r0.l;
	ssync;
#endif
	/*
	 * PLL_LOCKCNT - how many SCLK Cycles to delay while PLL becomes stable
	 */
	p0.h = hi(PLL_LOCKCNT);
	p0.l = lo(PLL_LOCKCNT);
	r0 = 0x300(Z);
	w[p0] = r0.l;
	ssync;

	/*
	 * Put SDRAM in self-refresh, incase anything is running
	 */
	P2.H = hi(EBIU_SDGCTL);
	P2.L = lo(EBIU_SDGCTL);
	R0 = [P2];
	BITSET (R0, 24);
	[P2] = R0;
	SSYNC;

	/*
	 *  Set PLL_CTL with the value that we calculate in R0
	 *   - [14:09] = MSEL[5:0] : CLKIN / VCO multiplication factors
	 *   - [8]     = BYPASS    : BYPASS the PLL, run CLKIN into CCLK/SCLK
	 *   - [7]     = output delay (add 200ps of delay to mem signals)
	 *   - [6]     = input delay (add 200ps of input delay to mem signals)
	 *   - [5]     = PDWN      : 1=All Clocks off
	 *   - [3]     = STOPCK    : 1=Core Clock off
	 *   - [1]     = PLL_OFF   : 1=Disable Power to PLL
	 *   - [0]     = DF	: 1=Pass CLKIN/2 to PLL / 0=Pass CLKIN to PLL
	 *   all other bits set to zero
	 */

	r0 = CONFIG_VCO_MULT & 63;	/* Load the VCO multiplier */
	r0 = r0 << 9;			/* Shift it over */
	r1 = CONFIG_CLKIN_HALF;		/* Do we need to divide CLKIN by 2?*/
	r0 = r1 | r0;
	r1 = CONFIG_PLL_BYPASS;		/* Bypass the PLL? */
	r1 = r1 << 8;			/* Shift it over */
	r0 = r1 | r0;			/* add them all together */

	p0.h = hi(PLL_CTL);
	p0.l = lo(PLL_CTL);		/* Load the address */
	cli r2;				/* Disable interrupts */
	ssync;
	w[p0] = r0.l;			/* Set the value */
	idle;				/* Wait for the PLL to stablize */
	sti r2;				/* Enable interrupts */

check_again:
	p0.h = hi(PLL_STAT);
	p0.l = lo(PLL_STAT);
	R0 = W[P0](Z);
	CC = BITTST(R0,5);
	if ! CC jump check_again;

	/* Configure SCLK & CCLK Dividers */
	r0 = (CONFIG_CCLK_ACT_DIV | CONFIG_SCLK_DIV);
	p0.h = hi(PLL_DIV);
	p0.l = lo(PLL_DIV);
	w[p0] = r0.l;
	ssync;
#endif

	/*
	 * Now, Initialize the SDRAM,
	 * start with the SDRAM Refresh Rate Control Register
	 */
	p0.l = lo(EBIU_SDRRC);
	p0.h = hi(EBIU_SDRRC);
	r0 = mem_SDRRC;
	w[p0] = r0.l;
	ssync;

	/*
	 * SDRAM Memory Bank Control Register - bank specific parameters
	 */
	p0.l = (EBIU_SDBCTL & 0xFFFF);
	p0.h = (EBIU_SDBCTL >> 16);
	r0 = mem_SDBCTL;
	w[p0] = r0.l;
	ssync;

	/*
	 * SDRAM Global Control Register - global programmable parameters
	 * Disable self-refresh
	 */
	P2.H = hi(EBIU_SDGCTL);
	P2.L = lo(EBIU_SDGCTL);
	R0 = [P2];
	BITCLR (R0, 24);

	/*
	 * Check if SDRAM is already powered up, if it is, enable self-refresh
	 */
	p0.h = hi(EBIU_SDSTAT);
	p0.l = lo(EBIU_SDSTAT);
	r2.l = w[p0];
	cc = bittst(r2,3);
	if !cc jump skip;
	NOP;
	BITSET (R0, 23);
skip:
	[P2] = R0;
	SSYNC;

	/* Write in the new value in the register */
	R0.L = lo(mem_SDGCTL);
	R0.H = hi(mem_SDGCTL);
	[P2] = R0;
	SSYNC;
	nop;

	(P5:0) = [SP++];
	(R7:0) = [SP++];
	RETS   = [SP++];
	ASTAT  = [SP++];
	RTS;
