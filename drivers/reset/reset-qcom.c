// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Sartura Ltd.
 * Copyright (c) 2022 Linaro Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 *         Sumit Garg <sumit.garg@linaro.org>
 *
 * Based on Linux driver
 */

#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <reset-uclass.h>
#include <linux/bitops.h>
#include <malloc.h>

struct qcom_reset_priv {
	phys_addr_t base;
};

struct qcom_reset_map {
	unsigned int reg;
	u8 bit;
};

#ifdef CONFIG_ARCH_IPQ40XX
#include <dt-bindings/reset/qcom,ipq4019-reset.h>
static const struct qcom_reset_map gcc_qcom_resets[] = {
	[WIFI0_CPU_INIT_RESET] = { 0x1f008, 5 },
	[WIFI0_RADIO_SRIF_RESET] = { 0x1f008, 4 },
	[WIFI0_RADIO_WARM_RESET] = { 0x1f008, 3 },
	[WIFI0_RADIO_COLD_RESET] = { 0x1f008, 2 },
	[WIFI0_CORE_WARM_RESET] = { 0x1f008, 1 },
	[WIFI0_CORE_COLD_RESET] = { 0x1f008, 0 },
	[WIFI1_CPU_INIT_RESET] = { 0x20008, 5 },
	[WIFI1_RADIO_SRIF_RESET] = { 0x20008, 4 },
	[WIFI1_RADIO_WARM_RESET] = { 0x20008, 3 },
	[WIFI1_RADIO_COLD_RESET] = { 0x20008, 2 },
	[WIFI1_CORE_WARM_RESET] = { 0x20008, 1 },
	[WIFI1_CORE_COLD_RESET] = { 0x20008, 0 },
	[USB3_UNIPHY_PHY_ARES] = { 0x1e038, 5 },
	[USB3_HSPHY_POR_ARES] = { 0x1e038, 4 },
	[USB3_HSPHY_S_ARES] = { 0x1e038, 2 },
	[USB2_HSPHY_POR_ARES] = { 0x1e01c, 4 },
	[USB2_HSPHY_S_ARES] = { 0x1e01c, 2 },
	[PCIE_PHY_AHB_ARES] = { 0x1d010, 11 },
	[PCIE_AHB_ARES] = { 0x1d010, 10 },
	[PCIE_PWR_ARES] = { 0x1d010, 9 },
	[PCIE_PIPE_STICKY_ARES] = { 0x1d010, 8 },
	[PCIE_AXI_M_STICKY_ARES] = { 0x1d010, 7 },
	[PCIE_PHY_ARES] = { 0x1d010, 6 },
	[PCIE_PARF_XPU_ARES] = { 0x1d010, 5 },
	[PCIE_AXI_S_XPU_ARES] = { 0x1d010, 4 },
	[PCIE_AXI_M_VMIDMT_ARES] = { 0x1d010, 3 },
	[PCIE_PIPE_ARES] = { 0x1d010, 2 },
	[PCIE_AXI_S_ARES] = { 0x1d010, 1 },
	[PCIE_AXI_M_ARES] = { 0x1d010, 0 },
	[ESS_RESET] = { 0x12008, 0},
	[GCC_BLSP1_BCR] = {0x01000, 0},
	[GCC_BLSP1_QUP1_BCR] = {0x02000, 0},
	[GCC_BLSP1_UART1_BCR] = {0x02038, 0},
	[GCC_BLSP1_QUP2_BCR] = {0x03008, 0},
	[GCC_BLSP1_UART2_BCR] = {0x03028, 0},
	[GCC_BIMC_BCR] = {0x04000, 0},
	[GCC_TLMM_BCR] = {0x05000, 0},
	[GCC_IMEM_BCR] = {0x0E000, 0},
	[GCC_ESS_BCR] = {0x12008, 0},
	[GCC_PRNG_BCR] = {0x13000, 0},
	[GCC_BOOT_ROM_BCR] = {0x13008, 0},
	[GCC_CRYPTO_BCR] = {0x16000, 0},
	[GCC_SDCC1_BCR] = {0x18000, 0},
	[GCC_SEC_CTRL_BCR] = {0x1A000, 0},
	[GCC_AUDIO_BCR] = {0x1B008, 0},
	[GCC_QPIC_BCR] = {0x1C000, 0},
	[GCC_PCIE_BCR] = {0x1D000, 0},
	[GCC_USB2_BCR] = {0x1E008, 0},
	[GCC_USB2_PHY_BCR] = {0x1E018, 0},
	[GCC_USB3_BCR] = {0x1E024, 0},
	[GCC_USB3_PHY_BCR] = {0x1E034, 0},
	[GCC_SYSTEM_NOC_BCR] = {0x21000, 0},
	[GCC_PCNOC_BCR] = {0x2102C, 0},
	[GCC_DCD_BCR] = {0x21038, 0},
	[GCC_SNOC_BUS_TIMEOUT0_BCR] = {0x21064, 0},
	[GCC_SNOC_BUS_TIMEOUT1_BCR] = {0x2106C, 0},
	[GCC_SNOC_BUS_TIMEOUT2_BCR] = {0x21074, 0},
	[GCC_SNOC_BUS_TIMEOUT3_BCR] = {0x2107C, 0},
	[GCC_PCNOC_BUS_TIMEOUT0_BCR] = {0x21084, 0},
	[GCC_PCNOC_BUS_TIMEOUT1_BCR] = {0x2108C, 0},
	[GCC_PCNOC_BUS_TIMEOUT2_BCR] = {0x21094, 0},
	[GCC_PCNOC_BUS_TIMEOUT3_BCR] = {0x2109C, 0},
	[GCC_PCNOC_BUS_TIMEOUT4_BCR] = {0x210A4, 0},
	[GCC_PCNOC_BUS_TIMEOUT5_BCR] = {0x210AC, 0},
	[GCC_PCNOC_BUS_TIMEOUT6_BCR] = {0x210B4, 0},
	[GCC_PCNOC_BUS_TIMEOUT7_BCR] = {0x210BC, 0},
	[GCC_PCNOC_BUS_TIMEOUT8_BCR] = {0x210C4, 0},
	[GCC_PCNOC_BUS_TIMEOUT9_BCR] = {0x210CC, 0},
	[GCC_TCSR_BCR] = {0x22000, 0},
	[GCC_MPM_BCR] = {0x24000, 0},
	[GCC_SPDM_BCR] = {0x25000, 0},
};
#endif

#ifdef CONFIG_TARGET_QCS404EVB
#include <dt-bindings/clock/qcom,gcc-qcs404.h>
static const struct qcom_reset_map gcc_qcom_resets[] = {
	[GCC_GENI_IR_BCR] = { 0x0F000 },
	[GCC_CDSP_RESTART] = { 0x18000 },
	[GCC_USB_HS_BCR] = { 0x41000 },
	[GCC_USB2_HS_PHY_ONLY_BCR] = { 0x41034 },
	[GCC_QUSB2_PHY_BCR] = { 0x4103c },
	[GCC_USB_HS_PHY_CFG_AHB_BCR] = { 0x0000c, 1 },
	[GCC_USB2A_PHY_BCR] = { 0x0000c, 0 },
	[GCC_USB3_PHY_BCR] = { 0x39004 },
	[GCC_USB_30_BCR] = { 0x39000 },
	[GCC_USB3PHY_PHY_BCR] = { 0x39008 },
	[GCC_PCIE_0_BCR] = { 0x3e000 },
	[GCC_PCIE_0_PHY_BCR] = { 0x3e004 },
	[GCC_PCIE_0_LINK_DOWN_BCR] = { 0x3e038 },
	[GCC_PCIEPHY_0_PHY_BCR] = { 0x3e03c },
	[GCC_PCIE_0_AXI_MASTER_STICKY_ARES] = { 0x3e040, 6},
	[GCC_PCIE_0_AHB_ARES] = { 0x3e040, 5 },
	[GCC_PCIE_0_AXI_SLAVE_ARES] = { 0x3e040, 4 },
	[GCC_PCIE_0_AXI_MASTER_ARES] = { 0x3e040, 3 },
	[GCC_PCIE_0_CORE_STICKY_ARES] = { 0x3e040, 2 },
	[GCC_PCIE_0_SLEEP_ARES] = { 0x3e040, 1 },
	[GCC_PCIE_0_PIPE_ARES] = { 0x3e040, 0 },
	[GCC_EMAC_BCR] = { 0x4e000 },
	[GCC_WDSP_RESTART] = {0x19000},
};
#endif

static int qcom_reset_assert(struct reset_ctl *rst)
{
	struct qcom_reset_priv *priv = dev_get_priv(rst->dev);
	const struct qcom_reset_map *reset_map = gcc_qcom_resets;
	const struct qcom_reset_map *map;
	u32 value;

	map = &reset_map[rst->id];

	value = readl(priv->base + map->reg);
	value |= BIT(map->bit);
	writel(value, priv->base + map->reg);

	return 0;
}

static int qcom_reset_deassert(struct reset_ctl *rst)
{
	struct qcom_reset_priv *priv = dev_get_priv(rst->dev);
	const struct qcom_reset_map *reset_map = gcc_qcom_resets;
	const struct qcom_reset_map *map;
	u32 value;

	map = &reset_map[rst->id];

	value = readl(priv->base + map->reg);
	value &= ~BIT(map->bit);
	writel(value, priv->base + map->reg);

	return 0;
}

static const struct reset_ops qcom_reset_ops = {
	.rst_assert = qcom_reset_assert,
	.rst_deassert = qcom_reset_deassert,
};

static const struct udevice_id qcom_reset_ids[] = {
	{ .compatible = "qcom,gcc-reset-ipq4019" },
	{ .compatible = "qcom,gcc-reset-qcs404" },
	{ }
};

static int qcom_reset_probe(struct udevice *dev)
{
	struct qcom_reset_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

U_BOOT_DRIVER(qcom_reset) = {
	.name = "qcom_reset",
	.id = UCLASS_RESET,
	.of_match = qcom_reset_ids,
	.ops = &qcom_reset_ops,
	.probe = qcom_reset_probe,
	.priv_auto = sizeof(struct qcom_reset_priv),
};
