/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * Layerscape PCIe driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/fsl_serdes.h>
#include <pci.h>
#include <asm/io.h>
#include <errno.h>
#include <malloc.h>
#include <asm/arch-fsl-lsch3/fdt.h>

#ifndef CONFIG_SYS_PCI_MEMORY_BUS
#define CONFIG_SYS_PCI_MEMORY_BUS CONFIG_SYS_SDRAM_BASE
#endif

#ifndef CONFIG_SYS_PCI_MEMORY_PHYS
#define CONFIG_SYS_PCI_MEMORY_PHYS CONFIG_SYS_SDRAM_BASE
#endif

#ifndef CONFIG_SYS_PCI_MEMORY_SIZE
#define CONFIG_SYS_PCI_MEMORY_SIZE (2 * 1024 * 1024 * 1024UL) /* 2G */
#endif

#ifndef CONFIG_SYS_PCI_EP_MEMORY_BASE
#define CONFIG_SYS_PCI_EP_MEMORY_BASE CONFIG_SYS_LOAD_ADDR
#endif

/* iATU registers */
#define PCIE_ATU_VIEWPORT		0x900
#define PCIE_ATU_REGION_INBOUND		(0x1 << 31)
#define PCIE_ATU_REGION_OUTBOUND	(0x0 << 31)
#define PCIE_ATU_REGION_INDEX0		(0x0 << 0)
#define PCIE_ATU_REGION_INDEX1		(0x1 << 0)
#define PCIE_ATU_REGION_INDEX2		(0x2 << 0)
#define PCIE_ATU_REGION_INDEX3		(0x3 << 0)
#define PCIE_ATU_CR1			0x904
#define PCIE_ATU_TYPE_MEM		(0x0 << 0)
#define PCIE_ATU_TYPE_IO		(0x2 << 0)
#define PCIE_ATU_TYPE_CFG0		(0x4 << 0)
#define PCIE_ATU_TYPE_CFG1		(0x5 << 0)
#define PCIE_ATU_CR2			0x908
#define PCIE_ATU_ENABLE			(0x1 << 31)
#define PCIE_ATU_BAR_MODE_ENABLE	(0x1 << 30)
#define PCIE_ATU_BAR_NUM(bar)		((bar) << 8)
#define PCIE_ATU_LOWER_BASE		0x90C
#define PCIE_ATU_UPPER_BASE		0x910
#define PCIE_ATU_LIMIT			0x914
#define PCIE_ATU_LOWER_TARGET		0x918
#define PCIE_ATU_BUS(x)			(((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)			(((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)		(((x) & 0x7) << 16)
#define PCIE_ATU_UPPER_TARGET		0x91C

/* LUT registers */
#define PCIE_LUT_BASE		0x80000
#define PCIE_LUT_LCTRL0		0x7F8
#define PCIE_LUT_DBG		0x7FC

#define PCIE_DBI_RO_WR_EN	0x8bc

#define PCIE_LINK_CAP		0x7c
#define PCIE_LINK_SPEED_MASK	0xf
#define PCIE_LINK_STA		0x82

#define LTSSM_STATE_MASK	0x3f
#define LTSSM_PCIE_L0		0x11 /* L0 state */

#define PCIE_DBI_SIZE		0x100000 /* 1M */

#define PCIE_LCTRL0_CFG2_ENABLE	(1 << 31)
#define PCIE_LCTRL0_VF(vf)	((vf) << 22)
#define PCIE_LCTRL0_PF(pf)	((pf) << 16)
#define PCIE_LCTRL0_VF_ACTIVE	(1 << 21)
#define PCIE_LCTRL0_VAL(pf, vf)	(PCIE_LCTRL0_PF(pf) |			   \
				 PCIE_LCTRL0_VF(vf) |			   \
				 ((vf) == 0 ? 0 : PCIE_LCTRL0_VF_ACTIVE) | \
				 PCIE_LCTRL0_CFG2_ENABLE)

#define PCIE_NO_SRIOV_BAR_BASE	0x1000

#define PCIE_PF_NUM		2
#define PCIE_VF_NUM		64

#define PCIE_BAR0_SIZE		(4 * 1024) /* 4K */
#define PCIE_BAR1_SIZE		(8 * 1024) /* 8K for MSIX */
#define PCIE_BAR2_SIZE		(4 * 1024) /* 4K */
#define PCIE_BAR4_SIZE		(1 * 1024 * 1024) /* 1M */

struct ls_pcie {
	int idx;
	void __iomem *dbi;
	void __iomem *va_cfg0;
	void __iomem *va_cfg1;
	struct pci_controller hose;
};

struct ls_pcie_info {
	unsigned long regs;
	int pci_num;
	u64 phys_base;
	u64 cfg0_phys;
	u64 cfg0_size;
	u64 cfg1_phys;
	u64 cfg1_size;
	u64 mem_bus;
	u64 mem_phys;
	u64 mem_size;
	u64 io_bus;
	u64 io_phys;
	u64 io_size;
};

#define SET_LS_PCIE_INFO(x, num)			\
{							\
	x.regs = CONFIG_SYS_PCIE##num##_ADDR;		\
	x.phys_base = CONFIG_SYS_PCIE##num##_PHYS_ADDR;	\
	x.cfg0_phys = CONFIG_SYS_PCIE_CFG0_PHYS_OFF +	\
		      CONFIG_SYS_PCIE##num##_PHYS_ADDR;	\
	x.cfg0_size = CONFIG_SYS_PCIE_CFG0_SIZE;	\
	x.cfg1_phys = CONFIG_SYS_PCIE_CFG1_PHYS_OFF +	\
		      CONFIG_SYS_PCIE##num##_PHYS_ADDR;	\
	x.cfg1_size = CONFIG_SYS_PCIE_CFG1_SIZE;	\
	x.mem_bus = CONFIG_SYS_PCIE_MEM_BUS;		\
	x.mem_phys = CONFIG_SYS_PCIE_MEM_PHYS_OFF +	\
		     CONFIG_SYS_PCIE##num##_PHYS_ADDR;	\
	x.mem_size = CONFIG_SYS_PCIE_MEM_SIZE;		\
	x.io_bus = CONFIG_SYS_PCIE_IO_BUS;		\
	x.io_phys = CONFIG_SYS_PCIE_IO_PHYS_OFF +	\
		    CONFIG_SYS_PCIE##num##_PHYS_ADDR;	\
	x.io_size = CONFIG_SYS_PCIE_IO_SIZE;		\
	x.pci_num = num;				\
}

#ifdef CONFIG_LS102XA
#include <asm/arch/immap_ls102xa.h>

/* PEX1/2 Misc Ports Status Register */
#define LTSSM_STATE_SHIFT	20

static int ls_pcie_link_state(struct ls_pcie *pcie)
{
	u32 state;
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

	state = in_be32(&scfg->pexmscportsr[pcie->idx]);
	state = (state >> LTSSM_STATE_SHIFT) & LTSSM_STATE_MASK;
	if (state < LTSSM_PCIE_L0) {
		debug("....PCIe link error. LTSSM=0x%02x.\n", state);
		return 0;
	}

	return 1;
}
#else
static int ls_pcie_link_state(struct ls_pcie *pcie)
{
	u32 state;

	state = readl(pcie->dbi + PCIE_LUT_BASE + PCIE_LUT_DBG) &
		LTSSM_STATE_MASK;
	if (state < LTSSM_PCIE_L0) {
		debug("....PCIe link error. LTSSM=0x%02x.\n", state);
		return 0;
	}

	return 1;
}
#endif

static int ls_pcie_link_up(struct ls_pcie *pcie)
{
	int state;
	u32 cap;

	state = ls_pcie_link_state(pcie);
	if (state)
		return state;

	/* Try to download speed to gen1 */
	cap = readl(pcie->dbi + PCIE_LINK_CAP);
	writel((cap & (~PCIE_LINK_SPEED_MASK)) | 1, pcie->dbi + PCIE_LINK_CAP);
	/*
	 * Notice: the following delay has critical impact on link training
	 * if too short (<30ms) the link doesn't get up.
	 */
	mdelay(100);
	state = ls_pcie_link_state(pcie);
	if (state)
		return state;

	writel(cap, pcie->dbi + PCIE_LINK_CAP);

	return 0;
}

static void ls_pcie_cfg0_set_busdev(struct ls_pcie *pcie, u32 busdev)
{
	writel(PCIE_ATU_REGION_OUTBOUND | PCIE_ATU_REGION_INDEX0,
	       pcie->dbi + PCIE_ATU_VIEWPORT);
	writel(busdev, pcie->dbi + PCIE_ATU_LOWER_TARGET);
}

static void ls_pcie_cfg1_set_busdev(struct ls_pcie *pcie, u32 busdev)
{
	writel(PCIE_ATU_REGION_OUTBOUND | PCIE_ATU_REGION_INDEX1,
	       pcie->dbi + PCIE_ATU_VIEWPORT);
	writel(busdev, pcie->dbi + PCIE_ATU_LOWER_TARGET);
}

static void ls_pcie_iatu_outbound_set(struct ls_pcie *pcie, int idx, int type,
				      u64 phys, u64 bus_addr, pci_size_t size)
{
	writel(PCIE_ATU_REGION_OUTBOUND | idx, pcie->dbi + PCIE_ATU_VIEWPORT);
	writel((u32)phys, pcie->dbi + PCIE_ATU_LOWER_BASE);
	writel(phys >> 32, pcie->dbi + PCIE_ATU_UPPER_BASE);
	writel(phys + size - 1, pcie->dbi + PCIE_ATU_LIMIT);
	writel((u32)bus_addr, pcie->dbi + PCIE_ATU_LOWER_TARGET);
	writel(bus_addr >> 32, pcie->dbi + PCIE_ATU_UPPER_TARGET);
	writel(type, pcie->dbi + PCIE_ATU_CR1);
	writel(PCIE_ATU_ENABLE, pcie->dbi + PCIE_ATU_CR2);
}

/* Use bar match mode and MEM type as default */
static void ls_pcie_iatu_inbound_set(struct ls_pcie *pcie, int idx,
				     int bar, u64 phys)
{
	writel(PCIE_ATU_REGION_INBOUND | idx, pcie->dbi + PCIE_ATU_VIEWPORT);
	writel((u32)phys, pcie->dbi + PCIE_ATU_LOWER_TARGET);
	writel(phys >> 32, pcie->dbi + PCIE_ATU_UPPER_TARGET);
	writel(PCIE_ATU_TYPE_MEM, pcie->dbi + PCIE_ATU_CR1);
	writel(PCIE_ATU_ENABLE | PCIE_ATU_BAR_MODE_ENABLE |
	       PCIE_ATU_BAR_NUM(bar), pcie->dbi + PCIE_ATU_CR2);
}

static void ls_pcie_setup_atu(struct ls_pcie *pcie, struct ls_pcie_info *info)
{
#ifdef DEBUG
	int i;
#endif

	/* ATU 0 : OUTBOUND : CFG0 */
	ls_pcie_iatu_outbound_set(pcie, PCIE_ATU_REGION_INDEX0,
				  PCIE_ATU_TYPE_CFG0,
				  info->cfg0_phys,
				  0,
				  info->cfg0_size);
	/* ATU 1 : OUTBOUND : CFG1 */
	ls_pcie_iatu_outbound_set(pcie, PCIE_ATU_REGION_INDEX1,
				  PCIE_ATU_TYPE_CFG1,
				  info->cfg1_phys,
				  0,
				  info->cfg1_size);
	/* ATU 2 : OUTBOUND : MEM */
	ls_pcie_iatu_outbound_set(pcie, PCIE_ATU_REGION_INDEX2,
				  PCIE_ATU_TYPE_MEM,
				  info->mem_phys,
				  info->mem_bus,
				  info->mem_size);
	/* ATU 3 : OUTBOUND : IO */
	ls_pcie_iatu_outbound_set(pcie, PCIE_ATU_REGION_INDEX3,
				  PCIE_ATU_TYPE_IO,
				  info->io_phys,
				  info->io_bus,
				  info->io_size);

#ifdef DEBUG
	for (i = 0; i <= PCIE_ATU_REGION_INDEX3; i++) {
		writel(PCIE_ATU_REGION_OUTBOUND | i,
		       pcie->dbi + PCIE_ATU_VIEWPORT);
		debug("iATU%d:\n", i);
		debug("\tLOWER PHYS 0x%08x\n",
		      readl(pcie->dbi + PCIE_ATU_LOWER_BASE));
		debug("\tUPPER PHYS 0x%08x\n",
		      readl(pcie->dbi + PCIE_ATU_UPPER_BASE));
		debug("\tLOWER BUS  0x%08x\n",
		      readl(pcie->dbi + PCIE_ATU_LOWER_TARGET));
		debug("\tUPPER BUS  0x%08x\n",
		      readl(pcie->dbi + PCIE_ATU_UPPER_TARGET));
		debug("\tLIMIT      0x%08x\n",
		      readl(pcie->dbi + PCIE_ATU_LIMIT));
		debug("\tCR1        0x%08x\n",
		      readl(pcie->dbi + PCIE_ATU_CR1));
		debug("\tCR2        0x%08x\n",
		      readl(pcie->dbi + PCIE_ATU_CR2));
	}
#endif
}

int pci_skip_dev(struct pci_controller *hose, pci_dev_t dev)
{
	/* Do not skip controller */
	return 0;
}

static int ls_pcie_addr_valid(struct pci_controller *hose, pci_dev_t d)
{
	if (PCI_DEV(d) > 0)
		return -EINVAL;

	/* Controller does not support multi-function in RC mode */
	if ((PCI_BUS(d) == hose->first_busno) && (PCI_FUNC(d) > 0))
		return -EINVAL;

	return 0;
}

static int ls_pcie_read_config(struct pci_controller *hose, pci_dev_t d,
			       int where, u32 *val)
{
	struct ls_pcie *pcie = hose->priv_data;
	u32 busdev, *addr;

	if (ls_pcie_addr_valid(hose, d)) {
		*val = 0xffffffff;
		return -EINVAL;
	}

	if (PCI_BUS(d) == hose->first_busno) {
		addr = pcie->dbi + (where & ~0x3);
	} else {
		busdev = PCIE_ATU_BUS(PCI_BUS(d)) |
			 PCIE_ATU_DEV(PCI_DEV(d)) |
			 PCIE_ATU_FUNC(PCI_FUNC(d));

		if (PCI_BUS(d) == hose->first_busno + 1) {
			ls_pcie_cfg0_set_busdev(pcie, busdev);
			addr = pcie->va_cfg0 + (where & ~0x3);
		} else {
			ls_pcie_cfg1_set_busdev(pcie, busdev);
			addr = pcie->va_cfg1 + (where & ~0x3);
		}
	}

	*val = readl(addr);

	return 0;
}

static int ls_pcie_write_config(struct pci_controller *hose, pci_dev_t d,
				int where, u32 val)
{
	struct ls_pcie *pcie = hose->priv_data;
	u32 busdev, *addr;

	if (ls_pcie_addr_valid(hose, d))
		return -EINVAL;

	if (PCI_BUS(d) == hose->first_busno) {
		addr = pcie->dbi + (where & ~0x3);
	} else {
		busdev = PCIE_ATU_BUS(PCI_BUS(d)) |
			 PCIE_ATU_DEV(PCI_DEV(d)) |
			 PCIE_ATU_FUNC(PCI_FUNC(d));

		if (PCI_BUS(d) == hose->first_busno + 1) {
			ls_pcie_cfg0_set_busdev(pcie, busdev);
			addr = pcie->va_cfg0 + (where & ~0x3);
		} else {
			ls_pcie_cfg1_set_busdev(pcie, busdev);
			addr = pcie->va_cfg1 + (where & ~0x3);
		}
	}

	writel(val, addr);

	return 0;
}

static void ls_pcie_setup_ctrl(struct ls_pcie *pcie,
			       struct ls_pcie_info *info)
{
	struct pci_controller *hose = &pcie->hose;
	pci_dev_t dev = PCI_BDF(hose->first_busno, 0, 0);

	ls_pcie_setup_atu(pcie, info);

	pci_hose_write_config_dword(hose, dev, PCI_BASE_ADDRESS_0, 0);

	/* program correct class for RC */
	writel(1, pcie->dbi + PCIE_DBI_RO_WR_EN);
	pci_hose_write_config_word(hose, dev, PCI_CLASS_DEVICE,
				   PCI_CLASS_BRIDGE_PCI);
#ifndef CONFIG_LS102XA
	writel(0, pcie->dbi + PCIE_DBI_RO_WR_EN);
#endif
}

static void ls_pcie_ep_setup_atu(struct ls_pcie *pcie,
				 struct ls_pcie_info *info)
{
	u64 phys = CONFIG_SYS_PCI_EP_MEMORY_BASE;

	/* ATU 0 : INBOUND : map BAR0 */
	ls_pcie_iatu_inbound_set(pcie, PCIE_ATU_REGION_INDEX0, 0, phys);
	/* ATU 1 : INBOUND : map BAR1 */
	phys += PCIE_BAR1_SIZE;
	ls_pcie_iatu_inbound_set(pcie, PCIE_ATU_REGION_INDEX1, 1, phys);
	/* ATU 2 : INBOUND : map BAR2 */
	phys += PCIE_BAR2_SIZE;
	ls_pcie_iatu_inbound_set(pcie, PCIE_ATU_REGION_INDEX2, 2, phys);
	/* ATU 3 : INBOUND : map BAR4 */
	phys = CONFIG_SYS_PCI_EP_MEMORY_BASE + PCIE_BAR4_SIZE;
	ls_pcie_iatu_inbound_set(pcie, PCIE_ATU_REGION_INDEX3, 4, phys);

	/* ATU 0 : OUTBOUND : map 4G MEM */
	ls_pcie_iatu_outbound_set(pcie, PCIE_ATU_REGION_INDEX0,
				  PCIE_ATU_TYPE_MEM,
				  info->phys_base,
				  0,
				  4 * 1024 * 1024 * 1024ULL);
}

/* BAR0 and BAR1 are 32bit BAR2 and BAR4 are 64bit */
static void ls_pcie_ep_setup_bar(void *bar_base, int bar, u32 size)
{
	if (size < 4 * 1024)
		return;

	switch (bar) {
	case 0:
		writel(size - 1, bar_base + PCI_BASE_ADDRESS_0);
		break;
	case 1:
		writel(size - 1, bar_base + PCI_BASE_ADDRESS_1);
		break;
	case 2:
		writel(size - 1, bar_base + PCI_BASE_ADDRESS_2);
		writel(0, bar_base + PCI_BASE_ADDRESS_3);
		break;
	case 4:
		writel(size - 1, bar_base + PCI_BASE_ADDRESS_4);
		writel(0, bar_base + PCI_BASE_ADDRESS_5);
		break;
	default:
		break;
	}
}

static void ls_pcie_ep_setup_bars(void *bar_base)
{
	/* BAR0 - 32bit - 4K configuration */
	ls_pcie_ep_setup_bar(bar_base, 0, PCIE_BAR0_SIZE);
	/* BAR1 - 32bit - 8K MSIX*/
	ls_pcie_ep_setup_bar(bar_base, 1, PCIE_BAR1_SIZE);
	/* BAR2 - 64bit - 4K MEM desciptor */
	ls_pcie_ep_setup_bar(bar_base, 2, PCIE_BAR2_SIZE);
	/* BAR4 - 64bit - 1M MEM*/
	ls_pcie_ep_setup_bar(bar_base, 4, PCIE_BAR4_SIZE);
}

static void ls_pcie_setup_ep(struct ls_pcie *pcie, struct ls_pcie_info *info)
{
	struct pci_controller *hose = &pcie->hose;
	pci_dev_t dev = PCI_BDF(hose->first_busno, 0, 0);
	int sriov;

	sriov = pci_hose_find_ext_capability(hose, dev, PCI_EXT_CAP_ID_SRIOV);
	if (sriov) {
		int pf, vf;

		for (pf = 0; pf < PCIE_PF_NUM; pf++) {
			for (vf = 0; vf <= PCIE_VF_NUM; vf++) {
				writel(PCIE_LCTRL0_VAL(pf, vf),
				       pcie->dbi + PCIE_LUT_BASE +
				       PCIE_LUT_LCTRL0);
				ls_pcie_ep_setup_bars(pcie->dbi);
				ls_pcie_ep_setup_atu(pcie, info);
			}
		}

		/* Disable CFG2 */
		writel(0, pcie->dbi + PCIE_LUT_BASE + PCIE_LUT_LCTRL0);
	} else {
		ls_pcie_ep_setup_bars(pcie->dbi + PCIE_NO_SRIOV_BAR_BASE);
		ls_pcie_ep_setup_atu(pcie, info);
	}
}

int ls_pcie_init_ctrl(int busno, enum srds_prtcl dev, struct ls_pcie_info *info)
{
	struct ls_pcie *pcie;
	struct pci_controller *hose;
	int num = dev - PCIE1;
	pci_dev_t pdev = PCI_BDF(busno, 0, 0);
	int i, linkup, ep_mode;
	u8 header_type;
	u16 temp16;

	if (!is_serdes_configured(dev)) {
		printf("PCIe%d: disabled\n", num + 1);
		return busno;
	}

	pcie = malloc(sizeof(*pcie));
	if (!pcie)
		return busno;
	memset(pcie, 0, sizeof(*pcie));

	hose = &pcie->hose;
	hose->priv_data = pcie;
	hose->first_busno = busno;
	pcie->idx = num;
	pcie->dbi = map_physmem(info->regs, PCIE_DBI_SIZE, MAP_NOCACHE);
	pcie->va_cfg0 = map_physmem(info->cfg0_phys,
				    info->cfg0_size,
				    MAP_NOCACHE);
	pcie->va_cfg1 = map_physmem(info->cfg1_phys,
				    info->cfg1_size,
				    MAP_NOCACHE);

	/* outbound memory */
	pci_set_region(&hose->regions[0],
		       (pci_size_t)info->mem_bus,
		       (phys_size_t)info->mem_phys,
		       (pci_size_t)info->mem_size,
		       PCI_REGION_MEM);

	/* outbound io */
	pci_set_region(&hose->regions[1],
		       (pci_size_t)info->io_bus,
		       (phys_size_t)info->io_phys,
		       (pci_size_t)info->io_size,
		       PCI_REGION_IO);

	/* System memory space */
	pci_set_region(&hose->regions[2],
		       CONFIG_SYS_PCI_MEMORY_BUS,
		       CONFIG_SYS_PCI_MEMORY_PHYS,
		       CONFIG_SYS_PCI_MEMORY_SIZE,
		       PCI_REGION_SYS_MEMORY);

	hose->region_count = 3;

	for (i = 0; i < hose->region_count; i++)
		debug("PCI reg:%d %016llx:%016llx %016llx %08lx\n",
		      i,
		      (u64)hose->regions[i].phys_start,
		      (u64)hose->regions[i].bus_start,
		      (u64)hose->regions[i].size,
		      hose->regions[i].flags);

	pci_set_ops(hose,
		    pci_hose_read_config_byte_via_dword,
		    pci_hose_read_config_word_via_dword,
		    ls_pcie_read_config,
		    pci_hose_write_config_byte_via_dword,
		    pci_hose_write_config_word_via_dword,
		    ls_pcie_write_config);

	pci_hose_read_config_byte(hose, pdev, PCI_HEADER_TYPE, &header_type);
	ep_mode = (header_type & 0x7f) == PCI_HEADER_TYPE_NORMAL;
	printf("PCIe%u: %s ", info->pci_num,
	       ep_mode ? "Endpoint" : "Root Complex");

	if (ep_mode)
		ls_pcie_setup_ep(pcie, info);
	else
		ls_pcie_setup_ctrl(pcie, info);

	linkup = ls_pcie_link_up(pcie);

	if (!linkup) {
		/* Let the user know there's no PCIe link */
		printf("no link, regs @ 0x%lx\n", info->regs);
		hose->last_busno = hose->first_busno;
		return busno;
	}

	/* Print the negotiated PCIe link width */
	pci_hose_read_config_word(hose, pdev, PCIE_LINK_STA, &temp16);
	printf("x%d gen%d, regs @ 0x%lx\n", (temp16 & 0x3f0) >> 4,
	       (temp16 & 0xf), info->regs);

	if (ep_mode)
		return busno;

	pci_register_hose(hose);

	hose->last_busno = pci_hose_scan(hose);

	printf("PCIe%x: Bus %02x - %02x\n",
	       info->pci_num, hose->first_busno, hose->last_busno);

	return hose->last_busno + 1;
}

int ls_pcie_init_board(int busno)
{
	struct ls_pcie_info info;

#ifdef CONFIG_PCIE1
	SET_LS_PCIE_INFO(info, 1);
	busno = ls_pcie_init_ctrl(busno, PCIE1, &info);
#endif

#ifdef CONFIG_PCIE2
	SET_LS_PCIE_INFO(info, 2);
	busno = ls_pcie_init_ctrl(busno, PCIE2, &info);
#endif

#ifdef CONFIG_PCIE3
	SET_LS_PCIE_INFO(info, 3);
	busno = ls_pcie_init_ctrl(busno, PCIE3, &info);
#endif

#ifdef CONFIG_PCIE4
	SET_LS_PCIE_INFO(info, 4);
	busno = ls_pcie_init_ctrl(busno, PCIE4, &info);
#endif

	return busno;
}

void pci_init_board(void)
{
	ls_pcie_init_board(0);
}

#ifdef CONFIG_OF_BOARD_SETUP
#include <libfdt.h>
#include <fdt_support.h>

static void ft_pcie_ls_setup(void *blob, const char *pci_compat,
			     unsigned long ctrl_addr, enum srds_prtcl dev)
{
	int off;

	off = fdt_node_offset_by_compat_reg(blob, pci_compat,
					    (phys_addr_t)ctrl_addr);
	if (off < 0)
		return;

	if (!is_serdes_configured(dev))
		fdt_set_node_status(blob, off, FDT_STATUS_DISABLED, 0);
}

void ft_pci_setup(void *blob, bd_t *bd)
{
	#ifdef CONFIG_PCIE1
	ft_pcie_ls_setup(blob, FSL_PCIE_COMPAT, CONFIG_SYS_PCIE1_ADDR, PCIE1);
	#endif

	#ifdef CONFIG_PCIE2
	ft_pcie_ls_setup(blob, FSL_PCIE_COMPAT, CONFIG_SYS_PCIE2_ADDR, PCIE2);
	#endif

	#ifdef CONFIG_PCIE3
	ft_pcie_ls_setup(blob, FSL_PCIE_COMPAT, CONFIG_SYS_PCIE3_ADDR, PCIE3);
	#endif

	#ifdef CONFIG_PCIE4
	ft_pcie_ls_setup(blob, FSL_PCIE_COMPAT, CONFIG_SYS_PCIE4_ADDR, PCIE4);
	#endif
}

#else
void ft_pci_setup(void *blob, bd_t *bd)
{
}
#endif

#ifdef CONFIG_LS2085A

void pcie_set_available_streamids(void *blob, const char *pcie_path,
				  u32 *stream_ids, int count)
{
	int nodeoffset;
	int i;

	nodeoffset = fdt_path_offset(blob, pcie_path);
	if (nodeoffset < 0) {
		printf("\n%s: ERROR: unable to update PCIe node\n", __func__);
		return;
	}

	/* for each stream ID, append to mmu-masters */
	for (i = 0; i < count; i++) {
		fdt_appendprop_u32(blob, nodeoffset, "available-stream-ids",
				   stream_ids[i]);
	}
}

#define MAX_STREAM_IDS 4
void fdt_fixup_smmu_pcie(void *blob)
{
	int count;
	u32 stream_ids[MAX_STREAM_IDS];
	u32 ctlr_streamid = 0x300;

	#ifdef CONFIG_PCIE1
	/* PEX1 stream ID fixup */
	count =	FSL_PEX1_STREAM_ID_END - FSL_PEX1_STREAM_ID_START + 1;
	alloc_stream_ids(FSL_PEX1_STREAM_ID_START, count, stream_ids,
			 MAX_STREAM_IDS);
	pcie_set_available_streamids(blob, "/pcie@3400000", stream_ids, count);
	append_mmu_masters(blob, "/iommu@5000000", "/pcie@3400000",
			   &ctlr_streamid, 1);
	#endif

	#ifdef CONFIG_PCIE2
	/* PEX2 stream ID fixup */
	count =	FSL_PEX2_STREAM_ID_END - FSL_PEX2_STREAM_ID_START + 1;
	alloc_stream_ids(FSL_PEX2_STREAM_ID_START, count, stream_ids,
			 MAX_STREAM_IDS);
	pcie_set_available_streamids(blob, "/pcie@3500000", stream_ids, count);
	append_mmu_masters(blob, "/iommu@5000000", "/pcie@3500000",
			   &ctlr_streamid, 1);
	#endif

	#ifdef CONFIG_PCIE3
	/* PEX3 stream ID fixup */
	count =	FSL_PEX3_STREAM_ID_END - FSL_PEX3_STREAM_ID_START + 1;
	alloc_stream_ids(FSL_PEX3_STREAM_ID_START, count, stream_ids,
			 MAX_STREAM_IDS);
	pcie_set_available_streamids(blob, "/pcie@3600000", stream_ids, count);
	append_mmu_masters(blob, "/iommu@5000000", "/pcie@3600000",
			   &ctlr_streamid, 1);
	#endif

	#ifdef CONFIG_PCIE4
	/* PEX4 stream ID fixup */
	count =	FSL_PEX4_STREAM_ID_END - FSL_PEX4_STREAM_ID_START + 1;
	alloc_stream_ids(FSL_PEX4_STREAM_ID_START, count, stream_ids,
			 MAX_STREAM_IDS);
	pcie_set_available_streamids(blob, "/pcie@3700000", stream_ids, count);
	append_mmu_masters(blob, "/iommu@5000000", "/pcie@3700000",
			   &ctlr_streamid, 1);
	#endif
}
#endif
