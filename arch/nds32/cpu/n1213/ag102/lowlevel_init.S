/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

.text

#include <common.h>
#include <config.h>

#include <asm/macro.h>
#include <generated/asm-offsets.h>

/*
 * parameters for Synopsys DWC DDR2/DDR1 Memory Controller
 */
#define DDR2C_BASE_A		(CONFIG_DWCDDR21MCTL_BASE)
#define DDR2C_CCR_A		(DDR2C_BASE_A + DWCDDR21MCTL_CCR)
#define DDR2C_DCR_A		(DDR2C_BASE_A + DWCDDR21MCTL_DCR)
#define DDR2C_IOCR_A		(DDR2C_BASE_A + DWCDDR21MCTL_IOCR)
#define DDR2C_CSR_A		(DDR2C_BASE_A + DWCDDR21MCTL_CSR)
#define DDR2C_DRR_A		(DDR2C_BASE_A + DWCDDR21MCTL_DRR)
#define DDR2C_DLLCR0_A		(DDR2C_BASE_A + DWCDDR21MCTL_DLLCR0)
#define DDR2C_DLLCR1_A		(DDR2C_BASE_A + DWCDDR21MCTL_DLLCR1)
#define DDR2C_DLLCR2_A		(DDR2C_BASE_A + DWCDDR21MCTL_DLLCR2)
#define DDR2C_DLLCR3_A		(DDR2C_BASE_A + DWCDDR21MCTL_DLLCR3)
#define DDR2C_DLLCR4_A		(DDR2C_BASE_A + DWCDDR21MCTL_DLLCR4)
#define DDR2C_DLLCR5_A		(DDR2C_BASE_A + DWCDDR21MCTL_DLLCR5)
#define DDR2C_DLLCR6_A		(DDR2C_BASE_A + DWCDDR21MCTL_DLLCR6)
#define DDR2C_DLLCR7_A		(DDR2C_BASE_A + DWCDDR21MCTL_DLLCR7)
#define DDR2C_DLLCR8_A		(DDR2C_BASE_A + DWCDDR21MCTL_DLLCR8)
#define DDR2C_DLLCR9_A		(DDR2C_BASE_A + DWCDDR21MCTL_DLLCR9)
#define DDR2C_RSLR0_A		(DDR2C_BASE_A + DWCDDR21MCTL_RSLR0)
#define DDR2C_RDGR0_A		(DDR2C_BASE_A + DWCDDR21MCTL_RDGR0)
#define DDR2C_DTAR_A		(DDR2C_BASE_A + DWCDDR21MCTL_DTAR)
#define DDR2C_MR_A		(DDR2C_BASE_A + DWCDDR21MCTL_MR)

#define DDR2C_CCR_D		CONFIG_SYS_DWCDDR21MCTL_CCR
#define DDR2C_CCR_D2		CONFIG_SYS_DWCDDR21MCTL_CCR2
#define DDR2C_DCR_D		CONFIG_SYS_DWCDDR21MCTL_DCR
#define DDR2C_IOCR_D		CONFIG_SYS_DWCDDR21MCTL_IOCR
#define DDR2C_CSR_D		CONFIG_SYS_DWCDDR21MCTL_CSR
#define DDR2C_DRR_D		CONFIG_SYS_DWCDDR21MCTL_DRR
#define DDR2C_RSLR0_D		CONFIG_SYS_DWCDDR21MCTL_RSLR0
#define DDR2C_RDGR0_D		CONFIG_SYS_DWCDDR21MCTL_RDGR0
#define DDR2C_DTAR_D		CONFIG_SYS_DWCDDR21MCTL_DTAR
#define DDR2C_MR_D		CONFIG_SYS_DWCDDR21MCTL_MR

#define DDR2C_DLLCR0_D	CONFIG_SYS_DWCDDR21MCTL_DLLCR0	/* 0-9 are same */

/*
 * parameters for the ahbc controller
 */
#define AHBC_CR_A		(CONFIG_FTAHBC020S_BASE + FTAHBC020S_CR)
#define AHBC_BSR6_A	(CONFIG_FTAHBC020S_BASE + FTAHBC020S_SLAVE_BSR_6)

#define AHBC_BSR6_D		CONFIG_SYS_FTAHBC020S_SLAVE_BSR_6

/*
 * parameters for the ANDES PCU controller
 */
#define PCU_PCS4_A		(CONFIG_ANDES_PCU_BASE + ANDES_PCU_PCS4)
#define PCU_PCS4_D		CONFIG_SYS_ANDES_CPU_PCS4

/*
 * numeric 7 segment display
 */
.macro	led, num
	write32	CONFIG_DEBUG_LED, \num
.endm

/*
 * Waiting for SDRAM to set up
 */
/*
.macro	wait_sdram
	li	$r0, DDR2C_CSR_A
1:
	lwi	$r1, [$r0+FTSDMC021_CR2]
	bnez	$r1, 1b
.endm
*/

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
.globl	lowlevel_init
lowlevel_init:
	move	$r10, $lp

	/* U200 */
!	led	0x00
!	jal	scale_to_500mhz

	led	0x10
	jal	mem_init

	led	0x20
	jal	remap

#if (defined(NDS32_EXT_FPU_DP) || defined(NDS32_EXT_FPU_SP))
	led	0x2f
	jal	enable_fpu
#endif

	led	0x30
	ret	$r10

scale_to_500mhz:
	move	$r11, $lp

	/*
	 * scale to 500Mhz
	 */
	led	0x01
	write32	PCU_PCS4_A, 0x1102000f			! save data to PCS4

	move	$lp, $r11
	ret

mem_init:
	move	$r11, $lp

	/*
	 * config AHB Controller
	 */
	led	0x12
	write32	AHBC_BSR6_A, AHBC_BSR6_D

	/*
	 * config Synopsys DWC DDR2/DDR1 Memory Controller
	 */
ddr2c_init:
set_dcr:
	led	0x14
	write32	DDR2C_DCR_A, DDR2C_DCR_D			! 0x000020d4

auto_sizing:
	/*
	 * ebios: $r10->$r7, $r11->$r8, $r12->$r9, $r13->$r12, $r14->$r13
	 */
set_iocr:
	led	0x19
	write32 DDR2C_IOCR_A, DDR2C_IOCR_D
set_drr:
	led	0x16
	write32	DDR2C_DRR_A, DDR2C_DRR_D			! 0x00034812
set_dllcr:
	led	0x18
	write32 DDR2C_DLLCR0_A, DDR2C_DLLCR0_D
	write32 DDR2C_DLLCR1_A, DDR2C_DLLCR0_D
	write32 DDR2C_DLLCR2_A, DDR2C_DLLCR0_D
	write32 DDR2C_DLLCR3_A, DDR2C_DLLCR0_D
	write32 DDR2C_DLLCR4_A, DDR2C_DLLCR0_D
	write32 DDR2C_DLLCR5_A, DDR2C_DLLCR0_D
	write32 DDR2C_DLLCR6_A, DDR2C_DLLCR0_D
	write32 DDR2C_DLLCR7_A, DDR2C_DLLCR0_D
	write32 DDR2C_DLLCR8_A, DDR2C_DLLCR0_D
	write32 DDR2C_DLLCR9_A, DDR2C_DLLCR0_D
set_rslr0:
	write32 DDR2C_RSLR0_A, DDR2C_RSLR0_D			! 0x00000040
set_rdgr0:
	write32 DDR2C_RDGR0_A, DDR2C_RDGR0_D			! 0x000055cf
set_dtar:
	led	0x15
	write32	DDR2C_DTAR_A, DDR2C_DTAR_D			! 0x00100000
set_mode:
	led	0x17
	write32	DDR2C_MR_A, DDR2C_MR_D				! 0x00000852
set_ccr:
	write32 DDR2C_CCR_A, DDR2C_CCR_D

#ifdef TRIGGER_INIT:
trigger_init:
	write32 DDR2C_CCR_A, DDR2C_CCR_D			! 0x80020000

	/* Wait for ddr init state to be set */
	msync	ALL
	isb

	/* Wait until the config initialization is finish */
1:
	la	$r4, DDR2C_CSR_A
	lwi	$r5, [$r4]
	srli	$r5, $r5, 23
	bnez	$r5, 1b
#endif

data_training:
!	write32	DDR2C_CCR_A, DDR2C_CCR_D2			! 0x40020004

	/* Wait for ddr init state to be set */
	msync	ALL
	isb

	/* wait until the ddr data trainning is complete */
1:
	la	$r4, DDR2C_CSR_A
	lwi	$r5, [$r4]
	srli	$r6, $r5, 23
	bnez	$r6, 1b

	lwi	$r1, [$r4]
	srli	$r6, $r5, 20
	li	$r5, 0x00ffffff
	swi	$r1, [$r4]
	bnez	$r6, ddr2c_init

	led	0x1a
	move	$lp, $r11
	ret

remap:
	move	$r11, $lp
#ifdef __NDS32_N1213_43U1H__	/* NDS32 V0 ISA - AG101 Only */
	bal	2f
relo_base:
	move	$r0, $lp
#else
relo_base:
	mfusr	$r0, $pc
#endif /* __NDS32_N1213_43U1H__ */

	/*
	 * Remapping
	 */
#ifdef CONFIG_MEM_REMAP
	/*
	 * Copy ROM code to SDRAM base for memory remap layout.
	 * This is not the real relocation, the real relocation is the function
	 * relocate_code() is start.S which supports the systems is memory
	 * remapped or not.
	 */
	/*
	 * Doing memory remap is essential for preparing some non-OS or RTOS
	 * applications.
	 *
	 * This is also a must on ADP-AG101 board.
	 * The reason is because the ROM/FLASH circuit on PCB board.
	 * AG101-A0 board has 2 jumpers MA17 and SW5 to configure which
	 * ROM/FLASH is used to boot.
	 *
	 * When SW5 = "0101", MA17 = LO, the ROM is connected to BANK0,
	 * and the FLASH is connected to BANK1.
	 * When SW5 = "1010", MA17 = HI, the ROM is disabled (still at BANK0),
	 * and the FLASH is connected to BANK0.
	 * It will occur problem when doing flash probing if the flash is at
	 * BANK0 (0x00000000) while memory remapping was skipped.
	 *
	 * Other board like ADP-AG101P may not enable this since there is only
	 * a FLASH connected to bank0.
	 */
	led	0x21
	li	$r4, PHYS_SDRAM_0_AT_INIT		/* 0x10000000 */
	li	$r5, 0x0
	la	$r1, relo_base				/* get $pc or $lp */
	sub	$r2, $r0, $r1
	sethi	$r6, hi20(_end)
	ori	$r6, $r6, lo12(_end)
	add	$r6, $r6, $r2
1:
	lwi.p	$r7, [$r5], #4
	swi.p	$r7, [$r4], #4
	blt	$r5, $r6, 1b

	/* set remap bit */
	/*
	 * MEM remap bit is operational
	 * - use it to map writeable memory at 0x00000000, in place of flash
	 * - before remap: flash/rom 0x00000000, sdram: 0x10000000-0x4fffffff
	 * - after  remap: flash/rom 0x80000000, sdram: 0x00000000
	 */
	led	0x2c
	setbf15	AHBC_CR_A, FTAHBC020S_CR_REMAP		! 0x1

#endif /* #ifdef CONFIG_MEM_REMAP */
	move	$lp, $r11
2:
	ret

	/*
	 * enable_fpu:
	 *  Some of Andes CPU version support FPU coprocessor, if so,
	 *  and toolchain support FPU instruction set, we should enable it.
	 */
#if (defined(NDS32_EXT_FPU_DP) || defined(NDS32_EXT_FPU_SP))
enable_fpu:
	mfsr    $r0, $CPU_VER     /* enable FPU if it exists */
	srli    $r0, $r0, 3
	andi    $r0, $r0, 1
	beqz    $r0, 1f           /* skip if no COP */
	mfsr    $r0, $FUCOP_EXIST
	srli    $r0, $r0, 31
	beqz    $r0, 1f           /* skip if no FPU */
	mfsr    $r0, $FUCOP_CTL
	ori     $r0, $r0, 1
	mtsr    $r0, $FUCOP_CTL
1:
	ret
#endif

.globl show_led
show_led:
    li      $r8, (CONFIG_DEBUG_LED)
    swi     $r7, [$r8]
    ret
#endif /* #ifndef CONFIG_SKIP_LOWLEVEL_INIT */
