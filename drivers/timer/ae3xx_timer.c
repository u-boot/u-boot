/*
 * Andestech ATCPIT100 timer driver
 *
 * (C) Copyright 2016
 * Rick Chen, NDS32 Software Engineering, rick@andestech.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <linux/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define REG32_TMR(x)	(*(unsigned long *)	((plat->regs) + (x>>2)))

/*
 * Definition of register offsets
 */

/* ID and Revision Register */
#define ID_REV		0x0

/* Configuration Register */
#define CFG		0x10

/* Interrupt Enable Register */
#define INT_EN		0x14
#define CH_INT_EN(c , i)	((1<<i)<<(4*c))

/* Interrupt Status Register */
#define INT_STA		0x18
#define CH_INT_STA(c , i)	((1<<i)<<(4*c))

/* Channel Enable Register */
#define CH_EN		0x1C
#define CH_TMR_EN(c , t)	((1<<t)<<(4*c))

/* Ch n Control REgister */
#define CH_CTL(n)	(0x20+0x10*n)
/* Channel clock source , bit 3 , 0:External clock , 1:APB clock */
#define APB_CLK		(1<<3)
/* Channel mode , bit 0~2 */
#define TMR_32		1
#define TMR_16		2
#define TMR_8		3
#define PWM		4

#define CH_REL(n)	(0x24+0x10*n)
#define CH_CNT(n)	(0x28+0x10*n)

struct atctmr_timer_regs {
	u32	id_rev;		/* 0x00 */
	u32	reservd[3];	/* 0x04 ~ 0x0c */
	u32	cfg;		/* 0x10 */
	u32	int_en;		/* 0x14 */
	u32	int_st;		/* 0x18 */
	u32	ch_en;		/* 0x1c */
	u32	ch0_ctrl;	/* 0x20 */
	u32	ch0_reload;	/* 0x24 */
	u32	ch0_cntr;	/* 0x28 */
	u32	reservd1;	/* 0x2c */
	u32	ch1_ctrl;	/* 0x30 */
	u32	ch1_reload;	/* 0x34 */
	u32	int_mask;	/* 0x38 */
};

struct atftmr_timer_platdata {
	unsigned long *regs;
};

static int atftmr_timer_get_count(struct udevice *dev, u64 *count)
{
	struct atftmr_timer_platdata *plat = dev->platdata;
	u32 val;
	val = ~(REG32_TMR(CH_CNT(1))+0xffffffff);
	*count = timer_conv_64(val);
	return 0;
}

static int atctmr_timer_probe(struct udevice *dev)
{
	struct atftmr_timer_platdata *plat = dev->platdata;
	REG32_TMR(CH_REL(1)) = 0xffffffff;
	REG32_TMR(CH_CTL(1)) = APB_CLK|TMR_32;
	REG32_TMR(CH_EN) |= CH_TMR_EN(1 , 0);
	return 0;
}

static int atctme_timer_ofdata_to_platdata(struct udevice *dev)
{
	struct atftmr_timer_platdata *plat = dev_get_platdata(dev);
	plat->regs = map_physmem(devfdt_get_addr(dev) , 0x100 , MAP_NOCACHE);
	return 0;
}

static const struct timer_ops ag101p_timer_ops = {
	.get_count = atftmr_timer_get_count,
};

static const struct udevice_id ag101p_timer_ids[] = {
	{ .compatible = "andestech,atcpit100" },
	{}
};

U_BOOT_DRIVER(altera_timer) = {
	.name	= "ae3xx_timer",
	.id	= UCLASS_TIMER,
	.of_match = ag101p_timer_ids,
	.ofdata_to_platdata = atctme_timer_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct atftmr_timer_platdata),
	.probe = atctmr_timer_probe,
	.ops	= &ag101p_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
