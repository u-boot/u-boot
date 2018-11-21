// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell Armada 37xx SoC Watchdog Driver
 *
 * Marek Behun <marek.behun@nic.cz>
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

struct a37xx_wdt {
	void __iomem *sel_reg;
	void __iomem *reg;
	ulong clk_rate;
	u64 timeout;
};

/*
 * We use Counter 1 for watchdog timer, because so does Marvell's Linux by
 * default.
 */

#define CNTR_CTRL			0x10
#define CNTR_CTRL_ENABLE		0x0001
#define CNTR_CTRL_ACTIVE		0x0002
#define CNTR_CTRL_MODE_MASK		0x000c
#define CNTR_CTRL_MODE_ONESHOT		0x0000
#define CNTR_CTRL_PRESCALE_MASK		0xff00
#define CNTR_CTRL_PRESCALE_MIN		2
#define CNTR_CTRL_PRESCALE_SHIFT	8

#define CNTR_COUNT_LOW			0x14
#define CNTR_COUNT_HIGH			0x18

static void set_counter_value(struct a37xx_wdt *priv)
{
	writel(priv->timeout & 0xffffffff, priv->reg + CNTR_COUNT_LOW);
	writel(priv->timeout >> 32, priv->reg + CNTR_COUNT_HIGH);
}

static void a37xx_wdt_enable(struct a37xx_wdt *priv)
{
	u32 reg = readl(priv->reg + CNTR_CTRL);

	reg |= CNTR_CTRL_ENABLE;
	writel(reg, priv->reg + CNTR_CTRL);
}

static void a37xx_wdt_disable(struct a37xx_wdt *priv)
{
	u32 reg = readl(priv->reg + CNTR_CTRL);

	reg &= ~CNTR_CTRL_ENABLE;
	writel(reg, priv->reg + CNTR_CTRL);
}

static int a37xx_wdt_reset(struct udevice *dev)
{
	struct a37xx_wdt *priv = dev_get_priv(dev);

	if (!priv->timeout)
		return -EINVAL;

	a37xx_wdt_disable(priv);
	set_counter_value(priv);
	a37xx_wdt_enable(priv);

	return 0;
}

static int a37xx_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct a37xx_wdt *priv = dev_get_priv(dev);

	a37xx_wdt_disable(priv);
	priv->timeout = 0;
	set_counter_value(priv);
	a37xx_wdt_enable(priv);

	return 0;
}

static int a37xx_wdt_start(struct udevice *dev, u64 ms, ulong flags)
{
	struct a37xx_wdt *priv = dev_get_priv(dev);
	u32 reg;

	reg = readl(priv->reg + CNTR_CTRL);

	if (reg & CNTR_CTRL_ACTIVE)
		return -EBUSY;

	/* set mode */
	reg = (reg & ~CNTR_CTRL_MODE_MASK) | CNTR_CTRL_MODE_ONESHOT;

	/* set prescaler to the min value */
	reg &= ~CNTR_CTRL_PRESCALE_MASK;
	reg |= CNTR_CTRL_PRESCALE_MIN << CNTR_CTRL_PRESCALE_SHIFT;

	priv->timeout = ms * priv->clk_rate / 1000 / CNTR_CTRL_PRESCALE_MIN;

	writel(reg, priv->reg + CNTR_CTRL);

	set_counter_value(priv);
	a37xx_wdt_enable(priv);

	return 0;
}

static int a37xx_wdt_stop(struct udevice *dev)
{
	struct a37xx_wdt *priv = dev_get_priv(dev);

	a37xx_wdt_disable(priv);

	return 0;
}

static int a37xx_wdt_probe(struct udevice *dev)
{
	struct a37xx_wdt *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr_index(dev, 0);
	if (addr == FDT_ADDR_T_NONE)
		goto err;
	priv->sel_reg = (void __iomem *)addr;

	addr = dev_read_addr_index(dev, 1);
	if (addr == FDT_ADDR_T_NONE)
		goto err;
	priv->reg = (void __iomem *)addr;

	priv->clk_rate = (ulong)get_ref_clk() * 1000000;

	a37xx_wdt_disable(priv);

	/*
	 * We use timer 1 as watchdog timer (because Marvell's Linux uses that
	 * timer as default), therefore we only set bit TIMER1_IS_WCHDOG_TIMER.
	 */
	writel(1 << 1, priv->sel_reg);

	return 0;
err:
	dev_err(dev, "no io address\n");
	return -ENODEV;
}

static const struct wdt_ops a37xx_wdt_ops = {
	.start = a37xx_wdt_start,
	.reset = a37xx_wdt_reset,
	.stop = a37xx_wdt_stop,
	.expire_now = a37xx_wdt_expire_now,
};

static const struct udevice_id a37xx_wdt_ids[] = {
	{ .compatible = "marvell,armada-3700-wdt" },
	{}
};

U_BOOT_DRIVER(a37xx_wdt) = {
	.name = "armada_37xx_wdt",
	.id = UCLASS_WDT,
	.of_match = a37xx_wdt_ids,
	.probe = a37xx_wdt_probe,
	.priv_auto_alloc_size = sizeof(struct a37xx_wdt),
	.ops = &a37xx_wdt_ops,
};
