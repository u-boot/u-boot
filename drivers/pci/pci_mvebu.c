// SPDX-License-Identifier: GPL-2.0
/*
 * PCIe driver for Marvell MVEBU SoCs
 *
 * Based on Barebox drivers/pci/pci-mvebu.c
 *
 * Ported to U-Boot by:
 * Anton Schubert <anton.schubert@gmx.de>
 * Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/mbus.h>

DECLARE_GLOBAL_DATA_PTR;

/* PCIe unit register offsets */
#define SELECT(x, n)			((x >> n) & 1UL)

#define PCIE_DEV_ID_OFF			0x0000
#define PCIE_CMD_OFF			0x0004
#define PCIE_DEV_REV_OFF		0x0008
#define  PCIE_BAR_LO_OFF(n)		(0x0010 + ((n) << 3))
#define  PCIE_BAR_HI_OFF(n)		(0x0014 + ((n) << 3))
#define PCIE_EXP_ROM_BAR_OFF		0x0030
#define PCIE_CAPAB_OFF			0x0060
#define PCIE_CTRL_STAT_OFF		0x0068
#define PCIE_HEADER_LOG_4_OFF		0x0128
#define  PCIE_BAR_CTRL_OFF(n)		(0x1804 + (((n) - 1) * 4))
#define  PCIE_WIN04_CTRL_OFF(n)		(0x1820 + ((n) << 4))
#define  PCIE_WIN04_BASE_OFF(n)		(0x1824 + ((n) << 4))
#define  PCIE_WIN04_REMAP_OFF(n)	(0x182c + ((n) << 4))
#define PCIE_WIN5_CTRL_OFF		0x1880
#define PCIE_WIN5_BASE_OFF		0x1884
#define PCIE_WIN5_REMAP_OFF		0x188c
#define PCIE_CONF_ADDR_OFF		0x18f8
#define  PCIE_CONF_ADDR_EN		BIT(31)
#define  PCIE_CONF_REG(r)		((((r) & 0xf00) << 16) | ((r) & 0xfc))
#define  PCIE_CONF_BUS(b)		(((b) & 0xff) << 16)
#define  PCIE_CONF_DEV(d)		(((d) & 0x1f) << 11)
#define  PCIE_CONF_FUNC(f)		(((f) & 0x7) << 8)
#define  PCIE_CONF_ADDR(b, d, f, reg) \
	(PCIE_CONF_BUS(b) | PCIE_CONF_DEV(d)    | \
	 PCIE_CONF_FUNC(f) | PCIE_CONF_REG(reg) | \
	 PCIE_CONF_ADDR_EN)
#define PCIE_CONF_DATA_OFF		0x18fc
#define PCIE_MASK_OFF			0x1910
#define  PCIE_MASK_ENABLE_INTS          (0xf << 24)
#define PCIE_CTRL_OFF			0x1a00
#define  PCIE_CTRL_X1_MODE		BIT(0)
#define  PCIE_CTRL_RC_MODE		BIT(1)
#define PCIE_STAT_OFF			0x1a04
#define  PCIE_STAT_BUS                  (0xff << 8)
#define  PCIE_STAT_DEV                  (0x1f << 16)
#define  PCIE_STAT_LINK_DOWN		BIT(0)
#define PCIE_DEBUG_CTRL			0x1a60
#define  PCIE_DEBUG_SOFT_RESET		BIT(20)

struct mvebu_pcie {
	struct pci_controller hose;
	void __iomem *base;
	void __iomem *membase;
	struct resource mem;
	void __iomem *iobase;
	struct resource io;
	u32 port;
	u32 lane;
	int devfn;
	u32 lane_mask;
	int first_busno;
	int sec_busno;
	char name[16];
	unsigned int mem_target;
	unsigned int mem_attr;
	unsigned int io_target;
	unsigned int io_attr;
	u32 cfgcache[(0x3c - 0x10) / 4];
};

/*
 * MVEBU PCIe controller needs MEMORY and I/O BARs to be mapped
 * into SoCs address space. Each controller will map 128M of MEM
 * and 64K of I/O space when registered.
 */
static void __iomem *mvebu_pcie_membase = (void __iomem *)MBUS_PCI_MEM_BASE;
static void __iomem *mvebu_pcie_iobase = (void __iomem *)MBUS_PCI_IO_BASE;

static inline bool mvebu_pcie_link_up(struct mvebu_pcie *pcie)
{
	u32 val;
	val = readl(pcie->base + PCIE_STAT_OFF);
	return !(val & PCIE_STAT_LINK_DOWN);
}

static void mvebu_pcie_set_local_bus_nr(struct mvebu_pcie *pcie, int busno)
{
	u32 stat;

	stat = readl(pcie->base + PCIE_STAT_OFF);
	stat &= ~PCIE_STAT_BUS;
	stat |= busno << 8;
	writel(stat, pcie->base + PCIE_STAT_OFF);
}

static void mvebu_pcie_set_local_dev_nr(struct mvebu_pcie *pcie, int devno)
{
	u32 stat;

	stat = readl(pcie->base + PCIE_STAT_OFF);
	stat &= ~PCIE_STAT_DEV;
	stat |= devno << 16;
	writel(stat, pcie->base + PCIE_STAT_OFF);
}

static inline struct mvebu_pcie *hose_to_pcie(struct pci_controller *hose)
{
	return container_of(hose, struct mvebu_pcie, hose);
}

static bool mvebu_pcie_valid_addr(struct mvebu_pcie *pcie,
				  int busno, int dev, int func)
{
	/* On primary bus is only one PCI Bridge */
	if (busno == pcie->first_busno && (dev != 0 || func != 0))
		return false;

	/* Access to other buses is possible when link is up */
	if (busno != pcie->first_busno && !mvebu_pcie_link_up(pcie))
		return false;

	/* On secondary bus can be only one PCIe device */
	if (busno == pcie->sec_busno && dev != 0)
		return false;

	return true;
}

static int mvebu_pcie_read_config(const struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong *valuep,
				  enum pci_size_t size)
{
	struct mvebu_pcie *pcie = dev_get_plat(bus);
	int busno = PCI_BUS(bdf) - dev_seq(bus);
	u32 addr, data;

	debug("PCIE CFG read: (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));

	if (!mvebu_pcie_valid_addr(pcie, busno, PCI_DEV(bdf), PCI_FUNC(bdf))) {
		debug("- out of range\n");
		*valuep = pci_get_ff(size);
		return 0;
	}

	/*
	 * The configuration space of the PCI Bridge on primary (first) bus is
	 * of Type 0 but the BAR registers (including ROM BAR) don't have the
	 * same meaning as in the PCIe specification. Therefore do not access
	 * BAR registers and non-common registers (those which have different
	 * meaning for Type 0 and Type 1 config space) of the PCI Bridge and
	 * instead read their content from driver virtual cfgcache[].
	 */
	if (busno == pcie->first_busno && ((offset >= 0x10 && offset < 0x34) ||
					   (offset >= 0x38 && offset < 0x3c))) {
		data = pcie->cfgcache[(offset - 0x10) / 4];
		debug("(addr,size,val)=(0x%04x, %d, 0x%08x) from cfgcache\n",
		      offset, size, data);
		*valuep = pci_conv_32_to_size(data, offset, size);
		return 0;
	}

	/*
	 * PCI bridge is device 0 at primary bus but mvebu has it mapped on
	 * secondary bus with device number 1.
	 */
	if (busno == pcie->first_busno)
		addr = PCIE_CONF_ADDR(pcie->sec_busno, 1, 0, offset);
	else
		addr = PCIE_CONF_ADDR(busno, PCI_DEV(bdf), PCI_FUNC(bdf), offset);

	/* write address */
	writel(addr, pcie->base + PCIE_CONF_ADDR_OFF);

	/* read data */
	switch (size) {
	case PCI_SIZE_8:
		data = readb(pcie->base + PCIE_CONF_DATA_OFF + (offset & 3));
		break;
	case PCI_SIZE_16:
		data = readw(pcie->base + PCIE_CONF_DATA_OFF + (offset & 2));
		break;
	case PCI_SIZE_32:
		data = readl(pcie->base + PCIE_CONF_DATA_OFF);
		break;
	default:
		return -EINVAL;
	}

	if (busno == pcie->first_busno &&
	    (offset & ~3) == (PCI_HEADER_TYPE & ~3)) {
		/*
		 * Change Header Type of PCI Bridge device to Type 1
		 * (0x01, used by PCI Bridges) because mvebu reports
		 * Type 0 (0x00, used by Upstream and Endpoint devices).
		 */
		data = pci_conv_size_to_32(data, 0, offset, size);
		data &= ~0x007f0000;
		data |= PCI_HEADER_TYPE_BRIDGE << 16;
		data = pci_conv_32_to_size(data, offset, size);
	}

	debug("(addr,size,val)=(0x%04x, %d, 0x%08x)\n", offset, size, data);
	*valuep = data;

	return 0;
}

static int mvebu_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong value,
				   enum pci_size_t size)
{
	struct mvebu_pcie *pcie = dev_get_plat(bus);
	int busno = PCI_BUS(bdf) - dev_seq(bus);
	u32 addr, data;

	debug("PCIE CFG write: (b,d,f)=(%2d,%2d,%2d) ",
	      PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf));
	debug("(addr,size,val)=(0x%04x, %d, 0x%08lx)\n", offset, size, value);

	if (!mvebu_pcie_valid_addr(pcie, busno, PCI_DEV(bdf), PCI_FUNC(bdf))) {
		debug("- out of range\n");
		return 0;
	}

	/*
	 * As explained in mvebu_pcie_read_config(), PCI Bridge Type 1 specific
	 * config registers are not available, so we write their content only
	 * into driver virtual cfgcache[].
	 * And as explained in mvebu_pcie_probe(), mvebu has its own specific
	 * way for configuring primary and secondary bus numbers.
	 */
	if (busno == pcie->first_busno && ((offset >= 0x10 && offset < 0x34) ||
					   (offset >= 0x38 && offset < 0x3c))) {
		debug("Writing to cfgcache only\n");
		data = pcie->cfgcache[(offset - 0x10) / 4];
		data = pci_conv_size_to_32(data, value, offset, size);
		/* mvebu PCI bridge does not have configurable bars */
		if ((offset & ~3) == PCI_BASE_ADDRESS_0 ||
		    (offset & ~3) == PCI_BASE_ADDRESS_1 ||
		    (offset & ~3) == PCI_ROM_ADDRESS1)
			data = 0x0;
		pcie->cfgcache[(offset - 0x10) / 4] = data;
		/* mvebu has its own way how to set PCI primary bus number */
		if (offset == PCI_PRIMARY_BUS) {
			pcie->first_busno = data & 0xff;
			debug("Primary bus number was changed to %d\n",
			      pcie->first_busno);
		}
		/* mvebu has its own way how to set PCI secondary bus number */
		if (offset == PCI_SECONDARY_BUS ||
		    (offset == PCI_PRIMARY_BUS && size != PCI_SIZE_8)) {
			pcie->sec_busno = (data >> 8) & 0xff;
			mvebu_pcie_set_local_bus_nr(pcie, pcie->sec_busno);
			debug("Secondary bus number was changed to %d\n",
			      pcie->sec_busno);
		}
		return 0;
	}

	/*
	 * PCI bridge is device 0 at primary bus but mvebu has it mapped on
	 * secondary bus with device number 1.
	 */
	if (busno == pcie->first_busno)
		addr = PCIE_CONF_ADDR(pcie->sec_busno, 1, 0, offset);
	else
		addr = PCIE_CONF_ADDR(busno, PCI_DEV(bdf), PCI_FUNC(bdf), offset);

	/* write address */
	writel(addr, pcie->base + PCIE_CONF_ADDR_OFF);

	/* write data */
	switch (size) {
	case PCI_SIZE_8:
		writeb(value, pcie->base + PCIE_CONF_DATA_OFF + (offset & 3));
		break;
	case PCI_SIZE_16:
		writew(value, pcie->base + PCIE_CONF_DATA_OFF + (offset & 2));
		break;
	case PCI_SIZE_32:
		writel(value, pcie->base + PCIE_CONF_DATA_OFF);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/*
 * Setup PCIE BARs and Address Decode Wins:
 * BAR[0,2] -> disabled, BAR[1] -> covers all DRAM banks
 * WIN[0-3] -> DRAM bank[0-3]
 */
static void mvebu_pcie_setup_wins(struct mvebu_pcie *pcie)
{
	const struct mbus_dram_target_info *dram = mvebu_mbus_dram_info();
	u32 size;
	int i;

	/* First, disable and clear BARs and windows. */
	for (i = 1; i < 3; i++) {
		writel(0, pcie->base + PCIE_BAR_CTRL_OFF(i));
		writel(0, pcie->base + PCIE_BAR_LO_OFF(i));
		writel(0, pcie->base + PCIE_BAR_HI_OFF(i));
	}

	for (i = 0; i < 5; i++) {
		writel(0, pcie->base + PCIE_WIN04_CTRL_OFF(i));
		writel(0, pcie->base + PCIE_WIN04_BASE_OFF(i));
		writel(0, pcie->base + PCIE_WIN04_REMAP_OFF(i));
	}

	writel(0, pcie->base + PCIE_WIN5_CTRL_OFF);
	writel(0, pcie->base + PCIE_WIN5_BASE_OFF);
	writel(0, pcie->base + PCIE_WIN5_REMAP_OFF);

	/* Setup windows for DDR banks. Count total DDR size on the fly. */
	size = 0;
	for (i = 0; i < dram->num_cs; i++) {
		const struct mbus_dram_window *cs = dram->cs + i;

		writel(cs->base & 0xffff0000,
		       pcie->base + PCIE_WIN04_BASE_OFF(i));
		writel(0, pcie->base + PCIE_WIN04_REMAP_OFF(i));
		writel(((cs->size - 1) & 0xffff0000) |
		       (cs->mbus_attr << 8) |
		       (dram->mbus_dram_target_id << 4) | 1,
		       pcie->base + PCIE_WIN04_CTRL_OFF(i));

		size += cs->size;
	}

	/* Round up 'size' to the nearest power of two. */
	if ((size & (size - 1)) != 0)
		size = 1 << fls(size);

	/* Setup BAR[1] to all DRAM banks. */
	writel(dram->cs[0].base | 0xc, pcie->base + PCIE_BAR_LO_OFF(1));
	writel(0, pcie->base + PCIE_BAR_HI_OFF(1));
	writel(((size - 1) & 0xffff0000) | 0x1,
	       pcie->base + PCIE_BAR_CTRL_OFF(1));
}

static int mvebu_pcie_probe(struct udevice *dev)
{
	struct mvebu_pcie *pcie = dev_get_plat(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	u32 reg;

	/* Setup PCIe controller to Root Complex mode */
	reg = readl(pcie->base + PCIE_CTRL_OFF);
	reg |= PCIE_CTRL_RC_MODE;
	writel(reg, pcie->base + PCIE_CTRL_OFF);

	/*
	 * Change Class Code of PCI Bridge device to PCI Bridge (0x600400)
	 * because default value is Memory controller (0x508000) which
	 * U-Boot cannot recognize as P2P Bridge.
	 *
	 * Note that this mvebu PCI Bridge does not have compliant Type 1
	 * Configuration Space. Header Type is reported as Type 0 and it
	 * has format of Type 0 config space.
	 *
	 * Moreover Type 0 BAR registers (ranges 0x10 - 0x28 and 0x30 - 0x34)
	 * have the same format in Marvell's specification as in PCIe
	 * specification, but their meaning is totally different and they do
	 * different things: they are aliased into internal mvebu registers
	 * (e.g. PCIE_BAR_LO_OFF) and these should not be changed or
	 * reconfigured by pci device drivers.
	 *
	 * So our driver converts Type 0 config space to Type 1 and reports
	 * Header Type as Type 1. Access to BAR registers and to non-existent
	 * Type 1 registers is redirected to the virtual cfgcache[] buffer,
	 * which avoids changing unrelated registers.
	 */
	reg = readl(pcie->base + PCIE_DEV_REV_OFF);
	reg &= ~0xffffff00;
	reg |= (PCI_CLASS_BRIDGE_PCI << 8) << 8;
	writel(reg, pcie->base + PCIE_DEV_REV_OFF);

	/*
	 * mvebu uses local bus number and local device number to determinate
	 * type of config request. Type 0 is used if target bus number equals
	 * local bus number and target device number differs from local device
	 * number. Type 1 is used if target bus number differs from local bus
	 * number. And when target bus number equals local bus number and
	 * target device equals local device number then request is routed to
	 * PCI Bridge which represent local PCIe Root Port.
	 *
	 * It means that PCI primary and secondary buses shares one bus number
	 * which is configured via local bus number. Determination if config
	 * request should go to primary or secondary bus is done based on local
	 * device number.
	 *
	 * PCIe is point-to-point bus, so at secondary bus is always exactly one
	 * device with number 0. So set local device number to 1, it would not
	 * conflict with any device on secondary bus number and will ensure that
	 * accessing secondary bus and all buses behind secondary would work
	 * automatically and correctly. Therefore this configuration of local
	 * device number implies that setting of local bus number configures
	 * secondary bus number. Set it to 0 as U-Boot CONFIG_PCI_PNP code will
	 * later configure it via config write requests to the correct value.
	 * mvebu_pcie_write_config() catches config write requests which tries
	 * to change primary/secondary bus number and correctly updates local
	 * bus number based on new secondary bus number.
	 *
	 * With this configuration is PCI Bridge available at secondary bus as
	 * device number 1. But it must be available at primary bus as device
	 * number 0. So in mvebu_pcie_read_config() and mvebu_pcie_write_config()
	 * functions rewrite address to the real one when accessing primary bus.
	 */
	mvebu_pcie_set_local_bus_nr(pcie, 0);
	mvebu_pcie_set_local_dev_nr(pcie, 1);

	pcie->mem.start = (u32)mvebu_pcie_membase;
	pcie->mem.end = pcie->mem.start + MBUS_PCI_MEM_SIZE - 1;
	mvebu_pcie_membase += MBUS_PCI_MEM_SIZE;

	if (mvebu_mbus_add_window_by_id(pcie->mem_target, pcie->mem_attr,
					(phys_addr_t)pcie->mem.start,
					MBUS_PCI_MEM_SIZE)) {
		printf("PCIe unable to add mbus window for mem at %08x+%08x\n",
		       (u32)pcie->mem.start, MBUS_PCI_MEM_SIZE);
	}

	pcie->io.start = (u32)mvebu_pcie_iobase;
	pcie->io.end = pcie->io.start + MBUS_PCI_IO_SIZE - 1;
	mvebu_pcie_iobase += MBUS_PCI_IO_SIZE;

	if (mvebu_mbus_add_window_by_id(pcie->io_target, pcie->io_attr,
					(phys_addr_t)pcie->io.start,
					MBUS_PCI_IO_SIZE)) {
		printf("PCIe unable to add mbus window for IO at %08x+%08x\n",
		       (u32)pcie->io.start, MBUS_PCI_IO_SIZE);
	}

	/* Setup windows and configure host bridge */
	mvebu_pcie_setup_wins(pcie);

	/* PCI memory space */
	pci_set_region(hose->regions + 0, pcie->mem.start,
		       pcie->mem.start, MBUS_PCI_MEM_SIZE, PCI_REGION_MEM);
	pci_set_region(hose->regions + 1,
		       0, 0,
		       gd->ram_size,
		       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);
	pci_set_region(hose->regions + 2, pcie->io.start,
		       pcie->io.start, MBUS_PCI_IO_SIZE, PCI_REGION_IO);
	hose->region_count = 3;

	/* Set BAR0 to internal registers */
	writel(SOC_REGS_PHY_BASE, pcie->base + PCIE_BAR_LO_OFF(0));
	writel(0, pcie->base + PCIE_BAR_HI_OFF(0));

	/* PCI Bridge support 32-bit I/O and 64-bit prefetch mem addressing */
	pcie->cfgcache[(PCI_IO_BASE - 0x10) / 4] =
		PCI_IO_RANGE_TYPE_32 | (PCI_IO_RANGE_TYPE_32 << 8);
	pcie->cfgcache[(PCI_PREF_MEMORY_BASE - 0x10) / 4] =
		PCI_PREF_RANGE_TYPE_64 | (PCI_PREF_RANGE_TYPE_64 << 16);

	return 0;
}

static int mvebu_pcie_port_parse_dt(ofnode node, struct mvebu_pcie *pcie)
{
	const u32 *addr;
	int len;

	addr = ofnode_get_property(node, "assigned-addresses", &len);
	if (!addr) {
		pr_err("property \"assigned-addresses\" not found");
		return -FDT_ERR_NOTFOUND;
	}

	pcie->base = (void *)(fdt32_to_cpu(addr[2]) + SOC_REGS_PHY_BASE);

	return 0;
}

#define DT_FLAGS_TO_TYPE(flags)       (((flags) >> 24) & 0x03)
#define    DT_TYPE_IO                 0x1
#define    DT_TYPE_MEM32              0x2
#define DT_CPUADDR_TO_TARGET(cpuaddr) (((cpuaddr) >> 56) & 0xFF)
#define DT_CPUADDR_TO_ATTR(cpuaddr)   (((cpuaddr) >> 48) & 0xFF)

static int mvebu_get_tgt_attr(ofnode node, int devfn,
			      unsigned long type,
			      unsigned int *tgt,
			      unsigned int *attr)
{
	const int na = 3, ns = 2;
	const __be32 *range;
	int rlen, nranges, rangesz, pna, i;

	*tgt = -1;
	*attr = -1;

	range = ofnode_get_property(node, "ranges", &rlen);
	if (!range)
		return -EINVAL;

	/*
	 * Linux uses of_n_addr_cells() to get the number of address cells
	 * here. Currently this function is only available in U-Boot when
	 * CONFIG_OF_LIVE is enabled. Until this is enabled for MVEBU in
	 * general, lets't hardcode the "pna" value in the U-Boot code.
	 */
	pna = 2; /* hardcoded for now because of lack of of_n_addr_cells() */
	rangesz = pna + na + ns;
	nranges = rlen / sizeof(__be32) / rangesz;

	for (i = 0; i < nranges; i++, range += rangesz) {
		u32 flags = of_read_number(range, 1);
		u32 slot = of_read_number(range + 1, 1);
		u64 cpuaddr = of_read_number(range + na, pna);
		unsigned long rtype;

		if (DT_FLAGS_TO_TYPE(flags) == DT_TYPE_IO)
			rtype = IORESOURCE_IO;
		else if (DT_FLAGS_TO_TYPE(flags) == DT_TYPE_MEM32)
			rtype = IORESOURCE_MEM;
		else
			continue;

		/*
		 * The Linux code used PCI_SLOT() here, which expects devfn
		 * in bits 7..0. PCI_DEV() in U-Boot is similar to PCI_SLOT(),
		 * only expects devfn in 15..8, where its saved in this driver.
		 */
		if (slot == PCI_DEV(devfn) && type == rtype) {
			*tgt = DT_CPUADDR_TO_TARGET(cpuaddr);
			*attr = DT_CPUADDR_TO_ATTR(cpuaddr);
			return 0;
		}
	}

	return -ENOENT;
}

static int mvebu_pcie_of_to_plat(struct udevice *dev)
{
	struct mvebu_pcie *pcie = dev_get_plat(dev);
	int ret = 0;

	/* Get port number, lane number and memory target / attr */
	if (ofnode_read_u32(dev_ofnode(dev), "marvell,pcie-port",
			    &pcie->port)) {
		ret = -ENODEV;
		goto err;
	}

	if (ofnode_read_u32(dev_ofnode(dev), "marvell,pcie-lane", &pcie->lane))
		pcie->lane = 0;

	sprintf(pcie->name, "pcie%d.%d", pcie->port, pcie->lane);

	/* pci_get_devfn() returns devfn in bits 15..8, see PCI_DEV usage */
	pcie->devfn = pci_get_devfn(dev);
	if (pcie->devfn < 0) {
		ret = -ENODEV;
		goto err;
	}

	ret = mvebu_get_tgt_attr(dev_ofnode(dev->parent), pcie->devfn,
				 IORESOURCE_MEM,
				 &pcie->mem_target, &pcie->mem_attr);
	if (ret < 0) {
		printf("%s: cannot get tgt/attr for mem window\n", pcie->name);
		goto err;
	}

	ret = mvebu_get_tgt_attr(dev_ofnode(dev->parent), pcie->devfn,
				 IORESOURCE_IO,
				 &pcie->io_target, &pcie->io_attr);
	if (ret < 0) {
		printf("%s: cannot get tgt/attr for IO window\n", pcie->name);
		goto err;
	}

	/* Parse PCIe controller register base from DT */
	ret = mvebu_pcie_port_parse_dt(dev_ofnode(dev), pcie);
	if (ret < 0)
		goto err;

	return 0;

err:
	return ret;
}

static const struct dm_pci_ops mvebu_pcie_ops = {
	.read_config	= mvebu_pcie_read_config,
	.write_config	= mvebu_pcie_write_config,
};

static struct driver pcie_mvebu_drv = {
	.name			= "pcie_mvebu",
	.id			= UCLASS_PCI,
	.ops			= &mvebu_pcie_ops,
	.probe			= mvebu_pcie_probe,
	.of_to_plat	= mvebu_pcie_of_to_plat,
	.plat_auto	= sizeof(struct mvebu_pcie),
};

/*
 * Use a MISC device to bind the n instances (child nodes) of the
 * PCIe base controller in UCLASS_PCI.
 */
static int mvebu_pcie_bind(struct udevice *parent)
{
	struct mvebu_pcie *pcie;
	struct uclass_driver *drv;
	struct udevice *dev;
	ofnode subnode;

	/* Lookup pci driver */
	drv = lists_uclass_lookup(UCLASS_PCI);
	if (!drv) {
		puts("Cannot find PCI driver\n");
		return -ENOENT;
	}

	ofnode_for_each_subnode(subnode, dev_ofnode(parent)) {
		if (!ofnode_is_available(subnode))
			continue;

		pcie = calloc(1, sizeof(*pcie));
		if (!pcie)
			return -ENOMEM;

		/* Create child device UCLASS_PCI and bind it */
		device_bind(parent, &pcie_mvebu_drv, pcie->name, pcie, subnode,
			    &dev);
	}

	return 0;
}

static const struct udevice_id mvebu_pcie_ids[] = {
	{ .compatible = "marvell,armada-xp-pcie" },
	{ .compatible = "marvell,armada-370-pcie" },
	{ }
};

U_BOOT_DRIVER(pcie_mvebu_base) = {
	.name			= "pcie_mvebu_base",
	.id			= UCLASS_MISC,
	.of_match		= mvebu_pcie_ids,
	.bind			= mvebu_pcie_bind,
};
