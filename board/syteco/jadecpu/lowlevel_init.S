/*
 * Board specific setup info
 *
 * (C) Copyright 2007, mycable GmbH
 * Carsten Schneider <cs@mycable.de>, Alexander Bigga <ab@mycable.de>
 *
 * (C) Copyright 2003, ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <version.h>
#include <asm/macro.h>
#include <asm/arch/mb86r0x.h>
#include <asm/arch/asm-offsets.h>

/* Set up the platform, once the cpu has been initialized */
.globl lowlevel_init
lowlevel_init:
/*
 * Initialize Clock Reset Generator (CRG)
 */

	ldr		r0, =MB86R0x_CRG_BASE

	/* Not change the initial value that is set by external pin.*/
WAIT_PLL:
	ldr		r2, [r0, #CRG_CRPR]	/* Wait for PLLREADY */
	tst		r2, #MB86R0x_CRG_CRPR_PLLRDY
	beq		WAIT_PLL

	/* Set clock gate control */
	ldr		r1, =CONFIG_SYS_CRG_CRHA_INIT
	str		r1, [r0, #CRG_CRHA]
	ldr		r1, =CONFIG_SYS_CRG_CRPA_INIT
	str		r1, [r0, #CRG_CRPA]
	ldr		r1, =CONFIG_SYS_CRG_CRPB_INIT
	str		r1, [r0, #CRG_CRPB]
	ldr		r1, =CONFIG_SYS_CRG_CRHB_INIT
	str		r1, [r0, #CRG_CRHB]
	ldr		r1, =CONFIG_SYS_CRG_CRAM_INIT
	str		r1, [r0, #CRG_CRAM]

/*
 * Initialize External Bus Interface
 */
	ldr		r0, =MB86R0x_MEMC_BASE

	ldr		r1, =CONFIG_SYS_MEMC_MCFMODE0_INIT
	str		r1, [r0, #MEMC_MCFMODE0]
	ldr		r1, =CONFIG_SYS_MEMC_MCFMODE2_INIT
	str		r1, [r0, #MEMC_MCFMODE2]
	ldr		r1, =CONFIG_SYS_MEMC_MCFMODE4_INIT
	str		r1, [r0, #MEMC_MCFMODE4]

	ldr		r1, =CONFIG_SYS_MEMC_MCFTIM0_INIT
	str		r1, [r0, #MEMC_MCFTIM0]
	ldr		r1, =CONFIG_SYS_MEMC_MCFTIM2_INIT
	str		r1, [r0, #MEMC_MCFTIM2]
	ldr		r1, =CONFIG_SYS_MEMC_MCFTIM4_INIT
	str		r1, [r0, #MEMC_MCFTIM4]

	ldr		r1, =CONFIG_SYS_MEMC_MCFAREA0_INIT
	str		r1, [r0, #MEMC_MCFAREA0]
	ldr		r1, =CONFIG_SYS_MEMC_MCFAREA2_INIT
	str		r1, [r0, #MEMC_MCFAREA2]
	ldr		r1, =CONFIG_SYS_MEMC_MCFAREA4_INIT
	str		r1, [r0, #MEMC_MCFAREA4]

/*
 * Initialize DDR2 Controller
 */

	/* Wait for PLL LOCK up time or more */
	wait_timer	20

	/*
	 * (2) Initialize DDRIF
	 */
	ldr	r0, =MB86R0x_DDR2_BASE
	ldr	r1, =CONFIG_SYS_DDR2_DRIMS_INIT
	strh	r1, [r0, #DDR2_DRIMS]

	/*
	 * (3) Wait for 20MCKPs(120nsec) or more
	 */
	wait_timer	20

	/*
	 * (4) IRESET/IUSRRST release
	 */
	ldr	r0, =MB86R0x_CCNT_BASE
	ldr	r1, =CONFIG_SYS_CCNT_CDCRC_INIT_1
	str	r1, [r0, #CCNT_CDCRC]

	/*
	 * (5) Wait for 20MCKPs(120nsec) or more
	 */
	wait_timer	20

	/*
	 * (6) IDLLRST release
	 */
	ldr	r0, =MB86R0x_CCNT_BASE
	ldr	r1, =CONFIG_SYS_CCNT_CDCRC_INIT_2
	str	r1, [r0, #CCNT_CDCRC]

	/*
	 * (7+8) Wait for 200us(=200000ns) or more (DDR2 Spec)
	 */
	wait_timer	33536

	/*
	 * (9) MCKE ON
	 */
	ldr	r0, =MB86R0x_DDR2_BASE
	ldr	r1, =CONFIG_SYS_DDR2_DRIC1_INIT
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_DRIC2_INIT
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =CONFIG_SYS_DDR2_DRCA_INIT
	strh	r1, [r0, #DDR2_DRCA]
	ldr	r1, =MB86R0x_DDR2_DRCI_INIT
	strh	r1, [r0, #DDR2_DRIC]

	/*
	 * (10) Initialize SDRAM
	 */

	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	wait_timer	67			/* 400ns wait */

	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC1_1
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC2_1
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC1_2
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC2_2
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC1_3
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC2_3
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC1_4
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC2_4
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC1_5
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC2_5
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	wait_timer 200

	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC1_6
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC2_6
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC1_7
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC2_7
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	wait_timer	18			/* 105ns wait */

	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC1_8
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC2_8
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	wait_timer	200			/* MRS to OCD: 200clock */

	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC1_9
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC2_9
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC1_10
	strh	r1, [r0, #DDR2_DRIC1]
	ldr	r1, =CONFIG_SYS_DDR2_INIT_DRIC2_10
	strh	r1, [r0, #DDR2_DRIC2]
	ldr	r1, =MB86R0x_DDR2_DRCI_CMD
	strh	r1, [r0, #DDR2_DRIC]

	ldr	r1, =CONFIG_SYS_DDR2_DRCM_INIT
	strh	r1, [r0, #DDR2_DRCM]

	ldr	r1, =CONFIG_SYS_DDR2_DRCST1_INIT
	strh	r1, [r0, #DDR2_DRCST1]

	ldr	r1, =CONFIG_SYS_DDR2_DRCST2_INIT
	strh	r1, [r0, #DDR2_DRCST2]

	ldr	r1, =CONFIG_SYS_DDR2_DRCR_INIT
	strh	r1, [r0, #DDR2_DRCR]

	ldr	r1, =CONFIG_SYS_DDR2_DRCF_INIT
	strh	r1, [r0, #DDR2_DRCF]

	ldr	r1, =CONFIG_SYS_DDR2_DRASR_INIT
	strh	r1, [r0, #DDR2_DRASR]

	/*
	 * (11) ODT setting
	 */
	ldr	r1, =CONFIG_SYS_DDR2_DROBS_INIT
	strh	r1, [r0, #DDR2_DROBS]
	ldr	r1, =CONFIG_SYS_DDR2_DROABA_INIT
	strh	r1, [r0, #DDR2_DROABA]
	ldr	r1, =CONFIG_SYS_DDR2_DRIBSODT1_INIT
	strh	r1, [r0, #DDR2_DRIBSODT1]

	/*
	 * (12) Shift to ODTCONT ON (SDRAM side) and DDR2 usual operation mode
	 */
	ldr	r1, =CONFIG_SYS_DDR2_DROS_INIT
	strh	r1, [r0, #DDR2_DROS]
	ldr	r1, =MB86R0x_DDR2_DRCI_NORMAL
	strh	r1, [r0, #DDR2_DRIC]

	mov pc, lr
