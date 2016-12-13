/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * Layerscape PCIe driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PCIE_LAYERSCAPE_H_
#define _PCIE_LAYERSCAPE_H_
#include <pci.h>

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
#define PCIE_ATU_REGION_NUM		6
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

/* DBI registers */
#define PCIE_SRIOV		0x178
#define PCIE_STRFMR1		0x71c /* Symbol Timer & Filter Mask Register1 */
#define PCIE_DBI_RO_WR_EN	0x8bc

#define PCIE_LINK_CAP		0x7c
#define PCIE_LINK_SPEED_MASK	0xf
#define PCIE_LINK_WIDTH_MASK	0x3f0
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
	int next_lut_index;
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

#endif /* _PCIE_LAYERSCAPE_H_ */
