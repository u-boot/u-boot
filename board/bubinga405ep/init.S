/*------------------------------------------------------------------------------+ */
/* */
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
/*	Bank 0 - Flash and SRAM */
/*	Bank 1 - NVRAM/RTC */
/*	Bank 2 - Keyboard/Mouse controller */
/*	Bank 3 - IR controller */
/*	Bank 4 - not used */
/*	Bank 5 - not used */
/*	Bank 6 - not used */
/*	Bank 7 - FPGA registers */
/*----------------------------------------------------------------------------- */
#include <ppc4xx.h>

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>


/*----------------------------------------------------------------------------- */
/* Function:     sdram_init */
/* Description:  Dummy implementation here - done in C later */
/*----------------------------------------------------------------------------- */
	.globl  sdram_init
sdram_init:
	blr
