// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021, Xilinx, Inc.
 */

#define LOG_CATEGORY UCLASS_RTC

#include <clk.h>
#include <dm.h>
#include <rtc.h>
#include <asm/io.h>
#include <dm/device_compat.h>

/* RTC Registers */
#define RTC_SET_TM_WR		0x00
#define RTC_SET_TM_RD		0x04
#define RTC_CALIB_WR		0x08
#define RTC_CALIB_RD		0x0C
#define RTC_CUR_TM		0x10
#define RTC_INT_STS		0x20
#define RTC_CTRL		0x40

#define RTC_INT_SEC		BIT(0)
#define RTC_BATT_EN		BIT(31)
#define RTC_CALIB_DEF		0x7FFF
#define RTC_FREQ_MAX		0x10000
#define RTC_CALIB_MASK		0x1FFFFF

struct zynqmp_rtc_priv {
	fdt_addr_t	base;
	unsigned long	calibval;
};

static int zynqmp_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	struct zynqmp_rtc_priv *priv = dev_get_priv(dev);
	u32 status;
	unsigned long read_time;

	status = readl(priv->base + RTC_INT_STS);

	if (status & RTC_INT_SEC) {
		/*
		 * RTC has updated the CURRENT_TIME with the time written into
		 * SET_TIME_WRITE register.
		 */
		read_time = readl(priv->base + RTC_CUR_TM);
	} else {
		/*
		 * Time written in SET_TIME_WRITE has not yet updated into
		 * the seconds read register, so read the time from the
		 * SET_TIME_WRITE instead of CURRENT_TIME register.
		 * Since we add +1 sec while writing, we need to -1 sec while
		 * reading.
		 */
		read_time = readl(priv->base + RTC_SET_TM_RD) - 1;
	}

	rtc_to_tm(read_time, tm);

	return 0;
}

static int zynqmp_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	struct zynqmp_rtc_priv *priv = dev_get_priv(dev);
	unsigned long new_time = 0;

	if (tm)
		/*
		 * The value written will be updated after 1 sec into the
		 * seconds read register, so we need to program time +1 sec
		 * to get the correct time on read.
		 */
		new_time = rtc_mktime(tm) + 1;

	writel(new_time, priv->base + RTC_SET_TM_WR);

	/*
	 * Clear the rtc interrupt status register after setting the
	 * time. During a read_time function, the code should read the
	 * RTC_INT_STATUS register and if bit 0 is still 0, it means
	 * that one second has not elapsed yet since RTC was set and
	 * the current time should be read from SET_TIME_READ register;
	 * otherwise, CURRENT_TIME register is read to report the time
	 */
	writel(RTC_INT_SEC, priv->base + RTC_INT_STS);

	return 0;
}

static int zynqmp_rtc_reset(struct udevice *dev)
{
	return zynqmp_rtc_set(dev, NULL);
}

static int zynqmp_rtc_init(struct udevice *dev)
{
	struct zynqmp_rtc_priv *priv = dev_get_priv(dev);
	u32 rtc_ctrl;

	/* Enable RTC switch to battery when VCC_PSAUX is not available */
	rtc_ctrl = readl(priv->base + RTC_CTRL);
	rtc_ctrl |= RTC_BATT_EN;
	writel(rtc_ctrl, priv->base + RTC_CTRL);

	return 0;
}

static int zynqmp_rtc_probe(struct udevice *dev)
{
	struct zynqmp_rtc_priv *priv = dev_get_priv(dev);
	int ret;

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = readl(priv->base + RTC_CALIB_RD);
	if (!ret) {
		struct clk rtc_clk;
		unsigned long clk_rate;

		/* Get the RTC clock rate */
		ret = clk_get_by_name_optional(dev, "rtc", &rtc_clk);
		if (!ret) {
			clk_rate = clk_get_rate(&rtc_clk);
			/* Use clock frequency if valid, fallback to calibration value */
			if (clk_rate > 0 && clk_rate <= RTC_FREQ_MAX) {
				/* Valid clock frequency */
				priv->calibval = clk_rate - 1;
			} else if (clk_rate == 0) {
				priv->calibval = dev_read_u32_default(dev, "calibration",
								      RTC_CALIB_DEF);
			} else {
				dev_err(dev, "Invalid clock frequency 0x%lx\n",
					clk_rate);
				return -EINVAL;
			}
		} else {
			/* Clock framework unavailable, use DT calibration */
			priv->calibval = dev_read_u32_default(dev, "calibration",
							      RTC_CALIB_DEF);
		}

		/* Validate final calibration value */
		if (priv->calibval > RTC_FREQ_MAX) {
			dev_err(dev, "Invalid calibration 0x%lx\n",
				priv->calibval);
			return -EINVAL;
		}

		writel(priv->calibval, (priv->base + RTC_CALIB_WR));
	} else {
		priv->calibval = ret & RTC_CALIB_MASK;
	}

	ret = zynqmp_rtc_init(dev);

	return ret;
}

static const struct rtc_ops zynqmp_rtc_ops = {
	.get = zynqmp_rtc_get,
	.set = zynqmp_rtc_set,
	.reset = zynqmp_rtc_reset,
};

static const struct udevice_id zynqmp_rtc_ids[] = {
	{ .compatible = "xlnx,zynqmp-rtc" },
	{ }
};

U_BOOT_DRIVER(rtc_zynqmp) = {
	.name = "rtc-zynqmp",
	.id = UCLASS_RTC,
	.probe = zynqmp_rtc_probe,
	.of_match = zynqmp_rtc_ids,
	.ops = &zynqmp_rtc_ops,
	.priv_auto = sizeof(struct zynqmp_rtc_priv),
};
