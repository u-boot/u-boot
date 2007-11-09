/*
 * Board specific setup info
 *
 * (C) Copyright 2005-2007
 * Samsung Electronics,
 * Kyungmin Park <kyungmin.park@samsung.com>
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
#include <asm/arch/omap2420.h>
#include <asm/arch/mem.h>
#include <asm/arch/clocks.h>
#include "mem.h"

#define APOLLON_CS0_BASE        0x00000000

#ifdef PRCM_CONFIG_I
# define SDRC_ACTIM_CTRLA_0_VAL  0x7BA35907
# define SDRC_ACTIM_CTRLB_0_VAL  0x00000013
# define SDRC_RFR_CTRL_0_VAL     0x00044C01
#elif defined(PRCM_CONFIG_II)
# define SDRC_ACTIM_CTRLA_0_VAL  0x4A59B485
# define SDRC_ACTIM_CTRLB_0_VAL  0x0000000C
# define SDRC_RFR_CTRL_0_VAL     0x00030001
#endif

#define SDRAM_BASE_ADDRESS	0x80008000

_TEXT_BASE:
	.word	TEXT_BASE	/* sdram load addr from config.mk */

.globl lowlevel_init
lowlevel_init:

#ifdef CFG_NOR_BOOT
	/* Check running in SDRAM */
	mov	r0, pc, lsr #28
	cmp	r0, #8
	beq	prcm_setup
	
flash_setup:
	/* In Flash */
	ldr	r0, =WD2_BASE
	ldr	r1, =WD_UNLOCK1
	str	r1, [r0, #WSPR]

	ldr	r1, =WD_UNLOCK2
	str	r1, [r0, #WSPR]

        /* Pin muxing for SDRC */
        mov     r1, #0x00
        ldr     r0, =0x480000A1         /* ball C12, mode 0 */
        strb    r1, [r0]

        ldr     r0, =0x48000032         /* ball D11, mode 0 */
        strb    r1, [r0]

        ldr     r0, =0x480000A3         /* ball B13, mode 0 */
        strb    r1, [r0]

        /* SDRC setting */
        ldr     r0, =OMAP2420_SDRC_BASE
        ldr     r1, =0x00000010
        str     r1, [r0, #0x10]

        ldr     r1, =0x00000100
        str     r1, [r0, #0x44]

        /* SDRC CS0 configuration */
        ldr     r1, =0x00d04011
        str     r1, [r0, #0x80]

        ldr     r1, =SDRC_ACTIM_CTRLA_0_VAL
        str     r1, [r0, #0x9C]

        ldr     r1, =SDRC_ACTIM_CTRLB_0_VAL
        str     r1, [r0, #0xA0]

        ldr     r1, =SDRC_RFR_CTRL_0_VAL
        str     r1, [r0, #0xA4]

        ldr     r1, =0x00000041
        str     r1, [r0, #0x70]

        /* Manual command sequence */
        ldr     r1, =0x00000007
        str     r1, [r0, #0xA8]

        ldr     r1, =0x00000000
        str     r1, [r0, #0xA8]

        ldr     r1, =0x00000001
        str     r1, [r0, #0xA8]

        ldr     r1, =0x00000002
        str     r1, [r0, #0xA8]
        str     r1, [r0, #0xA8]

        /*
         * CS0 SDRC Mode register
         *   Burst length = 4 - DDR memory
         *   Serial mode
         *   CAS latency = 3
         */
        ldr     r1, =0x00000032
        str     r1, [r0, #0x84]

        /* Note: You MUST set EMR values */
        /* EMR1 & EMR2 */
        ldr     r1, =0x00000000
        str     r1, [r0, #0x88]
        str     r1, [r0, #0x8C]

#ifdef OLD_SDRC_DLLA_CTRL
        /* SDRC_DLLA_CTRL */
        ldr     r1, =0x00007306
        str     r1, [r0, #0x60]

        ldr     r1, =0x00007303
        str     r1, [r0, #0x60]
#else
        /* SDRC_DLLA_CTRL */
        ldr     r1, =0x00000506
        str     r1, [r0, #0x60]

        ldr     r1, =0x00000503
        str     r1, [r0, #0x60]
#endif

#ifdef __BROKEN_FEATURE__
        /* SDRC_DLLB_CTRL */
        ldr     r1, =0x00000506
        str     r1, [r0, #0x68]

        ldr     r1, =0x00000503
        str     r1, [r0, #0x68]
#endif

        /* little delay after init */
        mov     r2, #0x1800
1:
        subs    r2, r2, #0x1
        bne     1b

	/* Setup base address */
	ldr	r0, =0x00000000		/* NOR address */
	ldr	r1, =SDRAM_BASE_ADDRESS	/* SDRAM address */
	ldr	r2, =0x20000		/* Size: 128KB */

copy_loop:
	ldmia	r0!, {r3-r10}
	stmia	r1!, {r3-r10}
	cmp	r0, r2
	ble	copy_loop

	ldr	r1, =SDRAM_BASE_ADDRESS
	mov	lr, pc
	mov	pc, r1
#endif

prcm_setup:
        ldr     r0, =OMAP2420_CM_BASE
        ldr     r1, [r0, #0x544]        /* CLKSEL2_PLL */
        bic     r1, r1, #0x03
        orr     r1, r1, #0x02
        str     r1, [r0, #0x544]

        ldr     r1, [r0, #0x500]
        bic     r1, r1, #0x03
        orr     r1, r1, #0x01
        str     r1, [r0, #0x500]

        ldr     r1, [r0, #0x140]
        bic     r1, r1, #0x1f
        orr     r1, r1, #0x02
        str     r1, [r0, #0x140]

#ifdef PRCM_CONFIG_I
	ldr	r1, =0x000003C3
#else
	ldr	r1, =0x00000343
#endif
	str	r1, [r0, #0x840]

	ldr	r1, =0x00000002
	str	r1, [r0, #0x340]
	
        ldr     r1, =CM_CLKSEL1_CORE
#ifdef PRCM_CONFIG_I
	ldr     r2, =0x08300C44
#else
	ldr     r2, =0x04600C26
#endif
        str     r2, [r1]
	
        ldr     r0, =OMAP2420_CM_BASE
        ldr     r1, [r0, #0x084]
        and     r1, r1, #0x01
        cmp     r1, #0x01
        bne     clkvalid

        b       .

clkvalid:
        mov     r1, #0x01
        str     r1, [r0, #0x080]

waitvalid:
        ldr     r1, [r0, #0x084]
        and     r1, r1, #0x01
        cmp     r1, #0x00
        bne     waitvalid

        ldr     r0, =CM_CLKSEL1_PLL
#ifdef PRCM_CONFIG_I
        ldr     r1, =0x01837100
#else
        ldr     r1, =0x01832100
#endif
        str     r1, [r0]

        ldr     r0, =PRCM_CLKCFG_CTRL
        mov     r1, #0x01
        str     r1, [r0]
        mov     r6, #0x50
loop1:
        subs    r6, r6, #0x01
        cmp     r6, #0x01
        bne     loop1

        ldr     r0, =CM_CLKEN_PLL
	mov	r1, #0x0f
        str     r1, [r0]

        mov     r6, #0x100
loop2:
        subs    r6, r6, #0x01
        cmp     r6, #0x01
        bne     loop2

	ldr	r0, =0x48008200
	ldr	r1, =0xbfffffff
	str	r1, [r0]

	ldr	r0, =0x48008210
	ldr	r1, =0xfffffff9
	str	r1, [r0]

	ldr	r0, =0x4806a004
	ldr	r1, =0x00
	strb	r1, [r0]

	ldr	r0, =0x4806a020
	ldr	r1, =0x07
	strb	r1, [r0]

	ldr	r0, =0x4806a00c
	ldr	r1, =0x83
	strb	r1, [r0]

	ldr	r0, =0x4806a000
	ldr	r1, =0x1a
	strb	r1, [r0]

	ldr	r0, =0x4806a004
	ldr	r1, =0x00
	strb	r1, [r0]

	ldr	r0, =0x4806a00c
	ldr	r1, =0x03
	strb	r1, [r0]

	ldr	r0, =0x4806a010
	ldr	r1, =0x03
	strb	r1, [r0]

	ldr	r0, =0x4806a008
	ldr	r1, =0x04
	strb	r1, [r0]

	ldr	r0, =0x4806a020
	ldr	r1, =0x00
	strb	r1, [r0]

#if 0
	ldr	r0, =0x4806a000
	mov	r1, #'u'
	strb	r1, [r0]
#endif

#if 0
	/* LED0 OFF */
	ldr	r3, =0x480000E5
	mov	r4, #0x0b
	strb	r4, [r3]
#endif

	ldr	sp,	SRAM_STACK
	str	ip,	[sp]	/* stash old link register */
	mov	ip,	lr	/* save link reg across call */
	bl	s_init		/* go setup pll,mux,memory */
	ldr	ip,	[sp]	/* restore save ip */
	mov	lr,	ip	/* restore link reg */

	/* map interrupt controller */
	ldr	r0,	VAL_INTH_SETUP
	mcr	p15, 0, r0, c15, c2, 4

	/* back to arch calling code */
	mov	pc,	lr

	/* the literal pools origin */
	.ltorg

VAL_INTH_SETUP:
	.word PERIFERAL_PORT_BASE
SRAM_STACK:
	.word LOW_LEVEL_SRAM_STACK
