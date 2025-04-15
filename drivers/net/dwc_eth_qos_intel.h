/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2023-2024 DENX Software Engineering GmbH
 * Philip Oberfichtner <pro@denx.de>
 *
 * This header is based on linux v6.6.39,
 *
 *	drivers/net/pcs/pcs-xpcs.h
 *	drivers/net/ethernet/stmicro/stmmac/dwmac-intel.h,
 *
 *      Copyright (c) 2020 Synopsys, Inc. and/or its affiliates
 *      Copyright (c) 2020 Intel Corporation
 */

#ifndef __DWMAC_INTEL_H__
#define __DWMAC_INTEL_H__

#define POLL_DELAY_US 8

/* SERDES Register */
#define SERDES_GCR	0x0	/* Global Conguration */
#define SERDES_GSR0	0x5	/* Global Status Reg0 */
#define SERDES_GCR0	0xb	/* Global Configuration Reg0 */

/* SERDES defines */
#define SERDES_PLL_CLK		BIT(0)		/* PLL clk valid signal */
#define SERDES_PHY_RX_CLK	BIT(1)		/* PSE SGMII PHY rx clk */
#define SERDES_RST		BIT(2)		/* Serdes Reset */
#define SERDES_PWR_ST_MASK	GENMASK(6, 4)	/* Serdes Power state*/
#define SERDES_RATE_MASK	GENMASK(9, 8)
#define SERDES_PCLK_MASK	GENMASK(14, 12)	/* PCLK rate to PHY */
#define SERDES_LINK_MODE_MASK	GENMASK(2, 1)
#define SERDES_PWR_ST_SHIFT	4
#define SERDES_PWR_ST_P0	0x0
#define SERDES_PWR_ST_P3	0x3
#define SERDES_LINK_MODE_2G5	0x3
#define SERSED_LINK_MODE_1G	0x2
#define SERDES_PCLK_37p5MHZ	0x0
#define SERDES_PCLK_70MHZ	0x1
#define SERDES_RATE_PCIE_GEN1	0x0
#define SERDES_RATE_PCIE_GEN2	0x1
#define SERDES_RATE_PCIE_SHIFT	8
#define SERDES_PCLK_SHIFT	12

#define INTEL_MGBE_ADHOC_ADDR	0x15
#define INTEL_MGBE_XPCS_ADDR	0x16

/* XPCS defines */
#define XPCS_MODE_SGMII		BIT(2)
#define XPCS_MAC_AUTO_SW	BIT(9)
#define XPCS_AN_CL37_EN		BIT(12)

#define VR_MII_MMD_CTRL		0x0000
#define VR_MII_DIG_CTRL1	0x8000
#define VR_MII_AN_CTRL		0x8001

#endif /* __DWMAC_INTEL_H__ */
