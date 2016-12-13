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
#include <dm.h>
#ifndef CONFIG_LS102XA
#include <asm/arch/fdt.h>
#include <asm/arch/soc.h>
#else
#include <asm/arch/immap_ls102xa.h>
#endif
#include "pcie_layerscape.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_DM_PCI
#ifdef CONFIG_LS102XA
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

	state = pex_lut_in32(pcie->dbi + PCIE_LUT_BASE + PCIE_LUT_DBG) &
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
		return 0;
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
#ifndef CONFIG_LS102XA
				writel(PCIE_LCTRL0_VAL(pf, vf),
				       pcie->dbi + PCIE_LUT_BASE +
				       PCIE_LUT_LCTRL0);
#endif
				ls_pcie_ep_setup_bars(pcie->dbi);
				ls_pcie_ep_setup_atu(pcie, info);
			}
		}

		/* Disable CFG2 */
#ifndef CONFIG_LS102XA
		writel(0, pcie->dbi + PCIE_LUT_BASE + PCIE_LUT_LCTRL0);
#endif
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
	pcie->next_lut_index = 0;

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

#else
LIST_HEAD(ls_pcie_list);

static unsigned int dbi_readl(struct ls_pcie *pcie, unsigned int offset)
{
	return in_le32(pcie->dbi + offset);
}

static void dbi_writel(struct ls_pcie *pcie, unsigned int value,
		       unsigned int offset)
{
	out_le32(pcie->dbi + offset, value);
}

static unsigned int ctrl_readl(struct ls_pcie *pcie, unsigned int offset)
{
	if (pcie->big_endian)
		return in_be32(pcie->ctrl + offset);
	else
		return in_le32(pcie->ctrl + offset);
}

static void ctrl_writel(struct ls_pcie *pcie, unsigned int value,
			unsigned int offset)
{
	if (pcie->big_endian)
		out_be32(pcie->ctrl + offset, value);
	else
		out_le32(pcie->ctrl + offset, value);
}

static int ls_pcie_ltssm(struct ls_pcie *pcie)
{
	u32 state;
	uint svr;

	svr = get_svr();
	if (((svr >> SVR_VAR_PER_SHIFT) & SVR_LS102XA_MASK) == SVR_LS102XA) {
		state = ctrl_readl(pcie, LS1021_PEXMSCPORTSR(pcie->idx));
		state = (state >> LS1021_LTSSM_STATE_SHIFT) & LTSSM_STATE_MASK;
	} else {
		state = ctrl_readl(pcie, PCIE_PF_DBG) & LTSSM_STATE_MASK;
	}

	return state;
}

static int ls_pcie_link_up(struct ls_pcie *pcie)
{
	int ltssm;

	ltssm = ls_pcie_ltssm(pcie);
	if (ltssm < LTSSM_PCIE_L0)
		return 0;

	return 1;
}

static void ls_pcie_cfg0_set_busdev(struct ls_pcie *pcie, u32 busdev)
{
	dbi_writel(pcie, PCIE_ATU_REGION_OUTBOUND | PCIE_ATU_REGION_INDEX0,
		   PCIE_ATU_VIEWPORT);
	dbi_writel(pcie, busdev, PCIE_ATU_LOWER_TARGET);
}

static void ls_pcie_cfg1_set_busdev(struct ls_pcie *pcie, u32 busdev)
{
	dbi_writel(pcie, PCIE_ATU_REGION_OUTBOUND | PCIE_ATU_REGION_INDEX1,
		   PCIE_ATU_VIEWPORT);
	dbi_writel(pcie, busdev, PCIE_ATU_LOWER_TARGET);
}

static void ls_pcie_atu_outbound_set(struct ls_pcie *pcie, int idx, int type,
				      u64 phys, u64 bus_addr, pci_size_t size)
{
	dbi_writel(pcie, PCIE_ATU_REGION_OUTBOUND | idx, PCIE_ATU_VIEWPORT);
	dbi_writel(pcie, (u32)phys, PCIE_ATU_LOWER_BASE);
	dbi_writel(pcie, phys >> 32, PCIE_ATU_UPPER_BASE);
	dbi_writel(pcie, (u32)phys + size - 1, PCIE_ATU_LIMIT);
	dbi_writel(pcie, (u32)bus_addr, PCIE_ATU_LOWER_TARGET);
	dbi_writel(pcie, bus_addr >> 32, PCIE_ATU_UPPER_TARGET);
	dbi_writel(pcie, type, PCIE_ATU_CR1);
	dbi_writel(pcie, PCIE_ATU_ENABLE, PCIE_ATU_CR2);
}

/* Use bar match mode and MEM type as default */
static void ls_pcie_atu_inbound_set(struct ls_pcie *pcie, int idx,
				     int bar, u64 phys)
{
	dbi_writel(pcie, PCIE_ATU_REGION_INBOUND | idx, PCIE_ATU_VIEWPORT);
	dbi_writel(pcie, (u32)phys, PCIE_ATU_LOWER_TARGET);
	dbi_writel(pcie, phys >> 32, PCIE_ATU_UPPER_TARGET);
	dbi_writel(pcie, PCIE_ATU_TYPE_MEM, PCIE_ATU_CR1);
	dbi_writel(pcie, PCIE_ATU_ENABLE | PCIE_ATU_BAR_MODE_ENABLE |
		   PCIE_ATU_BAR_NUM(bar), PCIE_ATU_CR2);
}

static void ls_pcie_dump_atu(struct ls_pcie *pcie)
{
	int i;

	for (i = 0; i < PCIE_ATU_REGION_NUM; i++) {
		dbi_writel(pcie, PCIE_ATU_REGION_OUTBOUND | i,
			   PCIE_ATU_VIEWPORT);
		debug("iATU%d:\n", i);
		debug("\tLOWER PHYS 0x%08x\n",
		      dbi_readl(pcie, PCIE_ATU_LOWER_BASE));
		debug("\tUPPER PHYS 0x%08x\n",
		      dbi_readl(pcie, PCIE_ATU_UPPER_BASE));
		debug("\tLOWER BUS  0x%08x\n",
		      dbi_readl(pcie, PCIE_ATU_LOWER_TARGET));
		debug("\tUPPER BUS  0x%08x\n",
		      dbi_readl(pcie, PCIE_ATU_UPPER_TARGET));
		debug("\tLIMIT      0x%08x\n",
		      readl(pcie->dbi + PCIE_ATU_LIMIT));
		debug("\tCR1        0x%08x\n",
		      dbi_readl(pcie, PCIE_ATU_CR1));
		debug("\tCR2        0x%08x\n",
		      dbi_readl(pcie, PCIE_ATU_CR2));
	}
}

static void ls_pcie_setup_atu(struct ls_pcie *pcie)
{
	struct pci_region *io, *mem, *pref;
	unsigned long long offset = 0;
	int idx = 0;
	uint svr;

	svr = get_svr();
	if (((svr >> SVR_VAR_PER_SHIFT) & SVR_LS102XA_MASK) == SVR_LS102XA) {
		offset = LS1021_PCIE_SPACE_OFFSET +
			 LS1021_PCIE_SPACE_SIZE * pcie->idx;
	}

	/* ATU 0 : OUTBOUND : CFG0 */
	ls_pcie_atu_outbound_set(pcie, PCIE_ATU_REGION_INDEX0,
				 PCIE_ATU_TYPE_CFG0,
				 pcie->cfg_res.start + offset,
				 0,
				 fdt_resource_size(&pcie->cfg_res) / 2);
	/* ATU 1 : OUTBOUND : CFG1 */
	ls_pcie_atu_outbound_set(pcie, PCIE_ATU_REGION_INDEX1,
				 PCIE_ATU_TYPE_CFG1,
				 pcie->cfg_res.start + offset +
				 fdt_resource_size(&pcie->cfg_res) / 2,
				 0,
				 fdt_resource_size(&pcie->cfg_res) / 2);

	pci_get_regions(pcie->bus, &io, &mem, &pref);
	idx = PCIE_ATU_REGION_INDEX1 + 1;

	if (io)
		/* ATU : OUTBOUND : IO */
		ls_pcie_atu_outbound_set(pcie, idx++,
					 PCIE_ATU_TYPE_IO,
					 io->phys_start + offset,
					 io->bus_start,
					 io->size);

	if (mem)
		/* ATU : OUTBOUND : MEM */
		ls_pcie_atu_outbound_set(pcie, idx++,
					 PCIE_ATU_TYPE_MEM,
					 mem->phys_start + offset,
					 mem->bus_start,
					 mem->size);

	if (pref)
		/* ATU : OUTBOUND : pref */
		ls_pcie_atu_outbound_set(pcie, idx++,
					 PCIE_ATU_TYPE_MEM,
					 pref->phys_start + offset,
					 pref->bus_start,
					 pref->size);

	ls_pcie_dump_atu(pcie);
}

/* Return 0 if the address is valid, -errno if not valid */
static int ls_pcie_addr_valid(struct ls_pcie *pcie, pci_dev_t bdf)
{
	struct udevice *bus = pcie->bus;

	if (!pcie->enabled)
		return -ENXIO;

	if (PCI_BUS(bdf) < bus->seq)
		return -EINVAL;

	if ((PCI_BUS(bdf) > bus->seq) && (!ls_pcie_link_up(pcie)))
		return -EINVAL;

	if (PCI_BUS(bdf) <= (bus->seq + 1) && (PCI_DEV(bdf) > 0))
		return -EINVAL;

	return 0;
}

void *ls_pcie_conf_address(struct ls_pcie *pcie, pci_dev_t bdf,
				   int offset)
{
	struct udevice *bus = pcie->bus;
	u32 busdev;

	if (PCI_BUS(bdf) == bus->seq)
		return pcie->dbi + offset;

	busdev = PCIE_ATU_BUS(PCI_BUS(bdf)) |
		 PCIE_ATU_DEV(PCI_DEV(bdf)) |
		 PCIE_ATU_FUNC(PCI_FUNC(bdf));

	if (PCI_BUS(bdf) == bus->seq + 1) {
		ls_pcie_cfg0_set_busdev(pcie, busdev);
		return pcie->cfg0 + offset;
	} else {
		ls_pcie_cfg1_set_busdev(pcie, busdev);
		return pcie->cfg1 + offset;
	}
}

static int ls_pcie_read_config(struct udevice *bus, pci_dev_t bdf,
			       uint offset, ulong *valuep,
			       enum pci_size_t size)
{
	struct ls_pcie *pcie = dev_get_priv(bus);
	void *address;

	if (ls_pcie_addr_valid(pcie, bdf)) {
		*valuep = pci_get_ff(size);
		return 0;
	}

	address = ls_pcie_conf_address(pcie, bdf, offset);

	switch (size) {
	case PCI_SIZE_8:
		*valuep = readb(address);
		return 0;
	case PCI_SIZE_16:
		*valuep = readw(address);
		return 0;
	case PCI_SIZE_32:
		*valuep = readl(address);
		return 0;
	default:
		return -EINVAL;
	}
}

static int ls_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong value,
				enum pci_size_t size)
{
	struct ls_pcie *pcie = dev_get_priv(bus);
	void *address;

	if (ls_pcie_addr_valid(pcie, bdf))
		return 0;

	address = ls_pcie_conf_address(pcie, bdf, offset);

	switch (size) {
	case PCI_SIZE_8:
		writeb(value, address);
		return 0;
	case PCI_SIZE_16:
		writew(value, address);
		return 0;
	case PCI_SIZE_32:
		writel(value, address);
		return 0;
	default:
		return -EINVAL;
	}
}

/* Clear multi-function bit */
static void ls_pcie_clear_multifunction(struct ls_pcie *pcie)
{
	writeb(PCI_HEADER_TYPE_BRIDGE, pcie->dbi + PCI_HEADER_TYPE);
}

/* Fix class value */
static void ls_pcie_fix_class(struct ls_pcie *pcie)
{
	writew(PCI_CLASS_BRIDGE_PCI, pcie->dbi + PCI_CLASS_DEVICE);
}

/* Drop MSG TLP except for Vendor MSG */
static void ls_pcie_drop_msg_tlp(struct ls_pcie *pcie)
{
	u32 val;

	val = dbi_readl(pcie, PCIE_STRFMR1);
	val &= 0xDFFFFFFF;
	dbi_writel(pcie, val, PCIE_STRFMR1);
}

/* Disable all bars in RC mode */
static void ls_pcie_disable_bars(struct ls_pcie *pcie)
{
	u32 sriov;

	sriov = in_le32(pcie->dbi + PCIE_SRIOV);

	/*
	 * TODO: For PCIe controller with SRIOV, the method to disable bars
	 * is different and more complex, so will add later.
	 */
	if (PCI_EXT_CAP_ID(sriov) == PCI_EXT_CAP_ID_SRIOV)
		return;

	dbi_writel(pcie, 0, PCIE_CS2_OFFSET + PCI_BASE_ADDRESS_0);
	dbi_writel(pcie, 0, PCIE_CS2_OFFSET + PCI_BASE_ADDRESS_1);
	dbi_writel(pcie, 0, PCIE_CS2_OFFSET + PCI_ROM_ADDRESS1);
}

static void ls_pcie_setup_ctrl(struct ls_pcie *pcie)
{
	ls_pcie_setup_atu(pcie);

	dbi_writel(pcie, 1, PCIE_DBI_RO_WR_EN);
	ls_pcie_fix_class(pcie);
	ls_pcie_clear_multifunction(pcie);
	ls_pcie_drop_msg_tlp(pcie);
	dbi_writel(pcie, 0, PCIE_DBI_RO_WR_EN);

	ls_pcie_disable_bars(pcie);
}

static void ls_pcie_ep_setup_atu(struct ls_pcie *pcie)
{
	u64 phys = CONFIG_SYS_PCI_EP_MEMORY_BASE;

	/* ATU 0 : INBOUND : map BAR0 */
	ls_pcie_atu_inbound_set(pcie, 0, 0, phys);
	/* ATU 1 : INBOUND : map BAR1 */
	phys += PCIE_BAR1_SIZE;
	ls_pcie_atu_inbound_set(pcie, 1, 1, phys);
	/* ATU 2 : INBOUND : map BAR2 */
	phys += PCIE_BAR2_SIZE;
	ls_pcie_atu_inbound_set(pcie, 2, 2, phys);
	/* ATU 3 : INBOUND : map BAR4 */
	phys = CONFIG_SYS_PCI_EP_MEMORY_BASE + PCIE_BAR4_SIZE;
	ls_pcie_atu_inbound_set(pcie, 3, 4, phys);

	/* ATU 0 : OUTBOUND : map MEM */
	ls_pcie_atu_outbound_set(pcie, 0,
				 PCIE_ATU_TYPE_MEM,
				 pcie->cfg_res.start,
				 0,
				 CONFIG_SYS_PCI_MEMORY_SIZE);
}

/* BAR0 and BAR1 are 32bit BAR2 and BAR4 are 64bit */
static void ls_pcie_ep_setup_bar(void *bar_base, int bar, u32 size)
{
	/* The least inbound window is 4KiB */
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

static void ls_pcie_setup_ep(struct ls_pcie *pcie)
{
	u32 sriov;

	sriov = readl(pcie->dbi + PCIE_SRIOV);
	if (PCI_EXT_CAP_ID(sriov) == PCI_EXT_CAP_ID_SRIOV) {
		int pf, vf;

		for (pf = 0; pf < PCIE_PF_NUM; pf++) {
			for (vf = 0; vf <= PCIE_VF_NUM; vf++) {
				ctrl_writel(pcie, PCIE_LCTRL0_VAL(pf, vf),
					    PCIE_PF_VF_CTRL);

				ls_pcie_ep_setup_bars(pcie->dbi);
				ls_pcie_ep_setup_atu(pcie);
			}
		}
		/* Disable CFG2 */
		ctrl_writel(pcie, 0, PCIE_PF_VF_CTRL);
	} else {
		ls_pcie_ep_setup_bars(pcie->dbi + PCIE_NO_SRIOV_BAR_BASE);
		ls_pcie_ep_setup_atu(pcie);
	}
}

static int ls_pcie_probe(struct udevice *dev)
{
	struct ls_pcie *pcie = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev->of_offset;
	u8 header_type;
	u16 link_sta;
	bool ep_mode;
	int ret;

	pcie->bus = dev;

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "dbi", &pcie->dbi_res);
	if (ret) {
		printf("ls-pcie: resource \"dbi\" not found\n");
		return ret;
	}

	pcie->idx = (pcie->dbi_res.start - PCIE_SYS_BASE_ADDR) / PCIE_CCSR_SIZE;

	list_add(&pcie->list, &ls_pcie_list);

	pcie->enabled = is_serdes_configured(PCIE_SRDS_PRTCL(pcie->idx));
	if (!pcie->enabled) {
		printf("PCIe%d: %s disabled\n", pcie->idx, dev->name);
		return 0;
	}

	pcie->dbi = map_physmem(pcie->dbi_res.start,
				fdt_resource_size(&pcie->dbi_res),
				MAP_NOCACHE);

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "lut", &pcie->lut_res);
	if (!ret)
		pcie->lut = map_physmem(pcie->lut_res.start,
					fdt_resource_size(&pcie->lut_res),
					MAP_NOCACHE);

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "ctrl", &pcie->ctrl_res);
	if (!ret)
		pcie->ctrl = map_physmem(pcie->ctrl_res.start,
					 fdt_resource_size(&pcie->ctrl_res),
					 MAP_NOCACHE);
	if (!pcie->ctrl)
		pcie->ctrl = pcie->lut;

	if (!pcie->ctrl) {
		printf("%s: NOT find CTRL\n", dev->name);
		return -1;
	}

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "config", &pcie->cfg_res);
	if (ret) {
		printf("%s: resource \"config\" not found\n", dev->name);
		return ret;
	}

	pcie->cfg0 = map_physmem(pcie->cfg_res.start,
				 fdt_resource_size(&pcie->cfg_res),
				 MAP_NOCACHE);
	pcie->cfg1 = pcie->cfg0 + fdt_resource_size(&pcie->cfg_res) / 2;

	pcie->big_endian = fdtdec_get_bool(fdt, node, "big-endian");

	debug("%s dbi:%lx lut:%lx ctrl:0x%lx cfg0:0x%lx, big-endian:%d\n",
	      dev->name, (unsigned long)pcie->dbi, (unsigned long)pcie->lut,
	      (unsigned long)pcie->ctrl, (unsigned long)pcie->cfg0,
	      pcie->big_endian);

	header_type = readb(pcie->dbi + PCI_HEADER_TYPE);
	ep_mode = (header_type & 0x7f) == PCI_HEADER_TYPE_NORMAL;
	printf("PCIe%u: %s %s", pcie->idx, dev->name,
	       ep_mode ? "Endpoint" : "Root Complex");

	if (ep_mode)
		ls_pcie_setup_ep(pcie);
	else
		ls_pcie_setup_ctrl(pcie);

	if (!ls_pcie_link_up(pcie)) {
		/* Let the user know there's no PCIe link */
		printf(": no link\n");
		return 0;
	}

	/* Print the negotiated PCIe link width */
	link_sta = readw(pcie->dbi + PCIE_LINK_STA);
	printf(": x%d gen%d\n", (link_sta & PCIE_LINK_WIDTH_MASK) >> 4,
	       link_sta & PCIE_LINK_SPEED_MASK);

	return 0;
}

static const struct dm_pci_ops ls_pcie_ops = {
	.read_config	= ls_pcie_read_config,
	.write_config	= ls_pcie_write_config,
};

static const struct udevice_id ls_pcie_ids[] = {
	{ .compatible = "fsl,ls-pcie" },
	{ }
};

U_BOOT_DRIVER(pci_layerscape) = {
	.name = "pci_layerscape",
	.id = UCLASS_PCI,
	.of_match = ls_pcie_ids,
	.ops = &ls_pcie_ops,
	.probe	= ls_pcie_probe,
	.priv_auto_alloc_size = sizeof(struct ls_pcie),
};
#endif /* CONFIG_DM_PCI */
