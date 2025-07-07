// SPDX-License-Identifier: GPL-2.0+

#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <pci.h>
#include <u-boot/crc.h>
#include <power-domain.h>
#include <reset.h>
#include <syscon.h>
#include <malloc.h>
#include <power/regulator.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <linux/iopoll.h>
#include <linux/delay.h>
#include <linux/log2.h>
#include <linux/bitfield.h>

#include "pcie_dw_common.h"

DECLARE_GLOBAL_DATA_PTR;

struct qcom_pcie;

struct qcom_pcie_ops {
	int (*config_sid)(struct qcom_pcie *priv);
};

#define NUM_SUPPLIES	2

struct qcom_pcie {
	/* Must be first member of the struct */
	struct pcie_dw dw;
	void *parf;
	struct phy phy;
	struct reset_ctl_bulk rsts;
	struct clk_bulk clks;
	struct gpio_desc rst_gpio;
	struct qcom_pcie_ops *ops;
	struct udevice *vregs[NUM_SUPPLIES];
};

/* PARF registers */
#define PARF_SYS_CTRL				0x00
#define PARF_PM_CTRL				0x20
#define PARF_PCS_DEEMPH				0x34
#define PARF_PCS_SWING				0x38
#define PARF_PHY_CTRL				0x40
#define PARF_PHY_REFCLK				0x4c
#define PARF_CONFIG_BITS			0x50
#define PARF_DBI_BASE_ADDR			0x168
#define PARF_MHI_CLOCK_RESET_CTRL		0x174
#define PARF_AXI_MSTR_WR_ADDR_HALT		0x178
#define PARF_AXI_MSTR_WR_ADDR_HALT_V2		0x1a8
#define PARF_Q2A_FLUSH				0x1ac
#define PARF_LTSSM				0x1b0
#define PARF_SID_OFFSET				0x234
#define PARF_BDF_TRANSLATE_CFG			0x24c
#define PARF_SLV_ADDR_SPACE_SIZE		0x358
#define PARF_DEVICE_TYPE			0x1000
#define PARF_BDF_TO_SID_TABLE_N			0x2000

/* ELBI registers */
#define ELBI_SYS_CTRL				0x04

/* DBI registers */
#define AXI_MSTR_RESP_COMP_CTRL0		0x818
#define AXI_MSTR_RESP_COMP_CTRL1		0x81c
#define MISC_CONTROL_1_REG			0x8bc

/* MHI registers */
#define PARF_DEBUG_CNT_PM_LINKST_IN_L2		0xc04
#define PARF_DEBUG_CNT_PM_LINKST_IN_L1		0xc0c
#define PARF_DEBUG_CNT_PM_LINKST_IN_L0S		0xc10
#define PARF_DEBUG_CNT_AUX_CLK_IN_L1SUB_L1	0xc84
#define PARF_DEBUG_CNT_AUX_CLK_IN_L1SUB_L2	0xc88

/* PARF_SYS_CTRL register fields */
#define MAC_PHY_POWERDOWN_IN_P2_D_MUX_EN	BIT(29)
#define MST_WAKEUP_EN				BIT(13)
#define SLV_WAKEUP_EN				BIT(12)
#define MSTR_ACLK_CGC_DIS			BIT(10)
#define SLV_ACLK_CGC_DIS			BIT(9)
#define CORE_CLK_CGC_DIS			BIT(6)
#define AUX_PWR_DET				BIT(4)
#define L23_CLK_RMV_DIS				BIT(2)
#define L1_CLK_RMV_DIS				BIT(1)

/* PARF_PM_CTRL register fields */
#define REQ_NOT_ENTR_L1				BIT(5)

/* PARF_PCS_DEEMPH register fields */
#define PCS_DEEMPH_TX_DEEMPH_GEN1(x)		FIELD_PREP(GENMASK(21, 16), x)
#define PCS_DEEMPH_TX_DEEMPH_GEN2_3_5DB(x)	FIELD_PREP(GENMASK(13, 8), x)
#define PCS_DEEMPH_TX_DEEMPH_GEN2_6DB(x)	FIELD_PREP(GENMASK(5, 0), x)

/* PARF_PCS_SWING register fields */
#define PCS_SWING_TX_SWING_FULL(x)		FIELD_PREP(GENMASK(14, 8), x)
#define PCS_SWING_TX_SWING_LOW(x)		FIELD_PREP(GENMASK(6, 0), x)

/* PARF_PHY_CTRL register fields */
#define PHY_CTRL_PHY_TX0_TERM_OFFSET_MASK	GENMASK(20, 16)
#define PHY_CTRL_PHY_TX0_TERM_OFFSET(x)		FIELD_PREP(PHY_CTRL_PHY_TX0_TERM_OFFSET_MASK, x)
#define PHY_TEST_PWR_DOWN			BIT(0)

/* PARF_PHY_REFCLK register fields */
#define PHY_REFCLK_SSP_EN			BIT(16)
#define PHY_REFCLK_USE_PAD			BIT(12)

/* PARF_CONFIG_BITS register fields */
#define PHY_RX0_EQ(x)				FIELD_PREP(GENMASK(26, 24), x)

/* PARF_SLV_ADDR_SPACE_SIZE register value */
#define SLV_ADDR_SPACE_SZ			0x10000000

/* PARF_MHI_CLOCK_RESET_CTRL register fields */
#define AHB_CLK_EN				BIT(0)
#define MSTR_AXI_CLK_EN				BIT(1)
#define BYPASS					BIT(4)

/* PARF_AXI_MSTR_WR_ADDR_HALT register fields */
#define EN					BIT(31)

/* PARF_LTSSM register fields */
#define LTSSM_EN				BIT(8)

/* PARF_DEVICE_TYPE register fields */
#define DEVICE_TYPE_RC				0x4

/* ELBI_SYS_CTRL register fields */
#define ELBI_SYS_CTRL_LT_ENABLE			BIT(0)

/* AXI_MSTR_RESP_COMP_CTRL0 register fields */
#define CFG_REMOTE_RD_REQ_BRIDGE_SIZE_2K	0x4
#define CFG_REMOTE_RD_REQ_BRIDGE_SIZE_4K	0x5

/* AXI_MSTR_RESP_COMP_CTRL1 register fields */
#define CFG_BRIDGE_SB_INIT			BIT(0)

/* MISC_CONTROL_1_REG register fields */
#define DBI_RO_WR_EN				1

/* PCI_EXP_SLTCAP register fields */
#define PCIE_CAP_SLOT_POWER_LIMIT_VAL		FIELD_PREP(PCI_EXP_SLTCAP_SPLV, 250)
#define PCIE_CAP_SLOT_POWER_LIMIT_SCALE		FIELD_PREP(PCI_EXP_SLTCAP_SPLS, 1)
#define PCIE_CAP_SLOT_VAL			(PCI_EXP_SLTCAP_ABP | \
						PCI_EXP_SLTCAP_PCP | \
						PCI_EXP_SLTCAP_MRLSP | \
						PCI_EXP_SLTCAP_AIP | \
						PCI_EXP_SLTCAP_PIP | \
						PCI_EXP_SLTCAP_HPS | \
						PCI_EXP_SLTCAP_HPC | \
						PCI_EXP_SLTCAP_EIP | \
						PCIE_CAP_SLOT_POWER_LIMIT_VAL | \
						PCIE_CAP_SLOT_POWER_LIMIT_SCALE)

#define PERST_DELAY_US				1000

#define LINK_WAIT_MAX_RETRIES			10
#define LINK_WAIT_USLEEP			100000

#define QCOM_PCIE_CRC8_POLYNOMIAL		(BIT(2) | BIT(1) | BIT(0))

#define CRC8_TABLE_SIZE				256

static bool qcom_pcie_wait_link_up(struct qcom_pcie *priv)
{
	u8 offset = pcie_dw_find_capability(&priv->dw, PCI_CAP_ID_EXP);
	unsigned int cnt = 0;
	u16 val;

	do {
		val = readw(priv->dw.dbi_base + offset + PCI_EXP_LNKSTA);

		if ((val & PCI_EXP_LNKSTA_DLLLA))
			return true;
		cnt++;

		udelay(LINK_WAIT_USLEEP);
	} while (cnt < LINK_WAIT_MAX_RETRIES);

	return false;
}

static void qcom_pcie_clear_aspm_l0s(struct qcom_pcie *priv)
{
	u8 offset = pcie_dw_find_capability(&priv->dw, PCI_CAP_ID_EXP);
	u32 val;

	dw_pcie_dbi_write_enable(&priv->dw, true);

	val = readl(priv->dw.dbi_base + offset + PCI_EXP_LNKCAP);
	val &= ~PCI_EXP_LNKCAP_ASPM_L0S;
	writel(val, priv->dw.dbi_base + offset + PCI_EXP_LNKCAP);

	dw_pcie_dbi_write_enable(&priv->dw, false);
}

static void qcom_pcie_clear_hpc(struct qcom_pcie *priv)
{
	u8 offset = pcie_dw_find_capability(&priv->dw, PCI_CAP_ID_EXP);
	u32 val;

	dw_pcie_dbi_write_enable(&priv->dw, true);

	val = readl(priv->dw.dbi_base + offset + PCI_EXP_SLTCAP);
	val &= ~PCI_EXP_SLTCAP_HPC;
	writel(val, priv->dw.dbi_base + offset + PCI_EXP_SLTCAP);

	dw_pcie_dbi_write_enable(&priv->dw, false);
}

static int qcom_pcie_config_sid_1_9_0(struct qcom_pcie *priv)
{
	/* iommu map structure */
	struct {
		u32 bdf;
		u32 phandle;
		u32 smmu_sid;
		u32 smmu_sid_len;
	} *map;
	void *bdf_to_sid_base = priv->parf + PARF_BDF_TO_SID_TABLE_N;
	int i, nr_map, size = 0;
	u32 smmu_sid_base;

	dev_read_prop(priv->dw.dev, "iommu-map", &size);
	if (!size)
		return 0;

	map = malloc(size);
	if (!map)
		return -ENOMEM;

	dev_read_u32_array(priv->dw.dev, "iommu-map", (u32 *)map, size / sizeof(u32));

	nr_map = size / (sizeof(*map));

	/* Registers need to be zero out first */
	memset_io(bdf_to_sid_base, 0, CRC8_TABLE_SIZE * sizeof(u32));

	/* Extract the SMMU SID base from the first entry of iommu-map */
	smmu_sid_base = map[0].smmu_sid;

	/* Look for an available entry to hold the mapping */
	for (i = 0; i < nr_map; i++) {
		__be16 bdf_be = cpu_to_be16(map[i].bdf);
		u32 val;
		u8 hash;

		hash = crc8(QCOM_PCIE_CRC8_POLYNOMIAL, (u8 *)&bdf_be, sizeof(bdf_be));

		val = readl(bdf_to_sid_base + hash * sizeof(u32));

		/* If the register is already populated, look for next available entry */
		while (val) {
			u8 current_hash = hash++;
			u8 next_mask = 0xff;

			/* If NEXT field is NULL then update it with next hash */
			if (!(val & next_mask)) {
				val |= (u32)hash;
				writel(val, bdf_to_sid_base + current_hash * sizeof(u32));
			}

			val = readl(bdf_to_sid_base + hash * sizeof(u32));
		}

		/* BDF [31:16] | SID [15:8] | NEXT [7:0] */
		val = map[i].bdf << 16 | (map[i].smmu_sid - smmu_sid_base) << 8 | 0;
		writel(val, bdf_to_sid_base + hash * sizeof(u32));
	}

	free(map);

	return 0;
}

static void qcom_pcie_configure(struct qcom_pcie *priv)
{
	u32 val;

	dw_pcie_dbi_write_enable(&priv->dw, true);

	val = readl(priv->dw.dbi_base + PCIE_PORT_LINK_CONTROL);
	val &= ~PORT_LINK_FAST_LINK_MODE;
	val |= PORT_LINK_DLL_LINK_EN;
	val &= ~PORT_LINK_MODE_MASK;
	writel(val, priv->dw.dbi_base + PCIE_PORT_LINK_CONTROL);

	dw_pcie_link_set_max_link_width(&priv->dw, 2);

	dw_pcie_dbi_write_enable(&priv->dw, false);
}

static int qcom_pcie_init_port(struct udevice *dev)
{
	struct qcom_pcie *priv = dev_get_priv(dev);
	int vreg, ret;
	u32 val;

	dm_gpio_set_value(&priv->rst_gpio, 1);
	udelay(PERST_DELAY_US);

	ret = generic_phy_init(&priv->phy);
	if (ret) {
		dev_err(dev, "failed to init phy (%d)\n", ret);
		return ret;
	}

	udelay(PERST_DELAY_US);

	for (vreg = 0; vreg < NUM_SUPPLIES; ++vreg) {
		ret = regulator_set_enable(priv->vregs[vreg], true);
		if (ret && ret != -ENOSYS)
			dev_warn(dev, "failed to enable regulator %d (%d)\n", vreg, ret);
	}

	ret = clk_enable_bulk(&priv->clks);
	if (ret) {
		dev_err(dev, "failed to enable clocks (%d)\n", ret);
		goto err_power_off_phy;
	}

	ret = reset_assert_bulk(&priv->rsts);
	if (ret) {
		dev_err(dev, "failed to assert resets (%d)\n", ret);
		goto err_disable_clks;
	}

	udelay(PERST_DELAY_US);

	ret = reset_deassert_bulk(&priv->rsts);
	if (ret) {
		dev_err(dev, "failed to deassert resets (%d)\n", ret);
		goto err_power_off_phy;
	}

	udelay(PERST_DELAY_US);

	/* configure PCIe to RC mode */
	writel(DEVICE_TYPE_RC, priv->parf + PARF_DEVICE_TYPE);

	/* enable PCIe clocks and resets */
	val = readl(priv->parf + PARF_PHY_CTRL);
	val &= ~PHY_TEST_PWR_DOWN;
	writel(val, priv->parf + PARF_PHY_CTRL);

	/* change DBI base address */
	writel(0, priv->parf + PARF_DBI_BASE_ADDR);

	/* MAC PHY_POWERDOWN MUX DISABLE  */
	val = readl(priv->parf + PARF_SYS_CTRL);
	val &= ~MAC_PHY_POWERDOWN_IN_P2_D_MUX_EN;
	writel(val, priv->parf + PARF_SYS_CTRL);

	val = readl(priv->parf + PARF_MHI_CLOCK_RESET_CTRL);
	val |= BYPASS;
	writel(val, priv->parf + PARF_MHI_CLOCK_RESET_CTRL);

	/* Enable L1 and L1SS */
	val = readl(priv->parf + PARF_PM_CTRL);
	val &= ~REQ_NOT_ENTR_L1;
	writel(val, priv->parf + PARF_PM_CTRL);

	val = readl(priv->parf + PARF_AXI_MSTR_WR_ADDR_HALT_V2);
	val |= EN;
	writel(val, priv->parf + PARF_AXI_MSTR_WR_ADDR_HALT_V2);

	ret = generic_phy_power_on(&priv->phy);
	if (ret) {
		dev_err(dev, "failed to power on phy (%d)\n", ret);
		goto err_exit_phy;
	}

	qcom_pcie_clear_aspm_l0s(priv);
	qcom_pcie_clear_hpc(priv);

	mdelay(100);
	dm_gpio_set_value(&priv->rst_gpio, 0);
	udelay(PERST_DELAY_US);

	if (priv->ops && priv->ops->config_sid) {
		ret = priv->ops->config_sid(priv);
		if (ret)
			goto err_deassert_bulk;
	}

	qcom_pcie_configure(priv);

	pcie_dw_setup_host(&priv->dw);

	/* enable link training */
	val = readl(priv->parf + PARF_LTSSM);
	val |= LTSSM_EN;
	writel(val, priv->parf + PARF_LTSSM);

	return 0;
err_deassert_bulk:
	reset_assert_bulk(&priv->rsts);
err_disable_clks:
	clk_disable_bulk(&priv->clks);
err_power_off_phy:
	generic_phy_power_off(&priv->phy);
err_exit_phy:
	generic_phy_exit(&priv->phy);

	return ret;
}

static const char *qcom_pcie_vregs[NUM_SUPPLIES] = {
	"vdda-supply",
	"vddpe-3v3-supply",
};

static int qcom_pcie_parse_dt(struct udevice *dev)
{
	struct qcom_pcie *priv = dev_get_priv(dev);
	int vreg, ret;

	priv->dw.dbi_base = dev_read_addr_name_ptr(dev, "dbi");
	if (!priv->dw.dbi_base)
		return -EINVAL;

	dev_dbg(dev, "DBI address is 0x%p\n", priv->dw.dbi_base);

	priv->dw.atu_base = dev_read_addr_name_ptr(dev, "atu");
	if (!priv->dw.atu_base)
		return -EINVAL;

	dev_dbg(dev, "ATU address is 0x%p\n", priv->dw.atu_base);

	priv->parf = dev_read_addr_name_ptr(dev, "parf");
	if (!priv->parf)
		return -EINVAL;

	dev_dbg(dev, "PARF address is 0x%p\n", priv->parf);

	ret = gpio_request_by_name(dev, "perst-gpios", 0,
				   &priv->rst_gpio, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "failed to find reset-gpios property\n");
		return ret;
	}

	ret = reset_get_bulk(dev, &priv->rsts);
	if (ret) {
		dev_err(dev, "failed to get resets (%d)\n", ret);
		return ret;
	}

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret) {
		dev_err(dev, "failed to get clocks (%d)\n", ret);
		return ret;
	}

	ret = generic_phy_get_by_index(dev, 0, &priv->phy);
	if (ret) {
		dev_err(dev, "failed to get pcie phy (%d)\n", ret);
		return ret;
	}

	for (vreg = 0; vreg < NUM_SUPPLIES; ++vreg) {
		ret = device_get_supply_regulator(dev, qcom_pcie_vregs[vreg], &priv->vregs[vreg]);
		if (ret)
			dev_warn(dev, "failed to get regulator %d (%d)\n", vreg, ret);
	}

	return 0;
}

/**
 * qcom_pcie_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int qcom_pcie_probe(struct udevice *dev)
{
	struct qcom_pcie *priv = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	int ret = 0;

	priv->dw.first_busno = dev_seq(dev);
	priv->dw.dev = dev;

	ret = qcom_pcie_parse_dt(dev);
	if (ret)
		return ret;

	ret = qcom_pcie_init_port(dev);
	if (ret) {
		dm_gpio_free(dev, &priv->rst_gpio);
		return ret;
	}

	if (qcom_pcie_wait_link_up(priv))
		printf("PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n",
		       dev_seq(dev), pcie_dw_get_link_speed(&priv->dw),
		       pcie_dw_get_link_width(&priv->dw),
		       hose->first_busno);
	else
		printf("PCIE-%d: Link up timeout\n", dev_seq(dev));

	return pcie_dw_prog_outbound_atu_unroll(&priv->dw,
						PCIE_ATU_REGION_INDEX0,
						PCIE_ATU_TYPE_MEM,
						priv->dw.mem.phys_start,
						priv->dw.mem.bus_start,
						priv->dw.mem.size);
}

static const struct dm_pci_ops qcom_pcie_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct qcom_pcie_ops ops_1_9_0 = {
	.config_sid = qcom_pcie_config_sid_1_9_0,
};

static const struct udevice_id qcom_pcie_ids[] = {
	{ .compatible = "qcom,pcie-sa8540p", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-sc7280", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-sc8180x", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-sc8280xp", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-sdm845" },
	{ .compatible = "qcom,pcie-sdx55", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-sm8150", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-sm8250", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-sm8350", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-sm8450-pcie0", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-sm8450-pcie1", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-sm8550", .data = (ulong)&ops_1_9_0 },
	{ .compatible = "qcom,pcie-x1e80100", .data = (ulong)&ops_1_9_0 },
	{ }
};

U_BOOT_DRIVER(qcom_dw_pcie) = {
	.name			= "pcie_dw_qcom",
	.id			= UCLASS_PCI,
	.of_match		= qcom_pcie_ids,
	.ops			= &qcom_pcie_ops,
	.probe			= qcom_pcie_probe,
	.priv_auto		= sizeof(struct qcom_pcie),
};
