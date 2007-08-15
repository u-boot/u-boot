#include <common.h>
#include <command.h>

#if defined(CONFIG_CMD_KGDB)

#include <kgdb.h>
#include <asm/signal.h>
#include <asm/processor.h>

#define PC_REGNUM 64
#define SP_REGNUM 1

void breakinst(void);

int
kgdb_setjmp(long *buf)
{
	asm ("mflr 0; stw 0,0(%0);"
	     "stw 1,4(%0); stw 2,8(%0);"
	     "mfcr 0; stw 0,12(%0);"
	     "stmw 13,16(%0)"
	     : : "r" (buf));
	/* XXX should save fp regs as well */
	return 0;
}

void
kgdb_longjmp(long *buf, int val)
{
	if (val == 0)
		val = 1;
	asm ("lmw 13,16(%0);"
	     "lwz 0,12(%0); mtcrf 0x38,0;"
	     "lwz 0,0(%0); lwz 1,4(%0); lwz 2,8(%0);"
	     "mtlr 0; mr 3,%1"
	     : : "r" (buf), "r" (val));
}

static inline unsigned long
get_msr(void)
{
	unsigned long msr;
	asm volatile("mfmsr %0" : "=r" (msr):);
	return msr;
}

static inline void
set_msr(unsigned long msr)
{
	asm volatile("mtmsr %0" : : "r" (msr));
}

/* Convert the SPARC hardware trap type code to a unix signal number. */
/*
 * This table contains the mapping between PowerPC hardware trap types, and
 * signals, which are primarily what GDB understands.
 */
static struct hard_trap_info
{
	unsigned int tt;		/* Trap type code for powerpc */
	unsigned char signo;		/* Signal that we map this trap into */
} hard_trap_info[] = {
	{ 0x200, SIGSEGV },			/* machine check */
	{ 0x300, SIGSEGV },			/* address error (store) */
	{ 0x400, SIGBUS },			/* instruction bus error */
	{ 0x500, SIGINT },			/* interrupt */
	{ 0x600, SIGBUS },			/* alingment */
	{ 0x700, SIGTRAP },			/* breakpoint trap */
	{ 0x800, SIGFPE },			/* fpu unavail */
	{ 0x900, SIGALRM },			/* decrementer */
	{ 0xa00, SIGILL },			/* reserved */
	{ 0xb00, SIGILL },			/* reserved */
	{ 0xc00, SIGCHLD },			/* syscall */
	{ 0xd00, SIGTRAP },			/* single-step/watch */
	{ 0xe00, SIGFPE },			/* fp assist */
	{ 0, 0}				/* Must be last */
};

static int
computeSignal(unsigned int tt)
{
	struct hard_trap_info *ht;

	for (ht = hard_trap_info; ht->tt && ht->signo; ht++)
		if (ht->tt == tt)
			return ht->signo;

	return SIGHUP;         /* default for things we don't know about */
}

void
kgdb_enter(struct pt_regs *regs, kgdb_data *kdp)
{
	unsigned long msr;

	kdp->private[0] = msr = get_msr();
	set_msr(msr & ~MSR_EE);	/* disable interrupts */

	if (regs->nip == (unsigned long)breakinst) {
		/* Skip over breakpoint trap insn */
		regs->nip += 4;
	}
	regs->msr &= ~MSR_SE;

	/* reply to host that an exception has occurred */
	kdp->sigval = computeSignal(regs->trap);

	kdp->nregs = 2;

	kdp->regs[0].num = PC_REGNUM;
	kdp->regs[0].val = regs->nip;

	kdp->regs[1].num = SP_REGNUM;
	kdp->regs[1].val = regs->gpr[SP_REGNUM];
}

void
kgdb_exit(struct pt_regs *regs, kgdb_data *kdp)
{
	unsigned long msr = kdp->private[0];

	if (kdp->extype & KGDBEXIT_WITHADDR)
		regs->nip = kdp->exaddr;

	switch (kdp->extype & KGDBEXIT_TYPEMASK) {

	case KGDBEXIT_KILL:
	case KGDBEXIT_CONTINUE:
		set_msr(msr);
		break;

	case KGDBEXIT_SINGLE:
		regs->msr |= MSR_SE;
#if 0
		set_msr(msr | MSR_SE);
#endif
		break;
	}
}

int
kgdb_trap(struct pt_regs *regs)
{
	return (regs->trap);
}

/* return the value of the CPU registers.
 * some of them are non-PowerPC names :(
 * they are stored in gdb like:
 * struct {
 *     u32 gpr[32];
 *     f64 fpr[32];
 *     u32 pc, ps, cnd, lr; (ps=msr)
 *     u32 cnt, xer, mq;
 * }
 */

#define SPACE_REQUIRED	((32*4)+(32*8)+(6*4))

#ifdef CONFIG_8260
/* store floating double indexed */
#define STFDI(n,p)	__asm__ __volatile__ ("stfd " #n ",%0" : "=o"(p[2*n]))
/* store floating double multiple */
#define STFDM(p)	{ STFDI( 0,p); STFDI( 1,p); STFDI( 2,p); STFDI( 3,p); \
			  STFDI( 4,p); STFDI( 5,p); STFDI( 6,p); STFDI( 7,p); \
			  STFDI( 8,p); STFDI( 9,p); STFDI(10,p); STFDI(11,p); \
			  STFDI(12,p); STFDI(13,p); STFDI(14,p); STFDI(15,p); \
			  STFDI(16,p); STFDI(17,p); STFDI(18,p); STFDI(19,p); \
			  STFDI(20,p); STFDI(21,p); STFDI(22,p); STFDI(23,p); \
			  STFDI(24,p); STFDI(25,p); STFDI(26,p); STFDI(27,p); \
			  STFDI(28,p); STFDI(29,p); STFDI(30,p); STFDI(31,p); }
#endif

int
kgdb_getregs(struct pt_regs *regs, char *buf, int max)
{
	int i;
	unsigned long *ptr = (unsigned long *)buf;

	if (max < SPACE_REQUIRED)
		kgdb_error(KGDBERR_NOSPACE);

	if ((unsigned long)ptr & 3)
		kgdb_error(KGDBERR_ALIGNFAULT);

	/* General Purpose Regs */
	for (i = 0; i < 32; i++)
		*ptr++ = regs->gpr[i];

	/* Floating Point Regs */
#ifdef CONFIG_8260
	STFDM(ptr);
	ptr += 32*2;
#else
	for (i = 0; i < 32; i++) {
		*ptr++ = 0;
		*ptr++ = 0;
	}
#endif

	/* pc, msr, cr, lr, ctr, xer, (mq is unused) */
	*ptr++ = regs->nip;
	*ptr++ = regs->msr;
	*ptr++ = regs->ccr;
	*ptr++ = regs->link;
	*ptr++ = regs->ctr;
	*ptr++ = regs->xer;

	return (SPACE_REQUIRED);
}

/* set the value of the CPU registers */

#ifdef CONFIG_8260
/* load floating double */
#define LFD(n,v)	__asm__ __volatile__ ("lfd " #n ",%0" :: "o"(v))
/* load floating double indexed */
#define LFDI(n,p)	__asm__ __volatile__ ("lfd " #n ",%0" :: "o"((p)[2*n]))
/* load floating double multiple */
#define LFDM(p)		{ LFDI( 0,p); LFDI( 1,p); LFDI( 2,p); LFDI( 3,p); \
			  LFDI( 4,p); LFDI( 5,p); LFDI( 6,p); LFDI( 7,p); \
			  LFDI( 8,p); LFDI( 9,p); LFDI(10,p); LFDI(11,p); \
			  LFDI(12,p); LFDI(13,p); LFDI(14,p); LFDI(15,p); \
			  LFDI(16,p); LFDI(17,p); LFDI(18,p); LFDI(19,p); \
			  LFDI(20,p); LFDI(21,p); LFDI(22,p); LFDI(23,p); \
			  LFDI(24,p); LFDI(25,p); LFDI(26,p); LFDI(27,p); \
			  LFDI(28,p); LFDI(29,p); LFDI(30,p); LFDI(31,p); }
#endif

void
kgdb_putreg(struct pt_regs *regs, int regno, char *buf, int length)
{
	unsigned long *ptr = (unsigned long *)buf;

	if (regno < 0 || regno >= 70)
		kgdb_error(KGDBERR_BADPARAMS);
	else if (regno >= 32 && regno < 64) {
		if (length < 8)
			kgdb_error(KGDBERR_NOSPACE);
	}
	else {
		if (length < 4)
			kgdb_error(KGDBERR_NOSPACE);
	}

	if ((unsigned long)ptr & 3)
		kgdb_error(KGDBERR_ALIGNFAULT);

	if (regno >= 0 && regno < 32)
		regs->gpr[regno] = *ptr;
	else switch (regno) {

#ifdef CONFIG_8260
#define caseF(n) \
	case (n) + 32:	LFD(n, *ptr);		break;

caseF( 0) caseF( 1) caseF( 2) caseF( 3) caseF( 4) caseF( 5) caseF( 6) caseF( 7)
caseF( 8) caseF( 9) caseF(10) caseF(11) caseF(12) caseF(13) caseF(14) caseF(15)
caseF(16) caseF(17) caseF(18) caseF(19) caseF(20) caseF(21) caseF(22) caseF(23)
caseF(24) caseF(25) caseF(26) caseF(27) caseF(28) caseF(29) caseF(30) caseF(31)

#undef caseF
#endif

	case 64:	regs->nip = *ptr;	break;
	case 65:	regs->msr = *ptr;	break;
	case 66:	regs->ccr = *ptr;	break;
	case 67:	regs->link = *ptr;	break;
	case 68:	regs->ctr = *ptr;	break;
	case 69:	regs->ctr = *ptr;	break;

	default:
		kgdb_error(KGDBERR_BADPARAMS);
	}
}

void
kgdb_putregs(struct pt_regs *regs, char *buf, int length)
{
	int i;
	unsigned long *ptr = (unsigned long *)buf;

	if (length < SPACE_REQUIRED)
		kgdb_error(KGDBERR_NOSPACE);

	if ((unsigned long)ptr & 3)
		kgdb_error(KGDBERR_ALIGNFAULT);

	/*
	 * If the stack pointer has moved, you should pray.
	 * (cause only god can help you).
	 */

	/* General Purpose Regs */
	for (i = 0; i < 32; i++)
		regs->gpr[i] = *ptr++;

	/* Floating Point Regs */
#ifdef CONFIG_8260
	LFDM(ptr);
#endif
	ptr += 32*2;

	/* pc, msr, cr, lr, ctr, xer, (mq is unused) */
	regs->nip = *ptr++;
	regs->msr = *ptr++;
	regs->ccr = *ptr++;
	regs->link = *ptr++;
	regs->ctr = *ptr++;
	regs->xer = *ptr++;
}

/* This function will generate a breakpoint exception.  It is used at the
   beginning of a program to sync up with a debugger and can be used
   otherwise as a quick means to stop program execution and "break" into
   the debugger. */

void
kgdb_breakpoint(int argc, char *argv[])
{
	asm("	.globl breakinst\n\
	     breakinst: .long 0x7d821008\n\
	    ");
}

#endif
