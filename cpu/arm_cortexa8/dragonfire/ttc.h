/*
 * Derived from ep3_1 drivers/ttc_v1_00_a/src/xttcepb_l.h
 * as of svn rev 1377.
 */

#ifndef __TTC_H__
#define __TTC_H__

#include <asm/io.h>

#if defined(CONFIG_TTC0)
# define TTC_ID   0
# define TTC_BASE XPSS_TTC0_BASE
#elif defined(CONFIG_TTC1)
# define TTC_ID   1
# define TTC_BASE XPSS_TTC1_BASE
#else
# error "Need to configure a TTC (0 or 1)"
#endif

/* Register offsets */
#define XDF_TTC_CLK_CNTRL_OFFSET	0x00000000 /* Clock Control Register */
#define XDF_TTC_CNT_CNTRL_OFFSET	0x0000000C /* Counter Control Register*/
#define XDF_TTC_COUNT_VALUE_OFFSET	0x00000018 /* Current Counter Value */
#define XDF_TTC_INTERVAL_VAL_OFFSET	0x00000024 /* Interval Count Value */
#define XDF_TTC_MATCH_0_OFFSET		0x00000030 /* Match 1 value */
#define XDF_TTC_MATCH_1_OFFSET		0x0000003C /* Match 2 value */
#define XDF_TTC_MATCH_2_OFFSET		0x00000048 /* Match 3 value */
#define XDF_TTC_ISR_OFFSET		0x00000054 /* Interrupt Status Register */
#define XDF_TTC_IER_OFFSET		0x00000060 /* Interrupt Enable Register */

/* Clock Control Register */
#define XDF_TTC_CLK_CNTRL_PS_EN_MASK    0x00000001 /* Prescale enable */
#define XDF_TTC_CLK_CNTRL_PS_VAL_MASK   0x0000001E /* Prescale value */
#define XDF_TTC_CLK_CNTRL_PS_VAL_SHIFT           1 /* Prescale shift */
#define XDF_TTC_CLK_CNTRL_PS_DISABLE            16 /* Prescale disable */
#define XDF_TTC_CLK_CNTRL_SRC_MASK      0x00000020 /* Clock source */
#define XDF_TTC_CLK_CNTRL_EXT_EDGE_MASK 0x00000040 /* External Clock edge */

/* Counter Control Register */
#define XDF_TTC_CNT_CNTRL_DIS_MASK	0x00000001 /* Disable the counter */
#define XDF_TTC_CNT_CNTRL_INT_MASK	0x00000002 /* Interval mode */
#define XDF_TTC_CNT_CNTRL_DECR_MASK	0x00000004 /* Decriment mode */
#define XDF_TTC_CNT_CNTRL_MATCH_MASK	0x00000008 /* Match mode */
#define XDF_TTC_CNT_CNTRL_RST_MASK	0x00000010 /* Reset counter */
#define XDF_TTC_CNT_CNTRL_EN_WAVE_MASK	0x00000020 /* Enable waveform */
#define XDF_TTC_CNT_CNTRL_POL_WAVE_MASK	0x00000040 /* Waveform polarity */
#define XDF_TTC_CNT_CNTRL_RESET_VALUE	0x00000021 /* Reset value */

/* Current Counter Value Register */
#define XDF_TTC_COUNT_VALUE_MASK  0x0000FFFF /* 16-bit counter value */

/* Interval Value Register */
#define XDF_TTC_INTERVAL_VAL_MASK  0x0000FFFF /* 16-bit Interval value*/

/* Match Registers */
#define XDF_TTC_MATCH_MASK    0x0000FFFF /* 16-bit Match value */
#define XDF_TTC_NUM_MATCH_REG 3

/* Interrupt Registers */
#define XDF_TTC_IXR_INTERVAL_MASK 0x00000001 /* Interval Interrupt */
#define XDF_TTC_IXR_MATCH_0_MASK  0x00000002 /* Match 1 Interrupt */
#define XDF_TTC_IXR_MATCH_1_MASK  0x00000004 /* Match 2 Interrupt */
#define XDF_TTC_IXR_MATCH_2_MASK  0x00000008 /* Match 3 Interrupt */
#define XDF_TTC_IXR_CNT_OVR_MASK  0x00000010 /* Counter Overflow */
#define XDF_TTC_IXR_ALL_MASK      0x0000001F /* All valide Interrupts */

/* Access macros */
#define xdfttc_readl(reg) \
	readl((void*)TTC_BASE + XDF_TTC_##reg##_OFFSET)
#define xdfttc_writel(reg,value) \
	writel((value),(void*)TTC_BASE + XDF_TTC_##reg##_OFFSET)

#endif /* __TTC_H__ */
