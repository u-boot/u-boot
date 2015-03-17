/*
 * For clock initialization, see chapter 3 of the "MCIMX27 Multimedia
 * Applications Processor Reference Manual, Rev. 0.2".
 *
 * (C) Copyright 2008 Eric Jarrige <eric.jarrige@armadeus.org>
 * (C) Copyright 2009 Ilya Yanok <yanok@emcraft.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <config.h>
#include <asm/macro.h>
#include <asm/arch/imx-regs.h>
#include <generated/asm-offsets.h>

SOC_ESDCTL_BASE_W:	.word	IMX_ESD_BASE
SOC_SI_ID_REG_W:	.word	IMX_SYSTEM_CTL_BASE
SDRAM_ESDCFG_T1_W:	.word	SDRAM_ESDCFG_REGISTER_VAL(0)
SDRAM_ESDCFG_T2_W:	.word	SDRAM_ESDCFG_REGISTER_VAL(3)
SDRAM_PRECHARGE_CMD_W:	.word	(ESDCTL_SDE | ESDCTL_SMODE_PRECHARGE | \
				 ESDCTL_ROW13 | ESDCTL_COL10)
SDRAM_AUTOREF_CMD_W:	.word	(ESDCTL_SDE | ESDCTL_SMODE_AUTO_REF | \
				 ESDCTL_ROW13 | ESDCTL_COL10)
SDRAM_LOADMODE_CMD_W:	.word	(ESDCTL_SDE | ESDCTL_SMODE_LOAD_MODE | \
				 ESDCTL_ROW13 | ESDCTL_COL10)
SDRAM_NORMAL_CMD_W:	.word	SDRAM_ESDCTL_REGISTER_VAL

.macro init_aipi
	/*
	 * setup AIPI1 and AIPI2
	 */
	write32 AIPI1_PSR0, AIPI1_PSR0_VAL
	write32 AIPI1_PSR1, AIPI1_PSR1_VAL
	write32 AIPI2_PSR0, AIPI2_PSR0_VAL
	write32 AIPI2_PSR1, AIPI2_PSR1_VAL

.endm /* init_aipi */

.macro init_clock
	ldr r0, =CSCR
	/* disable MPLL/SPLL first */
	ldr r1, [r0]
	bic r1, r1, #(CSCR_MPEN|CSCR_SPEN)
	str r1, [r0]

	write32 MPCTL0, MPCTL0_VAL
	write32 SPCTL0, SPCTL0_VAL

	write32 CSCR, CSCR_VAL | CSCR_MPLL_RESTART | CSCR_SPLL_RESTART

	/*
	 * add some delay here
	 */
	wait_timer 0x1000

	/* peripheral clock divider */
	write32 PCDR0, PCDR0_VAL
	write32 PCDR1, PCDR1_VAL

	/* Configure PCCR0 and PCCR1 */
	write32 PCCR0, PCCR0_VAL
	write32 PCCR1, PCCR1_VAL

.endm /* init_clock */

.macro sdram_init
	ldr r0, SOC_ESDCTL_BASE_W
	mov r2, #PHYS_SDRAM_1

	/* Do initial reset */
	mov r1, #ESDMISC_MDDR_DL_RST
	str r1, [r0, #ESDMISC_ROF]

	/* Hold for more than 200ns */
	wait_timer 0x10000

	/* Activate LPDDR iface */
	mov r1, #ESDMISC_MDDREN
	str r1, [r0, #ESDMISC_ROF]

	/* Check The chip version TO1 or TO2 */
	ldr r1, SOC_SI_ID_REG_W
	ldr r1, [r1]
	ands r1, r1, #0xF0000000
	/* add Latency on CAS only for TO2 */
	ldreq r1, SDRAM_ESDCFG_T2_W
	ldrne r1, SDRAM_ESDCFG_T1_W
	str r1, [r0, #ESDCFG0_ROF]

	/* Run initialization sequence */
	ldr r1, SDRAM_PRECHARGE_CMD_W
	str r1, [r0, #ESDCTL0_ROF]
	ldr r1, [r2, #SDRAM_ALL_VAL]

	ldr r1, SDRAM_AUTOREF_CMD_W
	str r1, [r0, #ESDCTL0_ROF]
	ldr r1, [r2, #SDRAM_ALL_VAL]
	ldr r1, [r2, #SDRAM_ALL_VAL]

	ldr r1, SDRAM_LOADMODE_CMD_W
	str r1, [r0, #ESDCTL0_ROF]
	ldrb r1, [r2, #SDRAM_MODE_REGISTER_VAL]
	add r3, r2, #SDRAM_EXT_MODE_REGISTER_VAL
	ldrb r1, [r3]

	ldr r1, SDRAM_NORMAL_CMD_W
	str r1, [r0, #ESDCTL0_ROF]

#if (CONFIG_NR_DRAM_BANKS > 1)
	/* 2nd sdram */
	mov r2, #PHYS_SDRAM_2

	/* Check The chip version TO1 or TO2 */
	ldr r1, SOC_SI_ID_REG_W
	ldr r1, [r1]
	ands r1, r1, #0xF0000000
	/* add Latency on CAS only for TO2 */
	ldreq r1, SDRAM_ESDCFG_T2_W
	ldrne r1, SDRAM_ESDCFG_T1_W
	str r1, [r0, #ESDCFG1_ROF]

	/* Run initialization sequence */
	ldr r1, SDRAM_PRECHARGE_CMD_W
	str r1, [r0, #ESDCTL1_ROF]
	ldr r1, [r2, #SDRAM_ALL_VAL]

	ldr r1, SDRAM_AUTOREF_CMD_W
	str r1, [r0, #ESDCTL1_ROF]
	ldr r1, [r2, #SDRAM_ALL_VAL]
	ldr r1, [r2, #SDRAM_ALL_VAL]

	ldr r1, SDRAM_LOADMODE_CMD_W
	str r1, [r0, #ESDCTL1_ROF]
	ldrb r1, [r2, #SDRAM_MODE_REGISTER_VAL]
	add r3, r2, #SDRAM_EXT_MODE_REGISTER_VAL
	ldrb r1, [r3]

	ldr r1, SDRAM_NORMAL_CMD_W
	str r1, [r0, #ESDCTL1_ROF]
#endif  /* CONFIG_NR_DRAM_BANKS > 1 */

.endm /* sdram_init */

.globl	lowlevel_init
lowlevel_init:

	mov	r10, lr

	init_aipi

	init_clock

	sdram_init

	mov	pc,r10
