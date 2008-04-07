/*
 * interrupt.S - trampoline default exceptions/interrupts to C handlers
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#include <asm/blackfin.h>
#include <asm/entry.h>

.text

/* default entry point for exceptions */
ENTRY(_trap)
	SAVE_ALL_SYS
	r0 = sp;	/* stack frame pt_regs pointer argument ==> r0 */
	sp += -12;
	call _trap_c;
	sp += 12;
	RESTORE_ALL_SYS
	rtx;
ENDPROC(_trap)

/* default entry point for interrupts */
ENTRY(_evt_default)
	SAVE_ALL_SYS
	r0 = sp;	/* stack frame pt_regs pointer argument ==> r0 */
	sp += -12;
	call _bfin_panic;
	sp += 12;
	RESTORE_ALL_SYS
	rti;
ENDPROC(_evt_default)
