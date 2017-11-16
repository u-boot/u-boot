/*
 * PCIe driver for Marvell MVEBU SoCs
 *
 * Based on Barebox drivers/pci/pci-mvebu.c
 *
 * Ported to U-Boot by:
 * Anton Schubert <anton.schubert@gmx.de>
 * Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <pci.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/mbus.h>

DECLARE_GLOBAL_DATA_PTR;

/* PCIe unit register offsets */
#define SELECT(x, n)			((x >> n) & 1UL)

#define PCIE_DEV_ID_OFF			0x0000
#define PCIE_CMD_OFF			0x0004
#define PCIE_DEV_REV_OFF		0x0008
#define  PCIE_BAR_LO_OFF(n)		(0x0010 + ((n) << 3))
#define  PCIE_BAR_HI_OFF(n)		(0x0014 + ((n) << 3))
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
#define  PCIE_CONF_ADDR(dev, reg) \
	(PCIE_CONF_BUS(PCI_BUS(dev)) | PCIE_CONF_DEV(PCI_DEV(dev))    | \
	 PCIE_CONF_FUNC(PCI_FUNC(dev)) | PCIE_CONF_REG(reg) | \
	 PCIE_CONF_ADDR_EN)
#define PCIE_CONF_DATA_OFF		0x18fc
#define PCIE_MASK_OFF			0x1910
#define  PCIE_MASK_ENABLE_INTS          (0xf << 24)
#define PCIE_CTRL_OFF			0x1a00
#define  PCIE_CTRL_X1_MODE		BIT(0)
#define PCIE_STAT_OFF			0x1a04
#define  PCIE_STAT_BUS                  (0xff << 8)
#define  PCIE_STAT_DEV                  (0x1f << 16)
#define  PCIE_STAT_LINK_DOWN		BIT(0)
#define PCIE_DEBUG_CTRL			0x1a60
#define  PCIE_DEBUG_SOFT_RESET		BIT(20)

struct resource {
	u32 start;
	u32 end;
};

struct mvebu_pcie {
	struct pci_controller hose;
	char *name;
	void __iomem *base;
	void __iomem *membase;
	struct resource mem;
	void __iomem *iobase;
	u32 port;
	u32 lane;
	u32 lane_mask;
	pci_dev_t dev;
};

#define to_pcie(_hc)	container_of(_hc, struct mvebu_pcie, pci)

/*
 * MVEBU PCIe controller needs MEMORY and I/O BARs to be mapped
 * into SoCs address space. Each controller will map 128M of MEM
 * and 64K of I/O space when registered.
 */
static void __iomem *mvebu_pcie_membase = (void __iomem *)MBUS_PCI_MEM_BASE;
#define PCIE_MEM_SIZE	(128 << 20)

#if defined(CONFIG_ARMADA_38X)
#define PCIE_BASE(if)					\
	((if) == 0 ?					\
	 MVEBU_REG_PCIE0_BASE :				\
	 (MVEBU_REG_PCIE_BASE + 0x4000 * (if - 1)))

/*
 * On A38x MV6820 these PEX ports are supported:
 *  0 - Port 0.0
 *  1 - Port 1.0
 *  2 - Port 2.0
 *  3 - Port 3.0
 */
#define MAX_PEX 4
static struct mvebu_pcie pcie_bus[MAX_PEX];

static void mvebu_get_port_lane(struct mvebu_pcie *pcie, int pex_idx,
				int *mem_target, int *mem_attr)
{
	u8 port[] = { 0, 1, 2, 3 };
	u8 lane[] = { 0, 0, 0, 0 };
	u8 target[] = { 8, 4, 4, 4 };
	u8 attr[] = { 0xe8, 0xe8, 0xd8, 0xb8 };

	pcie->port = port[pex_idx];
	pcie->lane = lane[pex_idx];
	*mem_target = target[pex_idx];
	*mem_attr = attr[pex_idx];
}
#else
#define PCIE_BASE(if)							\
	((if) < 8 ?							\
	 (MVEBU_REG_PCIE_BASE + ((if) / 4) * 0x40000 + ((if) % 4) * 0x4000) : \
	 (MVEBU_REG_PCIE_BASE + 0x2000 + ((if) % 8) * 0x40000))

/*
 * On AXP MV78460 these PEX ports are supported:
 *  0 - Port 0.0
 *  1 - Port 0.1
 *  2 - Port 0.2
 *  3 - Port 0.3
 *  4 - Port 1.0
 *  5 - Port 1.1
 *  6 - Port 1.2
 *  7 - Port 1.3
 *  8 - Port 2.0
 *  9 - Port 3.0
 */
#define MAX_PEX 10
static struct mvebu_pcie pcie_bus[MAX_PEX];

static void mvebu_get_port_lane(struct mvebu_pcie *pcie, int pex_idx,
				int *mem_target, int *mem_attr)
{
	u8 port[] = { 0, 0, 0, 0, 1, 1, 1, 1, 2, 3 };
	u8 lane[] = { 0, 1, 2, 3, 0, 1, 2, 3, 0, 0 };
	u8 target[] = { 4, 4, 4, 4, 8, 8, 8, 8, 4, 8 };
	u8 attr[] = { 0xe8, 0xd8, 0xb8, 0x78,
		      0xe8, 0xd8, 0xb8, 0x78,
		      0xf8, 0xf8 };

	pcie->port = port[pex_idx];
	pcie->lane = lane[pex_idx];
	*mem_target = target[pex_idx];
	*mem_attr = attr[pex_idx];
}
#endif

static int mvebu_pex_unit_is_x4(int pex_idx)
{
	int pex_unit = pex_idx < 9 ? pex_idx >> 2 : 3;
	u32 mask = (0x0f << (pex_unit * 8));

	return (readl(COMPHY_REFCLK_ALIGNMENT) & mask) == mask;
}

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

static int mvebu_pcie_get_local_bus_nr(struct mvebu_pcie *pcie)
{
	u32 stat;

	stat = readl(pcie->base + PCIE_STAT_OFF);
	return (stat & PCIE_STAT_BUS) >> 8;
}

static int mvebu_pcie_get_local_dev_nr(struct mvebu_pcie *pcie)
{
	u32 stat;

	stat = readl(pcie->base + PCIE_STAT_OFF);
	return (stat & PCIE_STAT_DEV) >> 16;
}

static inline struct mvebu_pcie *hose_to_pcie(struct pci_controller *hose)
{
	return container_of(hose, struct mvebu_pcie, hose);
}

static int mvebu_pcie_read_config_dword(struct pci_controller *hose,
		pci_dev_t dev, int offset, u32 *val)
{
	struct mvebu_pcie *pcie = hose_to_pcie(hose);
	int local_bus = PCI_BUS(pcie->dev);
	int local_dev = PCI_DEV(pcie->dev);
	u32 reg;

	/* Only allow one other device besides the local one on the local bus */
	if (PCI_BUS(dev) == local_bus && PCI_DEV(dev) != local_dev) {
		if (local_dev == 0 && PCI_DEV(dev) != 1) {
			/*
			 * If local dev is 0, the first other dev can
			 * only be 1
			 */
			*val = 0xffffffff;
			return 1;
		} else if (local_dev != 0 && PCI_DEV(dev) != 0) {
			/*
			 * If local dev is not 0, the first other dev can
			 * only be 0
			 */
			*val = 0xffffffff;
			return 1;
		}
	}

	/* write address */
	reg = PCIE_CONF_ADDR(dev, offset);
	writel(reg, pcie->base + PCIE_CONF_ADDR_OFF);
	*val = readl(pcie->base + PCIE_CONF_DATA_OFF);

	return 0;
}

static int mvebu_pcie_write_config_dword(struct pci_controller *hose,
		pci_dev_t dev, int offset, u32 val)
{
	struct mvebu_pcie *pcie = hose_to_pcie(hose);
	int local_bus = PCI_BUS(pcie->dev);
	int local_dev = PCI_DEV(pcie->dev);

	/* Only allow one other device besides the local one on the local bus */
	if (PCI_BUS(dev) == local_bus && PCI_DEV(dev) != local_dev) {
		if (local_dev == 0 && PCI_DEV(dev) != 1) {
			/*
			 * If local dev is 0, the first other dev can
			 * only be 1
			 */
			return 1;
		} else if (local_dev != 0 && PCI_DEV(dev) != 0) {
			/*
			 * If local dev is not 0, the first other dev can
			 * only be 0
			 */
			return 1;
		}
	}

	writel(PCIE_CONF_ADDR(dev, offset), pcie->base + PCIE_CONF_ADDR_OFF);
	writel(val, pcie->base + PCIE_CONF_DATA_OFF);

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

void pci_init_board(void)
{
	int mem_target, mem_attr, i;
	int bus = 0;
	u32 reg;
	u32 soc_ctrl = readl(MVEBU_SYSTEM_REG_BASE + 0x4);

	/* Check SoC Control Power State */
	debug("%s: SoC Control %08x, 0en %01lx, 1en %01lx, 2en %01lx\n",
	      __func__, soc_ctrl, SELECT(soc_ctrl, 0), SELECT(soc_ctrl, 1),
	      SELECT(soc_ctrl, 2));

	for (i = 0; i < MAX_PEX; i++) {
		struct mvebu_pcie *pcie = &pcie_bus[i];
		struct pci_controller *hose = &pcie->hose;

		/* Get port number, lane number and memory target / attr */
		mvebu_get_port_lane(pcie, i, &mem_target, &mem_attr);

		/* Don't read at all from pci registers if port power is down */
		if (SELECT(soc_ctrl, pcie->port) == 0) {
			if (pcie->lane == 0)
				debug("%s: skipping port %d\n", __func__, pcie->port);
			continue;
		}

		pcie->base = (void __iomem *)PCIE_BASE(i);

		/* Check link and skip ports that have no link */
		if (!mvebu_pcie_link_up(pcie)) {
			debug("%s: PCIe %d.%d - down\n", __func__,
			      pcie->port, pcie->lane);
			continue;
		}
		debug("%s: PCIe %d.%d - up, base %08x\n", __func__,
		      pcie->port, pcie->lane, (u32)pcie->base);

		/* Read Id info and local bus/dev */
		debug("direct conf read %08x, local bus %d, local dev %d\n",
		      readl(pcie->base), mvebu_pcie_get_local_bus_nr(pcie),
		      mvebu_pcie_get_local_dev_nr(pcie));

		mvebu_pcie_set_local_bus_nr(pcie, bus);
		mvebu_pcie_set_local_dev_nr(pcie, 0);
		pcie->dev = PCI_BDF(bus, 0, 0);

		pcie->mem.start = (u32)mvebu_pcie_membase;
		pcie->mem.end = pcie->mem.start + PCIE_MEM_SIZE - 1;
		mvebu_pcie_membase += PCIE_MEM_SIZE;

		if (mvebu_mbus_add_window_by_id(mem_target, mem_attr,
						(phys_addr_t)pcie->mem.start,
						PCIE_MEM_SIZE)) {
			printf("PCIe unable to add mbus window for mem at %08x+%08x\n",
			       (u32)pcie->mem.start, PCIE_MEM_SIZE);
		}

		/* Setup windows and configure host bridge */
		mvebu_pcie_setup_wins(pcie);

		/* Master + slave enable. */
		reg = readl(pcie->base + PCIE_CMD_OFF);
		reg |= PCI_COMMAND_MEMORY;
		reg |= PCI_COMMAND_MASTER;
		reg |= BIT(10);		/* disable interrupts */
		writel(reg, pcie->base + PCIE_CMD_OFF);

		/* Setup U-Boot PCI Controller */
		hose->first_busno = 0;
		hose->current_busno = bus;

		/* PCI memory space */
		pci_set_region(hose->regions + 0, pcie->mem.start,
			       pcie->mem.start, PCIE_MEM_SIZE, PCI_REGION_MEM);
		pci_set_region(hose->regions + 1,
			       0, 0,
			       gd->ram_size,
			       PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);
		hose->region_count = 2;

		pci_set_ops(hose,
			    pci_hose_read_config_byte_via_dword,
			    pci_hose_read_config_word_via_dword,
			    mvebu_pcie_read_config_dword,
			    pci_hose_write_config_byte_via_dword,
			    pci_hose_write_config_word_via_dword,
			    mvebu_pcie_write_config_dword);
		pci_register_hose(hose);

		hose->last_busno = pci_hose_scan(hose);

		/* Set BAR0 to internal registers */
		writel(SOC_REGS_PHY_BASE, pcie->base + PCIE_BAR_LO_OFF(0));
		writel(0, pcie->base + PCIE_BAR_HI_OFF(0));

		bus = hose->last_busno + 1;

		/* need to skip more for X4 links, otherwise scan will hang */
		if (mvebu_soc_family() == MVEBU_SOC_AXP) {
			if (mvebu_pex_unit_is_x4(i))
				i += 3;
		}
	}
}
