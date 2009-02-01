#include <config.h>
/*
 * Processor reset for Nomadik
 */

	.align 5
.globl reset_cpu
reset_cpu:
#if defined CONFIG_NOMADIK_8815
	ldr	r0, =NOMADIK_SRC_BASE
	ldr	r1, =0x1
	str	r1, [r0, #0x18]
#else
	ldr	r1, rstctl1	/* get clkm1 reset ctl */
	mov	r3, #0x0
	strh	r3, [r1]	/* clear it */
	mov	r3, #0x8
	strh	r3, [r1]	/* force dsp+arm reset */
#endif

_loop_forever:
	b	_loop_forever

rstctl1:
	.word 0xfffece10
