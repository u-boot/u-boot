/*
 * interrupt.S - trampoline default exceptions/interrupts to C handlers
 *
 * Copyright (c) 2005-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#include <config.h>
#include <asm/blackfin.h>
#include <asm/entry.h>
#include <asm/ptrace.h>
#include <asm/deferred.h>
#include <asm/mach-common/bits/core.h>

.text

/* default entry point for exceptions */
ENTRY(_trap)
	CONFIG_BFIN_SCRATCH_REG = sp;
	sp.l = LO(L1_SRAM_SCRATCH_END - 20);
	sp.h = HI(L1_SRAM_SCRATCH_END - 20);
	SAVE_ALL_SYS

	r0 = sp;	/* stack frame pt_regs pointer argument ==> r0 */
	r1 = 3;		/* EVT3 space */
	sp += -12;
	call _trap_c;
	sp += 12;

#ifdef CONFIG_EXCEPTION_DEFER
	CC = R0 == 0;
	IF CC JUMP .Lexit_trap;

	/* To avoid double faults, lower our priority to IRQ5 */
	p4.l = lo(COREMMR_BASE);
	p4.h = hi(COREMMR_BASE);

	r7.h = _exception_to_level5;
	r7.l = _exception_to_level5;
	[p4 + (EVT5 - COREMMR_BASE)] = r7;

	/*
	 * Save these registers, as they are only valid in exception context
	 *  (where we are now - as soon as we defer to IRQ5, they can change)
	 */
	p5.l = _deferred_regs;
	p5.h = _deferred_regs;
	r6 = [p4 + (DCPLB_FAULT_ADDR - COREMMR_BASE)];
	[p5 + (deferred_regs_DCPLB_FAULT_ADDR * 4)] = r6;

	r6 = [p4 + (ICPLB_FAULT_ADDR - COREMMR_BASE)];
	[p5 + (deferred_regs_ICPLB_FAULT_ADDR * 4)] = r6;

	/* Save the state of single stepping */
	r6 = SYSCFG;
	[p5 + (deferred_regs_SYSCFG * 4)] = r6;
	/* Clear it while we handle the exception in IRQ5 mode
	 * RESTORE_ALL_SYS will load it, so all we need to do is store it
	 * in the right place
	 */
	BITCLR(r6, SYSCFG_SSSTEP_P);
	[SP + PT_SYSCFG] = r6;

	/* Since we are going to clobber RETX, we need to save it */
	r6 = retx;
	[p5 + (deferred_regs_retx * 4)] = r6;

	/* Save the current IMASK, since we change in order to jump to level 5 */
	cli r6;
	[p5 + (deferred_regs_IMASK * 4)] = r6;

	/* Disable all interrupts, but make sure level 5 is enabled so
	 * we can switch to that level.
	 */
	r6 = 0x3f;
	sti r6;

	/* Clobber RETX so we don't end up back at a faulting instruction */
	[sp + PT_RETX] = r7;

	/* In case interrupts are disabled IPEND[4] (global interrupt disable bit)
	 * clear it (re-enabling interrupts again) by the special sequence of pushing
	 * RETI onto the stack.  This way we can lower ourselves to IVG5 even if the
	 * exception was taken after the interrupt handler was called but before it
	 * got a chance to enable global interrupts itself.
	 */
	[--sp] = reti;
	sp += 4;

	RAISE 5;
.Lexit_trap:
#endif

#if ANOMALY_05000257
	R7  = LC0;
	LC0 = R7;
	R7  = LC1;
	LC1 = R7;
#endif

	RESTORE_ALL_SYS
	sp = CONFIG_BFIN_SCRATCH_REG;
	rtx;
ENDPROC(_trap)

#ifdef CONFIG_EXCEPTION_DEFER
/* Deferred (IRQ5) exceptions */
ENTRY(_exception_to_level5)
	SAVE_ALL_SYS

	/* Now we have to fix things up */
	p4.l = lo(EVT5);
	p4.h = hi(EVT5);
	r0.l = _evt_default;
	r0.h = _evt_default;
	[p4] = r0;
	csync;

	p4.l = _deferred_regs;
	p4.h = _deferred_regs;
	r0 = [p4 + (deferred_regs_retx * 4)];
	[sp + PT_PC] = r0;

	r0 = [p4 + (deferred_regs_SYSCFG * 4)];
	[sp + PT_SYSCFG] = r0;

	r0 = sp;	/* stack frame pt_regs pointer argument ==> r0 */
	r1 = 5;	/* EVT5 space */
	sp += -12;
	call _trap_c;
	sp += 12;

	/* Restore IMASK */
	r0 = [p4 + (deferred_regs_IMASK * 4)];
	sti r0;

	RESTORE_ALL_SYS

	rti;
ENDPROC(_exception_to_level5)
#endif

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
