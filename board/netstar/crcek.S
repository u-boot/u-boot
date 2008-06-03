/**
 * (C) Copyright 2005
 * 2N Telekomunikace, Ladislav Michl <michl@2n.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2.
 *
 * Image layout looks like following:
 *	u32 - size
 *	u32 - version
 *	... - data
 *	u32 - crc32
 */

#include <config.h>
#include "crcek.h"

/**
 * do_crc32 - calculate CRC32 of given buffer
 * r0 - crc
 * r1 - pointer to buffer
 * r2 - buffer len
 */
	.macro	do_crc32
	ldr	r5, FFFFFFFF
	eor	r0, r0, r5
	adr	r3, CRC32_TABLE
1:
	ldrb    r4, [r1], #1
	eor	r4, r4, r0
	and	r4, r4, #0xff
	ldr	r4, [r3, r4, lsl#2]
	eor	r0, r4, r0, lsr#8
	subs	r2, r2, #0x1
	bne	1b
	eor	r0, r0, r5
	.endm

	.macro crcuj, offset, size
	mov	r0, #0
	ldr	r1, \offset
	ldr	r2, [r1], #4
	cmp	r2, r0		@ no data, no problem
	beq	2f
	tst	r2, #3		@ unaligned size
	bne	2f
	ldr	r3, \size
	cmp	r2, r3		@ bogus size
	bhi	2f
	do_crc32
	ldr	r1, [r1]
2:
	cmp	r0, r1
	.endm

	.macro wait, reg
	mov	\reg, #0x100000
3:
	subs	\reg, \reg, #0x1
	bne	3b
	.endm

.text
.globl crcek
crcek:
	/* Enable I-cache */
	mrc	p15, 0, r1, c0, c0, 0		@ read C15 ID register
	mrc	p15, 0, r1, c0, c0, 1		@ read C15 Cache information register
	mrc	p15, 0, r1, c1, c0, 0		@ read C15 Control register
	orr	r1, r1, #0x1000			@ enable I-cache, map interrupt vector 0xffff0000
	mcr	p15, 0, r1, c1, c0, 0		@ write C15 Control register
	mov	r1, #0x00
	mcr	p15, 0, r1, c7, c5, 0		@ Flush I-cache
	nop
	nop
	nop
	nop

	/* Setup clocking mode */
	ldr	r0, MPU_CLKM_BASE		@ base of CLOCK unit
	ldrh	r1, [r0, #0x18]			@ ARM_SYST - get reset status
	bic	r1, r1, #(7 << 11)		@ clear clock select
	orr	r1, r1, #(2 << 11)		@ set synchronous scalable
	mov	r2, #0
loop:
	cmp	r2, #1				@ this loop will wait for at least 100 cycles
	streqh	r1, [r0, #0x18]			@ before issuing next request from MPU
	add	r2, r2, #1			@ on the 1st run code is loaded into I-cache
	cmp	r2, #16				@ and second run will set clocking mode
	bne	loop
	nop

	/* Setup clock dividers */
	ldr	r1, CKCTL_VAL
	orr	r1, r1, #0x2000			@ enable DSP clock
	strh	r1, [r0]			@ setup clock divisors

	/* Setup DPLL to generate requested freq */
	ldr	r0, DPLL1_BASE			@ base of DPLL1 register
	mov	r1, #0x0010			@ set PLL_ENABLE
	orr	r1, r1, #0x2000			@ set IOB to new locking
	orr	r1, r1, #(OMAP5910_DPLL_MUL << 7) @ setup multiplier CLKREF
	orr	r1, r1, #(OMAP5910_DPLL_DIV << 5) @ setup divider CLKREF
	strh	r1, [r0]			@ write

locking:
	ldrh	r1, [r0]			@ get DPLL value
	tst	r1, #0x01
	beq	locking				@ while LOCK not set

	/* Enable clock */
	ldr	r0, MPU_CLKM_BASE		@ base of CLOCK unit
	mov	r1, #(1 << 10)			@ disable idle mode do not check
						@ nWAKEUP pin, other remain active
	strh	r1, [r0, #0x04]
	ldr	r1, EN_CLK_VAL
	strh	r1, [r0, #0x08]
	mov	r1, #0x003f			@ FLASH.RP not enabled in idle and
	strh	r1, [r0, #0x0c]			@ max delayed ( 32 x CLKIN )


	mov	r6, #0
	crcuj	_LOADER1_OFFSET, _LOADER_SIZE
	bne	crc1_bad
	orr	r6, r6, #1
crc1_bad:
	crcuj	_LOADER2_OFFSET, _LOADER_SIZE
	bne	crc2_bad
	orr	r6, r6, #2
crc2_bad:
	ldr	r3, _LOADER1_OFFSET
	ldr	r4, _LOADER2_OFFSET
	teq	r6, #3
	bne	one_is_bad	@ one of them (or both) has bad crc
	ldr	r1, [r3, #4]
	ldr	r2, [r4, #4]
	cmp	r1, r2		@ boot 2nd loader if versions differ
	beq	boot_1st
	b	boot_2nd
one_is_bad:
	tst	r6, #1
	bne	boot_1st
	tst	r6, #2
	bne	boot_2nd
@ We are doomed, so let user know.
hell:
	ldr	r0, GPIO_BASE	@ configure GPIO pins
	ldr	r1, GPIO_DIRECTION
	strh	r1, [r0, #0x08]
blink_loop:
	mov	r1, #0x08
	strh    r1, [r0, #0x04]
	wait	r3
	mov	r1, #0x10
	strh    r1, [r0, #0x04]
	wait	r3
	b blink_loop
boot_1st:
	add	pc, r3, #8
boot_2nd:
	add	pc, r4, #8

_LOADER_SIZE:
	.word LOADER_SIZE - 8	@ minus size and crc32
_LOADER1_OFFSET:
	.word LOADER1_OFFSET
_LOADER2_OFFSET:
	.word LOADER2_OFFSET

FFFFFFFF:
	.word 0xffffffff
CRC32_TABLE:
	.word 0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419
	.word 0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4
	.word 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07
	.word 0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de
	.word 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856
	.word 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9
	.word 0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4
	.word 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b
	.word 0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3
	.word 0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a
	.word 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599
	.word 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924
	.word 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190
	.word 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f
	.word 0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e
	.word 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01
	.word 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed
	.word 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950
	.word 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3
	.word 0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2
	.word 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a
	.word 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5
	.word 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010
	.word 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f
	.word 0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17
	.word 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6
	.word 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615
	.word 0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8
	.word 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344
	.word 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb
	.word 0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a
	.word 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5
	.word 0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1
	.word 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c
	.word 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef
	.word 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236
	.word 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe
	.word 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31
	.word 0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c
	.word 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713
	.word 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b
	.word 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242
	.word 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1
	.word 0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c
	.word 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278
	.word 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7
	.word 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66
	.word 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9
	.word 0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605
	.word 0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8
	.word 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b
	.word 0x2d02ef8d

GPIO_BASE:
	.word 0xfffce000
MPU_CLKM_BASE:
	.word 0xfffece00
DPLL1_BASE:
	.word 0xfffecf00

CKCTL_VAL:
	.word OMAP5910_ARM_CKCTL
EN_CLK_VAL:
	.word OMAP5910_ARM_EN_CLK
GPIO_DIRECTION:
	.word 0x0000ffe7

.end
