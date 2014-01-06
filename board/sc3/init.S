/*
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */
#include <config.h>
#include <asm/ppc4xx.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

/**
 * ext_bus_cntlr_init - Initializes the External Bus Controller for the external peripherals
 *
 * IMPORTANT: For pass1 this code must run from cache since you can not
 * reliably change a peripheral banks timing register (pbxap) while running
 * code from that bank. For ex., since we are running from ROM on bank 0, we
 * can NOT execute the code that modifies bank 0 timings from ROM, so
 * we run it from cache.
 *
 * Bank 0 - Boot-Flash
 * Bank 1 - NAND-Flash
 * Bank 2 - ISA bus
 * Bank 3 - Second Flash
 * Bank 4 - USB controller
 */
	.globl ext_bus_cntlr_init
ext_bus_cntlr_init:
/*
 * We need the current boot up configuration to set correct
 * timings into internal flash and external flash
 */
		mfdcr r24,CPC0_PSR		/* xxxx xxxx xxxx xxx? ?xxx xxxx xxxx xxxx
						   0 0 -> 8 bit external ROM
						   0 1 -> 16 bit internal ROM */
		addi r4,0,2
		srw r24,r24,r4				/* shift right r24 two positions */
		andi. r24,r24,0x06000
/*
 * All calculations are based on 33MHz EBC clock.
 *
 * First, create a "very slow" timing (~250ns) with burst mode enabled
 * This is need for the external flash access
 */
		lis r25,0x0800
		/* 0000 1000 0xxx 0000 0000 0010 100x xxxx = 0x03800280 */
		ori r25,r25,0x0280
/*
 * Second, create a fast timing:
 * 90ns first cycle - 3 clock access
 * and 90ns burst cycle, plus 1 clock after the last access
 * This is used for the internal access
 */
		lis r26,0x8900
		/* 1000 1001 0xxx 0000 0000 0010 100x xxxx */
		ori r26,r26,0x0280
/*
 * We can't change settings on CS# if we currently use them.
 * -> load a few instructions into cache and run this code from cache
 */
		mflr r4					/* save link register */
		bl ..getAddr
..getAddr:
		mflr r3					/* get address of ..getAddr */
		mtlr r4					/* restore link register */
		addi r4,0,14				/* set ctr to 10; used to prefetch */
		mtctr r4				/* 10 cache lines to fit this function
							in cache (gives us 8x10=80 instructions) */
..ebcloop:
		icbt r0,r3				/* prefetch cache line for addr in r3 */
		addi r3,r3,32				/* move to next cache line */
		bdnz ..ebcloop				/* continue for 10 cache lines */
/*
 * Delay to ensure all accesses to ROM are complete before changing
 * bank 0 timings. 200usec should be enough.
 * 200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles
 */
		lis r3,0x0
		ori r3,r3,0xA000			/* ensure 200usec have passed since reset */
		mtctr r3
..spinlp:
		bdnz ..spinlp				/* spin loop */

/*-----------------------------------------------------------------------
 * Memory Bank 0 (BOOT-ROM) initialization
 * 0xFFEF00000....0xFFFFFFF
 * We only have to change the timing. Mapping is ok by boot-strapping
 *----------------------------------------------------------------------- */

		li r4,PB1AP				/* PB0AP=Peripheral Bank 0 Access Parameters */
		mtdcr EBC0_CFGADDR,r4

		mr r4,r26				/* assume internal fast flash is boot flash */
		cmpwi r24,0x2000			/* assumption true? ... */
		beq 1f					/* ...yes! */
		mr r4,r25				/* ...no, use the slow variant */
		mr r25,r26				/* use this for the other flash */
1:
		mtdcr EBC0_CFGDATA,r4			/* change timing now */

		li r4,PB0CR				/* PB0CR=Peripheral Bank 0 Control Register */
		mtdcr EBC0_CFGADDR,r4
		mfdcr r4,EBC0_CFGDATA
		lis r3,0x0001
		ori r3,r3,0x8000			/* allow reads and writes */
		or r4,r4,r3
		mtdcr EBC0_CFGDATA,r4

/*-----------------------------------------------------------------------
 * Memory Bank 3 (Second-Flash) initialization
 * 0xF0000000...0xF01FFFFF -> 2MB
 *----------------------------------------------------------------------- */

		li r4,PB3AP				/* Peripheral Bank 1 Access Parameter */
		mtdcr EBC0_CFGADDR,r4
		mtdcr EBC0_CFGDATA,r2			/* change timing */

		li r4,PB3CR				/* Peripheral Bank 1 Configuration Registers */
		mtdcr EBC0_CFGADDR,r4

		lis r4,0xF003
		ori r4,r4,0x8000
/*
 * Consider boot configuration
 */
		xori r24,r24,0x2000			/* invert current bus width */
		or r4,r4,r24
		mtdcr EBC0_CFGDATA,r4

/*-----------------------------------------------------------------------
 * Memory Bank 1 (NAND-Flash) initialization
 * 0x77D00000...0x77DFFFFF -> 1MB
 * - the write/read pulse to the NAND can be as short as 25ns, bus the cycle time is always 50ns
 * - the setup time is 0ns
 * - the hold time is 15ns
 * ->
 *   - TWT = 0
 *   - CSN = 0
 *   - OEN = 0
 *   - WBN = 0
 *   - WBF = 0
 *   - TH  = 1
 * ----> 2 clocks per cycle = 60ns cycle (30ns active, 30ns hold)
 *----------------------------------------------------------------------- */

		li r4,PB1AP				/* Peripheral Bank 1 Access Parameter */
		mtdcr EBC0_CFGADDR,r4

		lis r4,0x0000
		ori r4,r4,0x0200
		mtdcr EBC0_CFGDATA,r4

		li r4,PB1CR				/* Peripheral Bank 1 Configuration Registers */
		mtdcr EBC0_CFGADDR,r4

		lis r4,0x77D1
		ori r4,r4,0x8000
		mtdcr EBC0_CFGDATA,r4


/* USB init (without acceleration) */
#ifndef CONFIG_ISP1161_PRESENT
		li r4,PB4AP				/* PB4AP=Peripheral Bank 4 Access Parameters */
		mtdcr EBC0_CFGADDR,r4
		lis r4,0x0180
		ori r4,r4,0x5940
		mtdcr EBC0_CFGDATA,r4
#endif

/*-----------------------------------------------------------------------
 * Memory Bank 2 (ISA Access) initialization (plus memory bank 6 and 7)
 * 0x78000000...0x7BFFFFFF -> 64 MB
 * Wir arbeiten bei 33 MHz -> 30ns
 *-----------------------------------------------------------------------

 A7 (ppc notation) or A24 (standard notation) decides about
 the type of access:
 A7/A24=0 -> memory cycle
 A7/ /A24=1 -> I/O cycle
*/
		li r4,PB2AP				/* PB2AP=Peripheral Bank 2 Access Parameters */
		mtdcr EBC0_CFGADDR,r4
/*
 We emulate an ISA access

 1. Address active
 2. wait 0 EBC clocks -> CSN=0
 3. set CS#
 4. wait 0 EBC clock -> OEN/WBN=0
 5. set OE#/WE#
 6. wait 4 clocks (ca. 90ns) and for Ready signal
 7. hold for 4 clocks -> TH=4
*/

#if 1
/* faster access to isa-bus */
		lis r4,0x0180
		ori r4,r4,0x5940
#else
		lis r4,0x0100
		ori r4,r4,0x0340
#endif
		mtdcr EBC0_CFGDATA,r4

#ifdef IDE_USES_ISA_EMULATION
		li r25,PB5AP				/* PB5AP=Peripheral Bank 5 Access Parameters */
		mtdcr EBC0_CFGADDR,r25
		mtdcr EBC0_CFGDATA,r4
#endif

		li r25,PB6AP				/* PB6AP=Peripheral Bank 6 Access Parameters */
		mtdcr EBC0_CFGADDR,r25
		mtdcr EBC0_CFGDATA,r4
		li r25,PB7AP				/* PB7AP=Peripheral Bank 7 Access Parameters */
		mtdcr EBC0_CFGADDR,r25
		mtdcr EBC0_CFGDATA,r4

		li r25,PB2CR				/* PB2CR=Peripheral Bank 2 Configuration Register */
		mtdcr EBC0_CFGADDR,r25

		lis r4,0x780B
		ori r4,r4,0xA000
		mtdcr EBC0_CFGDATA,r4
/*
 * the other areas are only 1MiB in size
 */
		lis r4,0x7401
		ori r4,r4,0xA000

		li r25,PB6CR				/* PB6CR=Peripheral Bank 6 Configuration Register */
		mtdcr EBC0_CFGADDR,r25
		lis r4,0x7401
		ori r4,r4,0xA000
		mtdcr EBC0_CFGDATA,r4

		li r25,PB7CR				/* PB7CR=Peripheral Bank 7 Configuration Register */
		mtdcr EBC0_CFGADDR,r25
		lis r4,0x7411
		ori r4,r4,0xA000
		mtdcr EBC0_CFGDATA,r4

#ifndef CONFIG_ISP1161_PRESENT
		li r25,PB4CR				/* PB4CR=Peripheral Bank 4 Configuration Register */
		mtdcr EBC0_CFGADDR,r25
		lis r4,0x7421
		ori r4,r4,0xA000
		mtdcr EBC0_CFGDATA,r4
#endif
#ifdef IDE_USES_ISA_EMULATION
		li r25,PB5CR				/* PB5CR=Peripheral Bank 5 Configuration Register */
		mtdcr EBC0_CFGADDR,r25
		lis r4,0x0000
		ori r4,r4,0x0000
		mtdcr EBC0_CFGDATA,r4
#endif

/*-----------------------------------------------------------------------
 * Memory bank 4: USB controller Philips ISP6111
 * 0x77C00000 ... 0x77CFFFFF
 *
 * The chip is connected to:
 * - CPU CS#4
 * - CPU IRQ#2
 * - CPU DMA 3
 *
 * Timing:
 * - command to first data: 300ns. Software must ensure this timing!
 * - Write pulse: 26ns
 * - Read pulse: 33ns
 * - read cycle time: 150ns
 * - write cycle time: 140ns
 *
 * Note: All calculations are based on 33MHz EBC clock. One '#' or '_' is 30ns
 *
 *			  |- 300ns --|
 *		  |---- 420ns ---|---- 420ns ---| cycle
 * CS ############:###____#######:###____#######
 * OE ############:####___#######:####___#######
 * WE ############:####__########:####__########
 *
 * ----> 2 clocks RD/WR pulses: 60ns
 * ----> CSN: 3 clock, 90ns
 * ----> OEN: 1 clocks (read cycle)
 * ----> WBN: 1 clocks (write cycle)
 * ----> WBE: 2 clocks
 * ----> TH: 7 clock, 210ns
 * ----> TWT: 7 clocks
 *----------------------------------------------------------------------- */

#ifdef CONFIG_ISP1161_PRESENT

		li r4,PB4AP				/* PB4AP=Peripheral Bank 4 Access Parameters */
		mtdcr EBC0_CFGADDR,r4

		lis r4,0x030D
		ori r4,r4,0x5E80
		mtdcr EBC0_CFGDATA,r4

		li r4,PB4CR				/* PB2CR=Peripheral Bank 4 Configuration Register */
		mtdcr EBC0_CFGADDR,r4

		lis r4,0x77C1
		ori r4,r4,0xA000
		mtdcr EBC0_CFGDATA,r4

#endif

#ifndef IDE_USES_ISA_EMULATION

/*-----------------------------------------------------------------------
 * Memory Bank 5 used for IDE access
 *
 * Timings for IDE Interface
 *
 * SETUP / LENGTH / HOLD - cycles valid for 33.3 MHz clk -> 30ns cycle time
 *  70		165		30		PIO-Mode 0, [ns]
 *   3		  6		 1		[Cycles] ----> AP=0x040C0200
 *  50		125		20	   PIO-Mode 1, [ns]
 *   2		  5		 1		[Cycles] ----> AP=0x03080200
 *  30		100		15	   PIO-Mode 2, [ns]
 *   1		  4		 1		[Cycles] ----> AP=0x02040200
 *  30		 80		10	   PIO-Mode 3, [ns]
 *   1		  3		 1		[Cycles] ----> AP=0x01840200
 *  25		 70		10	   PIO-Mode 4, [ns]
 *   1		  3		 1		[Cycles] ----> AP=0x01840200
 *
 *----------------------------------------------------------------------- */

		li r4,PB5AP
		mtdcr EBC0_CFGADDR,r4
		lis r4,0x040C
		ori r4,r4,0x0200
		mtdcr EBC0_CFGDATA,r4

		li r4,PB5CR			/* PB2CR=Peripheral Bank 2 Configuration Register */
		mtdcr EBC0_CFGADDR,r4

		lis r4,0x7A01
		ori r4,r4,0xA000
		mtdcr EBC0_CFGDATA,r4
#endif
/*
 * External Peripheral Control Register
 */
		li r4,EBC0_CFG
		mtdcr EBC0_CFGADDR,r4

		lis r4,0xB84E
		ori r4,r4,0xF000
		mtdcr EBC0_CFGDATA,r4
/*
 * drive POST code
 */
		lis r4,0x7900
		ori r4,r4,0x0080
		li r3,0x0001
		stb r3,0(r4)			/* 01 -> external bus controller is initialized */
		nop				/* pass2 DCR errata #8 */
		blr
