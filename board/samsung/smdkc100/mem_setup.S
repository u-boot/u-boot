/*
 * Originates from Samsung's u-boot 1.1.6 port to S5PC1xx
 *
 * Copyright (C) 2009 Samsung Electrnoics
 * Inki Dae <inki.dae@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 * Minkyu Kang <mk7.kang@samsung.com>
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:
	ldr	r6, =S5PC100_DMC_BASE			@ 0xE6000000

	/* DLL parameter setting */
	ldr	r1, =0x50101000
	str	r1, [r6, #0x018]			@ PHYCONTROL0
	ldr	r1, =0xf4
	str	r1, [r6, #0x01C]			@ PHYCONTROL1
	ldr	r1, =0x0
	str	r1, [r6, #0x020]			@ PHYCONTROL2

	/* DLL on */
	ldr	r1, =0x50101002
	str	r1, [r6, #0x018]			@ PHYCONTROL0

	/* DLL start */
	ldr	r1, =0x50101003
	str	r1, [r6, #0x018]			@ PHYCONTROL0

	/* Force value locking for DLL off */
	str	r1, [r6, #0x018]			@ PHYCONTROL0

	/* DLL off */
	ldr	r1, =0x50101001
	str	r1, [r6, #0x018]			@ PHYCONTROL0

	/* auto refresh off */
	ldr	r1, =0xff001010
	str	r1, [r6, #0x000]			@ CONCONTROL

	/*
	 * Burst Length 4, 2 chips, 32-bit, LPDDR
	 * OFF: dynamic self refresh, force precharge, dynamic power down off
	 */
	ldr	r1, =0x00212100
	str	r1, [r6, #0x004]			@ MEMCONTROL

	/*
	 * Note:
	 * If Bank0 has OneDRAM we place it at 0x2800'0000
	 * So finally Bank1 should address start at at 0x2000'0000
	 */
	mov	r4, #0x0

swap_memory:
	/*
	 * Bank0
	 * 0x30 -> 0x30000000
	 * 0xf8 -> 0x37FFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	ldr	r1, =0x30f80222
	/* if r4 is 1, swap the bank */
	cmp	r4, #0x1
	orreq	r1, r1, #0x08000000
	str	r1, [r6, #0x008]			@ MEMCONFIG0

	/*
	 * Bank1
	 * 0x38 -> 0x38000000
	 * 0xf8 -> 0x3fFFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	ldr	r1, =0x38f80222
	/* if r4 is 1, swap the bank */
	cmp	r4, #0x1
	biceq	r1, r1, #0x08000000
	str	r1, [r6, #0x00c]			@ MEMCONFIG1

	ldr	r1, =0x20000000
	str	r1, [r6, #0x014]			@ PRECHCONFIG

	/*
	 * FIXME: Please verify these values
	 * 7.8us * 166MHz %LE %LONG1294(0x50E)
	 * 7.8us * 133MHz %LE %LONG1038(0x40E),
	 * 7.8us * 100MHz %LE %LONG780(0x30C),
	 * 7.8us * 20MHz  %LE %LONG156(0x9C),
	 * 7.8us * 10MHz  %LE %LONG78(0x4E)
	 */
	ldr	r1, =0x0000050e
	str	r1, [r6, #0x030]			@ TIMINGAREF

	/* 166 MHz */
	ldr	r1, =0x0c233287
	str	r1, [r6, #0x034]			@ TIMINGROW

	/* twtr=3 twr=2 trtp=3 cl=3 wl=3 rl=3 */
	ldr	r1, =0x32330303
	str	r1, [r6, #0x038]			@ TIMINGDATA

	/* tfaw=4 sxsr=0x14 txp=0x14 tcke=3 tmrd=3 */
	ldr	r1, =0x04141433
	str	r1, [r6, #0x03C]			@ TIMINGPOWER

	/* chip0 Deselect */
	ldr	r1, =0x07000000
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* chip0 PALL */
	ldr	r1, =0x01000000
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* chip0 REFA */
	ldr	r1, =0x05000000
	str	r1, [r6, #0x010]			@ DIRECTCMD
	/* chip0 REFA */
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* chip0 MRS, CL%LE %LONG3, BL%LE %LONG4 */
	ldr	r1, =0x00000032
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* chip1 Deselect */
	ldr	r1, =0x07100000
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* chip1 PALL */
	ldr	r1, =0x01100000
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* chip1 REFA */
	ldr	r1, =0x05100000
	str	r1, [r6, #0x010]			@ DIRECTCMD
	/* chip1 REFA */
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* chip1 MRS, CL%LE %LONG3, BL%LE %LONG4 */
	ldr	r1, =0x00100032
	str	r1, [r6, #0x010]			@ DIRECTCMD

	/* auto refresh on */
	ldr	r1, =0xff002030
	str	r1, [r6, #0x000]			@ CONCONTROL

	/* PwrdnConfig */
	ldr	r1, =0x00100002
	str	r1, [r6, #0x028]			@ PWRDNCONFIG

	/* BL%LE %LONG */
	ldr	r1, =0xff212100
	str	r1, [r6, #0x004]			@ MEMCONTROL


	/* Try to test memory area */
	cmp	r4, #0x1
	beq	1f

	mov	r4, #0x1
	ldr	r1, =0x37ffff00
	str	r4, [r1]
	str	r4, [r1, #0x4]				@ dummy write
	ldr	r0, [r1]
	cmp	r0, r4
	bne	swap_memory

1:
	mov	pc, lr

	.ltorg
