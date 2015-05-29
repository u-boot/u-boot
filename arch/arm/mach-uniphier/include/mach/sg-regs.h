/*
 * UniPhier SG (SoC Glue) block registers
 *
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_SG_REGS_H
#define ARCH_SG_REGS_H

/* Base Address */
#define SG_CTRL_BASE			0x5f800000
#define SG_DBG_BASE			0x5f900000

/* Revision */
#define SG_REVISION			(SG_CTRL_BASE | 0x0000)
#define SG_REVISION_TYPE_SHIFT		16
#define SG_REVISION_TYPE_MASK		(0xff << SG_REVISION_TYPE_SHIFT)
#define SG_REVISION_MODEL_SHIFT		8
#define SG_REVISION_MODEL_MASK		(0x3 << SG_REVISION_MODEL_SHIFT)
#define SG_REVISION_REV_SHIFT		0
#define SG_REVISION_REV_MASK		(0x1f << SG_REVISION_REV_SHIFT)

/* Memory Configuration */
#define SG_MEMCONF			(SG_CTRL_BASE | 0x0400)

#define SG_MEMCONF_CH0_SZ_64M		((0x0 << 10) | (0x01 << 0))
#define SG_MEMCONF_CH0_SZ_128M		((0x0 << 10) | (0x02 << 0))
#define SG_MEMCONF_CH0_SZ_256M		((0x0 << 10) | (0x03 << 0))
#define SG_MEMCONF_CH0_SZ_512M		((0x1 << 10) | (0x00 << 0))
#define SG_MEMCONF_CH0_SZ_1G		((0x1 << 10) | (0x01 << 0))
#define SG_MEMCONF_CH0_NUM_1		(0x1 << 8)
#define SG_MEMCONF_CH0_NUM_2		(0x0 << 8)

#define SG_MEMCONF_CH1_SZ_64M		((0x0 << 11) | (0x01 << 2))
#define SG_MEMCONF_CH1_SZ_128M		((0x0 << 11) | (0x02 << 2))
#define SG_MEMCONF_CH1_SZ_256M		((0x0 << 11) | (0x03 << 2))
#define SG_MEMCONF_CH1_SZ_512M		((0x1 << 11) | (0x00 << 2))
#define SG_MEMCONF_CH1_SZ_1G		((0x1 << 11) | (0x01 << 2))
#define SG_MEMCONF_CH1_NUM_1		(0x1 << 9)
#define SG_MEMCONF_CH1_NUM_2		(0x0 << 9)

#define SG_MEMCONF_CH2_SZ_64M		((0x0 << 26) | (0x01 << 16))
#define SG_MEMCONF_CH2_SZ_128M		((0x0 << 26) | (0x02 << 16))
#define SG_MEMCONF_CH2_SZ_256M		((0x0 << 26) | (0x03 << 16))
#define SG_MEMCONF_CH2_SZ_512M		((0x1 << 26) | (0x00 << 16))
#define SG_MEMCONF_CH2_NUM_1		(0x1 << 24)
#define SG_MEMCONF_CH2_NUM_2		(0x0 << 24)

#define SG_MEMCONF_SPARSEMEM		(0x1 << 4)

/* Pin Control */
#define SG_PINCTRL_BASE			(SG_CTRL_BASE | 0x1000)

#if defined(CONFIG_MACH_PH1_PRO4)
# define SG_PINCTRL(n)			(SG_PINCTRL_BASE + (n) * 8)
#elif defined(CONFIG_MACH_PH1_LD4) || defined(CONFIG_MACH_PH1_SLD8)
# define SG_PINCTRL(n)			(SG_PINCTRL_BASE + (n) * 4)
#endif

#if defined(CONFIG_MACH_PH1_PRO4)
#define SG_PINSELBITS			4
#elif defined(CONFIG_MACH_PH1_LD4) || defined(CONFIG_MACH_PH1_SLD8)
#define SG_PINSELBITS			8
#endif

#define SG_PINSEL_ADDR(n)		(SG_PINCTRL((n) * (SG_PINSELBITS) / 32))
#define SG_PINSEL_MASK(n)		(~(((1 << (SG_PINSELBITS)) - 1) << \
						((n) * (SG_PINSELBITS) % 32)))
#define SG_PINSEL_MODE(n, mode)		((mode) << ((n) * (SG_PINSELBITS) % 32))

/* Only for PH1-Pro4 */
#define SG_LOADPINCTRL			(SG_CTRL_BASE | 0x1700)

/* Input Enable */
#define SG_IECTRL			(SG_CTRL_BASE | 0x1d00)

/* Pin Monitor */
#define SG_PINMON0			(SG_DBG_BASE | 0x0100)

#define SG_PINMON0_CLK_MODE_UPLLSRC_MASK	(0x3 << 19)
#define SG_PINMON0_CLK_MODE_UPLLSRC_DEFAULT	(0x0 << 19)
#define SG_PINMON0_CLK_MODE_UPLLSRC_VPLL27A	(0x2 << 19)
#define SG_PINMON0_CLK_MODE_UPLLSRC_VPLL27B	(0x3 << 19)

#define SG_PINMON0_CLK_MODE_AXOSEL_MASK		(0x3 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_24576KHZ	(0x0 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ	(0x1 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_6144KHZ	(0x2 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_6250KHZ	(0x3 << 16)

#define SG_PINMON0_CLK_MODE_AXOSEL_DEFAULT	(0x0 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ_U	(0x1 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_20480KHZ	(0x2 << 16)
#define SG_PINMON0_CLK_MODE_AXOSEL_25000KHZ_A	(0x3 << 16)

#ifdef __ASSEMBLY__

	.macro	set_pinsel, n, value, ra, rd
	ldr	\ra, =SG_PINSEL_ADDR(\n)
	ldr	\rd, [\ra]
	and	\rd, \rd, #SG_PINSEL_MASK(\n)
	orr	\rd, \rd, #SG_PINSEL_MODE(\n, \value)
	str	\rd, [\ra]
	.endm

#else

#include <linux/types.h>
#include <linux/io.h>

static inline void sg_set_pinsel(int n, int value)
{
	writel((readl(SG_PINSEL_ADDR(n)) & SG_PINSEL_MASK(n))
	       | SG_PINSEL_MODE(n, value), SG_PINSEL_ADDR(n));
}

#endif /* __ASSEMBLY__ */

#endif /* ARCH_SG_REGS_H */
