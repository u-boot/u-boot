/*
 * Copyright (C) 2013-2015 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

.section .ivt, "a",@progbits
.align 4
	/* Critical system events */
.word	_start			/* 0 - 0x000 */
.word	memory_error		/* 1 - 0x008 */
.word	instruction_error	/* 2 - 0x010 */

	/* Exceptions */
.word	EV_MachineCheck		/* 0x100, Fatal Machine check  (0x20) */
.word	EV_TLBMissI		/* 0x108, Intruction TLB miss  (0x21) */
.word	EV_TLBMissD		/* 0x110, Data TLB miss        (0x22) */
.word	EV_TLBProtV		/* 0x118, Protection Violation (0x23)
							or Misaligned Access  */
.word	EV_PrivilegeV		/* 0x120, Privilege Violation  (0x24) */
.word	EV_Trap			/* 0x128, Trap exception       (0x25) */
.word	EV_Extension		/* 0x130, Extn Intruction Excp (0x26) */

	/* Device interrupts */
.rept	29
	j	interrupt_handler	/* 3:31 - 0x018:0xF8 */
.endr
