/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) 2022 Arm Ltd.
 */

#include <config.h>
#include <asm/macro.h>
#include <linux/linkage.h>

.pushsection .text.smh_trap, "ax"
/* long smh_trap(unsigned int sysnum, void *addr); */
ENTRY(smh_trap)

#if defined(CONFIG_ARM64)
	hlt	#0xf000
#elif defined(CONFIG_CPU_V7M)
	bkpt	#0xab
#elif defined(CONFIG_SYS_THUMB_BUILD)
	svc	#0xab
#else
	svc	#0x123456
#endif

#if defined(CONFIG_ARM64)
	ret
#else
	bx	lr
#endif

ENDPROC(smh_trap)
.popsection
