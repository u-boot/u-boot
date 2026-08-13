// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 *
 * From arch/x86/lib/asm-offsets.c
 *
 * This program is used to generate definitions needed by
 * assembly language modules.
 */

#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <linux/kbuild.h>

static void __used output_ptreg_defines(void)
{
	COMMENT("LoongArch pt_regs offsets.");
	OFFSET(PT_R0, pt_regs, regs[0]);
	OFFSET(PT_R1, pt_regs, regs[1]);
	OFFSET(PT_R2, pt_regs, regs[2]);
	OFFSET(PT_R3, pt_regs, regs[3]);
	OFFSET(PT_R4, pt_regs, regs[4]);
	OFFSET(PT_R5, pt_regs, regs[5]);
	OFFSET(PT_R6, pt_regs, regs[6]);
	OFFSET(PT_R7, pt_regs, regs[7]);
	OFFSET(PT_R8, pt_regs, regs[8]);
	OFFSET(PT_R9, pt_regs, regs[9]);
	OFFSET(PT_R10, pt_regs, regs[10]);
	OFFSET(PT_R11, pt_regs, regs[11]);
	OFFSET(PT_R12, pt_regs, regs[12]);
	OFFSET(PT_R13, pt_regs, regs[13]);
	OFFSET(PT_R14, pt_regs, regs[14]);
	OFFSET(PT_R15, pt_regs, regs[15]);
	OFFSET(PT_R16, pt_regs, regs[16]);
	OFFSET(PT_R17, pt_regs, regs[17]);
	OFFSET(PT_R18, pt_regs, regs[18]);
	OFFSET(PT_R19, pt_regs, regs[19]);
	OFFSET(PT_R20, pt_regs, regs[20]);
	OFFSET(PT_R21, pt_regs, regs[21]);
	OFFSET(PT_R22, pt_regs, regs[22]);
	OFFSET(PT_R23, pt_regs, regs[23]);
	OFFSET(PT_R24, pt_regs, regs[24]);
	OFFSET(PT_R25, pt_regs, regs[25]);
	OFFSET(PT_R26, pt_regs, regs[26]);
	OFFSET(PT_R27, pt_regs, regs[27]);
	OFFSET(PT_R28, pt_regs, regs[28]);
	OFFSET(PT_R29, pt_regs, regs[29]);
	OFFSET(PT_R30, pt_regs, regs[30]);
	OFFSET(PT_R31, pt_regs, regs[31]);
	OFFSET(PT_CRMD, pt_regs, csr_crmd);
	OFFSET(PT_PRMD, pt_regs, csr_prmd);
	OFFSET(PT_EUEN, pt_regs, csr_euen);
	OFFSET(PT_ECFG, pt_regs, csr_ecfg);
	OFFSET(PT_ESTAT, pt_regs, csr_estat);
	OFFSET(PT_ERA, pt_regs, csr_era);
	OFFSET(PT_BVADDR, pt_regs, csr_badvaddr);
	OFFSET(PT_ORIG_A0, pt_regs, orig_a0);
	DEFINE(PT_SIZE, sizeof(struct pt_regs));
	BLANK();
}

int main(void)
{
	output_ptreg_defines();
	return 0;
}
