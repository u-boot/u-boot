/*
 * Memory setup for SMDKV310 board based on S5PC210
 *
 * Copyright (C) 2011 Samsung Electronics
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

#define SET_MIU

#define MEM_DLL

#ifdef CONFIG_CLK_800_330_165
#define DRAM_CLK_330
#endif
#ifdef CONFIG_CLK_1000_200_200
#define DRAM_CLK_200
#endif
#ifdef CONFIG_CLK_1000_330_165
#define DRAM_CLK_330
#endif
#ifdef CONFIG_CLK_1000_400_200
#define DRAM_CLK_400
#endif

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:

	/*
	* Async bridge configuration at CPU_core:
	* 1: half_sync
	* 0: full_sync
	*/
	ldr r0, =0x10010350
	mov r1, #1
	str r1, [r0]

#ifdef SET_MIU
	ldr	r0, =S5PC210_MIU_BASE	@0x10600000
#ifdef CONFIG_MIU_1BIT_INTERLEAVED
	ldr	r1, =0x0000000c
	str	r1, [r0, #0x400]	@MIU_INTLV_CONFIG
	ldr	r1, =0x40000000
	str	r1, [r0, #0x808]	@MIU_INTLV_START_ADDR
	ldr	r1, =0xbfffffff
	str	r1, [r0, #0x810]	@MIU_INTLV_END_ADDR
	ldr	r1, =0x00000001
	str	r1, [r0, #0x800]	@MIU_MAPPING_UPDATE
#endif
#ifdef CONFIG_MIU_2BIT_INTERLEAVED
	ldr	r1, =0x2000150c
	str	r1, [r0, #0x400]	@MIU_INTLV_CONFIG
	ldr	r1, =0x40000000
	str	r1, [r0, #0x808]	@MIU_INTLV_START_ADDR
	ldr	r1, =0xbfffffff
	str	r1, [r0, #0x810]	@MIU_INTLV_END_ADDR
	ldr	r1, =0x00000001
	str	r1, [r0, #0x800]	@MIU_MAPPING_UPDATE
#endif
#ifdef CONFIG_MIU_LINEAR
	ldr	r1, =0x40000000
	str	r1, [r0, #0x818]	@MIU_SINGLE_MAPPING0_START_ADDR
	ldr	r1, =0x7fffffff
	str	r1, [r0, #0x820]	@MIU_SINGLE_MAPPING0_END_ADDR
	ldr	r1, =0x80000000
	str	r1, [r0, #0x828]	@MIU_SINGLE_MAPPING1_START_ADDR
	ldr	r1, =0xbfffffff
	str	r1, [r0, #0x830]	@MIU_SINGLE_MAPPING1_END_ADDR]
	ldr	r1, =0x00000006
	str	r1, [r0, #0x800]	@MIU_MAPPING_UPDATE
#endif
#endif
	/* DREX0 */
	ldr	r0, =S5PC210_DMC0_BASE	@0x10400000

	ldr	r1, =0xe0000086
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1

	ldr	r1, =0xE3855703
	str	r1, [r0, #0x44]		@DMC_PHYZQCONTROL

	mov	r2, #0x100000
1:	subs	r2, r2, #1
	bne	1b

	ldr	r1, =0xe000008e
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1
	ldr	r1, =0xe0000086
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1

	ldr	r1, =0x71101008
	str	r1, [r0, #0x18]		@DMC_PHYCONTROL0
	ldr	r1, =0x7110100A
	str	r1, [r0, #0x18]		@DMC_PHYCONTROL0
	ldr	r1, =0xe0000086
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1
	ldr	r1, =0x7110100B
	str	r1, [r0, #0x18]		@DMC_PHYCONTROL0

	ldr	r1, =0x00000000
	str	r1, [r0, #0x20]		@DMC_PHYCONTROL2

	ldr	r1, =0x0FFF301a
	str	r1, [r0, #0x00]		@DMC_CONCONTROL
	ldr	r1, =0x00312640
	str	r1, [r0, #0x04]		@DMC_MEMCONTROL]

#ifdef CONFIG_MIU_LINEAR
	ldr	r1, =0x40e01323
	str	r1, [r0, #0x08]		@DMC_MEMCONFIG0
	ldr	r1, =0x60e01323
	str	r1, [r0, #0x0C]		@DMC_MEMCONFIG1
#else	/* @MIU_1BIT_INTERLEAVED | MIU_2BIT_INTERLEAVED */
	ldr	r1, =0x20e01323
	str	r1, [r0, #0x08]		@DMC_MEMCONFIG0
	ldr	r1, =0x40e01323
	str	r1, [r0, #0x0C]		@DMC_MEMCONFIG1
#endif

	ldr	r1, =0xff000000
	str	r1, [r0, #0x14]		@DMC_PRECHCONFIG

	ldr	r1, =0x000000BC
	str	r1, [r0, #0x30]		@DMC_TIMINGAREF

#ifdef DRAM_CLK_330
	ldr	r1, =0x3545548d
	str	r1, [r0, #0x34]		@DMC_TIMINGROW
	ldr	r1, =0x45430506
	str	r1, [r0, #0x38]		@DMC_TIMINGDATA
	ldr	r1, =0x4439033c
	str	r1, [r0, #0x3C]		@DMC_TIMINGPOWER
#endif
#ifdef DRAM_CLK_400
	ldr	r1, =0x4046654f
	str	r1, [r0, #0x34]		@DMC_TIMINGROW
	ldr	r1, =0x56500506
	str	r1, [r0, #0x38]		@DMC_TIMINGDATA
	ldr	r1, =0x5444033d
	str	r1, [r0, #0x3C]		@DMC_TIMINGPOWER
#endif
	ldr	r1, =0x07000000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
2:	subs	r2, r2, #1
	bne	2b

	ldr	r1, =0x00020000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00030000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00010002
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00000328
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
3:	subs	r2, r2, #1
	bne	3b

	ldr	r1, =0x0a000000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
4:	subs	r2, r2, #1
	bne	4b

	ldr	r1, =0x07100000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
5:	subs	r2, r2, #1
	bne	5b

	ldr	r1, =0x00120000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00130000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00110002
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00100328
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
6:	subs	r2, r2, #1
	bne	6b

	ldr	r1, =0x0a100000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
7:	subs	r2, r2, #1
	bne	7b

	ldr	r1, =0xe000008e
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1
	ldr	r1, =0xe0000086
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1

	mov	r2, #0x100000
8:	subs	r2, r2, #1
	bne	8b

	/* DREX1 */
	ldr	r0, =S5PC210_DMC1_BASE	@0x10410000

	ldr	r1, =0xe0000086
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1

	ldr	r1, =0xE3855703
	str	r1, [r0, #0x44]		@DMC_PHYZQCONTROL

	mov	r2, #0x100000
1:	subs	r2, r2, #1
	bne	1b

	ldr	r1, =0xe000008e
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1
	ldr	r1, =0xe0000086
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1

	ldr	r1, =0x71101008
	str	r1, [r0, #0x18]		@DMC_PHYCONTROL0
	ldr	r1, =0x7110100A
	str	r1, [r0, #0x18]		@DMC_PHYCONTROL0
	ldr	r1, =0xe0000086
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1
	ldr	r1, =0x7110100B
	str	r1, [r0, #0x18]		@DMC_PHYCONTROL0

	ldr	r1, =0x00000000
	str	r1, [r0, #0x20]		@DMC_PHYCONTROL2

	ldr	r1, =0x0FFF301a
	str	r1, [r0, #0x00]		@DMC_CONCONTROL
	ldr	r1, =0x00312640
	str	r1, [r0, #0x04]		@DMC_MEMCONTROL]

#ifdef CONFIG_MIU_LINEAR
	ldr	r1, =0x40e01323
	str	r1, [r0, #0x08]		@DMC_MEMCONFIG0
	ldr	r1, =0x60e01323
	str	r1, [r0, #0x0C]		@DMC_MEMCONFIG1
#else	/* @MIU_1BIT_INTERLEAVED | MIU_2BIT_INTERLEAVED */
	ldr	r1, =0x20e01323
	str	r1, [r0, #0x08]		@DMC_MEMCONFIG0
	ldr	r1, =0x40e01323
	str	r1, [r0, #0x0C]		@DMC_MEMCONFIG1
#endif

	ldr	r1, =0xff000000
	str	r1, [r0, #0x14]		@DMC_PRECHCONFIG

	ldr	r1, =0x000000BC
	str	r1, [r0, #0x30]		@DMC_TIMINGAREF

#ifdef DRAM_CLK_330
	ldr	r1, =0x3545548d
	str	r1, [r0, #0x34]		@DMC_TIMINGROW
	ldr	r1, =0x45430506
	str	r1, [r0, #0x38]		@DMC_TIMINGDATA
	ldr	r1, =0x4439033c
	str	r1, [r0, #0x3C]		@DMC_TIMINGPOWER
#endif
#ifdef DRAM_CLK_400
	ldr	r1, =0x4046654f
	str	r1, [r0, #0x34]		@DMC_TIMINGROW
	ldr	r1, =0x56500506
	str	r1, [r0, #0x38]		@DMC_TIMINGDATA
	ldr	r1, =0x5444033d
	str	r1, [r0, #0x3C]		@DMC_TIMINGPOWER
#endif

	ldr	r1, =0x07000000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
2:	subs	r2, r2, #1
	bne	2b

	ldr	r1, =0x00020000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00030000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00010002
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00000328
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
3:	subs	r2, r2, #1
	bne	3b

	ldr	r1, =0x0a000000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
4:	subs	r2, r2, #1
	bne	4b

	ldr	r1, =0x07100000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
5:	subs	r2, r2, #1
	bne	5b

	ldr	r1, =0x00120000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00130000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00110002
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD
	ldr	r1, =0x00100328
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
6:	subs	r2, r2, #1
	bne	6b

	ldr	r1, =0x0a100000
	str	r1, [r0, #0x10]		@DMC_DIRECTCMD

	mov	r2, #0x100000
7:	subs	r2, r2, #1
	bne	7b

	ldr	r1, =0xe000008e
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1
	ldr	r1, =0xe0000086
	str	r1, [r0, #0x1C]		@DMC_PHYCONTROL1

	mov	r2, #0x100000
8:	subs	r2, r2, #1
	bne	8b

	/* turn on DREX0, DREX1 */
	ldr	r0, =0x10400000		@APB_DMC_0_BASE
	ldr	r1, =0x0FFF303a
	str	r1, [r0, #0x00]		@DMC_CONCONTROL

	ldr	r0, =0x10410000		@APB_DMC_1_BASE
	ldr	r1, =0x0FFF303a
	str	r1, [r0, #0x00]		@DMC_CONCONTROL

	mov	pc, lr
