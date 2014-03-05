/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARC_ARCREGS_H
#define _ASM_ARC_ARCREGS_H

/*
 * ARC architecture has additional address space - auxiliary registers.
 * These registers are mostly used for configuration purposes.
 * These registers are not memory mapped and special commands are used for
 * access: "lr"/"sr".
 */

#define ARC_AUX_IDENTITY	0x04
#define ARC_AUX_STATUS32	0x0a

/* Instruction cache related auxiliary registers */
#define ARC_AUX_IC_IVIC		0x10
#define ARC_AUX_IC_CTRL		0x11
#define ARC_AUX_IC_IVIL		0x19
#if (CONFIG_ARC_MMU_VER > 2)
#define ARC_AUX_IC_PTAG		0x1E
#endif

/* Timer related auxiliary registers */
#define ARC_AUX_TIMER0_CNT	0x21	/* Timer 0 count */
#define ARC_AUX_TIMER0_CTRL	0x22	/* Timer 0 control */
#define ARC_AUX_TIMER0_LIMIT	0x23	/* Timer 0 limit */

#define ARC_AUX_INTR_VEC_BASE	0x25

/* Data cache related auxiliary registers */
#define ARC_AUX_DC_IVDC		0x47
#define ARC_AUX_DC_CTRL		0x48

#define ARC_AUX_DC_IVDL		0x4A
#define ARC_AUX_DC_FLSH		0x4B
#define ARC_AUX_DC_FLDL		0x4C
#if (CONFIG_ARC_MMU_VER > 2)
#define ARC_AUX_DC_PTAG		0x5C
#endif

#ifndef __ASSEMBLY__
/* Accessors for auxiliary registers */
#define read_aux_reg(reg)	__builtin_arc_lr(reg)

/* gcc builtin sr needs reg param to be long immediate */
#define write_aux_reg(reg_immed, val)		\
		__builtin_arc_sr((unsigned int)val, reg_immed)
#endif /* __ASSEMBLY__ */

#endif /* _ASM_ARC_ARCREGS_H */
