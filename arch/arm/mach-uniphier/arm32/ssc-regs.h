/*
 * UniPhier System Cache (L2 Cache) registers
 *
 * Copyright (C) 2011-2014 Panasonic Corporation
 * Copyright (C) 2016      Socionext Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_SSC_REGS_H
#define ARCH_SSC_REGS_H

/* control registers */
#define UNIPHIER_SSCC		0x500c0000	/* Control Register */
#define    UNIPHIER_SSCC_BST			(0x1 << 20)	/* UCWG burst read */
#define    UNIPHIER_SSCC_ACT			(0x1 << 19)	/* Inst-Data separate */
#define    UNIPHIER_SSCC_WTG			(0x1 << 18)	/* WT gathering on */
#define    UNIPHIER_SSCC_PRD			(0x1 << 17)	/* enable pre-fetch */
#define    UNIPHIER_SSCC_ON			(0x1 <<  0)	/* enable cache */
#define UNIPHIER_SSCLPDAWCR	0x500c0030	/* Unified/Data Active Way Control */
#define UNIPHIER_SSCLPIAWCR	0x500c0034	/* Instruction Active Way Control */

/* revision registers */
#define UNIPHIER_SSCID		0x503c0100	/* ID Register */

/* operation registers */
#define UNIPHIER_SSCOPE		0x506c0244	/* Cache Operation Primitive Entry */
#define    UNIPHIER_SSCOPE_CM_INV		0x0	/* invalidate */
#define    UNIPHIER_SSCOPE_CM_CLEAN		0x1	/* clean */
#define    UNIPHIER_SSCOPE_CM_FLUSH		0x2	/* flush */
#define    UNIPHIER_SSCOPE_CM_SYNC		0x8	/* sync (drain bufs) */
#define    UNIPHIER_SSCOPE_CM_FLUSH_PREFETCH	0x9	/* flush p-fetch buf */
#define UNIPHIER_SSCOQM		0x506c0248
#define    UNIPHIER_SSCOQM_TID_MASK		(0x3 << 21)
#define    UNIPHIER_SSCOQM_TID_LRU_DATA		(0x0 << 21)
#define    UNIPHIER_SSCOQM_TID_LRU_INST		(0x1 << 21)
#define    UNIPHIER_SSCOQM_TID_WAY		(0x2 << 21)
#define    UNIPHIER_SSCOQM_S_MASK		(0x3 << 17)
#define    UNIPHIER_SSCOQM_S_RANGE		(0x0 << 17)
#define    UNIPHIER_SSCOQM_S_ALL		(0x1 << 17)
#define    UNIPHIER_SSCOQM_S_WAY		(0x2 << 17)
#define    UNIPHIER_SSCOQM_CE			(0x1 << 15)	/* notify completion */
#define    UNIPHIER_SSCOQM_CW			(0x1 << 14)
#define    UNIPHIER_SSCOQM_CM_MASK		(0x7)
#define    UNIPHIER_SSCOQM_CM_INV		0x0	/* invalidate */
#define    UNIPHIER_SSCOQM_CM_CLEAN		0x1	/* clean */
#define    UNIPHIER_SSCOQM_CM_FLUSH		0x2	/* flush */
#define    UNIPHIER_SSCOQM_CM_PREFETCH		0x3	/* prefetch to cache */
#define    UNIPHIER_SSCOQM_CM_PREFETCH_BUF	0x4	/* prefetch to pf-buf */
#define    UNIPHIER_SSCOQM_CM_TOUCH		0x5	/* touch */
#define    UNIPHIER_SSCOQM_CM_TOUCH_ZERO	0x6	/* touch to zero */
#define    UNIPHIER_SSCOQM_CM_TOUCH_DIRTY	0x7	/* touch with dirty */
#define UNIPHIER_SSCOQAD	0x506c024c	/* Cache Operation Queue Address */
#define UNIPHIER_SSCOQSZ	0x506c0250	/* Cache Operation Queue Size */
#define UNIPHIER_SSCOQMASK	0x506c0254	/* Cache Operation Queue Address Mask */
#define UNIPHIER_SSCOQWN	0x506c0258	/* Cache Operation Queue Way Number */
#define UNIPHIER_SSCOPPQSEF	0x506c025c	/* Cache Operation Queue Set Complete */
#define    UNIPHIER_SSCOPPQSEF_FE		(0x1 << 1)
#define    UNIPHIER_SSCOPPQSEF_OE		(0x1 << 0)
#define UNIPHIER_SSCOLPQS	0x506c0260	/* Cache Operation Queue Status */
#define    UNIPHIER_SSCOLPQS_EF			(0x1 << 2)
#define    UNIPHIER_SSCOLPQS_EST		(0x1 << 1)
#define    UNIPHIER_SSCOLPQS_QST		(0x1 << 0)

#define UNIPHIER_SSC_LINE_SIZE		128
#define UNIPHIER_SSC_RANGE_OP_MAX_SIZE	(0x00400000 - (UNIPHIER_SSC_LINE_SIZE))

#endif  /* ARCH_SSC_REGS_H */
