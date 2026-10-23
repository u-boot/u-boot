/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MediaTek display pipeline component helpers
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_DISP_COMP_H
#define _MTK_DISP_COMP_H

#include <clk.h>
#include <linux/types.h>

struct udevice;

/**
 * struct mtk_disp_comp_priv - common state of a display pipeline component
 * @base: register window of the component
 * @clk_bulk: clocks of the component
 * @larb: optional SMI larb device (UCLASS_MEMORY), enabled and disabled
 *	  together with the component. Only the components sitting behind an
 *	  SMI larb, the overlays and the rdmas, have one.
 * @larb_port: M4U port index of the component within @larb
 */
struct mtk_disp_comp_priv {
	void __iomem *base;
	struct clk_bulk clk_bulk;
	struct udevice *larb;
	unsigned int larb_port;
};

/**
 * mtk_disp_comp_probe() - probe a display pipeline component
 * @dev: component device, whose private data starts with a
 *	 struct mtk_disp_comp_priv
 *
 * Map the first register window of the component and get its clocks.
 *
 * Return: 0 on success, or a negative error code
 */
int mtk_disp_comp_probe(struct udevice *dev);

/**
 * mtk_disp_comp_get_larb() - resolve the SMI larb behind a DMA master
 * @dev: component device
 *
 * A DMA master component reaches DRAM through an SMI larb, which must be
 * ungated for its accesses. The component points at its IOMMU (M4U) port
 * through "iommus": MTK_M4U_TO_LARB() extracts the larb id from that port
 * and MTK_M4U_TO_PORT() the port index within it, kept in larb_port.
 * Resolve the matching larb in the IOMMU's "mediatek,larbs" list by its
 * "mediatek,larb-id", so that mtk_disp_comp_enable() and
 * mtk_disp_comp_disable() then enable and disable it together with the
 * component.
 *
 * Return: 0 on success, or a negative error code
 */
int mtk_disp_comp_get_larb(struct udevice *dev);

/**
 * mtk_disp_comp_enable() - enable a display pipeline component
 * @dev: component device
 *
 * Ungate the SMI larb of the component, if any, then enable its clocks.
 *
 * Return: 0 on success, or a negative error code
 */
int mtk_disp_comp_enable(struct udevice *dev);

/**
 * mtk_disp_comp_disable() - disable a display pipeline component
 * @dev: component device
 *
 * Undo mtk_disp_comp_enable(): disable the clocks of the component, then
 * gate its SMI larb, if any.
 *
 * Return: 0 on success, or a negative error code
 */
int mtk_disp_comp_disable(struct udevice *dev);

/**
 * mtk_disp_comp_write() - write a component register
 * @dev: component device
 * @offset: register offset in the component register window
 * @val: value to write
 */
void mtk_disp_comp_write(struct udevice *dev, u32 offset, u32 val);

/**
 * mtk_disp_comp_update() - update some bits of a component register
 * @dev: component device
 * @offset: register offset in the component register window
 * @val: new value of the bits in @mask
 * @mask: bits to update
 */
void mtk_disp_comp_update(struct udevice *dev, u32 offset, u32 val, u32 mask);

#endif /* _MTK_DISP_COMP_H */
