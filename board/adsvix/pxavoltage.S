/*
 * (C) Copyright 2004
 * Robert Whaley, Applied Data Systems, Inc. rwhaley@applieddata.net
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

#include <asm/arch/pxa-regs.h>

#define LTC1663_ADDR    0x20

#define LTC1663_SY	0x01	/* Sync ACK */
#define LTC1663_SD	0x04	/* shutdown */
#define LTC1663_BG	0x04	/* Internal Voltage Ref */

#define VOLT_1_55         18    /* DAC value for 1.55V */

		.global	initPXAvoltage

@ Set the voltage to 1.55V early in the boot process so we can run
@ at a high clock speed and boot quickly.  Note that this is necessary
@ because the reset button does not reset the CPU voltage, so if the
@ voltage was low (say 0.85V) then the CPU would crash without this
@ routine

@ This routine clobbers r0-r4

initializei2c:

		ldr	r2, =CKEN
		ldr	r3, [r2]
		orr	r3, r3, #CKEN15_PWRI2C
		str	r3, [r2]

		ldr	r2, =PCFR
		ldr	r3, [r2]
		orr	r3, r3, #PCFR_PI2C_EN
		str	r3, [r2]

		/* delay for about 250msec
		 */
		ldr	r3, =OSCR
		mov	r2, #0
		str	r2, [r3]
		ldr	r1, =0xC0000

1:
		ldr	r2, [r3]
		cmp	r1, r2
		bgt	1b
		ldr	r0, =PWRICR
		ldr	r1, [r0]
		bic	r1, r1, #(ICR_MA | ICR_START | ICR_STOP)
		str	r1, [r0]

		orr	r1, r1, #ICR_UR
		str	r1, [r0]

		ldr	r2, =PWRISR
		ldr	r3, =0x7ff
		str	r3, [r2]

		bic	r1, r1, #ICR_UR
		str	r1, [r0]

		mov	r1, #(ICR_GCD | ICR_SCLE)
		str	r1, [r0]

		orr	r1, r1, #ICR_IUE
		str	r1, [r0]

		orr	r1, r1, #ICR_FM
		str	r1, [r0]

		/* delay for about 1msec
		 */
		ldr	r3, =OSCR
		mov	r2, #0
		str	r2, [r3]
		ldr	r1, =0xC00

1:
		ldr	r2, [r3]
		cmp	r1, r2
		bgt	1b
		mov	pc, lr

sendbytei2c:
		ldr	r3, =PWRIDBR
		str	r0, [r3]
		ldr	r3, =PWRICR
		ldr	r0, [r3]
		orr	r0, r0, r1
		bic	r0, r0, r2
		str	r0, [r3]
		orr	r0, r0, #ICR_TB
		str	r0, [r3]

		mov	r2, #0x100000

waitfortxemptyi2c:

		ldr	r0, =PWRISR
		ldr	r1, [r0]

		/* take it from the top if we don't get empty after a while */
		subs	r2, r2, #1
		moveq	lr, r4
		beq	initPXAvoltage

		tst	r1, #ISR_ITE

		beq	waitfortxemptyi2c

		orr	r1, r1, #ISR_ITE
		str	r1, [r0]

		mov	pc, lr

initPXAvoltage:

		mov	r4, lr

		bl	setleds

		bl	initializei2c

		bl	setleds

		/* now send the real message to set the correct voltage */
		ldr	r0, =LTC1663_ADDR
		mov	r0, r0, LSL #1
		mov	r1, #ICR_START
		ldr	r2, =(ICR_STOP | ICR_ALDIE | ICR_ACKNAK)
		bl	sendbytei2c

		bl	setleds

		mov	r0, #LTC1663_BG
		mov	r1, #0
		mov	r2, #(ICR_STOP | ICR_START)
		bl	sendbytei2c

		bl	setleds

		ldr	r0, =VOLT_1_55
		and	r0, r0, #0xff
		mov	r1, #0
		mov	r2, #(ICR_STOP | ICR_START)
		bl	sendbytei2c

		bl	setleds

		ldr	r0, =VOLT_1_55
		mov	r0, r0, ASR #8
		and	r0, r0, #0xff
		mov	r1, #ICR_STOP
		mov	r2, #ICR_START
		bl	sendbytei2c

		bl	setleds

		@ delay a little for the volatage to stablize
		ldr	r3, =OSCR
		mov	r2, #0
		str	r2, [r3]
		ldr	r1, =0xC0

1:
		ldr	r2, [r3]
		cmp	r1, r2
		bgt	1b
		mov	pc, r4

setleds:
	mov		pc, lr

	ldr		r5, =0x40e00058
	ldr		r3, [r5]
	bic		r3, r3, #0x3
	str		r3, [r5]
	ldr		r5, =0x40e0000c
	ldr		r3, [r5]
	orr		r3, r3, #0x00010000
	str		r3, [r5]

	@ inner loop
	mov		r0, #0x2
1:

	ldr		r5, =0x40e00018
	mov		r3, #0x00010000
	str		r3, [r5]

	@ outer loop
	mov		r3, #0x00F00000
2:
	subs		r3, r3, #1
	bne		2b

	ldr		r5, =0x40e00024
	mov		r3, #0x00010000
	str		r3, [r5]

	@ outer loop
	mov		r3, #0x00F00000
3:
	subs		r3, r3, #1
	bne		3b

	subs		r0, r0, #1
	bne		1b

	mov		pc, lr
