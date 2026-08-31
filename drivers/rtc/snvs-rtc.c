// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018-2026 INIT GmbH, Alexander Koch <akoch@initse.com>
 */

#include <asm/io.h>
#include <dm.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <malloc.h>
#include <rtc.h>

#define SNVS_LPSRTCMR			0x1c
#define SNVS_LPSRTCLR			0x20
#define SNVS_LPCR			0x04
#define SNVS_LPCR_SRTC_ENV		BIT(0)
#define CNTR_TO_SECS_SH			15

struct snvs_rtc_priv {
	fdt_addr_t	base;
};

static int snvs_rtc_enable(struct udevice *dev)
{
	struct snvs_rtc_priv *priv = dev_get_priv(dev);
	u32 val;

	setbits_le32(priv->base + SNVS_LPCR, SNVS_LPCR_SRTC_ENV);

	return readl_poll_timeout(priv->base + SNVS_LPCR, val,
				  (val & SNVS_LPCR_SRTC_ENV), 1000);
}

static int snvs_rtc_disable(struct udevice *dev)
{
	struct snvs_rtc_priv *priv = dev_get_priv(dev);
	u32 val;

	clrbits_le32(priv->base + SNVS_LPCR, SNVS_LPCR_SRTC_ENV);

	return readl_poll_timeout(priv->base + SNVS_LPCR, val,
				  !(val & SNVS_LPCR_SRTC_ENV), 1000);
}

static int snvs_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	struct snvs_rtc_priv *priv = dev_get_priv(dev);
	u64 read1, read2;

	/* Require two identical consecutive reads as stated in manual */
	do {
		read1 = (u64)readl(priv->base + SNVS_LPSRTCMR) << 32ULL;
		read1 |= readl(priv->base + SNVS_LPSRTCLR);

		read2 = (u64)readl(priv->base + SNVS_LPSRTCMR) << 32ULL;
		read2 |= readl(priv->base + SNVS_LPSRTCLR);
	} while (read1 != read2);

	/* Convert 47-bit counter to 32-bit raw second count */
	rtc_to_tm((time_t)(read1 >> CNTR_TO_SECS_SH), tm);

	return 0;
}

static int snvs_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	struct snvs_rtc_priv *priv = dev_get_priv(dev);
	u32 time = rtc_mktime(tm);
	int ret;

	ret = snvs_rtc_disable(dev);
	if (ret)
		return ret;

	/* Write 32-bit time to 47-bit timer, leaving 15 LSBs blank */
	writel(time << CNTR_TO_SECS_SH, priv->base + SNVS_LPSRTCLR);
	writel(time >> (32 - CNTR_TO_SECS_SH), priv->base + SNVS_LPSRTCMR);

	return snvs_rtc_enable(dev);
}

static const struct rtc_ops snvs_rtc_ops = {
	.get	= snvs_rtc_get,
	.set	= snvs_rtc_set,
};

static int snvs_rtc_probe(struct udevice *dev)
{
	struct snvs_rtc_priv *priv = dev_get_priv(dev);
	u32 offset;
	int ret;

	priv->base = dev_read_addr(dev->parent);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = dev_read_u32(dev, "offset", &offset);
	if (ret)
		return ret;

	priv->base += offset;

	return snvs_rtc_enable(dev);
}

static const struct udevice_id snvs_rtc_ids[] = {
	{ .compatible = "fsl,sec-v4.0-mon-rtc-lp" },
	{ }
};

U_BOOT_DRIVER(rtc_snvs) = {
	.name		= "rtc-snvs",
	.id		= UCLASS_RTC,
	.probe		= snvs_rtc_probe,
	.of_match	= snvs_rtc_ids,
	.ops		= &snvs_rtc_ops,
	.priv_auto	= sizeof(struct snvs_rtc_priv),
};
