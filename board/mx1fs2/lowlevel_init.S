/*
 * Copyright (C) 2004 Sascha Hauer, Pengutronix
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */

#include <config.h>
#include <version.h>
#include <asm/arch/imx-regs.h>

.globl lowlevel_init
lowlevel_init:

	mov	r10, lr

/* Change PERCLK1DIV to 14 ie 14+1 */
	ldr		r0,	=PCDR
	ldr		r1,	=CFG_PCDR_VAL
	str		r1,   [r0]

/* set MCU PLL Control Register 0 */

	ldr		r0,	=MPCTL0
	ldr		r1,	=CFG_MPCTL0_VAL
	str		r1,   [r0]

/* set MCU PLL Control Register 1 */

	ldr		r0,	=MPCTL1
	ldr		r1,	=CFG_MPCTL1_VAL
	str		r1,   [r0]

/* set mpll restart bit */
	ldr		r0, =CSCR
	ldr		r1, [r0]
	orr		r1,r1,#(1<<21)
	str		r1, [r0]

	mov		r2,#0x10
1:
	mov		r3,#0x2000
2:
	subs	r3,r3,#1
	bne		2b

	subs	r2,r2,#1
	bne		1b

/* set System PLL Control Register 0 */

	ldr		r0,	=SPCTL0
	ldr		r1,	=CFG_SPCTL0_VAL
	str		r1,   [r0]

/* set System PLL Control Register 1 */

	ldr		r0,	=SPCTL1
	ldr		r1,	=CFG_SPCTL1_VAL
	str		r1,   [r0]

/* set spll restart bit */
	ldr		r0, =CSCR
	ldr		r1, [r0]
	orr		r1,r1,#(1<<22)
	str		r1, [r0]

	mov		r2,#0x10
1:
	mov		r3,#0x2000
2:
	subs	r3,r3,#1
	bne		2b

	subs	r2,r2,#1
	bne		1b

	ldr		r0,   =CSCR
	ldr		r1,   =CFG_CSCR_VAL
	str		r1,   [r0]

	ldr		r0,   =GPCR
	ldr		r1,   =CFG_GPCR_VAL
	str		r1,   [r0]

/*
 * I have now read the ARM920 DataSheet back-to-Back, and have stumbled upon
 * this.....
 *
 * It would appear that from a Cold-Boot the ARM920T enters "FastBus" mode CP15
 * register 1, this stops it using the output of the PLL and thus runs at the
 * slow rate. Unless you place the Core into "Asynch" mode, the CPU will never
 * use the value set in the CM_OSC registers...regardless of what you set it
 * too!  Thus, although i thought i was running at 140MHz, i'm actually running
 * at 40!..
 *
 * Slapping this into my bootloader does the trick...
 *
 * MRC p15,0,r0,c1,c0,0    ; read core configuration register
 * ORR r0,r0,#0xC0000000   ; set asynchronous clocks and not fastbus mode
 * MCR p15,0,r0,c1,c0,0    ; write modified value to core configuration
 * register
 *
 */
	MRC p15,0,r0,c1,c0,0
/*	ORR r0,r0,#0xC0000000 async mode */
/*	ORR r0,r0,#0x40000000 sync mode */
	ORR r0,r0,#0xC0000000
	MCR p15,0,r0,c1,c0,0

	ldr		r0,	=GIUS(0)
	ldr		r1,	=CFG_GIUS_A_VAL
	str		r1,   [r0]

	ldr		r0,	=FMCR
	ldr		r1,	=CFG_FMCR_VAL
	str		r1,   [r0]

	ldr		r0,	=CS0U
	ldr		r1,	=CFG_CS0U_VAL
	str		r1,   [r0]

	ldr		r0,	=CS0L
	ldr		r1,	=CFG_CS0L_VAL
	str		r1,   [r0]

	ldr		r0,	=CS1U
	ldr		r1,	=CFG_CS1U_VAL
	str		r1,   [r0]

	ldr		r0,	=CS1L
	ldr		r1,	=CFG_CS1L_VAL
	str		r1,   [r0]

	ldr		r0,	=CS4U
	ldr		r1,	=CFG_CS4U_VAL
	str		r1,   [r0]

	ldr		r0,	=CS4L
	ldr		r1,	=CFG_CS4L_VAL
	str		r1,   [r0]

	ldr		r0,	=CS5U
	ldr		r1,	=CFG_CS5U_VAL
	str		r1,   [r0]

	ldr		r0,	=CS5L
	ldr		r1,	=CFG_CS5L_VAL
	str		r1,   [r0]

/* SDRAM Setup */

	ldr  r1,=0x00221000   /* adr of SDCTRL0			*/
	ldr  r0,=0x92120200
	str  r0,[r1,#0]       /* put in precharge command mode    */
	ldr  r2,=0x08200000   /* adr for precharge cmd	   */
	ldr  r0,[r2,#0]       /* precharge			   */
	ldr  r0,=0xA2120200
	ldr  r2,=0x08000000   /* start of SDRAM		   */
	str  r0,[r1,#0]       /* put in auto-refresh mode	   */
	ldr  r0,[r2,#0]       /* auto-refresh		   */
	ldr  r0,[r2,#0]       /* auto-refresh		   */
	ldr  r0,[r2,#0]       /* auto-refresh		   */
	ldr  r0,[r2,#0]       /* auto-refresh		   */
	ldr  r0,[r2,#0]       /* auto-refresh		   */
	ldr  r0,[r2,#0]       /* auto-refresh		   */
	ldr  r0,[r2,#0]       /* auto-refresh		   */
	ldr  r0,=0xB2120200
	ldr  r2,=0x08111800
	str  r0,[r1,#0]       /* setup for mode register of SDRAM */
	ldr  r0,[r2,#0]       /* program mode register	   */
	ldr  r0,=0x82124267
	str  r0,[r1,#0]       /* back to normal operation	   */

	mov	pc,r10
