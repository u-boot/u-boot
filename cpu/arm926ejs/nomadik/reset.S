#include <config.h>
/*
 * Processor reset for Nomadik
 */

	.align 5
.globl reset_cpu
reset_cpu:
	ldr	r0, =NOMADIK_SRC_BASE	/* System and Reset Controller */
	ldr	r1, =0x1
	str	r1, [r0, #0x18]

_loop_forever:
	b	_loop_forever
