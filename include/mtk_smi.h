/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MediaTek SMI local arbiter (larb) API
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#ifndef _MTK_SMI_H
#define _MTK_SMI_H

struct udevice;

/**
 * mtk_smi_larb_enable() - ungate an SMI local arbiter
 * @dev: larb device
 *
 * Enable the clocks of the smi_common block behind the larb, if any, then
 * the larb ones, so that the DMA master in front of it can reach DRAM. On
 * the SoCs where the secure world owns the port mmu_en bits, also ask the
 * secure monitor to keep every port of the larb out of the IOMMU. Calls
 * are refcounted, so several masters can share one larb.
 *
 * Return: 0 on success, or a negative error code
 */
int mtk_smi_larb_enable(struct udevice *dev);

/**
 * mtk_smi_larb_disable() - gate an SMI local arbiter
 * @dev: larb device
 *
 * Drop one reference taken by mtk_smi_larb_enable(), and disable the larb
 * clocks, then the smi_common ones, when it was the last one.
 *
 * Return: 0 on success, -EINVAL if the larb is not enabled, or another
 * negative error code
 */
int mtk_smi_larb_disable(struct udevice *dev);

/**
 * mtk_smi_larb_port_set_pa() - switch a larb port to physical addressing
 * @dev: larb device
 * @port: M4U port index within the larb
 *
 * Take the port out of the IOMMU, so that it issues physical addresses (PA).
 * Without an IOMMU driver the DMA masters address DRAM directly, so their
 * ports must be taken out of the translated address space.
 */
void mtk_smi_larb_port_set_pa(struct udevice *dev, unsigned int port);

#endif /* _MTK_SMI_H */
