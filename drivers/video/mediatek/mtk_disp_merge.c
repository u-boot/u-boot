// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video Disp Merge Support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <dm.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>

#include "mtk_disp_comp.h"
#include "mtk_disp_merge.h"

#define DISP_REG_MERGE_CTRL		0x000
#define MERGE_EN				1
#define DISP_REG_MERGE_CFG_0		0x010
#define DISP_REG_MERGE_CFG_1		0x014
#define DISP_REG_MERGE_CFG_4		0x020
#define DISP_REG_MERGE_CFG_10		0x038
#define SWAP_MODE				0
#define FLD_SWAP_MODE				GENMASK(4, 0)
#define DISP_REG_MERGE_CFG_12		0x040
#define CFG_10_10_1PI_2PO_BUF_MODE		6
#define CFG_10_10_2PI_2PO_BUF_MODE		8
#define CFG_11_10_1PI_2PO_MERGE			18
#define FLD_CFG_MERGE_MODE			GENMASK(4, 0)
#define DISP_REG_MERGE_CFG_24		0x070
#define DISP_REG_MERGE_CFG_25		0x074
#define DISP_REG_MERGE_CFG_26		0x078
#define DISP_REG_MERGE_CFG_27		0x07c
#define DISP_REG_MERGE_CFG_36		0x0a0
#define ULTRA_EN				BIT(0)
#define PREULTRA_EN				BIT(4)
#define DISP_REG_MERGE_CFG_37		0x0a4
/* BUFFER_MODE: 0 - no fifo, 1 - SRAM0, 2 - SRAM1, 3 - SRAM0 and SRAM1 */
#define BUFFER_MODE				3
#define FLD_BUFFER_MODE				GENMASK(1, 0)
/*
 * ultra threshold in units of pixels, the recommended values are
 * 6 us and 8 us worth of pixels at 594 Mpixel/s.
 */
#define ULTRA_TH_LOW				(6 * 594)
#define ULTRA_TH_HIGH				(8 * 594)
#define DISP_REG_MERGE_CFG_40		0x0b0
#define FLD_ULTRA_TH_LOW			GENMASK(15, 0)
#define FLD_ULTRA_TH_HIGH			GENMASK(31, 16)
#define PREULTRA_TH_LOW				(8 * 594)
#define PREULTRA_TH_HIGH			(9 * 594)
#define DISP_REG_MERGE_CFG_41		0x0b4
#define FLD_PREULTRA_TH_LOW			GENMASK(15, 0)
#define FLD_PREULTRA_TH_HIGH			GENMASK(31, 16)
#define DISP_REG_MERGE_MUTE_0		0xf00

/* CFG_30 (0x088, "12 bit lsb off") is not present in the upstream driver */
#define DISP_REG_MERGE_CFG_30		0x088

/*
 * The DRAM latency thresholds that keep the merge from underrunning while it
 * buffers a line through its SRAMs.
 */
static void mtk_disp_merge_fifo_setting(struct udevice *dev)
{
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_36,
			    ULTRA_EN | PREULTRA_EN);
	mtk_disp_comp_update(dev, DISP_REG_MERGE_CFG_37,
			     FIELD_PREP(FLD_BUFFER_MODE, BUFFER_MODE),
			     FLD_BUFFER_MODE);
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_40,
			    FIELD_PREP(FLD_ULTRA_TH_LOW, ULTRA_TH_LOW) |
			    FIELD_PREP(FLD_ULTRA_TH_HIGH, ULTRA_TH_HIGH));
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_41,
			    FIELD_PREP(FLD_PREULTRA_TH_LOW, PREULTRA_TH_LOW) |
			    FIELD_PREP(FLD_PREULTRA_TH_HIGH,
				       PREULTRA_TH_HIGH));
}

void mtk_disp_merge_config(struct udevice *dev, u16 l_w, u16 r_w, u16 h)
{
	struct mtk_disp_merge_priv *priv = dev_get_priv(dev);
	u32 mode = CFG_10_10_1PI_2PO_BUF_MODE;

	if (priv->fifo_en) {
		mtk_disp_merge_fifo_setting(dev);
		mode = CFG_10_10_2PI_2PO_BUF_MODE;
	}

	if (r_w)
		mode = CFG_11_10_1PI_2PO_MERGE;

	/* input 1 and input 2 sizes */
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_0, h << 16 | l_w);
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_1, h << 16 | r_w);

	/* output size */
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_4, h << 16 | (l_w + r_w));

	/* no pixel/channel swap */
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_10,
			    FIELD_PREP(FLD_SWAP_MODE, SWAP_MODE));

	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_12,
			    FIELD_PREP(FLD_CFG_MERGE_MODE, mode));

	/*
	 * CFG_24 is the merge SRAM0 size and CFG_25 the SRAM1 size. In merge
	 * mode input 1 goes to SRAM0 and input 2 to SRAM1; in buffer mode the
	 * single input goes through SRAM0 and then SRAM1, so both hold it.
	 */
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_24, h << 16 | l_w);
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_25,
			    h << 16 | (r_w ? r_w : l_w));

	/* merged sizes of inputs 1 and 2, only used in merge mode */
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_26, h << 16 | l_w);
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_27, h << 16 | r_w);

	/* 12 bit lsb off */
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CFG_30, 0x0);

	if (priv->mute_support)
		mtk_disp_comp_write(dev, DISP_REG_MERGE_MUTE_0, 0x0);

	/* enable */
	mtk_disp_comp_write(dev, DISP_REG_MERGE_CTRL, MERGE_EN);
}

static int mtk_disp_merge_probe(struct udevice *dev)
{
	struct mtk_disp_merge_priv *priv = dev_get_priv(dev);
	int ret;

	ret = mtk_disp_comp_probe(dev);
	if (ret)
		return ret;

	priv->fifo_en = dev_read_bool(dev, "mediatek,merge-fifo-en");
	priv->mute_support = dev_read_bool(dev, "mediatek,merge-mute");

	return 0;
}

static const struct udevice_id mtk_disp_merge_ids[] = {
	{ .compatible = "mediatek,mt8195-disp-merge" },
	{ }
};

U_BOOT_DRIVER(mtk_disp_merge) = {
	.name	   = "mtk_disp_merge",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_merge_ids,
	.probe	   = mtk_disp_merge_probe,
	.priv_auto = sizeof(struct mtk_disp_merge_priv),
};
