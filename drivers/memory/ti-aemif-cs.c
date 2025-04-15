// SPDX-License-Identifier: GPL-2.0+
/*
 * DaVinci / Keystone2 : AEMIF's chip select configuration
 *
 */
#include <asm/io.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include "ti-aemif-cs.h"

#define AEMIF_CONFIG(cs)		(0x10 + ((cs) * 4))

#define AEMIF_CFG_SELECT_STROBE(v)	((v) ? 1 << 31 : 0)
#define AEMIF_CFG_EXTEND_WAIT(v)	((v) ? 1 << 30 : 0)
#define AEMIF_CFG_WR_SETUP(v)		(((v) & 0x0f) << 26)
#define AEMIF_CFG_WR_STROBE(v)		(((v) & 0x3f) << 20)
#define AEMIF_CFG_WR_HOLD(v)		(((v) & 0x07) << 17)
#define AEMIF_CFG_RD_SETUP(v)		(((v) & 0x0f) << 13)
#define AEMIF_CFG_RD_STROBE(v)		(((v) & 0x3f) << 7)
#define AEMIF_CFG_RD_HOLD(v)		(((v) & 0x07) << 4)
#define AEMIF_CFG_TURN_AROUND(v)	(((v) & 0x03) << 2)
#define AEMIF_CFG_WIDTH(v)		(((v) & 0x03) << 0)

#define SSTROBE_EN			0x1
#define EW_EN				0x1

#define WSETUP_MAX			0xf
#define WSTROBE_MAX			0x3f
#define WHOLD_MAX			0x7
#define RSETUP_MAX			0xf
#define RSTROBE_MAX			0x3f
#define RHOLD_MAX			0x7
#define TA_MAX				0x3

#define WIDTH_8BITS			0x0
#define WIDTH_16BITS			0x1

#define set_config_field(reg, field, val)				\
	do {								\
		if ((val) != -1) {					\
			(reg) &= ~AEMIF_CFG_##field(0xffffffff);	\
			(reg) |=	AEMIF_CFG_##field((val));	\
		}							\
	} while (0)

void aemif_cs_configure(int cs, struct aemif_config *cfg)
{
	unsigned long tmp;

	tmp = __raw_readl(cfg->base + AEMIF_CONFIG(cs));

	set_config_field(tmp, SELECT_STROBE,	cfg->select_strobe);
	set_config_field(tmp, EXTEND_WAIT,	cfg->extend_wait);
	set_config_field(tmp, WR_SETUP,		cfg->wr_setup);
	set_config_field(tmp, WR_STROBE,	cfg->wr_strobe);
	set_config_field(tmp, WR_HOLD,		cfg->wr_hold);
	set_config_field(tmp, RD_SETUP,		cfg->rd_setup);
	set_config_field(tmp, RD_STROBE,	cfg->rd_strobe);
	set_config_field(tmp, RD_HOLD,		cfg->rd_hold);
	set_config_field(tmp, TURN_AROUND,	cfg->turn_around);
	set_config_field(tmp, WIDTH,		cfg->width);

	__raw_writel(tmp, cfg->base + AEMIF_CONFIG(cs));
}

struct ti_aemif_cs {
	void __iomem *base;
	struct clk *clk;
};

static unsigned int aemif_calc_cfg(ulong rate, u64 timing_ns, u32 max_cfg)
{
	u64 result;

	if (!timing_ns)
		return 0;

	result = DIV_ROUND_UP_ULL(timing_ns * rate, 1000000000ULL);

	if (result - 1 > max_cfg)
		return max_cfg;

	return result - 1;
}

static int aemif_cs_set_timings(struct udevice *dev)
{
	struct ti_aemif_cs *priv = dev_get_priv(dev);
	ulong rate = clk_get_rate(priv->clk);
	struct aemif_config cfg = {};
	u32 val;
	u32 cs;

	if (dev_read_u32(dev, "ti,cs-chipselect", &cs))
		return -EINVAL;

/*
 * On DaVinci SoCs, chipselect is in range [2-5]
 * On Keystone SoCs, chipselect is in range [0-3]
 * The logic to access the configuration registers expects the CS to be in the
 * Keystone range so a -2 offset is applied on DaVinci SoCs
 */
	if (IS_ENABLED(CONFIG_ARCH_DAVINCI)) {
		if (cs < 2 || cs > 5)
			return -EINVAL;
		cs -= 2;
	} else if (IS_ENABLED(CONFIG_ARCH_KEYSTONE)) {
		if (cs > 3)
			return -EINVAL;
	}

	if (dev_read_bool(dev, "ti,cs-select-strobe-mode"))
		cfg.select_strobe = SSTROBE_EN;

	if (dev_read_bool(dev, "ti,cs-extended-wait-mode"))
		cfg.extend_wait = EW_EN;

	val = dev_read_u32_default(dev, "ti,cs-write-setup-ns", U32_MAX);
	cfg.wr_setup = aemif_calc_cfg(rate, val, WSETUP_MAX);

	val = dev_read_u32_default(dev, "ti,cs-write-strobe-ns", U32_MAX);
	cfg.wr_strobe = aemif_calc_cfg(rate, val, WSTROBE_MAX);

	val = dev_read_u32_default(dev, "ti,cs-write-hold-ns", U32_MAX);
	cfg.wr_hold = aemif_calc_cfg(rate, val, WHOLD_MAX);

	val = dev_read_u32_default(dev, "ti,cs-read-setup-ns", U32_MAX);
	cfg.rd_setup = aemif_calc_cfg(rate, val, RSETUP_MAX);

	val = dev_read_u32_default(dev, "ti,cs-read-strobe-ns", U32_MAX);
	cfg.rd_strobe = aemif_calc_cfg(rate, val, RSTROBE_MAX);

	val = dev_read_u32_default(dev, "ti,cs-read-hold-ns", U32_MAX);
	cfg.rd_hold = aemif_calc_cfg(rate, val, RHOLD_MAX);

	val = dev_read_u32_default(dev, "ti,cs-min-turnaround-ns", U32_MAX);
	cfg.turn_around = aemif_calc_cfg(rate, val, TA_MAX);

	val = dev_read_u32_default(dev, "ti,cs-bus-width", 8);
	if (val == 16)
		cfg.width = WIDTH_16BITS;
	else
		cfg.width = WIDTH_8BITS;

	cfg.base = priv->base;
	aemif_cs_configure(cs, &cfg);

	return 0;
}

static int aemif_cs_probe(struct udevice *dev)
{
	struct ti_aemif_cs *priv = dev_get_priv(dev);
	struct udevice *aemif;

	aemif = dev_get_parent(dev);
	if (!aemif)
		return -ENODEV;

	priv->base = dev_read_addr_ptr(aemif);
	if (!priv->base)
		return -EINVAL;

	priv->clk = devm_clk_get(aemif, "aemif");
	if (IS_ERR(priv->clk))
		return -EINVAL;

	return aemif_cs_set_timings(dev);
}

static const struct udevice_id aemif_cs_ids[] = {
	{ .compatible = "ti,da850-aemif-cs", },
	{},
};

U_BOOT_DRIVER(ti_aemif_cs) = {
	.name = "ti_aemif_cs",
	.id = UCLASS_MEMORY,
	.of_match = aemif_cs_ids,
	.probe = aemif_cs_probe,
	.priv_auto = sizeof(struct ti_aemif_cs),
};
