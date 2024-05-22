// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <linux/compat.h>
#include <linux/bitfield.h>
#include <linux/linkage.h>
#include <hang.h>
#include <interrupt.h>
#include <irq_func.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/regdef.h>
#include <asm/loongarch.h>

DECLARE_GLOBAL_DATA_PTR;

static struct resume_data *resume;

void set_resume(struct resume_data *data)
{
	resume = data;
}

static void show_regs(struct pt_regs *regs)
{
#if IS_ENABLED(CONFIG_SHOW_REGS)
	const int field = 2 * sizeof(unsigned long);

#define GPR_FIELD(x) field, regs->regs[x]
	printf("pc %0*lx ra %0*lx tp %0*lx sp %0*lx\n",
	       field, regs->csr_era, GPR_FIELD(1), GPR_FIELD(2), GPR_FIELD(3));
	printf("a0 %0*lx a1 %0*lx a2 %0*lx a3 %0*lx\n",
	       GPR_FIELD(4), GPR_FIELD(5), GPR_FIELD(6), GPR_FIELD(7));
	printf("a4 %0*lx a5 %0*lx a6 %0*lx a7 %0*lx\n",
	       GPR_FIELD(8), GPR_FIELD(9), GPR_FIELD(10), GPR_FIELD(11));
	printf("t0 %0*lx t1 %0*lx t2 %0*lx t3 %0*lx\n",
	       GPR_FIELD(12), GPR_FIELD(13), GPR_FIELD(14), GPR_FIELD(15));
	printf("t4 %0*lx t5 %0*lx t6 %0*lx t7 %0*lx\n",
	       GPR_FIELD(16), GPR_FIELD(17), GPR_FIELD(18), GPR_FIELD(19));
	printf("t8 %0*lx u0 %0*lx s9 %0*lx s0 %0*lx\n",
	       GPR_FIELD(20), GPR_FIELD(21), GPR_FIELD(22), GPR_FIELD(23));
	printf("s1 %0*lx s2 %0*lx s3 %0*lx s4 %0*lx\n",
	       GPR_FIELD(24), GPR_FIELD(25), GPR_FIELD(26), GPR_FIELD(27));
	printf("s5 %0*lx s6 %0*lx s7 %0*lx s8 %0*lx\n",
	       GPR_FIELD(28), GPR_FIELD(29), GPR_FIELD(30), GPR_FIELD(31));
#endif
}

static void __maybe_unused show_backtrace(struct pt_regs *regs)
{
	uintptr_t *fp = (uintptr_t *)regs->regs[0x16];
	unsigned int count = 0;
	ulong ra;

	printf("\nbacktrace:\n");

	/*
	 * there are a few entry points where the s0 register is
	 * set to gd, so to avoid changing those, just abort if
	 * the value is the same.
	 */
	while (fp && fp != (uintptr_t *)gd) {
		ra = fp[-1];
		printf("%3d: fp: " REG_FMT " ra: " REG_FMT,
		       count, (ulong)fp, ra);

		if (gd && gd->flags & GD_FLG_RELOC)
			printf(" - ra: " REG_FMT " reloc adjusted\n",
			       ra - gd->reloc_off);
		else
			printf("\n");

		fp = (uintptr_t *)fp[-2];
		count++;
	}
}

static const char *humanize_exc_name(unsigned int ecode, unsigned int esubcode)
{
	/*
	 * LoongArch users and developers are probably more familiar with
	 * those names found in the ISA manual, so we are going to print out
	 * the latter. This will require some mapping.
	 */
	switch (ecode) {
	case EXCCODE_RSV: return "INT";
	case EXCCODE_TLBL: return "PIL";
	case EXCCODE_TLBS: return "PIS";
	case EXCCODE_TLBI: return "PIF";
	case EXCCODE_TLBM: return "PME";
	case EXCCODE_TLBNR: return "PNR";
	case EXCCODE_TLBNX: return "PNX";
	case EXCCODE_TLBPE: return "PPI";
	case EXCCODE_ADE:
		switch (esubcode) {
		case EXSUBCODE_ADEF: return "ADEF";
		case EXSUBCODE_ADEM: return "ADEM";
		}
		break;
	case EXCCODE_ALE: return "ALE";
	case EXCCODE_BCE: return "BCE";
	case EXCCODE_SYS: return "SYS";
	case EXCCODE_BP: return "BRK";
	case EXCCODE_INE: return "INE";
	case EXCCODE_IPE: return "IPE";
	case EXCCODE_FPDIS: return "FPD";
	case EXCCODE_LSXDIS: return "SXD";
	case EXCCODE_LASXDIS: return "ASXD";
	case EXCCODE_FPE:
		switch (esubcode) {
		case EXCSUBCODE_FPE: return "FPE";
		case EXCSUBCODE_VFPE: return "VFPE";
		}
		break;
	case EXCCODE_WATCH:
		switch (esubcode) {
		case EXCSUBCODE_WPEF: return "WPEF";
		case EXCSUBCODE_WPEM: return "WPEM";
		}
		break;
	case EXCCODE_BTDIS: return "BTD";
	case EXCCODE_BTE: return "BTE";
	case EXCCODE_GSPR: return "GSPR";
	case EXCCODE_HVC: return "HVC";
	case EXCCODE_GCM:
		switch (esubcode) {
		case EXCSUBCODE_GCSC: return "GCSC";
		case EXCSUBCODE_GCHC: return "GCHC";
		}
		break;
	/*
	 * The manual did not mention the EXCCODE_SE case, but print out it
	 * nevertheless.
	 */
	case EXCCODE_SE: return "SE";
	}

	return "???";
}

asmlinkage void do_exceptions(struct pt_regs *regs)
{
	unsigned int ecode = FIELD_GET(CSR_ESTAT_EXC, regs->csr_estat);
	unsigned int esubcode = FIELD_GET(CSR_ESTAT_ESUBCODE, regs->csr_estat);

	printf("Unhandled exception: %s\n", humanize_exc_name(ecode, esubcode));

	printf("ERA: " REG_FMT " ra: " REG_FMT "\n",
	       regs->csr_era, regs->regs[1]);
	/* Print relocation adjustments, but only if gd is initialized */
	if (gd && gd->flags & GD_FLG_RELOC)
		printf("ERA: " REG_FMT " ra: " REG_FMT " reloc adjusted\n",
		       regs->csr_era - gd->reloc_off, regs->regs[1] - gd->reloc_off);

	printf("CRMD: " REG_FMT "\n", regs->csr_crmd);
	if (ecode >= EXCCODE_TLBL && ecode <= EXCCODE_ALE)
		printf("BADV: " REG_FMT "\n", regs->csr_badvaddr);

	printf("\n");
	show_regs(regs);

	if (CONFIG_IS_ENABLED(FRAMEPOINTER))
		show_backtrace(regs);

	panic("\n");
}

int interrupt_init(void)
{
	return 0;
}

/*
 * enable interrupts
 */
void enable_interrupts(void)
{
}

/*
 * disable interrupts
 */
int disable_interrupts(void)
{
	return 0;
}
