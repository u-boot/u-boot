/*
 * Board specific setup info
 *
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
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

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE	/* sdram load addr from config.mk */

/**************************************************************************
 * cpy_clk_code: relocates clock code into SRAM where its safer to execute
 * R1 = SRAM destination address.
 *************************************************************************/
.global cpy_clk_code
 cpy_clk_code:
	/* Copy DPLL code into SRAM */
	adr	r0, go_to_speed		/* get addr of clock setting code */
	mov	r2, #384		/* r2 size to copy (div by 32 bytes) */
	mov	r1, r1			/* r1 <- dest address (passed in) */
	add	r2, r2, r0		/* r2 <- source end address */
next2:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp	r0, r2			/* until source end address [r2]    */
	bne	next2
	mov	pc, lr			/* back to caller */

/* ****************************************************************************
 *  go_to_speed: -Moves to bypass, -Commits clock dividers, -puts dpll at speed
 *		 -executed from SRAM.
 *  R0 = PRCM_CLKCFG_CTRL - addr of valid reg
 *  R1 = CM_CLKEN_PLL - addr dpll ctlr reg
 *  R2 = dpll value
 *  R3 = CM_IDLEST_CKGEN - addr dpll lock wait
 ******************************************************************************/
.global go_to_speed
 go_to_speed:
	sub	sp, sp, #0x4 /* get some stack space */
	str	r4, [sp]     /* save r4's value */

	/* move into fast relock bypass */
	ldr	r8, pll_ctl_add
	mov	r4, #0x2
	str	r4, [r8]
	ldr	r4, pll_stat
block:
	ldr	r8, [r4]	/* wait for bypass to take effect */
	and	r8, r8, #0x3
	cmp	r8, #0x1
	bne	block

	/* set new dpll dividers _after_ in bypass */
	ldr	r4, pll_div_add
	ldr	r8, pll_div_val
	str	r8, [r4]

	/* now prepare GPMC (flash) for new dpll speed */
	/* flash needs to be stable when we jump back to it */
	ldr	r4, cfg3_0_addr
	ldr	r8, cfg3_0_val
	str	r8, [r4]
	ldr	r4, cfg4_0_addr
	ldr	r8, cfg4_0_val
	str	r8, [r4]
	ldr	r4, cfg1_0_addr
	ldr	r8, [r4]
	orr	r8, r8, #0x3	 /* up gpmc divider */
	str	r8, [r4]

	/* setup to 2x loop though code.  The first loop pre-loads the
	 * icache, the 2nd commits the prcm config, and locks the dpll
	 */
	mov	r4, #0x1000	 /* spin spin spin */
	mov	r8, #0x4	 /* first pass condition & set registers */
	cmp	r8, #0x4
2:
	ldrne	r8, [r3]	 /* DPLL lock check */
	and	r8, r8, #0x7
	cmp	r8, #0x2
	beq	4f
3:
	subeq	r8, r8, #0x1
	streq	r8, [r0]	 /* commit dividers (2nd time) */
	nop
lloop1:
	sub	r4, r4, #0x1	/* Loop currently necessary else bad jumps */
	nop
	cmp	r4, #0x0
	bne	lloop1
	mov	r4, #0x40000
	cmp	r8, #0x1
	nop
	streq	r2, [r1]	/* lock dpll (2nd time) */
	nop
lloop2:
	sub	r4, r4, #0x1	/* loop currently necessary else bad jumps */
	nop
	cmp	r4, #0x0
	bne	lloop2
	mov	r4, #0x40000
	cmp	r8, #0x1
	nop
	ldreq	r8, [r3]	 /* get lock condition for dpll */
	cmp	r8, #0x4	 /* first time though? */
	bne	2b
	moveq	r8, #0x2	 /* set to dpll check condition. */
	beq	3b		 /* if condition not true branch */
4:
	ldr	r4, [sp]
	add	sp, sp, #0x4	 /* return stack space */
	mov	pc, lr		 /* back to caller, locked */

_go_to_speed: .word go_to_speed

/* these constants need to be close for PIC code */
cfg3_0_addr:
    .word  GPMC_CONFIG3_0
cfg3_0_val:
    .word  H4_24XX_GPMC_CONFIG3_0
cfg4_0_addr:
    .word  GPMC_CONFIG4_0
cfg4_0_val:
    .word  H4_24XX_GPMC_CONFIG4_0
cfg1_0_addr:
    .word  GPMC_CONFIG1_0
pll_ctl_add:
    .word CM_CLKEN_PLL
pll_stat:
    .word CM_IDLEST_CKGEN
pll_div_add:
    .word CM_CLKSEL1_PLL
pll_div_val:
    .word DPLL_VAL	/* DPLL setting (300MHz default) */

.globl lowlevel_init
lowlevel_init:
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

REG_CONTROL_STATUS:
	.word CONTROL_STATUS
VAL_INTH_SETUP:
	.word PERIFERAL_PORT_BASE
SRAM_STACK:
	.word LOW_LEVEL_SRAM_STACK
