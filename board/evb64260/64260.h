#ifndef __64260_H__
#define __64260_H__

/* CPU Configuration bits */
#define CPU_CONF_ADDR_MISS_EN	(1 << 8)
#define	CPU_CONF_AACK_DELAY	(1 << 11)
#define	CPU_CONF_ENDIANESS	(1 << 12)
#define CPU_CONF_PIPELINE	(1 << 13)
#define CPU_CONF_TA_DELAY	(1 << 15)
#define CPU_CONF_RD_OOO		(1 << 16)
#define CPU_CONF_STOP_RETRY	(1 << 17)
#define CPU_CONF_MULTI_DECODE	(1 << 18)
#define CPU_CONF_DP_VALID	(1 << 19)
#define CPU_CONF_PERR_PROP	(1 << 22)
#define CPU_CONF_FAST_CLK	(1 << 23)
#define CPU_CONF_AACK_DELAY_2	(1 << 25)
#define CPU_CONF_AP_VALID	(1 << 26)
#define CPU_CONF_REMAP_WR_DIS	(1 << 27)
#define CPU_CONF_CONF_SB_DIS	(1 << 28)
#define CPU_CONF_IO_SB_DIS	(1 << 29)
#define CPU_CONF_CLK_SYNC	(1 << 30)

/* CPU Master Control bits */
#define CPU_MAST_CTL_ARB_EN	(1 << 8)
#define CPU_MAST_CTL_MASK_BR_1	(1 << 9)
#define CPU_MAST_CTL_M_WR_TRIG	(1 << 10)
#define CPU_MAST_CTL_M_RD_TRIG	(1 << 11)
#define CPU_MAST_CTL_CLEAN_BLK	(1 << 12)
#define CPU_MAST_CTL_FLUSH_BLK	(1 << 13)

#endif /* __64260_H__ */
