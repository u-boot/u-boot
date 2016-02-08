/*
 * U-Boot - architecture specific kgdb code
 *
 * Copyright 2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>

#include <kgdb.h>
#include <asm/processor.h>
#include <asm/mach-common/bits/core.h>
#include "kgdb.h"
#include <asm/deferred.h>
#include <asm/traps.h>
#include <asm/signal.h>

void kgdb_enter(struct pt_regs *regs, kgdb_data *kdp)
{
	/* disable interrupts */
	disable_interrupts();

	/* reply to host that an exception has occurred */
	kdp->sigval = kgdb_trap(regs);

	/* send the PC and the Stack Pointer */
	kdp->nregs = 2;
	kdp->regs[0].num = BFIN_PC;
	kdp->regs[0].val = regs->pc;

	kdp->regs[1].num = BFIN_SP;
	kdp->regs[1].val = (unsigned long)regs;

}

void kgdb_exit(struct pt_regs *regs, kgdb_data *kdp)
{
	if (kdp->extype & KGDBEXIT_WITHADDR)
		printf("KGDBEXIT_WITHADDR\n");

	switch (kdp->extype & KGDBEXIT_TYPEMASK) {
	case KGDBEXIT_KILL:
		printf("KGDBEXIT_KILL:\n");
		break;
	case KGDBEXIT_CONTINUE:
		/* Make sure the supervisor single step bit is clear */
		regs->syscfg &= ~1;
		break;
	case KGDBEXIT_SINGLE:
		/* set the supervisor single step bit */
		regs->syscfg |= 1;
		break;
	default:
		printf("KGDBEXIT : %d\n", kdp->extype);
	}

	/* enable interrupts */
	enable_interrupts();
}

int kgdb_trap(struct pt_regs *regs)
{
	/* ipend doesn't get filled in properly */
	switch (regs->seqstat & EXCAUSE) {
	case VEC_EXCPT01:
		return SIGTRAP;
	case VEC_EXCPT03:
		return SIGSEGV;
	case VEC_EXCPT02:
		return SIGTRAP;
	case VEC_EXCPT04 ... VEC_EXCPT15:
		return SIGILL;
	case VEC_STEP:
		return SIGTRAP;
	case VEC_OVFLOW:
		return SIGTRAP;
	case VEC_UNDEF_I:
		return SIGILL;
	case VEC_ILGAL_I:
		return SIGILL;
	case VEC_CPLB_VL:
		return SIGSEGV;
	case VEC_MISALI_D:
		return SIGBUS;
	case VEC_UNCOV:
		return SIGILL;
	case VEC_CPLB_MHIT:
		return SIGSEGV;
	case VEC_MISALI_I:
		return SIGBUS;
	case VEC_CPLB_I_VL:
		return SIGBUS;
	case VEC_CPLB_I_MHIT:
		return SIGSEGV;
	default:
		return SIGBUS;
	}
}

/*
 * getregs - gets the pt_regs, and gives them to kgdb's buffer
 */
int kgdb_getregs(struct pt_regs *regs, char *buf, int max)
{
	unsigned long *gdb_regs = (unsigned long *)buf;

	if (max < NUMREGBYTES)
		kgdb_error(KGDBERR_NOSPACE);

	if ((unsigned long)gdb_regs & 3)
		kgdb_error(KGDBERR_ALIGNFAULT);

	gdb_regs[BFIN_R0] = regs->r0;
	gdb_regs[BFIN_R1] = regs->r1;
	gdb_regs[BFIN_R2] = regs->r2;
	gdb_regs[BFIN_R3] = regs->r3;
	gdb_regs[BFIN_R4] = regs->r4;
	gdb_regs[BFIN_R5] = regs->r5;
	gdb_regs[BFIN_R6] = regs->r6;
	gdb_regs[BFIN_R7] = regs->r7;
	gdb_regs[BFIN_P0] = regs->p0;
	gdb_regs[BFIN_P1] = regs->p1;
	gdb_regs[BFIN_P2] = regs->p2;
	gdb_regs[BFIN_P3] = regs->p3;
	gdb_regs[BFIN_P4] = regs->p4;
	gdb_regs[BFIN_P5] = regs->p5;
	gdb_regs[BFIN_SP] = (unsigned long)regs;
	gdb_regs[BFIN_FP] = regs->fp;
	gdb_regs[BFIN_I0] = regs->i0;
	gdb_regs[BFIN_I1] = regs->i1;
	gdb_regs[BFIN_I2] = regs->i2;
	gdb_regs[BFIN_I3] = regs->i3;
	gdb_regs[BFIN_M0] = regs->m0;
	gdb_regs[BFIN_M1] = regs->m1;
	gdb_regs[BFIN_M2] = regs->m2;
	gdb_regs[BFIN_M3] = regs->m3;
	gdb_regs[BFIN_B0] = regs->b0;
	gdb_regs[BFIN_B1] = regs->b1;
	gdb_regs[BFIN_B2] = regs->b2;
	gdb_regs[BFIN_B3] = regs->b3;
	gdb_regs[BFIN_L0] = regs->l0;
	gdb_regs[BFIN_L1] = regs->l1;
	gdb_regs[BFIN_L2] = regs->l2;
	gdb_regs[BFIN_L3] = regs->l3;
	gdb_regs[BFIN_A0_DOT_X] = regs->a0x;
	gdb_regs[BFIN_A0_DOT_W] = regs->a0w;
	gdb_regs[BFIN_A1_DOT_X] = regs->a1x;
	gdb_regs[BFIN_A1_DOT_W] = regs->a1w;
	gdb_regs[BFIN_ASTAT] = regs->astat;
	gdb_regs[BFIN_RETS] = regs->rets;
	gdb_regs[BFIN_LC0] = regs->lc0;
	gdb_regs[BFIN_LT0] = regs->lt0;
	gdb_regs[BFIN_LB0] = regs->lb0;
	gdb_regs[BFIN_LC1] = regs->lc1;
	gdb_regs[BFIN_LT1] = regs->lt1;
	gdb_regs[BFIN_LB1] = regs->lb1;
	gdb_regs[BFIN_CYCLES] = 0;
	gdb_regs[BFIN_CYCLES2] = 0;
	gdb_regs[BFIN_USP] = regs->usp;
	gdb_regs[BFIN_SEQSTAT] = regs->seqstat;
	gdb_regs[BFIN_SYSCFG] = regs->syscfg;
	gdb_regs[BFIN_RETI] = regs->pc;
	gdb_regs[BFIN_RETX] = regs->retx;
	gdb_regs[BFIN_RETN] = regs->retn;
	gdb_regs[BFIN_RETE] = regs->rete;
	gdb_regs[BFIN_PC] = regs->pc;
	gdb_regs[BFIN_CC] = 0;
	gdb_regs[BFIN_EXTRA1] = 0;
	gdb_regs[BFIN_EXTRA2] = 0;
	gdb_regs[BFIN_EXTRA3] = 0;
	gdb_regs[BFIN_IPEND] = regs->ipend;

	return NUMREGBYTES;
}

/*
 * putreg - put kgdb's reg (regno) into the pt_regs
 */
void kgdb_putreg(struct pt_regs *regs, int regno, char *buf, int length)
{
	unsigned long *ptr = (unsigned long *)buf;

	if (regno < 0 || regno > BFIN_NUM_REGS)
		kgdb_error(KGDBERR_BADPARAMS);

	if (length < 4)
		kgdb_error(KGDBERR_NOSPACE);

	if ((unsigned long)ptr & 3)
		kgdb_error(KGDBERR_ALIGNFAULT);

	switch (regno) {
	case BFIN_R0:
		regs->r0 = *ptr;
		break;
	case BFIN_R1:
		regs->r1 = *ptr;
		break;
	case BFIN_R2:
		regs->r2 = *ptr;
		break;
	case BFIN_R3:
		regs->r3 = *ptr;
		break;
	case BFIN_R4:
		regs->r4 = *ptr;
		break;
	case BFIN_R5:
		regs->r5 = *ptr;
		break;
	case BFIN_R6:
		regs->r6 = *ptr;
		break;
	case BFIN_R7:
		regs->r7 = *ptr;
		break;
	case BFIN_P0:
		regs->p0 = *ptr;
		break;
	case BFIN_P1:
		regs->p1 = *ptr;
		break;
	case BFIN_P2:
		regs->p2 = *ptr;
		break;
	case BFIN_P3:
		regs->p3 = *ptr;
		break;
	case BFIN_P4:
		regs->p4 = *ptr;
		break;
	case BFIN_P5:
		regs->p5 = *ptr;
		break;
	case BFIN_SP:
		regs->reserved = *ptr;
		break;
	case BFIN_FP:
		regs->fp = *ptr;
		break;
	case BFIN_I0:
		regs->i0 = *ptr;
		break;
	case BFIN_I1:
		regs->i1 = *ptr;
		break;
	case BFIN_I2:
		regs->i2 = *ptr;
		break;
	case BFIN_I3:
		regs->i3 = *ptr;
		break;
	case BFIN_M0:
		regs->m0 = *ptr;
		break;
	case BFIN_M1:
		regs->m1 = *ptr;
		break;
	case BFIN_M2:
		regs->m2 = *ptr;
		break;
	case BFIN_M3:
		regs->m3 = *ptr;
		break;
	case BFIN_B0:
		regs->b0 = *ptr;
		break;
	case BFIN_B1:
		regs->b1 = *ptr;
		break;
	case BFIN_B2:
		regs->b2 = *ptr;
		break;
	case BFIN_B3:
		regs->b3 = *ptr;
		break;
	case BFIN_L0:
		regs->l0 = *ptr;
		break;
	case BFIN_L1:
		regs->l1 = *ptr;
		break;
	case BFIN_L2:
		regs->l2 = *ptr;
		break;
	case BFIN_L3:
		regs->l3 = *ptr;
		break;
	case BFIN_A0_DOT_X:
		regs->a0x = *ptr;
		break;
	case BFIN_A0_DOT_W:
		regs->a0w = *ptr;
		break;
	case BFIN_A1_DOT_X:
		regs->a1x = *ptr;
		break;
	case BFIN_A1_DOT_W:
		regs->a1w = *ptr;
		break;
	case BFIN_ASTAT:
		regs->astat = *ptr;
		break;
	case BFIN_RETS:
		regs->rets = *ptr;
		break;
	case BFIN_LC0:
		regs->lc0 = *ptr;
		break;
	case BFIN_LT0:
		regs->lt0 = *ptr;
		break;
	case BFIN_LB0:
		regs->lb0 = *ptr;
		break;
	case BFIN_LC1:
		regs->lc1 = *ptr;
		break;
	case BFIN_LT1:
		regs->lt1 = *ptr;
		break;
	case BFIN_LB1:
		regs->lb1 = *ptr;
		break;
/*
  BFIN_CYCLES,
  BFIN_CYCLES2,
  BFIN_USP,
  BFIN_SEQSTAT,
  BFIN_SYSCFG,
*/
	case BFIN_RETX:
		regs->retx = *ptr;
		break;
	case BFIN_RETN:
		regs->retn = *ptr;
		break;
	case BFIN_RETE:
		regs->rete = *ptr;
		break;
	case BFIN_PC:
		regs->pc = *ptr;
		break;

	default:
		kgdb_error(KGDBERR_BADPARAMS);
	}
}

void kgdb_putregs(struct pt_regs *regs, char *buf, int length)
{
	unsigned long *gdb_regs = (unsigned long *)buf;

	if (length != BFIN_NUM_REGS)
		kgdb_error(KGDBERR_NOSPACE);

	if ((unsigned long)gdb_regs & 3)
		kgdb_error(KGDBERR_ALIGNFAULT);

	regs->r0 = gdb_regs[BFIN_R0];
	regs->r1 = gdb_regs[BFIN_R1];
	regs->r2 = gdb_regs[BFIN_R2];
	regs->r3 = gdb_regs[BFIN_R3];
	regs->r4 = gdb_regs[BFIN_R4];
	regs->r5 = gdb_regs[BFIN_R5];
	regs->r6 = gdb_regs[BFIN_R6];
	regs->r7 = gdb_regs[BFIN_R7];
	regs->p0 = gdb_regs[BFIN_P0];
	regs->p1 = gdb_regs[BFIN_P1];
	regs->p2 = gdb_regs[BFIN_P2];
	regs->p3 = gdb_regs[BFIN_P3];
	regs->p4 = gdb_regs[BFIN_P4];
	regs->p5 = gdb_regs[BFIN_P5];
	regs->fp = gdb_regs[BFIN_FP];
/*	regs->sp = gdb_regs[BFIN_ ]; */
	regs->i0 = gdb_regs[BFIN_I0];
	regs->i1 = gdb_regs[BFIN_I1];
	regs->i2 = gdb_regs[BFIN_I2];
	regs->i3 = gdb_regs[BFIN_I3];
	regs->m0 = gdb_regs[BFIN_M0];
	regs->m1 = gdb_regs[BFIN_M1];
	regs->m2 = gdb_regs[BFIN_M2];
	regs->m3 = gdb_regs[BFIN_M3];
	regs->b0 = gdb_regs[BFIN_B0];
	regs->b1 = gdb_regs[BFIN_B1];
	regs->b2 = gdb_regs[BFIN_B2];
	regs->b3 = gdb_regs[BFIN_B3];
	regs->l0 = gdb_regs[BFIN_L0];
	regs->l1 = gdb_regs[BFIN_L1];
	regs->l2 = gdb_regs[BFIN_L2];
	regs->l3 = gdb_regs[BFIN_L3];
	regs->a0x = gdb_regs[BFIN_A0_DOT_X];
	regs->a0w = gdb_regs[BFIN_A0_DOT_W];
	regs->a1x = gdb_regs[BFIN_A1_DOT_X];
	regs->a1w = gdb_regs[BFIN_A1_DOT_W];
	regs->rets = gdb_regs[BFIN_RETS];
	regs->lc0 = gdb_regs[BFIN_LC0];
	regs->lt0 = gdb_regs[BFIN_LT0];
	regs->lb0 = gdb_regs[BFIN_LB0];
	regs->lc1 = gdb_regs[BFIN_LC1];
	regs->lt1 = gdb_regs[BFIN_LT1];
	regs->lb1 = gdb_regs[BFIN_LB1];
	regs->usp = gdb_regs[BFIN_USP];
	regs->syscfg = gdb_regs[BFIN_SYSCFG];
	regs->retx = gdb_regs[BFIN_PC];
	regs->retn = gdb_regs[BFIN_RETN];
	regs->rete = gdb_regs[BFIN_RETE];
	regs->pc = gdb_regs[BFIN_PC];

#if 0	/* can't change these */
	regs->astat = gdb_regs[BFIN_ASTAT];
	regs->seqstat = gdb_regs[BFIN_SEQSTAT];
	regs->ipend = gdb_regs[BFIN_IPEND];
#endif

}

void kgdb_breakpoint(int argc, char * const argv[])
{
	asm volatile ("excpt 0x1\n");
}
