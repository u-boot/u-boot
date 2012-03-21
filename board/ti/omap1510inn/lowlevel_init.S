/*
 * Board specific setup info
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 *
 * -- Some bits of code used from rrload's head_OMAP1510.s --
 * Copyright (C) 2002 RidgeRun, Inc.
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

#if defined(CONFIG_OMAP1510)
#include <./configs/omap1510.h>
#endif

#define OMAP1510_CLKS ((1<<EN_XORPCK)|(1<<EN_PERCK)|(1<<EN_TIMCK)|(1<<EN_GPIOCK))


_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE        /* sdram load addr from config.mk */

.globl lowlevel_init
lowlevel_init:

	/*
	 * Configure 1510 pins functions to match our board.
	 */
	ldr     r0, REG_PULL_DWN_CTRL_0
	ldr     r1, VAL_PULL_DWN_CTRL_0
	str     r1, [r0]
	ldr     r0, REG_PULL_DWN_CTRL_1
	ldr     r1, VAL_PULL_DWN_CTRL_1
	str     r1, [r0]
	ldr     r0, REG_PULL_DWN_CTRL_2
	ldr     r1, VAL_PULL_DWN_CTRL_2
	str     r1, [r0]
	ldr     r0, REG_PULL_DWN_CTRL_3
	ldr     r1, VAL_PULL_DWN_CTRL_3
	str     r1, [r0]
	ldr     r0, REG_FUNC_MUX_CTRL_4
	ldr     r1, VAL_FUNC_MUX_CTRL_4
	str     r1, [r0]
	ldr     r0, REG_FUNC_MUX_CTRL_5
	ldr     r1, VAL_FUNC_MUX_CTRL_5
	str     r1, [r0]
	ldr     r0, REG_FUNC_MUX_CTRL_6
	ldr     r1, VAL_FUNC_MUX_CTRL_6
	str     r1, [r0]
	ldr     r0, REG_FUNC_MUX_CTRL_7
	ldr     r1, VAL_FUNC_MUX_CTRL_7
	str     r1, [r0]
	ldr     r0, REG_FUNC_MUX_CTRL_8
	ldr     r1, VAL_FUNC_MUX_CTRL_8
	str     r1, [r0]
	ldr     r0, REG_FUNC_MUX_CTRL_9
	ldr     r1, VAL_FUNC_MUX_CTRL_9
	str     r1, [r0]
	ldr     r0, REG_FUNC_MUX_CTRL_A
	ldr     r1, VAL_FUNC_MUX_CTRL_A
	str     r1, [r0]
	ldr     r0, REG_FUNC_MUX_CTRL_B
	ldr     r1, VAL_FUNC_MUX_CTRL_B
	str     r1, [r0]
	ldr     r0, REG_FUNC_MUX_CTRL_C
	ldr     r1, VAL_FUNC_MUX_CTRL_C
	str     r1, [r0]
	ldr     r0, REG_FUNC_MUX_CTRL_D
	ldr     r1, VAL_FUNC_MUX_CTRL_D
	str     r1, [r0]
	ldr     r0, REG_VOLTAGE_CTRL_0
	ldr     r1, VAL_VOLTAGE_CTRL_0
	str     r1, [r0]
	ldr     r0, REG_TEST_DBG_CTRL_0
	ldr     r1, VAL_TEST_DBG_CTRL_0
	str     r1, [r0]
	ldr     r0, REG_MOD_CONF_CTRL_0
	ldr     r1, VAL_MOD_CONF_CTRL_0
	str     r1, [r0]

	/* Move to 1510 mode */
	ldr     r0, REG_COMP_MODE_CTRL_0
	ldr     r1, VAL_COMP_MODE_CTRL_0
	str     r1, [r0]

	/* Set up Traffic Ctlr*/
	ldr r0, REG_TC_IMIF_PRIO
	mov r1, #0x0
	str r1, [r0]
	ldr r0, REG_TC_EMIFS_PRIO
	str r1, [r0]
	ldr r0, REG_TC_EMIFF_PRIO
	str r1, [r0]

	ldr r0, REG_TC_EMIFS_CONFIG
	ldr r1, [r0]
	bic r1, r1, #0x08   /* clear the global power-down enable PDE bit */
	bic r1, r1, #0x01   /* write protect flash by clearing the WP bit */
	str r1, [r0]        /* EMIFS GlB Configuration. (value 0x12 most likely) */

	/* Setup some clock domains */
	ldr r1, =OMAP1510_CLKS
	ldr r0, REG_ARM_IDLECT2
	strh r1, [r0]  /* CLKM, Clock domain control. */

	mov r1, #0x01  /* PER_EN bit */
	ldr r0, REG_ARM_RSTCT2
	strh r1, [r0]  /* CLKM; Peripheral reset. */

	/* Set CLKM to Sync-Scalable  */
	/* I supposidly need to enable the dsp clock before switching */
	mov r1, #0x1000
	ldr r0, REG_ARM_SYSST
	strh r1, [r0]
	mov r0, #0x400
1:
	subs r0, r0, #0x1   /* wait for any bubbles to finish */
	bne 1b

	ldr r1, VAL_ARM_CKCTL  /* use 12Mhz ref, PER must be <= 50Mhz so /2 */
	ldr r0, REG_ARM_CKCTL
	strh r1, [r0]

	/* setup DPLL 1 */
	ldr r1, VAL_DPLL1_CTL
	ldr r0, REG_DPLL1_CTL
	strh r1, [r0]
	ands r1, r1, #0x10  /* Check if PLL is enabled. */
	beq lock_end        /* Do not look for lock if BYPASS selected */
2:
	ldrh r1, [r0]
	ands r1, r1, #0x01  /* Check the LOCK bit. */
	beq 2b              /* ...loop until bit goes hi. */
lock_end:

	/* Set memory timings corresponding to the new clock speed */

	/* Check execution location to determine current execution location
	 * and branch to appropriate initialization code.
	 */
	mov r0, #0x10000000                 /* Load physical SDRAM base. */
	mov r1, pc                          /* Get current execution location. */
	/* Zero all but top 6 bits of PC, as they alone detect whether an
	 * address is in the range 0x1000:0000-0x13ff:ffff, the 64M sized
	 * valid range for SDRAM on the OMAP 1510/5910.
	 */
	and r1, r1, #0xfc000000
	cmp r1, r0			/* Compare. */
	beq skip_sdram			/* Skip over EMIF-fast initialization
					 * if running from SDRAM.
					 */

	/*
	 * Delay for SDRAM initialization.
	 */
	mov r3, #0x1800                        /* value should be checked */
3:
	subs r3, r3, #0x1                     /* Decrement count */
	bne 3b

	/*
	 * Set SDRAM control values. Disable refresh before MRS command.
	 */
	ldr r0, VAL_TC_EMIFF_SDRAM_CONFIG   /* get good value */
	bic r3, r0, #0xC                    /* (BIT3|BIT2) ulConfig with auto-refresh disabled. */
	orr r3, r3, #0x8000000              /* (BIT27) Disable CLK when Power down or Self-Refresh */
	orr r3, r3, #0x4000000              /* BIT26 Power Down Enable */
	ldr r2, REG_TC_EMIFF_SDRAM_CONFIG   /* Point to configuration register. */
	str r3, [r2]                        /* Store the passed value with AR disabled. */

	ldr r1, VAL_TC_EMIFF_MRS            /* get MRS value */
	ldr r2, REG_TC_EMIFF_MRS            /* Point to MRS register. */
	str r1, [r2]                        /* Store the passed value.*/

	ldr r2, REG_TC_EMIFF_SDRAM_CONFIG   /* Point to configuration register. */
	str r0, [r2]                        /* Store the passed value. */

	/*
	 * Delay for SDRAM initialization.
	 */
	mov r3, #0x1800
4:
	subs r3, r3, #1                     /* Decrement count. */
	bne 4b

skip_sdram:

	/* slow interface */
	ldr r1, VAL_TC_EMIFS_CS0_CONFIG
	ldr r0, REG_TC_EMIFS_CS0_CONFIG
	str r1, [r0] /* Chip Select 0 */
	ldr r1, VAL_TC_EMIFS_CS1_CONFIG
	ldr r0, REG_TC_EMIFS_CS1_CONFIG
	str r1, [r0] /* Chip Select 1 */
	ldr r1, VAL_TC_EMIFS_CS2_CONFIG
	ldr r0, REG_TC_EMIFS_CS2_CONFIG
	str r1, [r0] /* Chip Select 2 */
	ldr r1, VAL_TC_EMIFS_CS3_CONFIG
	ldr r0, REG_TC_EMIFS_CS3_CONFIG
	str r1, [r0] /* Chip Select 3 */

 /* Next, Enable the RS232 Line Drivers in the FPGA. */
 /* Also, power on the audio CODEC's amplifier here, */
 /* which will make a noise on the audio output. */
 /* This is done here instead of in the kernel so there */
 /* isn't a loud popping noise at the start of each */
 /* song. */
 /* Also, disable the CODEC's clocks. */
 /* omap1510-HelenP1 [specific] */

	ldr r0, REG_FPGA_POWER
	mov r1, #0
	ldr r2, REG_FPGA_DIP_SWITCH
	ldrb r3, [r2]
	cmp r3, #0x8
	movne r1, #0x62     /* Enable the RS232 Line Drivers in the EPLD */
	strb r1, [r0]
	ldr r0, REG_FPGA_AUDIO
	mov r1, #0x0     /* Disable sound driver (CODEC clocks) */
	strb r1, [r0]

	/* back to arch calling code */
	mov	pc, lr

/* the literal pools origin */
	.ltorg

/* OMAP configuration registers */
REG_FUNC_MUX_CTRL_0:		/* 32 bits */
	.word 0xfffe1000
REG_FUNC_MUX_CTRL_1:		/* 32 bits */
	.word 0xfffe1004
REG_FUNC_MUX_CTRL_2:		/* 32 bits */
	.word 0xfffe1008
REG_COMP_MODE_CTRL_0:		/* 32 bits */
	.word 0xfffe100c
REG_FUNC_MUX_CTRL_3:		/* 32 bits */
	.word 0xfffe1010
REG_FUNC_MUX_CTRL_4:		/* 32 bits */
	.word 0xfffe1014
REG_FUNC_MUX_CTRL_5:		/* 32 bits */
	.word 0xfffe1018
REG_FUNC_MUX_CTRL_6:		/* 32 bits */
	.word 0xfffe101c
REG_FUNC_MUX_CTRL_7:		/* 32 bits */
	.word 0xfffe1020
REG_FUNC_MUX_CTRL_8:		/* 32 bits */
	.word 0xfffe1024
REG_FUNC_MUX_CTRL_9:		/* 32 bits */
	.word 0xfffe1028
REG_FUNC_MUX_CTRL_A:		/* 32 bits */
	.word 0xfffe102C
REG_FUNC_MUX_CTRL_B:		/* 32 bits */
	.word 0xfffe1030
REG_FUNC_MUX_CTRL_C:		/* 32 bits */
	.word 0xfffe1034
REG_FUNC_MUX_CTRL_D:		/* 32 bits */
	.word 0xfffe1038
REG_PULL_DWN_CTRL_0:		/* 32 bits */
	.word 0xfffe1040
REG_PULL_DWN_CTRL_1:		/* 32 bits */
	.word 0xfffe1044
REG_PULL_DWN_CTRL_2:		/* 32 bits */
	.word 0xfffe1048
REG_PULL_DWN_CTRL_3:		/* 32 bits */
	.word 0xfffe104c
REG_VOLTAGE_CTRL_0:		/* 32 bits */
	.word 0xfffe1060
REG_TEST_DBG_CTRL_0:		/* 32 bits */
	.word 0xfffe1070
REG_MOD_CONF_CTRL_0:		/* 32 bits */
	.word 0xfffe1080
REG_TC_IMIF_PRIO:		/* 32 bits */
	.word 0xfffecc00
REG_TC_EMIFS_PRIO:		/* 32 bits */
	.word 0xfffecc04
REG_TC_EMIFF_PRIO:		/* 32 bits */
	.word 0xfffecc08
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
REG_TC_EMIFF_SDRAM_CONFIG:	/* 32 bits */
	.word 0xfffecc20
REG_TC_EMIFF_MRS:		/* 32 bits */
	.word 0xfffecc24
/* MPU clock/reset/power mode control registers */
REG_ARM_CKCTL:			/* 16 bits */
	.word 0xfffece00
REG_ARM_IDLECT2:		/* 16 bits */
	.word 0xfffece08
REG_ARM_RSTCT2:			/* 16 bits */
	.word 0xfffece14
REG_ARM_SYSST:			/* 16 bits */
	.word 0xfffece18
/* DPLL control registers */
REG_DPLL1_CTL:			/* 16 bits */
	.word 0xfffecf00
/* identification code register */
REG_IDCODE:			/* 32 bits */
	.word 0xfffed404

/* Innovator specific */
REG_FPGA_LED_DIGIT:		/* 8 bits (not used on Innovator) */
	.word 0x08000003
REG_FPGA_POWER:			/* 8 bits */
	.word 0x08000005
REG_FPGA_AUDIO:			/* 8 bits (not used on Innovator) */
	.word 0x0800000c
REG_FPGA_DIP_SWITCH:		/* 8 bits (not used on Innovator) */
	.word 0x0800000e

VAL_COMP_MODE_CTRL_0:
	.word 0x0000eaef
VAL_FUNC_MUX_CTRL_4:
	.word 0x00000000
VAL_FUNC_MUX_CTRL_5:
	.word 0x00000000
VAL_FUNC_MUX_CTRL_6:
	.word 0x00000001
VAL_FUNC_MUX_CTRL_7:
	.word 0x00000000
VAL_FUNC_MUX_CTRL_8:
	.word 0x10001200
VAL_FUNC_MUX_CTRL_9:
	.word 0x01201012
VAL_FUNC_MUX_CTRL_A:
	.word 0x00000248
VAL_FUNC_MUX_CTRL_B:
	.word 0x00000248
VAL_FUNC_MUX_CTRL_C:
	.word 0x09000000
VAL_FUNC_MUX_CTRL_D:
	.word 0x00000000
VAL_PULL_DWN_CTRL_0:
	.word 0x11a10000
VAL_PULL_DWN_CTRL_1:
	.word 0x2e047fff
VAL_PULL_DWN_CTRL_2:
	.word 0xffd603a6
VAL_PULL_DWN_CTRL_3:
	.word 0x00003e03
VAL_VOLTAGE_CTRL_0:
	.word 0x00000007
VAL_TEST_DBG_CTRL_0:
	/*  See Errata 4.13, This works around a SRAM bug, for chips below ES2.5 .
	 *   This slows down internal SRAM accesses.
	 */
	.word 0x00000007
VAL_MOD_CONF_CTRL_0:
	.word 0x0b000008
VAL_ARM_CKCTL:
	.word 0x010f
VAL_DPLL1_CTL:
	.word 0x2710
VAL_TC_EMIFS_CS1_CONFIG_PRELIM:
	.word 0x00001149
VAL_TC_EMIFS_CS2_CONFIG_PRELIM:
	.word 0x00004158
VAL_TC_EMIFS_CS0_CONFIG:
	.word 0x002130b0
VAL_TC_EMIFS_CS1_CONFIG:
	.word 0x0000f559
VAL_TC_EMIFS_CS2_CONFIG:
	.word 0x000055f0
VAL_TC_EMIFS_CS3_CONFIG:
	.word 0x00003331
VAL_TC_EMIFF_SDRAM_CONFIG:
	.word 0x010290fc
VAL_TC_EMIFF_MRS:
	.word 0x00000027
