/*
 * (C) Copyright 2003
 * Ingo Assmus <ingo.assmus@keymile.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * main board support/init for the Galileo Eval board DB64360.
 */

#ifndef __64360_H__
#define __64360_H__

/* CPU Configuration bits */
#define CPU_CONF_ADDR_MISS_EN	(1 << 8)
#define	CPU_CONF_SINGLE_CPU	(1 << 11)
#define	CPU_CONF_ENDIANESS	(1 << 12)
#define CPU_CONF_PIPELINE	(1 << 13)
#define CPU_CONF_STOP_RETRY	(1 << 17)
#define CPU_CONF_MULTI_DECODE	(1 << 18)
#define CPU_CONF_DP_VALID	(1 << 19)
#define CPU_CONF_PERR_PROP	(1 << 22)
#define CPU_CONF_AACK_DELAY_2	(1 << 25)
#define CPU_CONF_AP_VALID	(1 << 26)
#define CPU_CONF_REMAP_WR_DIS	(1 << 27)

/* CPU Master Control bits */
#define CPU_MAST_CTL_ARB_EN	(1 << 8)
#define CPU_MAST_CTL_MASK_BR_1	(1 << 9)
#define CPU_MAST_CTL_M_WR_TRIG	(1 << 10)
#define CPU_MAST_CTL_M_RD_TRIG	(1 << 11)
#define CPU_MAST_CTL_CLEAN_BLK	(1 << 12)
#define CPU_MAST_CTL_FLUSH_BLK	(1 << 13)

#endif /* __64360_H__ */
