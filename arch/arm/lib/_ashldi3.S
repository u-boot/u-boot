/* Copyright 1995, 1996, 1998, 1999, 2000, 2003, 2004, 2005
   Free Software Foundation, Inc.

 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/linkage.h>

#ifdef __ARMEB__
#define al r1
#define ah r0
#else
#define al r0
#define ah r1
#endif

.globl __ashldi3
__ashldi3:
ENTRY(__aeabi_llsl)

	subs	r3, r2, #32
	rsb	ip, r2, #32
	movmi	ah, ah, lsl r2
	movpl	ah, al, lsl r3
	orrmi	ah, ah, al, lsr ip
	mov	al, al, lsl r2
	mov	pc, lr
ENDPROC(__aeabi_llsl)
