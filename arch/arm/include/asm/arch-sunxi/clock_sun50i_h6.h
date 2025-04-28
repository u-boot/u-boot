/*
 * Allwinner H6 clock register definitions
 *
 * (C) Copyright 2017 Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_CLOCK_SUN50I_H6_H
#define _SUNXI_CLOCK_SUN50I_H6_H

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define CCU_H6_PLL1_CFG			0x000
#define CCU_H6_PLL5_CFG			0x010
#define CCU_H6_PLL6_CFG			0x020
#define CCU_H6_CPU_AXI_CFG		0x500
#define CCU_H6_PSI_AHB1_AHB2_CFG	0x510
#define CCU_H6_AHB3_CFG			0x51c
#define CCU_H6_APB1_CFG			0x520
#define CCU_H6_APB2_CFG			0x524
#define CCU_H6_MBUS_CFG			0x540
#define CCU_H6_DRAM_CLK_CFG		0x800
#define CCU_H6_DRAM_GATE_RESET		0x80c
#define CCU_MMC0_CLK_CFG		0x830
#define CCU_MMC1_CLK_CFG		0x834
#define CCU_MMC2_CLK_CFG		0x838
#define CCU_H6_MMC_GATE_RESET		0x84c
#define CCU_H6_UART_GATE_RESET		0x90c
#define CCU_H6_I2C_GATE_RESET		0x91c

/* pll1 bit field */
#define CCM_PLL1_CTRL_EN		BIT(31)
#define CCM_PLL1_LDO_EN			BIT(30)
#define CCM_PLL1_LOCK_EN		BIT(29)
#define CCM_PLL1_LOCK			BIT(28)
#define CCM_PLL1_OUT_EN			BIT(27)
#define CCM_PLL1_CLOCK_TIME_2		(2 << 24)
#define CCM_PLL1_CTRL_P(p)		((p) << 16)
#define CCM_PLL1_CTRL_N(n)		(((n) - 1) << 8)

/* pll5 bit field */
#define CCM_PLL5_CTRL_EN		BIT(31)
#define CCM_PLL5_LOCK_EN		BIT(29)
#define CCM_PLL5_LOCK			BIT(28)
#define CCM_PLL5_OUT_EN			BIT(27)
#define CCM_PLL5_CTRL_N(n)		(((n) - 1) << 8)
#define CCM_PLL5_CTRL_DIV1(div1)	((div1) << 0)
#define CCM_PLL5_CTRL_DIV2(div0)	((div0) << 1)

/* pll6 bit field */
#define CCM_PLL6_CTRL_EN		BIT(31)
#define CCM_PLL6_LOCK_EN		BIT(29)
#define CCM_PLL6_LOCK			BIT(28)
#define CCM_PLL6_CTRL_P0_SHIFT		16
#define CCM_PLL6_CTRL_P0_MASK		(0x7 << CCM_PLL6_CTRL_P0_SHIFT)
#define CCM_PLL6_CTRL_N_SHIFT		8
#define CCM_PLL6_CTRL_N_MASK		(0xff << CCM_PLL6_CTRL_N_SHIFT)
#define CCM_PLL6_CTRL_DIV1_SHIFT	0
#define CCM_PLL6_CTRL_DIV1_MASK		(0x1 << CCM_PLL6_CTRL_DIV1_SHIFT)
#define CCM_PLL6_CTRL_DIV2_SHIFT	1
#define CCM_PLL6_CTRL_DIV2_MASK		(0x1 << CCM_PLL6_CTRL_DIV2_SHIFT)

/* cpu_axi bit field*/
#define CCM_CPU_AXI_MUX_MASK		(0x3 << 24)
#define CCM_CPU_AXI_MUX_OSC24M		(0x0 << 24)
#define CCM_CPU_AXI_MUX_PLL_CPUX	(0x3 << 24)
#define CCM_CPU_AXI_APB_MASK		0x300
#define CCM_CPU_AXI_AXI_MASK		0x3
#define CCM_CPU_AXI_DEFAULT_FACTORS	0x301

#ifdef CONFIG_MACH_SUN50I_H6				/* H6 */

#define CCM_PLL6_DEFAULT		0xa0006300
#define CCM_PSI_AHB1_AHB2_DEFAULT	0x03000102
#define CCM_AHB3_DEFAULT		0x03000002
#define CCM_APB1_DEFAULT		0x03000102

#elif CONFIG_MACH_SUN50I_H616				/* H616 */

#define CCM_PLL6_DEFAULT		0xa8003100
#define CCM_PSI_AHB1_AHB2_DEFAULT	0x03000002
#define CCM_AHB3_DEFAULT		0x03000002
#define CCM_APB1_DEFAULT		0x03000102

#elif CONFIG_MACH_SUN8I_R528				/* R528 */

#define CCM_PLL6_DEFAULT		0xe8216300
#define CCM_PSI_AHB1_AHB2_DEFAULT	0x03000002
#define CCM_APB1_DEFAULT		0x03000102
#endif

/* apb2 bit field */
#define APB2_CLK_SRC_OSC24M		(0x0 << 24)
#define APB2_CLK_SRC_OSC32K		(0x1 << 24)
#define APB2_CLK_SRC_PSI		(0x2 << 24)
#define APB2_CLK_SRC_PLL6		(0x3 << 24)
#define APB2_CLK_SRC_MASK		(0x3 << 24)
#define APB2_CLK_RATE_N_1		(0x0 << 8)
#define APB2_CLK_RATE_N_2		(0x1 << 8)
#define APB2_CLK_RATE_N_4		(0x2 << 8)
#define APB2_CLK_RATE_N_8		(0x3 << 8)
#define APB2_CLK_RATE_N_MASK		(3 << 8)
#define APB2_CLK_RATE_M(m)		(((m)-1) << 0)
#define APB2_CLK_RATE_M_MASK            (3 << 0)

/* MBUS clock bit field */
#define MBUS_ENABLE			BIT(31)
#define MBUS_RESET			BIT(30)
#define MBUS_CLK_SRC_MASK		GENMASK(25, 24)
#define MBUS_CLK_SRC_OSCM24		(0 << 24)
#define MBUS_CLK_SRC_PLL6X2		(1 << 24)
#define MBUS_CLK_SRC_PLL5		(2 << 24)
#define MBUS_CLK_SRC_PLL6X4		(3 << 24)
#define MBUS_CLK_M(m)			(((m)-1) << 0)

/* Module gate/reset shift*/
#define RESET_SHIFT			(16)
#define GATE_SHIFT			(0)

/* DRAM clock bit field */
#define DRAM_MOD_RESET			BIT(30)
#define DRAM_CLK_UPDATE			BIT(27)
#define DRAM_CLK_SRC_MASK		GENMASK(25, 24)
#define DRAM_CLK_SRC_PLL5		(0 << 24)
#define DRAM_CLK_M(m)			(((m)-1) << 0)

/* MMC clock bit field */
#define CCM_MMC_CTRL_M(x)		((x) - 1)
#define CCM_MMC_CTRL_N(x)		((x) << 8)
#define CCM_MMC_CTRL_OSCM24		(0x0 << 24)
#define CCM_MMC_CTRL_PLL6		(0x1 << 24)
#define CCM_MMC_CTRL_PLL_PERIPH2X2	(0x2 << 24)
#define CCM_MMC_CTRL_ENABLE		(0x1 << 31)
/* H6 doesn't have these delays */
#define CCM_MMC_CTRL_OCLK_DLY(a)	((void) (a), 0)
#define CCM_MMC_CTRL_SCLK_DLY(a)	((void) (a), 0)

#ifndef __ASSEMBLY__
void clock_set_pll1(unsigned int hz);
unsigned int clock_get_pll6(void);
#endif

#endif /* _SUNXI_CLOCK_SUN50I_H6_H */
