/*------------------------------------------------------------------------------+ */
/* */
/*       This source code is dual-licensed.  You may use it under the terms */
/*       of the GNU General Public License version 2, or under the license  */
/*       below.                                                             */
/*                                                                          */
/*       This source code has been made available to you by IBM on an AS-IS */
/*       basis.  Anyone receiving this source is licensed under IBM */
/*       copyrights to use it in any way he or she deems fit, including */
/*       copying it, modifying it, compiling it, and redistributing it either */
/*       with or without modifications.  No license under IBM patents or */
/*       patent applications is to be implied by the copyright license. */
/* */
/*       Any user of this software should understand that IBM cannot provide */
/*       technical support for this software and will not be responsible for */
/*       any consequences resulting from the use of this software. */
/* */
/*       Any person who transfers this source code or any derivative work */
/*       must include the IBM copyright notice, this paragraph, and the */
/*       preceding two paragraphs in the transferred software. */
/* */
/*       COPYRIGHT   I B M   CORPORATION 1995 */
/*       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M */
/*------------------------------------------------------------------------------- */

/*----------------------------------------------------------------------------- */
/* Function:     ext_bus_cntlr_init */
/* Description:  Initializes the External Bus Controller for the external */
/*		peripherals. IMPORTANT: For pass1 this code must run from */
/*		cache since you can not reliably change a peripheral banks */
/*		timing register (pbxap) while running code from that bank. */
/*		For ex., since we are running from ROM on bank 0, we can NOT */
/*		execute the code that modifies bank 0 timings from ROM, so */
/*		we run it from cache. */
/* */
/*----------------------------------------------------------------------------- */
#include <config.h>
#include <ppc4xx.h>

#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

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

	/*------------------------------------------------------------------- */
	/* Delay to ensure all accesses to ROM are complete before changing */
	/* bank 0 timings. 200usec should be enough. */
	/*   200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles */
	/*------------------------------------------------------------------- */
	addis	r3,0,0x0
	ori     r3,r3,0xA000          /* ensure 200usec have passed since reset */
	mtctr   r3
..spinlp:
	bdnz    ..spinlp                /* spin loop */

	/*----------------------------------------------------------------------- */
	/* Memory Bank 0 (Flash) initialization (from openbios) */
	/*----------------------------------------------------------------------- */

	addi    r4,0,pb0ap
	mtdcr   ebccfga,r4
	addis   r4,0,CS0_AP@h
	ori     r4,r4,CS0_AP@l
	mtdcr   ebccfgd,r4

	addi    r4,0,pb0cr
	mtdcr   ebccfga,r4
	addis   r4,0,CS0_CR@h
	ori     r4,r4,CS0_CR@l
	mtdcr   ebccfgd,r4

	/*----------------------------------------------------------------------- */
	/* Memory Bank 1 (NVRAM/RTC) initialization */
	/*----------------------------------------------------------------------- */

	addi    r4,0,pb1ap
	mtdcr   ebccfga,r4
	addis   r4,0,CS1_AP@h
	ori     r4,r4,CS1_AP@l
	mtdcr   ebccfgd,r4

	addi    r4,0,pb1cr
	mtdcr   ebccfga,r4
	addis   r4,0,CS1_CR@h
	ori     r4,r4,CS1_CR@l
	mtdcr   ebccfgd,r4

	/*----------------------------------------------------------------------- */
	/* Memory Bank 2 (A/D converter) initialization */
	/*----------------------------------------------------------------------- */

	addi    r4,0,pb2ap
	mtdcr   ebccfga,r4
	addis   r4,0,CS2_AP@h
	ori     r4,r4,CS2_AP@l
	mtdcr   ebccfgd,r4

	addi    r4,0,pb2cr
	mtdcr   ebccfga,r4
	addis   r4,0,CS2_CR@h
	ori     r4,r4,CS2_CR@l
	mtdcr   ebccfgd,r4

	/*----------------------------------------------------------------------- */
	/* Memory Bank 3 (Ethernet PHY Reset) initialization */
	/*----------------------------------------------------------------------- */

	addi    r4,0,pb3ap
	mtdcr   ebccfga,r4
	addis   r4,0,CS3_AP@h
	ori     r4,r4,CS3_AP@l
	mtdcr   ebccfgd,r4

	addi    r4,0,pb3cr
	mtdcr   ebccfga,r4
	addis   r4,0,CS3_CR@h
	ori     r4,r4,CS3_CR@l
	mtdcr   ebccfgd,r4

	/*----------------------------------------------------------------------- */
	/* Memory Bank 4 (PC-MIP PRSNT1#) initialization */
	/*----------------------------------------------------------------------- */

	addi    r4,0,pb4ap
	mtdcr   ebccfga,r4
	addis   r4,0,CS4_AP@h
	ori     r4,r4,CS4_AP@l
	mtdcr   ebccfgd,r4

	addi    r4,0,pb4cr
	mtdcr   ebccfga,r4
	addis   r4,0,CS4_CR@h
	ori     r4,r4,CS4_CR@l
	mtdcr   ebccfgd,r4

	/*----------------------------------------------------------------------- */
	/* Memory Bank 5 (PC-MIP PRSNT2#) initialization */
	/*----------------------------------------------------------------------- */

	addi    r4,0,pb5ap
	mtdcr   ebccfga,r4
	addis   r4,0,CS5_AP@h
	ori     r4,r4,CS5_AP@l
	mtdcr   ebccfgd,r4

	addi    r4,0,pb5cr
	mtdcr   ebccfga,r4
	addis   r4,0,CS5_CR@h
	ori     r4,r4,CS5_CR@l
	mtdcr   ebccfgd,r4

	/*----------------------------------------------------------------------- */
	/* Memory Bank 6 (CPU LED0) initialization */
	/*----------------------------------------------------------------------- */

	addi    r4,0,pb6ap
	mtdcr   ebccfga,r4
	addis   r4,0,CS6_AP@h
	ori     r4,r4,CS6_AP@l
	mtdcr   ebccfgd,r4

	addi    r4,0,pb6cr
	mtdcr   ebccfga,r4
	addis   r4,0,CS6_CR@h
	ori     r4,r4,CS5_CR@l
	mtdcr   ebccfgd,r4

	/*----------------------------------------------------------------------- */
	/* Memory Bank 7 (CPU LED1) initialization */
	/*----------------------------------------------------------------------- */

	addi    r4,0,pb7ap
	mtdcr   ebccfga,r4
	addis   r4,0,CS7_AP@h
	ori     r4,r4,CS7_AP@l
	mtdcr   ebccfgd,r4

	addi    r4,0,pb7cr
	mtdcr   ebccfga,r4
	addis   r4,0,CS7_CR@h
	ori     r4,r4,CS7_CR@l
	mtdcr   ebccfgd,r4

/*	addis   r4,r0,FPGA_BRDC@h */
/*	ori     r4,r4,FPGA_BRDC@l */
/*	lbz     r3,0(r4)                /###*get FPGA board control reg */
/*	eieio */
/*	ori	r3,r3,0x01              /###*set UART1 control to select CTS/RTS */
/*	stb     r3,0(r4) */

	nop				/* pass2 DCR errata #8 */
	blr

/*----------------------------------------------------------------------------- */
/* Function:     sdram_init */
/* Description:  Configures SDRAM memory banks on ERIC. */
/*               We do manually init our SDRAM. */
/*               If we have two SDRAM banks, simply undef SINGLE_BANK (ROLF :-) */
/*		 It is assumed that a 32MB 12x8(2) SDRAM is used. */
/*----------------------------------------------------------------------------- */
	.globl  sdram_init

sdram_init:

	mflr	r31

#ifdef CONFIG_SYS_SDRAM_MANUALLY
	/*------------------------------------------------------------------- */
	/* Set MB0CF for bank 0. (0-32MB) Address Mode 4 since 12x8(2) */
	/*------------------------------------------------------------------- */

	addi    r4,0,mem_mb0cf
	mtdcr   memcfga,r4
	addis   r4,0,MB0CF@h
	ori     r4,r4,MB0CF@l
	mtdcr   memcfgd,r4

	/*------------------------------------------------------------------- */
	/* Set MB1CF for bank 1. (32MB-64MB) Address Mode 4 since 12x8(2) */
	/*------------------------------------------------------------------- */

	addi    r4,0,mem_mb1cf
	mtdcr   memcfga,r4
	addis   r4,0,MB1CF@h
	ori     r4,r4,MB1CF@l
	mtdcr   memcfgd,r4

	/*------------------------------------------------------------------- */
	/* Set MB2CF for bank 2. off */
	/*------------------------------------------------------------------- */

	addi    r4,0,mem_mb2cf
	mtdcr   memcfga,r4
	addis   r4,0,MB2CF@h
	ori     r4,r4,MB2CF@l
	mtdcr   memcfgd,r4

	/*------------------------------------------------------------------- */
	/* Set MB3CF for bank 3. off */
	/*------------------------------------------------------------------- */

	addi    r4,0,mem_mb3cf
	mtdcr   memcfga,r4
	addis   r4,0,MB3CF@h
	ori     r4,r4,MB3CF@l
	mtdcr   memcfgd,r4

	/*------------------------------------------------------------------- */
	/* Set the SDRAM Timing reg, SDTR1 and the refresh timer reg, RTR. */
	/* To set the appropriate timings, we need to know the SDRAM speed. */
	/* We can use the PLB speed since the SDRAM speed is the same as */
	/* the PLB speed. The PLB speed is the FBK divider times the */
	/* 405GP reference clock, which on the Walnut board is 33Mhz. */
	/* Thus, if FBK div is 2, SDRAM is 66Mhz; if FBK div is 3, SDRAM is */
	/* 100Mhz; if FBK is 3, SDRAM is 133Mhz. */
	/* NOTE: The Walnut board supports SDRAM speeds of 66Mhz, 100Mhz, and */
	/* maybe 133Mhz. */
	/*------------------------------------------------------------------- */

	mfdcr   r5,strap                 /* determine FBK divider */
					  /* via STRAP reg to calc PLB speed. */
					  /* SDRAM speed is the same as the PLB */
					  /* speed. */
	rlwinm  r4,r5,4,0x3             /* get FBK divide bits */

..chk_66:
	cmpi    %cr0,0,r4,0x1
	bne     ..chk_100
	addis	r6,0,SDTR_66@h		/* SDTR1 value for 66Mhz */
	ori     r6,r6,SDTR_66@l
	addis	r7,0,RTR_66		/* RTR value for 66Mhz */
	b	..sdram_ok
..chk_100:
	cmpi    %cr0,0,r4,0x2
	bne     ..chk_133
	addis   r6,0,SDTR_100@h        /* SDTR1 value for 100Mhz */
	ori     r6,r6,SDTR_100@l
	addis   r7,0,RTR_100           /* RTR value for 100Mhz */
	b       ..sdram_ok
..chk_133:
	addis   r6,0,0x0107            /* SDTR1 value for 133Mhz */
	ori     r6,r6,0x4015
	addis   r7,0,0x07F0            /* RTR value for 133Mhz */

..sdram_ok:
	/*------------------------------------------------------------------- */
	/* Set SDTR1 */
	/*------------------------------------------------------------------- */
	addi    r4,0,mem_sdtr1
	mtdcr   memcfga,r4
	mtdcr   memcfgd,r6

	/*------------------------------------------------------------------- */
	/* Set RTR */
	/*------------------------------------------------------------------- */
	addi    r4,0,mem_rtr
	mtdcr   memcfga,r4
	mtdcr   memcfgd,r7

	/*------------------------------------------------------------------- */
	/* Delay to ensure 200usec have elapsed since reset. Assume worst */
	/* case that the core is running 200Mhz: */
	/*   200,000,000 (cycles/sec) X .000200 (sec) = 0x9C40 cycles */
	/*------------------------------------------------------------------- */
	addis   r3,0,0x0000
	ori     r3,r3,0xA000          /* ensure 200usec have passed since reset */
	mtctr   r3
..spinlp2:
	bdnz    ..spinlp2               /* spin loop */

	/*------------------------------------------------------------------- */
	/* Set memory controller options reg, MCOPT1. */
	/* Set DC_EN to '1' and BRD_PRF to '01' for 16 byte PLB Burst */
	/* read/prefetch. */
	/*------------------------------------------------------------------- */
	addi    r4,0,mem_mcopt1
	mtdcr   memcfga,r4
	addis   r4,0,0x8080             /* set DC_EN=1 */
	ori     r4,r4,0x0000
	mtdcr   memcfgd,r4

	/*------------------------------------------------------------------- */
	/* Delay to ensure 10msec have elapsed since reset. This is */
	/* required for the MPC952 to stabalize. Assume worst */
	/* case that the core is running 200Mhz: */
	/*   200,000,000 (cycles/sec) X .010 (sec) = 0x1E8480 cycles */
	/* This delay should occur before accessing SDRAM. */
	/*------------------------------------------------------------------- */
	addis   r3,0,0x001E
	ori     r3,r3,0x8480          /* ensure 10msec have passed since reset */
	mtctr   r3
..spinlp3:
	bdnz    ..spinlp3                /* spin loop */

#else
/*fixme: do SDRAM Autoconfig from EEPROM here */

#endif
	mtlr    r31                     /* restore lr */
	blr
