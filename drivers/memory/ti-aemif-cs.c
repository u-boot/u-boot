// SPDX-License-Identifier: GPL-2.0+
/*
 * DaVinci / Keystone2 : AEMIF's chip select configuration
 *
 */
#include <asm/io.h>
#include <clk.h>
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

static const struct udevice_id aemif_cs_ids[] = {
	{ .compatible = "ti,da850-aemif-cs", },
	{},
};

U_BOOT_DRIVER(ti_aemif_cs) = {
	.name = "ti_aemif_cs",
	.id = UCLASS_MEMORY,
	.of_match = aemif_cs_ids,
};
