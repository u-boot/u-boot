/*
 * UniPhier System Cache (L2 Cache) registers
 *
 * Copyright (C) 2011-2014 Panasonic Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_SSC_REGS_H
#define ARCH_SSC_REGS_H

#define SSCC			0x500c0000
#define SSCC_BST		(0x1 << 20)
#define SSCC_ACT		(0x1 << 19)
#define SSCC_WTG		(0x1 << 18)
#define SSCC_PRD		(0x1 << 17)
#define SSCC_WBWA		(0x1 << 16)
#define SSCC_EX			(0x1 << 13)
#define SSCC_ON			(0x1 <<  0)

#define SSCLPDAWCR		0x500c0030

#define SSCOPE			0x506c0244
#define SSCOPE_CM_SYNC		0x00000008

#define SSCOQM			0x506c0248
#define SSCOQM_TID_MASK		(0x3 << 21)
#define SSCOQM_TID_BY_WAY	(0x2 << 21)
#define SSCOQM_TID_BY_INST_WAY	(0x1 << 21)
#define SSCOQM_TID_BY_DATA_WAY	(0x0 << 21)
#define SSCOQM_S_MASK		(0x3 << 17)
#define SSCOQM_S_WAY		(0x2 << 17)
#define SSCOQM_S_ALL		(0x1 << 17)
#define SSCOQM_S_ADDRESS	(0x0 << 17)
#define SSCOQM_CE		(0x1 << 15)
#define SSCOQM_CW		(0x1 << 14)
#define SSCOQM_CM_MASK		(0x7)
#define SSCOQM_CM_DIRT_TOUCH	(0x7)
#define SSCOQM_CM_ZERO_TOUCH	(0x6)
#define SSCOQM_CM_NORM_TOUCH	(0x5)
#define SSCOQM_CM_PREF_FETCH	(0x4)
#define SSCOQM_CM_SSC_FETCH	(0x3)
#define SSCOQM_CM_WB_INV	(0x2)
#define SSCOQM_CM_WB		(0x1)
#define SSCOQM_CM_INV		(0x0)

#define SSCOQAD			0x506c024c
#define SSCOQSZ			0x506c0250
#define SSCOQWN			0x506c0258

#define SSCOPPQSEF		0x506c025c
#define SSCOPPQSEF_FE		(0x1 << 1)
#define SSCOPPQSEF_OE		(0x1 << 0)

#define SSCOLPQS		0x506c0260
#define SSCOLPQS_EF		(0x1 << 2)
#define SSCOLPQS_EST		(0x1 << 1)
#define SSCOLPQS_QST		(0x1 << 0)

#define SSCOQCE0		0x506c0270

#define SSC_LINE_SIZE		128
#define SSC_NUM_ENTRIES		256
#define SSC_WAY_SIZE		((SSC_LINE_SIZE) * (SSC_NUM_ENTRIES))
#define SSC_RANGE_OP_MAX_SIZE	(0x00400000 - (SSC_LINE_SIZE))

#endif  /* ARCH_SSC_REGS_H */
