// SPDX-License-Identifier: GPL-2.0+
/*
 * PCIe host controller driver for AMD CPM
 *
 * Copyright (C) 2026 Advanced Micro Devices, Inc.
 */

#define LOG_CATEGORY UCLASS_PCI

#include <dm.h>
#include <pci.h>

#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/ioport.h>

/*
 * End of conventional reset (PERST# de-asserted) to first configuration
 * request (device able to respond with a "Request Retry Status" completion),
 * from PCIe r6.0, sec 6.6.1.
 */
#define PCIE_T_RRS_READY_MS	100

/*
 * "PERST# active time", as per Table 2-10: Power Sequencing and Reset
 * Signal Timings of the PCIe Electromechanical Specification,
 * Revision 6.0, symbol "T_PERST".
 */
#define PCIE_T_PERST_US		100

/* Register definitions */
#define AMD_CPM_PCIE0_RST		0x00000308
#define AMD_CPM5_PCIE0_RST		0x00000318
#define AMD_CPM5_PCIE1_RST		0x0000031C

#define AMD_CPM_PCIE_REG_IDR		0x00000E10
#define AMD_CPM_PCIE_REG_IMR		0x00000E14
#define AMD_CPM_PCIE_REG_PSCR		0x00000E1C
#define AMD_CPM_PCIE_REG_RPSC		0x00000E20

/* Interrupt registers definitions */
#define AMD_CPM_INTR_LINK_DOWN		0
#define AMD_CPM_INTR_HOT_RESET		3
#define AMD_CPM_INTR_CFG_PCIE_TIMEOUT	4
#define AMD_CPM_INTR_CFG_TIMEOUT	8
#define AMD_CPM_INTR_CORRECTABLE	9
#define AMD_CPM_INTR_NONFATAL		10
#define AMD_CPM_INTR_FATAL		11
#define AMD_CPM_INTR_CFG_ERR_POISON	12
#define AMD_CPM_INTR_PME_TO_ACK_RCVD	15
#define AMD_CPM_INTR_INTX		16
#define AMD_CPM_INTR_PM_PME_RCVD	17
#define AMD_CPM_INTR_MSI		17
#define AMD_CPM_INTR_SLV_UNSUPP		20
#define AMD_CPM_INTR_SLV_UNEXP		21
#define AMD_CPM_INTR_SLV_COMPL		22
#define AMD_CPM_INTR_SLV_ERRP		23
#define AMD_CPM_INTR_SLV_CMPABT		24
#define AMD_CPM_INTR_SLV_ILLBUR		25
#define AMD_CPM_INTR_MST_DECERR		26
#define AMD_CPM_INTR_MST_SLVERR		27
#define AMD_CPM_INTR_SLV_PCIE_TIMEOUT	28

#define IMR(x) BIT(AMD_CPM_INTR_ ##x)

#define AMD_CPM_PCIE_IMR_ALL_MASK		\
	(IMR(LINK_DOWN)		| \
	 IMR(HOT_RESET)		| \
	 IMR(CFG_PCIE_TIMEOUT)	| \
	 IMR(CFG_TIMEOUT)	| \
	 IMR(CORRECTABLE)	| \
	 IMR(NONFATAL)		| \
	 IMR(FATAL)		| \
	 IMR(CFG_ERR_POISON)	| \
	 IMR(PME_TO_ACK_RCVD)	| \
	 IMR(INTX)		| \
	 IMR(PM_PME_RCVD)	| \
	 IMR(SLV_UNSUPP)	| \
	 IMR(SLV_UNEXP)		| \
	 IMR(SLV_COMPL)		| \
	 IMR(SLV_ERRP)		| \
	 IMR(SLV_CMPABT)	| \
	 IMR(SLV_ILLBUR)	| \
	 IMR(MST_DECERR)	| \
	 IMR(MST_SLVERR)	| \
	 IMR(SLV_PCIE_TIMEOUT))

#define AMD_CPM_PCIE_IDR_ALL_MASK		0xFFFFFFFF

#define AMD_CPM_PCIE_REG_RPSC_BEN		BIT(0)

#define AMD_CPM_PCIE_REG_PSCR_LNKUP		BIT(11)

enum amd_cpm_version {
	AMD_CPM_VER_1,
	AMD_CPM_VER_5_HOST0,
	AMD_CPM_VER_5_HOST1,
};

/**
 * struct amd_cpm_variant - CPM variant information
 * @version: CPM version
 * @cpm_pcie_rst: Offset for the PCIe IP reset
 */
struct amd_cpm_variant {
	enum amd_cpm_version version;
	u32 cpm_pcie_rst;
};

/**
 * struct amd_cpm_pcie - AMD CPM PCIe bridge private data
 * @dev: PCIe host bridge device
 * @reg_base: Bridge register base address
 * @cfg_base: ECAM configuration space base address
 * @crx_base: CPM5 reset control base address (optional)
 * @variant: CPM version check pointer
 */
struct amd_cpm_pcie {
	struct udevice *dev;
	void __iomem *reg_base;
	void __iomem *cfg_base;
	void __iomem *crx_base;
	const struct amd_cpm_variant *variant;
};

static inline u32 amd_cpm_pcie_read(struct amd_cpm_pcie *pcie, u32 reg)
{
	u32 val = readl(pcie->reg_base + reg);

	dev_dbg(pcie->dev, "rd reg 0x%03x = 0x%08x\n", reg, val);

	return val;
}

static inline void amd_cpm_pcie_write(struct amd_cpm_pcie *pcie,
				      u32 val, u32 reg)
{
	dev_dbg(pcie->dev, "wr reg 0x%03x = 0x%08x\n", reg, val);
	writel(val, pcie->reg_base + reg);
}

static bool amd_cpm_pcie_link_up(struct amd_cpm_pcie *pcie)
{
	return !!(amd_cpm_pcie_read(pcie, AMD_CPM_PCIE_REG_PSCR) &
		  AMD_CPM_PCIE_REG_PSCR_LNKUP);
}

static int amd_cpm_pcie_config_address(const struct udevice *bus,
				       pci_dev_t bdf, uint offset,
				       void **paddress)
{
	struct amd_cpm_pcie *priv = dev_get_priv(bus);
	u32 busnum = PCI_BUS(bdf) - dev_seq(bus);
	void *addr;

	addr = priv->cfg_base;
	addr += PCIE_ECAM_OFFSET(busnum, PCI_DEV(bdf), PCI_FUNC(bdf), offset);
	dev_dbg(priv->dev, "cfg bdf=%02x:%02x.%x offset=0x%x addr=%p\n",
		PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset, addr);
	*paddress = addr;

	return 0;
}

/*
 * The root port's own function (bus 0, device 0, function 0) reports a
 * bogus 64-bit BAR0 that autoconfig can never place. Mask it off so it
 * always reads back as not implemented.
 */
static bool amd_cpm_pcie_is_own_bar(const struct udevice *bus, pci_dev_t bdf,
				    uint offset)
{
	u32 busnum = PCI_BUS(bdf) - dev_seq(bus);

	return busnum == 0 && PCI_DEV(bdf) == 0 && PCI_FUNC(bdf) == 0 &&
	       offset >= PCI_BASE_ADDRESS_0 && offset < PCI_BASE_ADDRESS_2;
}

static int amd_cpm_pcie_read_config(const struct udevice *bus,
				    pci_dev_t bdf, uint offset,
				    ulong *valuep, enum pci_size_t size)
{
	if (amd_cpm_pcie_is_own_bar(bus, bdf, offset)) {
		*valuep = 0;
		return 0;
	}

	return pci_generic_mmap_read_config(bus, amd_cpm_pcie_config_address,
					    bdf, offset, valuep, size);
}

static int amd_cpm_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				     uint offset, ulong value,
				     enum pci_size_t size)
{
	if (amd_cpm_pcie_is_own_bar(bus, bdf, offset))
		return 0;

	return pci_generic_mmap_write_config(bus, amd_cpm_pcie_config_address,
					     bdf, offset, value, size);
}

static const struct dm_pci_ops amd_cpm_pcie_ops = {
	.read_config = amd_cpm_pcie_read_config,
	.write_config = amd_cpm_pcie_write_config,
};

static int amd_cpm_pcie_host_init(struct amd_cpm_pcie *pcie)
{
	const struct amd_cpm_variant *variant = pcie->variant;
	struct udevice *dev = pcie->dev;
	struct gpio_desc *reset_gpio;
	u32 val;

	if (pcie->crx_base) {
		reset_gpio = devm_gpiod_get_optional(dev, "reset",
						     GPIOD_IS_OUT |
						     GPIOD_IS_OUT_ACTIVE);
		if (reset_gpio) {
			/* Assert the PCIe IP reset */
			writel(BIT(0), pcie->crx_base + variant->cpm_pcie_rst);

			udelay(PCIE_T_PERST_US);

			/* Deassert the PCIe IP reset */
			writel(0, pcie->crx_base + variant->cpm_pcie_rst);

			/* Deassert the reset signal */
			dm_gpio_set_value(reset_gpio, 0);
			mdelay(PCIE_T_RRS_READY_MS);
		}
	}

	dev_dbg(dev, "PCIe link %s\n",
		amd_cpm_pcie_link_up(pcie) ? "up" : "down");

	/* Disable all interrupts */
	amd_cpm_pcie_write(pcie, ~AMD_CPM_PCIE_IDR_ALL_MASK,
			   AMD_CPM_PCIE_REG_IMR);

	/* Clear pending interrupts */
	val = amd_cpm_pcie_read(pcie, AMD_CPM_PCIE_REG_IDR);
	amd_cpm_pcie_write(pcie, val & AMD_CPM_PCIE_IMR_ALL_MASK,
			   AMD_CPM_PCIE_REG_IDR);

	/* Enable root port bridge */
	val = amd_cpm_pcie_read(pcie, AMD_CPM_PCIE_REG_RPSC);
	amd_cpm_pcie_write(pcie, val | AMD_CPM_PCIE_REG_RPSC_BEN,
			   AMD_CPM_PCIE_REG_RPSC);

	return 0;
}

static int amd_cpm_pcie_map_res(struct udevice *dev, const char *name,
				void __iomem **map)
{
	struct resource res;
	int ret;

	ret = dev_read_resource_byname(dev, name, &res);
	if (ret)
		return ret;

	*map = devm_ioremap(dev, res.start, resource_size(&res));
	if (!*map)
		return -ENOMEM;

	return 0;
}

static int amd_cpm_pcie_parse_dt(struct amd_cpm_pcie *pcie)
{
	struct udevice *dev = pcie->dev;
	int ret;

	ret = amd_cpm_pcie_map_res(dev, "cfg", &pcie->cfg_base);
	if (ret)
		return ret;

	if (pcie->variant->version == AMD_CPM_VER_1) {
		/*
		 * On original CPM the ECAM and bridge registers share the same
		 * region; cfg_base and reg_base both point into it.
		 */
		pcie->reg_base = pcie->cfg_base;
	} else {
		ret = amd_cpm_pcie_map_res(dev, "cpm_csr", &pcie->reg_base);
		if (ret)
			return ret;
	}

	/* cpm_crx is optional; absence means no PCIe IP reset sequence */
	amd_cpm_pcie_map_res(dev, "cpm_crx", &pcie->crx_base);

	return 0;
}

static int amd_cpm_pcie_probe(struct udevice *dev)
{
	struct amd_cpm_pcie *priv = dev_get_priv(dev);
	int err;

	priv->dev = dev;
	priv->variant = (const struct amd_cpm_variant *)dev_get_driver_data(dev);
	if (!priv->variant)
		return -EINVAL;

	err = amd_cpm_pcie_parse_dt(priv);
	if (err) {
		dev_err(dev, "failed to parse DT: %d\n", err);
		return err;
	}

	err = amd_cpm_pcie_host_init(priv);
	if (err) {
		dev_err(dev, "failed to initialize host: %d\n", err);
		return err;
	}

	return 0;
}

static const struct amd_cpm_variant cpm_host = {
	.version = AMD_CPM_VER_1,
	.cpm_pcie_rst = AMD_CPM_PCIE0_RST,
};

static const struct amd_cpm_variant cpm5_host = {
	.version = AMD_CPM_VER_5_HOST0,
	.cpm_pcie_rst = AMD_CPM5_PCIE0_RST,
};

static const struct amd_cpm_variant cpm5_host1 = {
	.version = AMD_CPM_VER_5_HOST1,
	.cpm_pcie_rst = AMD_CPM5_PCIE1_RST,
};

static const struct udevice_id amd_cpm_pcie_of_match[] = {
	{
		.compatible = "xlnx,versal-cpm-host-1.00",
		.data = (ulong)&cpm_host,
	},
	{
		.compatible = "xlnx,versal-cpm5-host",
		.data = (ulong)&cpm5_host,
	},
	{
		.compatible = "xlnx,versal-cpm5-host1",
		.data = (ulong)&cpm5_host1,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(amd_cpm_pcie) = {
	.name = "amd_cpm_pcie",
	.id = UCLASS_PCI,
	.of_match = amd_cpm_pcie_of_match,
	.probe = amd_cpm_pcie_probe,
	.priv_auto = sizeof(struct amd_cpm_pcie),
	.ops = &amd_cpm_pcie_ops,
};
