// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Texas Instruments, Inc
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pci.h>
#include <generic-phy.h>
#include <power-domain.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>

DECLARE_GLOBAL_DATA_PTR;

#define PCIE_VENDORID_MASK	GENMASK(15, 0)
#define PCIE_DEVICEID_SHIFT	16

/* PCI DBICS registers */
#define PCIE_CONFIG_BAR0		0x10
#define PCIE_LINK_STATUS_REG		0x80
#define PCIE_LINK_STATUS_SPEED_OFF	16
#define PCIE_LINK_STATUS_SPEED_MASK	(0xf << PCIE_LINK_STATUS_SPEED_OFF)
#define PCIE_LINK_STATUS_WIDTH_OFF	20
#define PCIE_LINK_STATUS_WIDTH_MASK	(0xf << PCIE_LINK_STATUS_WIDTH_OFF)

#define PCIE_LINK_CAPABILITY		0x7c
#define PCIE_LINK_CTL_2			0xa0
#define TARGET_LINK_SPEED_MASK		0xf
#define LINK_SPEED_GEN_1		0x1
#define LINK_SPEED_GEN_2		0x2
#define LINK_SPEED_GEN_3		0x3

#define PCIE_MISC_CONTROL_1_OFF		0x8bc
#define PCIE_DBI_RO_WR_EN		BIT(0)

#define PLR_OFFSET			0x700
#define PCIE_PORT_DEBUG0		(PLR_OFFSET + 0x28)
#define PORT_LOGIC_LTSSM_STATE_MASK	0x1f
#define PORT_LOGIC_LTSSM_STATE_L0	0x11

#define PCIE_LINK_WIDTH_SPEED_CONTROL	0x80c
#define PORT_LOGIC_SPEED_CHANGE		(0x1 << 17)

#define PCIE_LINK_UP_TIMEOUT_MS		100

/*
 * iATU Unroll-specific register definitions
 * From 4.80 core version the address translation will be made by unroll.
 * The registers are offset from atu_base
 */
#define PCIE_ATU_UNR_REGION_CTRL1	0x00
#define PCIE_ATU_UNR_REGION_CTRL2	0x04
#define PCIE_ATU_UNR_LOWER_BASE		0x08
#define PCIE_ATU_UNR_UPPER_BASE		0x0c
#define PCIE_ATU_UNR_LIMIT		0x10
#define PCIE_ATU_UNR_LOWER_TARGET	0x14
#define PCIE_ATU_UNR_UPPER_TARGET	0x18

#define PCIE_ATU_REGION_INDEX1		(0x1 << 0)
#define PCIE_ATU_REGION_INDEX0		(0x0 << 0)
#define PCIE_ATU_TYPE_MEM		(0x0 << 0)
#define PCIE_ATU_TYPE_IO		(0x2 << 0)
#define PCIE_ATU_TYPE_CFG0		(0x4 << 0)
#define PCIE_ATU_TYPE_CFG1		(0x5 << 0)
#define PCIE_ATU_ENABLE			(0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE	(0x1 << 30)
#define PCIE_ATU_BUS(x)			(((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)			(((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)		(((x) & 0x7) << 16)

/* Register address builder */
#define PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(region)	((region) << 9)

/* Offsets from App base */
#define PCIE_CMD_STATUS			0x04
#define LTSSM_EN_VAL			BIT(0)

/* Parameters for the waiting for iATU enabled routine */
#define LINK_WAIT_MAX_IATU_RETRIES	5
#define LINK_WAIT_IATU			10000

#define AM654_PCIE_DEV_TYPE_MASK	0x3
#define EP				0x0
#define LEG_EP				0x1
#define RC				0x2

/**
 * struct pcie_dw_ti - TI DW PCIe controller state
 *
 * @app_base: The base address of application register space
 * @dbics_base: The base address of dbics register space
 * @cfg_base: The base address of configuration space
 * @atu_base: The base address of ATU space
 * @cfg_size: The size of the configuration space which is needed
 *            as it gets written into the PCIE_ATU_LIMIT register
 * @first_busno: This driver supports multiple PCIe controllers.
 *               first_busno stores the bus number of the PCIe root-port
 *               number which may vary depending on the PCIe setup
 *               (PEX switches etc).
 */
struct pcie_dw_ti {
	void *app_base;
	void *dbi_base;
	void *cfg_base;
	void *atu_base;
	fdt_size_t cfg_size;
	int first_busno;
	struct udevice *dev;

	/* IO and MEM PCI regions */
	struct pci_region io;
	struct pci_region mem;
};

enum dw_pcie_device_mode {
	DW_PCIE_UNKNOWN_TYPE,
	DW_PCIE_EP_TYPE,
	DW_PCIE_LEG_EP_TYPE,
	DW_PCIE_RC_TYPE,
};

static int pcie_dw_get_link_speed(struct pcie_dw_ti *pci)
{
	return (readl(pci->dbi_base + PCIE_LINK_STATUS_REG) &
		PCIE_LINK_STATUS_SPEED_MASK) >> PCIE_LINK_STATUS_SPEED_OFF;
}

static int pcie_dw_get_link_width(struct pcie_dw_ti *pci)
{
	return (readl(pci->dbi_base + PCIE_LINK_STATUS_REG) &
		PCIE_LINK_STATUS_WIDTH_MASK) >> PCIE_LINK_STATUS_WIDTH_OFF;
}

static void dw_pcie_writel_ob_unroll(struct pcie_dw_ti *pci, u32 index, u32 reg,
				     u32 val)
{
	u32 offset = PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(index);
	void __iomem *base = pci->atu_base;

	writel(val, base + offset + reg);
}

static u32 dw_pcie_readl_ob_unroll(struct pcie_dw_ti *pci, u32 index, u32 reg)
{
	u32 offset = PCIE_GET_ATU_OUTB_UNR_REG_OFFSET(index);
	void __iomem *base = pci->atu_base;

	return readl(base + offset + reg);
}

/**
 * pcie_dw_prog_outbound_atu_unroll() - Configure ATU for outbound accesses
 *
 * @pcie: Pointer to the PCI controller state
 * @index: ATU region index
 * @type: ATU accsess type
 * @cpu_addr: the physical address for the translation entry
 * @pci_addr: the pcie bus address for the translation entry
 * @size: the size of the translation entry
 */
static void pcie_dw_prog_outbound_atu_unroll(struct pcie_dw_ti *pci, int index,
					     int type, u64 cpu_addr,
					     u64 pci_addr, u32 size)
{
	u32 retries, val;

	debug("ATU programmed with: index: %d, type: %d, cpu addr: %8llx, pci addr: %8llx, size: %8x\n",
	      index, type, cpu_addr, pci_addr, size);

	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_LOWER_BASE,
				 lower_32_bits(cpu_addr));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_UPPER_BASE,
				 upper_32_bits(cpu_addr));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_LIMIT,
				 lower_32_bits(cpu_addr + size - 1));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_LOWER_TARGET,
				 lower_32_bits(pci_addr));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_UPPER_TARGET,
				 upper_32_bits(pci_addr));
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_REGION_CTRL1,
				 type);
	dw_pcie_writel_ob_unroll(pci, index, PCIE_ATU_UNR_REGION_CTRL2,
				 PCIE_ATU_ENABLE);

	/*
	 * Make sure ATU enable takes effect before any subsequent config
	 * and I/O accesses.
	 */
	for (retries = 0; retries < LINK_WAIT_MAX_IATU_RETRIES; retries++) {
		val = dw_pcie_readl_ob_unroll(pci, index,
					      PCIE_ATU_UNR_REGION_CTRL2);
		if (val & PCIE_ATU_ENABLE)
			return;

		udelay(LINK_WAIT_IATU);
	}
	dev_err(pci->dev, "outbound iATU is not being enabled\n");
}

/**
 * set_cfg_address() - Configure the PCIe controller config space access
 *
 * @pcie: Pointer to the PCI controller state
 * @d: PCI device to access
 * @where: Offset in the configuration space
 *
 * Configures the PCIe controller to access the configuration space of
 * a specific PCIe device and returns the address to use for this
 * access.
 *
 * Return: Address that can be used to access the configation space
 *         of the requested device / offset
 */
static uintptr_t set_cfg_address(struct pcie_dw_ti *pcie,
				 pci_dev_t d, uint where)
{
	int bus = PCI_BUS(d) - pcie->first_busno;
	uintptr_t va_address;
	u32 atu_type;

	/* Use dbi_base for own configuration read and write */
	if (!bus) {
		va_address = (uintptr_t)pcie->dbi_base;
		goto out;
	}

	if (bus == 1)
		/* For local bus, change TLP Type field to 4. */
		atu_type = PCIE_ATU_TYPE_CFG0;
	else
		/* Otherwise, change TLP Type field to 5. */
		atu_type = PCIE_ATU_TYPE_CFG1;

	/*
	 * Not accessing root port configuration space?
	 * Region #0 is used for Outbound CFG space access.
	 * Direction = Outbound
	 * Region Index = 0
	 */
	d = PCI_MASK_BUS(d);
	d = PCI_ADD_BUS(bus, d);
	pcie_dw_prog_outbound_atu_unroll(pcie, PCIE_ATU_REGION_INDEX1,
					 atu_type, (u64)pcie->cfg_base,
					 d << 8, pcie->cfg_size);

	va_address = (uintptr_t)pcie->cfg_base;

out:
	va_address += where & ~0x3;

	return va_address;
}

/**
 * pcie_dw_addr_valid() - Check for valid bus address
 *
 * @d: The PCI device to access
 * @first_busno: Bus number of the PCIe controller root complex
 *
 * Return 1 (true) if the PCI device can be accessed by this controller.
 *
 * Return: 1 on valid, 0 on invalid
 */
static int pcie_dw_addr_valid(pci_dev_t d, int first_busno)
{
	if ((PCI_BUS(d) == first_busno) && (PCI_DEV(d) > 0))
		return 0;
	if ((PCI_BUS(d) == first_busno + 1) && (PCI_DEV(d) > 0))
		return 0;

	return 1;
}

/**
 * pcie_dw_ti_read_config() - Read from configuration space
 *
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @valuep: A pointer at which to store the read value
 * @size: Indicates the size of access to perform
 *
 * Read a value of size @size from offset @offset within the configuration
 * space of the device identified by the bus, device & function numbers in @bdf
 * on the PCI bus @bus.
 *
 * Return: 0 on success
 */
static int pcie_dw_ti_read_config(const struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong *valuep,
				  enum pci_size_t size)
{
	struct pcie_dw_ti *pcie = dev_get_priv(bus);
	uintptr_t va_address;
	ulong value;

	debug("PCIE CFG read: bdf=%2x:%2x:%2x ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	if (!pcie_dw_addr_valid(bdf, pcie->first_busno)) {
		debug("- out of range\n");
		*valuep = pci_get_ff(size);
		return 0;
	}

	va_address = set_cfg_address(pcie, bdf, offset);

	value = readl(va_address);

	debug("(addr,val)=(0x%04x, 0x%08lx)\n", offset, value);
	*valuep = pci_conv_32_to_size(value, offset, size);

	pcie_dw_prog_outbound_atu_unroll(pcie, PCIE_ATU_REGION_INDEX1,
					 PCIE_ATU_TYPE_IO, pcie->io.phys_start,
					 pcie->io.bus_start, pcie->io.size);

	return 0;
}

/**
 * pcie_dw_ti_write_config() - Write to configuration space
 *
 * @bus: Pointer to the PCI bus
 * @bdf: Identifies the PCIe device to access
 * @offset: The offset into the device's configuration space
 * @value: The value to write
 * @size: Indicates the size of access to perform
 *
 * Write the value @value of size @size from offset @offset within the
 * configuration space of the device identified by the bus, device & function
 * numbers in @bdf on the PCI bus @bus.
 *
 * Return: 0 on success
 */
static int pcie_dw_ti_write_config(struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong value,
				   enum pci_size_t size)
{
	struct pcie_dw_ti *pcie = dev_get_priv(bus);
	uintptr_t va_address;
	ulong old;

	debug("PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	debug("(addr,val)=(0x%04x, 0x%08lx)\n", offset, value);

	if (!pcie_dw_addr_valid(bdf, pcie->first_busno)) {
		debug("- out of range\n");
		return 0;
	}

	va_address = set_cfg_address(pcie, bdf, offset);

	old = readl(va_address);
	value = pci_conv_size_to_32(old, value, offset, size);
	writel(value, va_address);

	pcie_dw_prog_outbound_atu_unroll(pcie, PCIE_ATU_REGION_INDEX1,
					 PCIE_ATU_TYPE_IO, pcie->io.phys_start,
					 pcie->io.bus_start, pcie->io.size);

	return 0;
}

static inline void dw_pcie_dbi_write_enable(struct pcie_dw_ti *pci, bool en)
{
	u32 val;

	val = readl(pci->dbi_base + PCIE_MISC_CONTROL_1_OFF);
	if (en)
		val |= PCIE_DBI_RO_WR_EN;
	else
		val &= ~PCIE_DBI_RO_WR_EN;
	writel(val, pci->dbi_base + PCIE_MISC_CONTROL_1_OFF);
}

/**
 * pcie_dw_configure() - Configure link capabilities and speed
 *
 * @regs_base: A pointer to the PCIe controller registers
 * @cap_speed: The capabilities and speed to configure
 *
 * Configure the link capabilities and speed in the PCIe root complex.
 */
static void pcie_dw_configure(struct pcie_dw_ti *pci, u32 cap_speed)
{
	u32 val;

	dw_pcie_dbi_write_enable(pci, true);

	val = readl(pci->dbi_base + PCIE_LINK_CAPABILITY);
	val &= ~TARGET_LINK_SPEED_MASK;
	val |= cap_speed;
	writel(val, pci->dbi_base + PCIE_LINK_CAPABILITY);

	val = readl(pci->dbi_base + PCIE_LINK_CTL_2);
	val &= ~TARGET_LINK_SPEED_MASK;
	val |= cap_speed;
	writel(val, pci->dbi_base + PCIE_LINK_CTL_2);

	dw_pcie_dbi_write_enable(pci, false);
}

/**
 * is_link_up() - Return the link state
 *
 * @regs_base: A pointer to the PCIe DBICS registers
 *
 * Return: 1 (true) for active line and 0 (false) for no link
 */
static int is_link_up(struct pcie_dw_ti *pci)
{
	u32 val;

	val = readl(pci->dbi_base + PCIE_PORT_DEBUG0);
	val &= PORT_LOGIC_LTSSM_STATE_MASK;

	return (val == PORT_LOGIC_LTSSM_STATE_L0);
}

/**
 * wait_link_up() - Wait for the link to come up
 *
 * @regs_base: A pointer to the PCIe controller registers
 *
 * Return: 1 (true) for active line and 0 (false) for no link (timeout)
 */
static int wait_link_up(struct pcie_dw_ti *pci)
{
	unsigned long timeout;

	timeout = get_timer(0) + PCIE_LINK_UP_TIMEOUT_MS;
	while (!is_link_up(pci)) {
		if (get_timer(0) > timeout)
			return 0;
	};

	return 1;
}

static int pcie_dw_ti_pcie_link_up(struct pcie_dw_ti *pci, u32 cap_speed)
{
	u32 val;

	if (is_link_up(pci)) {
		printf("PCI Link already up before configuration!\n");
		return 1;
	}

	/* DW pre link configurations */
	pcie_dw_configure(pci, cap_speed);

	/* Initiate link training */
	val = readl(pci->app_base + PCIE_CMD_STATUS);
	val |= LTSSM_EN_VAL;
	writel(val, pci->app_base + PCIE_CMD_STATUS);

	/* Check that link was established */
	if (!wait_link_up(pci))
		return 0;

	/*
	 * Link can be established in Gen 1. still need to wait
	 * till MAC nagaotiation is completed
	 */
	udelay(100);

	return 1;
}

/**
 * pcie_dw_setup_host() - Setup the PCIe controller for RC opertaion
 *
 * @pcie: Pointer to the PCI controller state
 *
 * Configure the host BARs of the PCIe controller root port so that
 * PCI(e) devices may access the system memory.
 */
static void pcie_dw_setup_host(struct pcie_dw_ti *pci)
{
	u32 val;

	/* setup RC BARs */
	writel(PCI_BASE_ADDRESS_MEM_TYPE_64,
	       pci->dbi_base + PCI_BASE_ADDRESS_0);
	writel(0x0, pci->dbi_base + PCI_BASE_ADDRESS_1);

	/* setup interrupt pins */
	dw_pcie_dbi_write_enable(pci, true);
	val = readl(pci->dbi_base + PCI_INTERRUPT_LINE);
	val &= 0xffff00ff;
	val |= 0x00000100;
	writel(val, pci->dbi_base + PCI_INTERRUPT_LINE);
	dw_pcie_dbi_write_enable(pci, false);

	/* setup bus numbers */
	val = readl(pci->dbi_base + PCI_PRIMARY_BUS);
	val &= 0xff000000;
	val |= 0x00ff0100;
	writel(val, pci->dbi_base + PCI_PRIMARY_BUS);

	/* setup command register */
	val = readl(pci->dbi_base + PCI_COMMAND);
	val &= 0xffff0000;
	val |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
		PCI_COMMAND_MASTER | PCI_COMMAND_SERR;
	writel(val, pci->dbi_base + PCI_COMMAND);

	/* Enable write permission for the DBI read-only register */
	dw_pcie_dbi_write_enable(pci, true);
	/* program correct class for RC */
	writew(PCI_CLASS_BRIDGE_PCI, pci->dbi_base + PCI_CLASS_DEVICE);
	/* Better disable write permission right after the update */
	dw_pcie_dbi_write_enable(pci, false);

	val = readl(pci->dbi_base + PCIE_LINK_WIDTH_SPEED_CONTROL);
	val |= PORT_LOGIC_SPEED_CHANGE;
	writel(val, pci->dbi_base + PCIE_LINK_WIDTH_SPEED_CONTROL);
}

static int pcie_am654_set_mode(struct pcie_dw_ti *pci,
			       enum dw_pcie_device_mode mode)
{
	struct regmap *syscon;
	u32 val;
	u32 mask;
	int ret;

	syscon = syscon_regmap_lookup_by_phandle(pci->dev,
						 "ti,syscon-pcie-mode");
	if (IS_ERR(syscon))
		return 0;

	mask = AM654_PCIE_DEV_TYPE_MASK;

	switch (mode) {
	case DW_PCIE_RC_TYPE:
		val = RC;
		break;
	case DW_PCIE_EP_TYPE:
		val = EP;
		break;
	default:
		dev_err(pci->dev, "INVALID device type %d\n", mode);
		return -EINVAL;
	}

	ret = regmap_update_bits(syscon, 0, mask, val);
	if (ret) {
		dev_err(pci->dev, "failed to set pcie mode\n");
		return ret;
	}

	return 0;
}

static int pcie_dw_init_id(struct pcie_dw_ti *pci)
{
	struct regmap *devctrl_regs;
	unsigned int id;
	int ret;

	devctrl_regs = syscon_regmap_lookup_by_phandle(pci->dev,
						       "ti,syscon-pcie-id");
	if (IS_ERR(devctrl_regs))
		return PTR_ERR(devctrl_regs);

	ret = regmap_read(devctrl_regs, 0, &id);
	if (ret)
		return ret;

	dw_pcie_dbi_write_enable(pci, true);
	writew(id & PCIE_VENDORID_MASK, pci->dbi_base + PCI_VENDOR_ID);
	writew(id >> PCIE_DEVICEID_SHIFT, pci->dbi_base + PCI_DEVICE_ID);
	dw_pcie_dbi_write_enable(pci, false);

	return 0;
}

/**
 * pcie_dw_ti_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int pcie_dw_ti_probe(struct udevice *dev)
{
	struct pcie_dw_ti *pci = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	struct power_domain pci_pwrdmn;
	struct phy phy0, phy1;
	int ret;

	ret = power_domain_get_by_index(dev, &pci_pwrdmn, 0);
	if (ret) {
		dev_err(dev, "failed to get power domain\n");
		return ret;
	}

	ret = power_domain_on(&pci_pwrdmn);
	if (ret) {
		dev_err(dev, "Power domain on failed\n");
		return ret;
	}

	ret = generic_phy_get_by_name(dev,  "pcie-phy0", &phy0);
	if (ret) {
		dev_err(dev, "Unable to get phy0");
		return ret;
	}
	generic_phy_reset(&phy0);
	generic_phy_init(&phy0);
	generic_phy_power_on(&phy0);

	ret = generic_phy_get_by_name(dev,  "pcie-phy1", &phy1);
	if (ret) {
		dev_err(dev, "Unable to get phy1");
		return ret;
	}
	generic_phy_reset(&phy1);
	generic_phy_init(&phy1);
	generic_phy_power_on(&phy1);

	pci->first_busno = dev_seq(dev);
	pci->dev = dev;

	pcie_dw_setup_host(pci);
	pcie_dw_init_id(pci);

	if (device_is_compatible(dev, "ti,am654-pcie-rc"))
		pcie_am654_set_mode(pci, DW_PCIE_RC_TYPE);

	if (!pcie_dw_ti_pcie_link_up(pci, LINK_SPEED_GEN_2)) {
		printf("PCIE-%d: Link down\n", dev_seq(dev));
		return -ENODEV;
	}

	printf("PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n", dev_seq(dev),
	       pcie_dw_get_link_speed(pci),
	       pcie_dw_get_link_width(pci),
	       hose->first_busno);

	/* Store the IO and MEM windows settings for future use by the ATU */
	pci->io.phys_start = hose->regions[0].phys_start; /* IO base */
	pci->io.bus_start  = hose->regions[0].bus_start;  /* IO_bus_addr */
	pci->io.size	    = hose->regions[0].size;	   /* IO size */

	pci->mem.phys_start = hose->regions[1].phys_start; /* MEM base */
	pci->mem.bus_start  = hose->regions[1].bus_start;  /* MEM_bus_addr */
	pci->mem.size	     = hose->regions[1].size;	    /* MEM size */

	pcie_dw_prog_outbound_atu_unroll(pci, PCIE_ATU_REGION_INDEX0,
					 PCIE_ATU_TYPE_MEM,
					 pci->mem.phys_start,
					 pci->mem.bus_start, pci->mem.size);

	return 0;
}

/**
 * pcie_dw_ti_of_to_plat() - Translate from DT to device state
 *
 * @dev: A pointer to the device being operated on
 *
 * Translate relevant data from the device tree pertaining to device @dev into
 * state that the driver will later make use of. This state is stored in the
 * device's private data structure.
 *
 * Return: 0 on success, else -EINVAL
 */
static int pcie_dw_ti_of_to_plat(struct udevice *dev)
{
	struct pcie_dw_ti *pcie = dev_get_priv(dev);

	/* Get the controller base address */
	pcie->dbi_base = (void *)dev_read_addr_name(dev, "dbics");
	if ((fdt_addr_t)pcie->dbi_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the config space base address and size */
	pcie->cfg_base = (void *)dev_read_addr_size_name(dev, "config",
							 &pcie->cfg_size);
	if ((fdt_addr_t)pcie->cfg_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the iATU base address and size */
	pcie->atu_base = (void *)dev_read_addr_name(dev, "atu");
	if ((fdt_addr_t)pcie->atu_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Get the app base address and size */
	pcie->app_base = (void *)dev_read_addr_name(dev, "app");
	if ((fdt_addr_t)pcie->app_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops pcie_dw_ti_ops = {
	.read_config	= pcie_dw_ti_read_config,
	.write_config	= pcie_dw_ti_write_config,
};

static const struct udevice_id pcie_dw_ti_ids[] = {
	{ .compatible = "ti,am654-pcie-rc" },
	{ }
};

U_BOOT_DRIVER(pcie_dw_ti) = {
	.name			= "pcie_dw_ti",
	.id			= UCLASS_PCI,
	.of_match		= pcie_dw_ti_ids,
	.ops			= &pcie_dw_ti_ops,
	.of_to_plat	= pcie_dw_ti_of_to_plat,
	.probe			= pcie_dw_ti_probe,
	.priv_auto	= sizeof(struct pcie_dw_ti),
};
