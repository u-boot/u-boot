/*
 * Board specific setup info
 *
 * (C) Copyright 2003-2004
 *
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * Modified for OMAP 1610 H2 board by Nishant Kamat, Jan 2004
 *
 * Modified for OMAP730 P2 Board by Dave Peverley, MPC-Data Limited
 * (http://www.mpc-data.co.uk)
 *
 * TODO : Tidy up and change to use system register defines
 *        from omap730.h where possible.
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

#include <config.h>
#include <version.h>

#if defined(CONFIG_OMAP730)
#include <./configs/omap730.h>
#endif

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE	/* sdram load addr from config.mk */

.globl lowlevel_init
lowlevel_init:
	/* Save callers address in r11 - r11 must never be modified */
	mov r11, lr

	/*------------------------------------------------------*
	 *mask all IRQs by setting all bits in the INTMR default*
	 *------------------------------------------------------*/
	mov	r1,	#0xffffffff
	ldr	r0,	=REG_IHL1_MIR
	str	r1,	[r0]
	ldr	r0,	=REG_IHL2_MIR
	str	r1,	[r0]

	/*------------------------------------------------------*
	 * Set up ARM CLM registers (IDLECT1)                   *
	 *------------------------------------------------------*/
	ldr	r0,	REG_ARM_IDLECT1
	ldr	r1,	VAL_ARM_IDLECT1
	str	r1,	[r0]

	/*------------------------------------------------------*
	 * Set up ARM CLM registers (IDLECT2)		        *
	 *------------------------------------------------------*/
	ldr	r0,	REG_ARM_IDLECT2
	ldr	r1,	VAL_ARM_IDLECT2
	str	r1,	[r0]

	/*------------------------------------------------------*
	 * Set up ARM CLM registers (IDLECT3)		        *
	 *------------------------------------------------------*/
	ldr	r0,	REG_ARM_IDLECT3
	ldr	r1,	VAL_ARM_IDLECT3
	str	r1,	[r0]


	mov	r1,	#0x01		/* PER_EN bit */
	ldr	r0,	REG_ARM_RSTCT2
	strh	r1,	[r0]		/* CLKM; Peripheral reset. */

	/* Set CLKM to Sync-Scalable	*/
	/* I supposedly need to enable the dsp clock before switching */
	mov	r1,	#0x1000
	ldr	r0,	REG_ARM_SYSST
	strh	r1,	[r0]
	mov	r0,	#0x400
1:
	subs	r0,	r0,	#0x1	/* wait for any bubbles to finish */
	bne	1b
	ldr	r1,	VAL_ARM_CKCTL
	ldr	r0,	REG_ARM_CKCTL
	strh	r1,	[r0]

	/* a few nops to let settle */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	/* setup DPLL 1 */
	/* Ramp up the clock to 96Mhz */
	ldr	r1,	VAL_DPLL1_CTL
	ldr	r0,	REG_DPLL1_CTL
	strh	r1,	[r0]
	ands	r1,	r1,	#0x10	/* Check if PLL is enabled. */
	beq	lock_end	/* Do not look for lock if BYPASS selected */
2:
	ldrh	r1,	[r0]
	ands	r1,	r1,	#0x01	/*	Check the LOCK bit.*/
	beq 2b			/*	loop until bit goes hi. */
lock_end:

	/*------------------------------------------------------*
	 * Turn off the watchdog during init...			*
	 *------------------------------------------------------*/
	ldr	r0,	REG_WATCHDOG
	ldr	r1,	WATCHDOG_VAL1
	str	r1,	[r0]
	ldr	r1,	WATCHDOG_VAL2
	str	r1,	[r0]
	ldr	r0,	REG_WSPRDOG
	ldr	r1,	WSPRDOG_VAL1
	str	r1,	[r0]
	ldr	r0,	REG_WWPSDOG

watch1Wait:
	ldr	r1,	[r0]
	tst	r1,	#0x10
	bne	watch1Wait

	ldr	r0,	REG_WSPRDOG
	ldr	r1,	WSPRDOG_VAL2
	str	r1,	[r0]
	ldr	r0,	REG_WWPSDOG
watch2Wait:
	ldr	r1,	[r0]
	tst	r1,	#0x10
	bne	watch2Wait

	/* Set memory timings corresponding to the new clock speed */

	/* Check execution location to determine current execution location
	 * and branch to appropriate initialization code.
	 */
	/* Compare physical SDRAM base & current execution location. */
	and     r0, pc, #0xF0000000
	/* Compare. */
	cmp     r0, #0
	/* Skip over EMIF-fast initialization if running from SDRAM. */
	bne	skip_sdram

	/*
	* Delay for SDRAM initialization.
	*/
	mov	r3,	#0x1800		/* value should be checked */
3:
	subs	r3,	r3,	#0x1	/* Decrement count */
	bne	3b

	ldr	r0,	REG_SDRAM_CONFIG
	ldr	r1,	SDRAM_CONFIG_VAL
	str	r1,	[r0]

	ldr	r0,	REG_SDRAM_MRS_LEGACY
	ldr	r1,	SDRAM_MRS_VAL
	str	r1,	[r0]

skip_sdram:

common_tc:
	/* slow interface */
	ldr	r1,	VAL_TC_EMIFS_CS0_CONFIG
	ldr	r0,	REG_TC_EMIFS_CS0_CONFIG
	str	r1,	[r0] /* Chip Select 0 */

	ldr	r1,	VAL_TC_EMIFS_CS1_CONFIG
	ldr	r0,	REG_TC_EMIFS_CS1_CONFIG
	str	r1,	[r0] /* Chip Select 1 */
	ldr	r1,	VAL_TC_EMIFS_CS2_CONFIG
	ldr	r0,	REG_TC_EMIFS_CS2_CONFIG
	str	r1,	[r0] /* Chip Select 2 */
	ldr	r1,	VAL_TC_EMIFS_CS3_CONFIG
	ldr	r0,	REG_TC_EMIFS_CS3_CONFIG
	str	r1,	[r0] /* Chip Select 3 */

	/* 48MHz clock request for UART1 */
	ldr	r1,	PERSEUS2_CONFIG_BASE
	ldrh	r0,	[r1, #CONFIG_PCC_CONF]
	orr	r0,	r0, #CONF_MOD_UART1_CLK_MODE_R
	strh	r0,	[r1, #CONFIG_PCC_CONF]

	/* Initialize public and private rheas
	 *  - set access factor 2 on both rhea / strobe
	 *  - disable write buffer on strb0, enable write buffer on strb1
	 */

	ldr	R0,	REG_RHEA_PUB_CTL
	ldr	R1,	REG_RHEA_PRIV_CTL
	ldr	R2,	VAL_RHEA_CTL
	strh	R2,	[R0]
	strh	R2,	[R1]
	mov	R3,	#2          /* disable write buffer on strb0, enable write buffer on strb1 */
	strh	R3,	[R0, #0x08]	/* arm rhea control reg */
	strh	R3,	[R1, #0x08]

	/* enable IRQ and FIQ */

	mrs	r4,	CPSR
	bic	r4,	r4, #IRQ_MASK
	bic	r4,	r4, #FIQ_MASK
	msr	CPSR,	r4

	/* set TAP CONF to TRI EMULATION */

	ldr	r1,	[r0, #CONFIG_MODE2]
	bic	r1,	r1, #0x18
	orr	r1,	r1, #0x10
	str	r1,	[r0, #CONFIG_MODE2]

	/* set tdbgen to 1 */

	ldr	r0,	PERSEUS2_CONFIG_BASE
	ldr	r1,	[r0, #CONFIG_MODE1]
	mov	r2,	#0x10000
	orr	r1,	r1, r2
	str	r1,	[r0, #CONFIG_MODE1]

#ifdef CONFIG_P2_OMAP1610
	/* inserting additional 2 clock cycle hold time for LAN */
	ldr     r0,     REG_TC_EMIFS_CS1_ADVANCED
	ldr	r1,     VAL_TC_EMIFS_CS1_ADVANCED
	str     r1,     [r0]
#endif
	/* Start MPU Timer 1 */
	ldr	r0,	REG_MPU_LOAD_TIMER
	ldr	r1,	VAL_MPU_LOAD_TIMER
	str	r1,	[r0]

	ldr	r0,	REG_MPU_CNTL_TIMER
	ldr	r1,	VAL_MPU_CNTL_TIMER
	str	r1,	[r0]

	/* back to arch calling code */
	mov	pc,	r11

	/* the literal pools origin */
	.ltorg

REG_TC_EMIFS_CONFIG:		/* 32 bits */
	.word 0xfffecc0c
REG_TC_EMIFS_CS0_CONFIG:	/* 32 bits */
	.word 0xfffecc10
REG_TC_EMIFS_CS1_CONFIG:	/* 32 bits */
	.word 0xfffecc14
REG_TC_EMIFS_CS2_CONFIG:	/* 32 bits */
	.word 0xfffecc18
REG_TC_EMIFS_CS3_CONFIG:	/* 32 bits */
	.word 0xfffecc1c

#ifdef CONFIG_P2_OMAP730
REG_TC_EMIFS_CS1_ADVANCED:	/* 32 bits */
	.word 0xfffecc54
#endif

/* MPU clock/reset/power mode control registers */
REG_ARM_CKCTL:			/* 16 bits */
	.word 0xfffece00

REG_ARM_IDLECT3:		/* 16 bits */
	.word 0xfffece24
REG_ARM_IDLECT2:		/* 16 bits */
	.word 0xfffece08
REG_ARM_IDLECT1:		/* 16 bits */
	.word 0xfffece04

REG_ARM_RSTCT2:			/* 16 bits */
	.word 0xfffece14
REG_ARM_SYSST:			/* 16 bits */
	.word 0xfffece18
/* DPLL control registers */
REG_DPLL1_CTL:			/* 16 bits */
	.word 0xfffecf00

/* Watch Dog register */
/* secure watchdog stop */
REG_WSPRDOG:
	.word 0xfffeb048
/* watchdog write pending */
REG_WWPSDOG:
	.word 0xfffeb034

WSPRDOG_VAL1:
	.word 0x0000aaaa
WSPRDOG_VAL2:
	.word 0x00005555

/* SDRAM config is: auto refresh enabled, 16 bit 4 bank,
 counter @8192 rows, 10 ns, 8 burst */
REG_SDRAM_CONFIG:
	.word 0xfffecc20

REG_SDRAM_MRS_LEGACY:
	.word 0xfffecc24

REG_WATCHDOG:
	.word 0xfffec808

REG_MPU_LOAD_TIMER:
	.word 0xfffec504
REG_MPU_CNTL_TIMER:
	.word 0xfffec500

/* Public and private rhea bridge registers definition */

REG_RHEA_PUB_CTL:
	.word 0xFFFECA00

REG_RHEA_PRIV_CTL:
	.word 0xFFFED300

/* EMIFF SDRAM Configuration register
   - self refresh disable
   - auto refresh enabled
   - SDRAM type 64 Mb, 16 bits bus 4 banks
   - power down enabled
   - SDRAM clock disabled
 */
SDRAM_CONFIG_VAL:
	.word 0x0C017DF4

/* Burst full page length ; cas latency = 3 */
SDRAM_MRS_VAL:
	.word 0x00000037

VAL_ARM_CKCTL:
	.word 0x6505
VAL_DPLL1_CTL:
	.word 0x3412

#ifdef CONFIG_P2_OMAP730
VAL_TC_EMIFS_CS0_CONFIG:
	.word 0x0000FFF3
VAL_TC_EMIFS_CS1_CONFIG:
	.word 0x00004278
VAL_TC_EMIFS_CS2_CONFIG:
	.word 0x00004278
VAL_TC_EMIFS_CS3_CONFIG:
	.word 0x00004278
VAL_TC_EMIFS_CS1_ADVANCED:
	.word 0x00000022
#endif

VAL_ARM_IDLECT1:
	.word 0x00000400
VAL_ARM_IDLECT2:
	.word 0x00000886
VAL_ARM_IDLECT3:
	.word 0x00000015

WATCHDOG_VAL1:
	.word 0x000000f5
WATCHDOG_VAL2:
	.word 0x000000a0

VAL_MPU_LOAD_TIMER:
	.word 0xffffffff
VAL_MPU_CNTL_TIMER:
	.word 0xffffffa1

VAL_RHEA_CTL:
	.word			0xFF22

/* Config Register vals */
PERSEUS2_CONFIG_BASE:
	.word			0xFFFE1000

.equ	CONFIG_PCC_CONF,	0xB4
.equ	CONFIG_MODE1,		0x10
.equ	CONFIG_MODE2,		0x14
.equ	CONF_MOD_UART1_CLK_MODE_R,	0x0A

/* misc values */
.equ	IRQ_MASK,		0x80           /* IRQ mask value */
.equ	FIQ_MASK,		0x40           /* FIQ mask value */
