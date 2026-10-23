// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Display Data Path
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#include <asm/io.h>
#include <linux/bitops.h>

#include "mtk_ddp.h"

/*
 * mt8365-generation mmsys: the path is built out of direct muxes, one
 * register per input or output. The offsets and the values are the ones of
 * the Linux mt8365_mmsys_routing_table.
 */
#define MT8365_DISP_MS_IN_OUT_MASK			GENMASK(3, 0)

#define MT8365_DISP_REG_CONFIG_DISP_OVL0_MOUT_EN	0xf3c
#define MT8365_OVL0_MOUT_PATH0_SEL				BIT(0)

#define MT8365_DISP_REG_CONFIG_DISP_RDMA0_SOUT_SEL	0xf4c
#define MT8365_RDMA0_SOUT_COLOR0				0x1

#define MT8365_DISP_REG_CONFIG_DISP_DITHER0_MOUT_EN	0xf50
#define MT8365_DITHER_MOUT_EN_DSI0				BIT(0)

#define MT8365_DISP_REG_CONFIG_DISP_RDMA0_SEL_IN	0xf54
#define MT8365_RDMA0_SEL_IN_OVL0				0x0

#define MT8365_DISP_REG_CONFIG_DISP_RDMA0_RSZ0_SEL_IN	0xf60
#define MT8365_RDMA0_RSZ0_SEL_IN_RDMA0				0x0

#define MT8365_DISP_REG_CONFIG_DISP_COLOR0_SEL_IN	0xf64
#define MT8365_DISP_COLOR_SEL_IN_COLOR0				0x0

#define MT8365_DISP_REG_CONFIG_DISP_DSI0_SEL_IN		0xf68
#define MT8365_DSI0_SEL_IN_DITHER				0x1

/*
 * mt8366-generation mmsys uses a multi-stage crossbar instead of the simple
 * direct muxes above.
 */
#define MT8366_DISP_OVL0_BGCLR_MOUT_EN			0xe24
#define MT8366_DISP_OVL_BGCLR_MOUT_MASK				GENMASK(1, 0)
#define MT8366_MOUT_OVL_TO_BLENDOUT				BIT(0)

#define MT8366_DISP_OVL0_OUT0_MOUT_EN			0xc10
#define MT8366_DISP_OVL_OUT0_MOUT_MASK				GENMASK(2, 0)
#define MT8366_MOUT_DISP_OVL0_TO_DISP_RDMA0			BIT(1)

#define MT8366_DISP_RDMA0_SEL_IN			0xe04
#define MT8366_DISP_RDMA_SEL_IN_MASK				BIT(0)
#define MT8366_SEL_IN_DISP_RDMA0_FROM_DISP_OVL0_OUT0_MOUT	0x1

#define MT8366_DISP_RDMA0_RSZ0_SOUT_SEL			0xe00
#define MT8366_SOUT_DISP_RDMA0_RSZ0_TO_OVL_PQ_OUT_CROSSBAR1	0x0

#define MT8366_OVL_PQ_OUT_CROSSBAR1_MOUT_EN		0xc78
#define MT8366_OVL_PQ_OUT_CROSSBAR_MOUT_MASK			GENMASK(5, 0)
#define MT8366_MOUT_OVL_PQ_OUT_CROSSBAR1_TO_COMP_OUT_CROSSBAR4	BIT(4)

#define MT8366_COMP_OUT_CROSSBAR4_MOUT_EN		0xd80
#define MT8366_COMP_OUT_CROSSBAR_MOUT_MASK			GENMASK(5, 0)
#define MT8366_MOUT_COMP_OUT_CROSSBAR4_TO_DISP_DSI0		BIT(0)
#define MT8366_MOUT_COMP_OUT_CROSSBAR4_TO_DISP_DVO0		BIT(1)

static void mtk_ddp_mask(void __iomem *base, u32 offset, u32 val, u32 mask)
{
	u32 tmp = readl(base + offset) & ~mask;

	writel(tmp | (val & mask), base + offset);
}

void mt8365_ddp_ovl_to_dsi(void __iomem *mmsys_base)
{
	/* OVL0 -> RDMA0 */
	mtk_ddp_mask(mmsys_base, MT8365_DISP_REG_CONFIG_DISP_OVL0_MOUT_EN,
		     MT8365_OVL0_MOUT_PATH0_SEL, MT8365_DISP_MS_IN_OUT_MASK);
	mtk_ddp_mask(mmsys_base, MT8365_DISP_REG_CONFIG_DISP_RDMA0_SEL_IN,
		     MT8365_RDMA0_SEL_IN_OVL0, MT8365_DISP_MS_IN_OUT_MASK);

	/* RDMA0 -> COLOR0 */
	mtk_ddp_mask(mmsys_base, MT8365_DISP_REG_CONFIG_DISP_RDMA0_RSZ0_SEL_IN,
		     MT8365_RDMA0_RSZ0_SEL_IN_RDMA0, MT8365_DISP_MS_IN_OUT_MASK);
	mtk_ddp_mask(mmsys_base, MT8365_DISP_REG_CONFIG_DISP_RDMA0_SOUT_SEL,
		     MT8365_RDMA0_SOUT_COLOR0, MT8365_DISP_MS_IN_OUT_MASK);

	/* COLOR0 -> CCORR0 -> AAL0 -> GAMMA0 -> DITHER0 -> DSI0 */
	mtk_ddp_mask(mmsys_base, MT8365_DISP_REG_CONFIG_DISP_COLOR0_SEL_IN,
		     MT8365_DISP_COLOR_SEL_IN_COLOR0, MT8365_DISP_MS_IN_OUT_MASK);
	mtk_ddp_mask(mmsys_base, MT8365_DISP_REG_CONFIG_DISP_DITHER0_MOUT_EN,
		     MT8365_DITHER_MOUT_EN_DSI0, MT8365_DISP_MS_IN_OUT_MASK);
	mtk_ddp_mask(mmsys_base, MT8365_DISP_REG_CONFIG_DISP_DSI0_SEL_IN,
		     MT8365_DSI0_SEL_IN_DITHER, MT8365_DISP_MS_IN_OUT_MASK);
}

/*
 * OVL0 -> RDMA0 -> COMP_OUT_CROSSBAR4 -> @sink, where @sink is one of the
 * MT8366_MOUT_COMP_OUT_CROSSBAR4_TO_* bits.
 */
static void mt8366_ddp_ovl_to_crossbar4(void __iomem *mmsys_base, u32 sink)
{
	/* OVL0 -> RDMA0 */
	mtk_ddp_mask(mmsys_base, MT8366_DISP_OVL0_BGCLR_MOUT_EN,
		     MT8366_MOUT_OVL_TO_BLENDOUT, MT8366_DISP_OVL_BGCLR_MOUT_MASK);
	mtk_ddp_mask(mmsys_base, MT8366_DISP_OVL0_OUT0_MOUT_EN,
		     MT8366_MOUT_DISP_OVL0_TO_DISP_RDMA0,
		     MT8366_DISP_OVL_OUT0_MOUT_MASK);
	mtk_ddp_mask(mmsys_base, MT8366_DISP_RDMA0_SEL_IN,
		     MT8366_SEL_IN_DISP_RDMA0_FROM_DISP_OVL0_OUT0_MOUT,
		     MT8366_DISP_RDMA_SEL_IN_MASK);

	/* RDMA0 -> COMP_OUT_CROSSBAR4 -> sink */
	mtk_ddp_mask(mmsys_base, MT8366_DISP_RDMA0_RSZ0_SOUT_SEL,
		     MT8366_SOUT_DISP_RDMA0_RSZ0_TO_OVL_PQ_OUT_CROSSBAR1,
		     MT8366_OVL_PQ_OUT_CROSSBAR_MOUT_MASK);
	mtk_ddp_mask(mmsys_base, MT8366_OVL_PQ_OUT_CROSSBAR1_MOUT_EN,
		     MT8366_MOUT_OVL_PQ_OUT_CROSSBAR1_TO_COMP_OUT_CROSSBAR4,
		     MT8366_OVL_PQ_OUT_CROSSBAR_MOUT_MASK);
	mtk_ddp_mask(mmsys_base, MT8366_COMP_OUT_CROSSBAR4_MOUT_EN, sink,
		     MT8366_COMP_OUT_CROSSBAR_MOUT_MASK);
}

void mt8366_ddp_ovl_to_dsi(void __iomem *mmsys_base)
{
	mt8366_ddp_ovl_to_crossbar4(mmsys_base,
				    MT8366_MOUT_COMP_OUT_CROSSBAR4_TO_DISP_DSI0);
}

void mt8366_ddp_ovl_to_dvo(void __iomem *mmsys_base)
{
	mt8366_ddp_ovl_to_crossbar4(mmsys_base,
				    MT8366_MOUT_COMP_OUT_CROSSBAR4_TO_DISP_DVO0);
}
