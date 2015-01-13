/*
 * UniPhier SG (SoC Glue) block registers
 *
 * Copyright (C) 2011-2014 Panasonic Corporation
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

#define SG_MEMCONF_CH0_SIZE_64MB	((0x0 << 10) | (0x01 << 0))
#define SG_MEMCONF_CH0_SIZE_128MB	((0x0 << 10) | (0x02 << 0))
#define SG_MEMCONF_CH0_SIZE_256MB	((0x0 << 10) | (0x03 << 0))
#define SG_MEMCONF_CH0_SIZE_512MB	((0x1 << 10) | (0x00 << 0))
#define SG_MEMCONF_CH0_SIZE_1024MB	((0x1 << 10) | (0x01 << 0))
#define SG_MEMCONF_CH0_NUM_1		(0x1 << 8)
#define SG_MEMCONF_CH0_NUM_2		(0x0 << 8)

#define SG_MEMCONF_CH1_SIZE_64MB	((0x0 << 11) | (0x01 << 2))
#define SG_MEMCONF_CH1_SIZE_128MB	((0x0 << 11) | (0x02 << 2))
#define SG_MEMCONF_CH1_SIZE_256MB	((0x0 << 11) | (0x03 << 2))
#define SG_MEMCONF_CH1_SIZE_512MB	((0x1 << 11) | (0x00 << 2))
#define SG_MEMCONF_CH1_SIZE_1024MB	((0x1 << 11) | (0x01 << 2))
#define SG_MEMCONF_CH1_NUM_1		(0x1 << 9)
#define SG_MEMCONF_CH1_NUM_2		(0x0 << 9)

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
#include <asm/io.h>

static inline void sg_set_pinsel(int n, int value)
{
	writel((readl(SG_PINSEL_ADDR(n)) & SG_PINSEL_MASK(n))
	       | SG_PINSEL_MODE(n, value), SG_PINSEL_ADDR(n));
}

static inline u32 sg_memconf_val_ch0(unsigned long size, int num)
{
	int size_mb = (size >> 20) / num;
	u32 ret;

	switch (size_mb) {
	case 64:
		ret = SG_MEMCONF_CH0_SIZE_64MB;
		break;
	case 128:
		ret = SG_MEMCONF_CH0_SIZE_128MB;
		break;
	case 256:
		ret = SG_MEMCONF_CH0_SIZE_256MB;
		break;
	case 512:
		ret = SG_MEMCONF_CH0_SIZE_512MB;
		break;
	case 1024:
		ret = SG_MEMCONF_CH0_SIZE_1024MB;
		break;
	default:
		BUG();
		break;
	}

	switch (num) {
	case 1:
		ret |= SG_MEMCONF_CH0_NUM_1;
		break;
	case 2:
		ret |= SG_MEMCONF_CH0_NUM_2;
		break;
	default:
		BUG();
		break;
	}
	return ret;
}

static inline u32 sg_memconf_val_ch1(unsigned long size, int num)
{
	int size_mb = (size >> 20) / num;
	u32 ret;

	switch (size_mb) {
	case 64:
		ret = SG_MEMCONF_CH1_SIZE_64MB;
		break;
	case 128:
		ret = SG_MEMCONF_CH1_SIZE_128MB;
		break;
	case 256:
		ret = SG_MEMCONF_CH1_SIZE_256MB;
		break;
	case 512:
		ret = SG_MEMCONF_CH1_SIZE_512MB;
		break;
	case 1024:
		ret = SG_MEMCONF_CH1_SIZE_1024MB;
		break;
	default:
		BUG();
		break;
	}

	switch (num) {
	case 1:
		ret |= SG_MEMCONF_CH1_NUM_1;
		break;
	case 2:
		ret |= SG_MEMCONF_CH1_NUM_2;
		break;
	default:
		BUG();
		break;
	}
	return ret;
}
#endif /* __ASSEMBLY__ */

#endif /* ARCH_SG_REGS_H */
