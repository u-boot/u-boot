/*
 * modified from SH-IPL+g (init-r0p751rlc0011rl.S)
 * Initial Register Data for R0P751RLC0011RL (SH7751R 240MHz/120MHz/60MHz)
 * Coyright (c) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
*/

#include <config.h>
#include <version.h>

#include <asm/processor.h>

	.global lowlevel_init
	.text
	.align  2

lowlevel_init:

	mov.l	CCR_A, r1
	mov.l	CCR_D_D, r0
	mov.l	r0,@r1

	mov.l	MMUCR_A,r1
	mov.l	MMUCR_D,r0
	mov.w	r0,@r1

	mov.l	BCR1_A,r1
	mov.l	BCR1_D,r0
	mov.l	r0,@r1

	mov.l	BCR2_A,r1
	mov.l	BCR2_D,r0
	mov.w	r0,@r1

	mov.l	BCR3_A,r1
	mov.l	BCR3_D,r0
	mov.w	r0,@r1

	mov.l	BCR4_A,r1
	mov.l	BCR4_D,r0
	mov.l	r0,@r1

	mov.l	WCR1_A,r1
	mov.l	WCR1_D,r0
	mov.l	r0,@r1

	mov.l	WCR2_A,r1
	mov.l	WCR2_D,r0
	mov.l	r0,@r1

	mov.l	WCR3_A,r1
	mov.l	WCR3_D,r0
	mov.l	r0,@r1

	mov.l	PCR_A,r1
	mov.l	PCR_D,r0
	mov.w	r0,@r1

	mov.l	LED_A,r1
	mov	#0xff,r0
	mov.w	r0,@r1

	mov.l	MCR_A,r1
	mov.l	MCR_D1,r0
	mov.l	r0,@r1

	mov.l	RTCNT_A,r1
	mov.l	RTCNT_D,r0
	mov.w	r0,@r1

	mov.l	RTCOR_A,r1
	mov.l	RTCOR_D,r0
	mov.w	r0,@r1

	mov.l	RFCR_A,r1
	mov.l	RFCR_D,r0
	mov.w	r0,@r1

	mov.l	RTCSR_A,r1
	mov.l	RTCSR_D,r0
	mov.w	r0,@r1

	mov.l	SDMR3_A,r1
	mov	#0x55,r0
	mov.b	r0,@r1

	/* Wait DRAM refresh 30 times */
	mov.l	RFCR_A,r1
	mov	#30,r3
1:
	mov.w	@r1,r0
	extu.w	r0,r2
	cmp/hi	r3,r2
	bf	1b

	mov.l	MCR_A,r1
	mov.l	MCR_D2,r0
	mov.l	r0,@r1

	mov.l	SDMR3_A,r1
	mov	#0,r0
	mov.b	r0,@r1

	mov.l	IRLMASK_A,r1
	mov.l	IRLMASK_D,r0
	mov.l	r0,@r1

	mov.l	CCR_A, r1
	mov.l	CCR_D_E, r0
	mov.l	r0, @r1

	rts
	nop

	.align	2
CCR_A:		.long	CCR		/* Cache Control Register */
CCR_D_D:	.long	0x0808		/* Flush the cache, disable */
CCR_D_E:	.long	0x8000090B

FRQCR_A:	.long	FRQCR		/* FRQCR Address */
FRQCR_D:	.long	0x00000e0a	/* 03/07/15 modify */
BCR1_A:	.long	BCR1		/* BCR1 Address */
BCR1_D:	.long	0x00180008
BCR2_A:	.long	BCR2		/* BCR2 Address */
BCR2_D:	.long   0xabe8
BCR3_A:	.long	BCR3		/* BCR3 Address */
BCR3_D:	.long	0x0000
BCR4_A:	.long	BCR4		/* BCR4 Address */
BCR4_D:	.long	0x00000010
WCR1_A:	.long	WCR1		/* WCR1 Address */
WCR1_D:	.long	0x33343333
WCR2_A:	.long	WCR2		/* WCR2 Address */
WCR2_D:	.long	0xcff86fbf
WCR3_A:	.long	WCR3		/* WCR3 Address */
WCR3_D:	.long	0x07777707
LED_A:		.long	0x04000036	/* LED Address */
RTCNT_A:	.long	RTCNT		/* RTCNT Address */
RTCNT_D:	.long	0xA500		/* RTCNT Write Code A5h Data 00h */
RTCOR_A:	.long	RTCOR		/* RTCOR Address */
RTCOR_D:	.long	0xA534		/* RTCOR Write Code  */
RTCSR_A:	.long	RTCSR		/* RTCSR Address */
RTCSR_D:	.long	0xA510		/* RTCSR Write Code */
SDMR3_A:	.long   0xFF9400CC	/* SDMR3 Address */
SDMR3_D:	.long	0x55
MCR_A:		.long	MCR		/* MCR Address */
MCR_D1:	.long	0x081901F4	/* MRSET:'0' */
MCR_D2:	.long	0x481901F4	/* MRSET:'1' */
RFCR_A:	.long	RFCR		/* RFCR Address */
RFCR_D:	.long	0xA400		/* RFCR Write Code A4h Data 00h */
PCR_A:		.long	PCR		/* PCR Address */
PCR_D:		.long	0x0000
MMUCR_A:	.long	MMUCR		/* MMUCCR Address */
MMUCR_D:	.long	0x00000000	/* MMUCCR Data */
IRLMASK_A:	.long	0xA4000000	/* IRLMASK Address */
IRLMASK_D:	.long	0x00000000	/* IRLMASK Data */
