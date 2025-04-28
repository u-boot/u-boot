/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024 Ventana Micro Systems Ltd.
 *
 * Ported from linux insn-def.h.
 */

#ifndef _ASM_RISCV_INSN_DEF_H
#define _ASM_RISCV_INSN_DEF_H

#define INSN_I_SIMM12_SHIFT		20
#define INSN_I_RS1_SHIFT		15
#define INSN_I_FUNC3_SHIFT		12
#define INSN_I_RD_SHIFT			 7
#define INSN_I_OPCODE_SHIFT		 0

#define RV_OPCODE(v)		__ASM_STR(v)
#define RV_FUNC3(v)		__ASM_STR(v)
#define RV_FUNC7(v)		__ASM_STR(v)
#define RV_SIMM12(v)		__ASM_STR(v)
#define RV_RD(v)		__ASM_STR(v)
#define RV_RS1(v)		__ASM_STR(v)
#define RV_RS2(v)		__ASM_STR(v)
#define __RV_REG(v)		__ASM_STR(x ## v)
#define RV___RD(v)		__RV_REG(v)
#define RV___RS1(v)		__RV_REG(v)
#define RV___RS2(v)		__RV_REG(v)

#define RV_OPCODE_MISC_MEM	RV_OPCODE(15)
#define RV_OPCODE_SYSTEM	RV_OPCODE(115)

#define __INSN_I(opcode, func3, rd, rs1, simm12)	\
	".insn	i " opcode ", " func3 ", " rd ", " rs1 ", " simm12 "\n"

#define INSN_I(opcode, func3, rd, rs1, simm12)			\
	__INSN_I(RV_##opcode, RV_##func3, RV_##rd,		\
		 RV_##rs1, RV_##simm12)

#endif /* _ASM_RISCV_INSN_DEF_H */
