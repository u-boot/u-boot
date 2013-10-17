/*
 * SPDX-License-Identifier:	GPL-2.0	IBM-pibs
 */
/*------------------------------------------------------------------------- */
/* Function:     ext_bus_cntlr_init */
/* Description:  Initializes the External Bus Controller for the external */
/*		peripherals. IMPORTANT: For pass1 this code must run from */
/*		cache since you can not reliably change a peripheral banks */
/*		timing register (pbxap) while running code from that bank. */
/*		For ex., since we are running from ROM on bank 0, we can NOT */
/*		execute the code that modifies bank 0 timings from ROM, so */
/*		we run it from cache. */
/*                                    */
/*                                    */
/* The layout for the PEI JSE board:  */
/*	Bank 0 - Flash and SRAM       */
/*	Bank 1 - SystemACE            */
/*	Bank 2 - not used             */
/*	Bank 3 - not used             */
/*	Bank 4 - not used             */
/*	Bank 5 - not used             */
/*	Bank 6 - not used             */
/*	Bank 7 - not used             */
/*------------------------------------------------------------------------- */
#include <asm/ppc4xx.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

	.globl	ext_bus_cntlr_init
ext_bus_cntlr_init:
	mflr    r4                      /* save link register */
	bl      ..getAddr
..getAddr:
	mflr    r3                      /* get address of ..getAddr */
	mtlr    r4                      /* restore link register */
	addi    r4,0,14                 /* set ctr to 10; used to prefetch */
	mtctr   r4                      /* 10 cache lines to fit this function */
					/* in cache (gives us 8x10=80 instrctns) */
..ebcloop:
	icbt    r0,r3                   /* prefetch cache line for addr in r3 */
	addi    r3,r3,32		/* move to next cache line */
	bdnz    ..ebcloop               /* continue for 10 cache lines */

	/*----------------------------------------------------------------- */
	/* Delay to ensure all accesses to ROM are complete before changing */
	/* bank 0 timings. 200usec should be enough. */
	/*   200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles */
	/*----------------------------------------------------------------- */
	addis	r3,0,0x0
	ori     r3,r3,0xA000          /* ensure 200usec have passed since reset */
	mtctr   r3
..spinlp:
	bdnz    ..spinlp                /* spin loop */

	/*----------------------------------------------------------------- */
	/* Memory Bank 0 (Flash) initialization */
	/*----------------------------------------------------------------- */

	addi    r4,0,PB1AP
	mtdcr   EBC0_CFGADDR,r4
	addis   r4,0,0x9B01
	ori     r4,r4,0x5480
	mtdcr   EBC0_CFGDATA,r4

	addi    r4,0,PB0CR
	mtdcr   EBC0_CFGADDR,r4
	addis   r4,0,0xFFF1           /* BAS=0xFFF,BS=0x0(1MB),BU=0x3(R/W), */
	ori     r4,r4,0x8000          /* BW=0x0( 8 bits) */
	mtdcr   EBC0_CFGDATA,r4

	blr
