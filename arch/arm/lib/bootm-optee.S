/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2025 Marek Vasut
 */
#include <config.h>
#include <linux/linkage.h>

ENTRY(boot_jump_linux_via_optee)
	mov	r4, r3
	mov	lr, r0
	mov	r3, #0
	mov	r0, #0

	/*
	 * Special TZC handling on this platform, the last
	 * 'str' has to be immediately before 'bx' and can
	 * not be interleaved with any return from function
	 * call, if it is then the system hangs.
	 */
#if defined(CONFIG_STM32MP13X) && !defined(CONFIG_TFABOOT)
	ldr	r6, =STM32_TZC_BASE + 0x114 + (0x20 * 2)
	mov	r7, #0x0
	str	r7, [r6]
	ldr	r6, =STM32_TZC_BASE + 0x110 + (0x20 * 1)
	mov	r7, #0x1
	str	r7, [r6]
#endif

	bx	r4
ENDPROC(boot_jump_linux_via_optee)
