/*
 * watchdog.c - driver for i.mx on-chip watchdog
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <wdt.h>
#include <watchdog.h>
#include <asm/arch/imx-regs.h>
#ifdef CONFIG_FSL_LSCH2
#include <asm/arch/immap_lsch2.h>
#endif
#include <fsl_wdog.h>

static void imx_watchdog_expire_now(struct watchdog_regs *wdog)
{
	clrsetbits_le16(&wdog->wcr, WCR_WT_MSK, WCR_WDE);

	writew(0x5555, &wdog->wsr);
	writew(0xaaaa, &wdog->wsr);	/* load minimum 1/2 second timeout */
	while (1) {
		/*
		 * spin for .5 seconds before reset
		 */
	}
}

#if !defined(CONFIG_IMX_WATCHDOG) || \
    (defined(CONFIG_IMX_WATCHDOG) && !CONFIG_IS_ENABLED(WDT))
void __attribute__((weak)) reset_cpu(ulong addr)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	imx_watchdog_expire_now(wdog);
}
#endif

#if defined(CONFIG_IMX_WATCHDOG)
static void imx_watchdog_reset(struct watchdog_regs *wdog)
{
#ifndef CONFIG_WATCHDOG_RESET_DISABLE
	writew(0x5555, &wdog->wsr);
	writew(0xaaaa, &wdog->wsr);
#endif /* CONFIG_WATCHDOG_RESET_DISABLE*/
}

static void imx_watchdog_init(struct watchdog_regs *wdog)
{
	u16 timeout;

	/*
	 * The timer watchdog can be set between
	 * 0.5 and 128 Seconds. If not defined
	 * in configuration file, sets 128 Seconds
	 */
#ifndef CONFIG_WATCHDOG_TIMEOUT_MSECS
#define CONFIG_WATCHDOG_TIMEOUT_MSECS 128000
#endif
	timeout = (CONFIG_WATCHDOG_TIMEOUT_MSECS / 500) - 1;
#ifdef CONFIG_FSL_LSCH2
	writew((WCR_WDA | WCR_SRS | WCR_WDE) << 8 | timeout, &wdog->wcr);
#else
	writew(WCR_WDZST | WCR_WDBG | WCR_WDE | WCR_WDT | WCR_SRS |
		WCR_WDA | SET_WCR_WT(timeout), &wdog->wcr);
#endif /* CONFIG_FSL_LSCH2*/
	imx_watchdog_reset(wdog);
}

#if !CONFIG_IS_ENABLED(WDT)
void hw_watchdog_reset(void)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	imx_watchdog_reset(wdog);
}

void hw_watchdog_init(void)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	imx_watchdog_init(wdog);
}
#else
struct imx_wdt_priv {
	void __iomem *base;
};

static int imx_wdt_reset(struct udevice *dev)
{
	struct imx_wdt_priv *priv = dev_get_priv(dev);

	imx_watchdog_reset(priv->base);

	return 0;
}

static int imx_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct imx_wdt_priv *priv = dev_get_priv(dev);

	imx_watchdog_expire_now(priv->base);
	hang();

	return 0;
}

static int imx_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct imx_wdt_priv *priv = dev_get_priv(dev);

	imx_watchdog_init(priv->base);

	return 0;
}

static int imx_wdt_probe(struct udevice *dev)
{
	struct imx_wdt_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOENT;

	return 0;
}

static const struct wdt_ops imx_wdt_ops = {
	.start		= imx_wdt_start,
	.reset		= imx_wdt_reset,
	.expire_now	= imx_wdt_expire_now,
};

static const struct udevice_id imx_wdt_ids[] = {
	{ .compatible = "fsl,imx21-wdt" },
	{}
};

U_BOOT_DRIVER(imx_wdt) = {
	.name		= "imx_wdt",
	.id		= UCLASS_WDT,
	.of_match	= imx_wdt_ids,
	.probe		= imx_wdt_probe,
	.ops		= &imx_wdt_ops,
	.priv_auto_alloc_size = sizeof(struct imx_wdt_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};
#endif
#endif
