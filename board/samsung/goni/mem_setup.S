/*
 * Copyright (C) 2009 Samsung Electrnoics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:
	cmp	r7, r8

	ldreq	r0, =S5PC100_DMC_BASE			@ 0xE6000000
	ldrne	r0, =S5PC110_DMC0_BASE			@ 0xF0000000
	ldrne	r6, =S5PC110_DMC1_BASE			@ 0xF1400000

	/* DLL parameter setting */
	ldr	r1, =0x50101000
	str	r1, [r0, #0x018]			@ PHYCONTROL0_OFFSET
	strne	r1, [r6, #0x018]			@ PHYCONTROL0_OFFSET
	ldr	r1, =0x000000f4
	str	r1, [r0, #0x01C]			@ PHYCONTROL1_OFFSET
	strne	r1, [r6, #0x01C]			@ PHYCONTROL1_OFFSET
	ldreq	r1, =0x0
	streq	r1, [r0, #0x020]			@ PHYCONTROL2_OFFSET

	/* DLL on */
	ldr	r1, =0x50101002
	str	r1, [r0, #0x018]			@ PHYCONTROL0_OFFSET
	strne	r1, [r6, #0x018]			@ PHYCONTROL0_OFFSET

	/* DLL start */
	ldr	r1, =0x50101003
	str	r1, [r0, #0x018]			@ PHYCONTROL0_OFFSET
	strne	r1, [r6, #0x018]			@ PHYCONTROL0_OFFSET

	mov	r2, #0x4000
wait:	subs	r2, r2, #0x1
	cmp	r2, #0x0
	bne	wait

	cmp	r7, r8
	/* Force value locking for DLL off */
	str	r1, [r0, #0x018]			@ PHYCONTROL0_OFFSET
	strne	r1, [r6, #0x018]			@ PHYCONTROL0_OFFSET

	/* DLL off */
	ldr	r1, =0x50101009
	str	r1, [r0, #0x018]			@ PHYCONTROL0_OFFSET
	strne	r1, [r6, #0x018]			@ PHYCONTROL0_OFFSET

	/* auto refresh off */
	ldr	r1, =0xff001010 | (1 << 7)
	ldr	r2, =0xff001010 | (1 << 7)
	str	r1, [r0, #0x000]			@ CONCONTROL_OFFSET
	strne	r2, [r6, #0x000]			@ CONCONTROL_OFFSET

	/*
	 * Burst Length 4, 2 chips, 32-bit, LPDDR
	 * OFF: dynamic self refresh, force precharge, dynamic power down off
	 */
	ldr	r1, =0x00212100
	ldr	r2, =0x00212100
	str	r1, [r0, #0x004]			@ MEMCONTROL_OFFSET
	strne	r2, [r6, #0x004]			@ MEMCONTROL_OFFSET

	/*
	 * Note:
	 * If Bank0 has Mobile RAM we place it at 0x3800'0000 (s5pc100 only)
	 * So finally Bank1 OneDRAM should address start at at 0x3000'0000
	 */

	/*
	 * DMC0: CS0 : S5PC100/S5PC110
	 * 0x30 -> 0x30000000
	 * 0xf8 -> 0x37FFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	ldr	r3, =0x30f80222
	ldr	r4, =0x40f00222
swap_memory:
	str	r3, [r0, #0x008]			@ MEMCONFIG0_OFFSET
	str	r4, [r0, #0x00C]			@ dummy write

	/*
	 * DMC1: CS0 : S5PC110
	 * 0x40 -> 0x40000000
	 * 0xf8 -> 0x47FFFFFF (1Gib)
	 * 0x40 -> 0x40000000
	 * 0xf0 -> 0x4FFFFFFF (2Gib)
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits  - Col (1Gib)
	 * [11:8 ] 3: 10 bits - Col (2Gib)
	 * [ 7:4 ] 2: 14 bits - Row
	 * [ 3:0 ] 2: 4 banks
	 */
	/* Default : 2GiB */
	ldr	r4, =0x40f01322				@ 2Gib: MCP B
	ldr	r5, =0x50f81312				@ dummy: MCP D
	cmp	r9, #1
	ldreq	r4, =0x40f81222				@ 1Gib: MCP A
	cmp	r9, #3
	ldreq	r5, =0x50f81312				@ 2Gib + 1Gib: MCP D
	cmp	r9, #4
	ldreq	r5, =0x50f01312				@ 2Gib + 2Gib: MCP E

	cmp	r7, r8
	strne	r4, [r6, #0x008]			@ MEMCONFIG0_OFFSET
	strne	r5, [r6, #0x00C]			@ MEMCONFIG1_OFFSET

	/*
	 * DMC0: CS1: S5PC100
	 * 0x38 -> 0x38000000
	 * 0xf8 -> 0x3fFFFFFF
	 * [15:12] 0: Linear
	 * [11:8 ] 2: 9 bits
	 * [ 7:4 ] 2: 14 bits
	 * [ 3:0 ] 2: 4 banks
	 */
	eoreq	r3, r3, #0x08000000
	streq	r3, [r0, #0xc]				@ MEMCONFIG1_OFFSET

	ldr	r1, =0x20000000
	str	r1, [r0, #0x014]			@ PRECHCONFIG_OFFSET
	strne	r1, [r0, #0x014]			@ PRECHCONFIG_OFFSET
	strne	r1, [r6, #0x014]			@ PRECHCONFIG_OFFSET

	/*
	 * S5PC100:
	 * DMC:  CS0: 166MHz
	 *       CS1: 166MHz
	 * S5PC110:
	 * DMC0: CS0: 166MHz
	 * DMC1: CS0: 200MHz
	 *
	 * 7.8us * 200MHz %LE %LONG1560(0x618)
	 * 7.8us * 166MHz %LE %LONG1294(0x50E)
	 * 7.8us * 133MHz %LE %LONG1038(0x40E),
	 * 7.8us * 100MHz %LE %LONG780(0x30C),
	 */
	ldr	r1, =0x0000050E
	str	r1, [r0, #0x030]			@ TIMINGAREF_OFFSET
	ldrne	r1, =0x00000618
	strne	r1, [r6, #0x030]			@ TIMINGAREF_OFFSET

	ldr	r1, =0x14233287
	str	r1, [r0, #0x034]			@ TIMINGROW_OFFSET
	ldrne	r1, =0x182332c8
	strne	r1, [r6, #0x034]			@ TIMINGROW_OFFSET

	ldr	r1, =0x12130005
	str	r1, [r0, #0x038]			@ TIMINGDATA_OFFSET
	ldrne	r1, =0x13130005
	strne	r1, [r6, #0x038]			@ TIMINGDATA_OFFSET

	ldr	r1, =0x0E140222
	str	r1, [r0, #0x03C]			@ TIMINGPOWER_OFFSET
	ldrne	r1, =0x0E180222
	strne	r1, [r6, #0x03C]			@ TIMINGPOWER_OFFSET

	/* chip0 Deselect */
	ldr	r1, =0x07000000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip0 PALL */
	ldr	r1, =0x01000000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip0 REFA */
	ldr	r1, =0x05000000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET
	/* chip0 REFA */
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip0 MRS */
	ldr	r1, =0x00000032
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip0 EMRS */
	ldr	r1, =0x00020020
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip1 Deselect */
	ldr	r1, =0x07100000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip1 PALL */
	ldr	r1, =0x01100000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip1 REFA */
	ldr	r1, =0x05100000
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET
	/* chip1 REFA */
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip1 MRS */
	ldr	r1, =0x00100032
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* chip1 EMRS */
	ldr	r1, =0x00120020
	str	r1, [r0, #0x010]			@ DIRECTCMD_OFFSET
	strne	r1, [r6, #0x010]			@ DIRECTCMD_OFFSET

	/* auto refresh on */
	ldr	r1, =0xFF002030 | (1 << 7)
	str	r1, [r0, #0x000]			@ CONCONTROL_OFFSET
	strne	r1, [r6, #0x000]			@ CONCONTROL_OFFSET

	/* PwrdnConfig */
	ldr	r1, =0x00100002
	str	r1, [r0, #0x028]			@ PWRDNCONFIG_OFFSET
	strne	r1, [r6, #0x028]			@ PWRDNCONFIG_OFFSET

	ldr	r1, =0x00212113
	str	r1, [r0, #0x004]			@ MEMCONTROL_OFFSET
	strne	r1, [r6, #0x004]			@ MEMCONTROL_OFFSET

	/* Skip when S5PC110 */
	bne	1f

	/* Check OneDRAM access area at s5pc100 */
	ldreq	r3, =0x38f80222
	ldreq	r1, =0x37ffff00
	str	r3, [r1]
	ldr	r2, [r1]
	cmp	r2, r3
	beq	swap_memory
1:
	mov	pc, lr

	.ltorg
