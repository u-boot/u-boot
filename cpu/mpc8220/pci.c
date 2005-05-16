/*
 * Copyright 2004 Freescale Semiconductor.
 * Copyright (C) 2003 Motorola Inc.
 * Xianghua Xiao (x.xiao@motorola.com)
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * PCI Configuration space access support for MPC8220 PCI Bridge
 */
#include <common.h>
#include <mpc8220.h>
#include <pci.h>
#include <asm/io.h>

#if defined(CONFIG_PCI)

/* System RAM mapped over PCI */
#define CONFIG_PCI_SYS_MEM_BUS	 CFG_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_PHYS	 CFG_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_SIZE	 (1024 * 1024 * 1024)

#define cfg_read(val, addr, type, op)		*val = op((type)(addr));
#define cfg_write(val, addr, type, op)		op((type *)(addr), (val));

#define PCI_OP(rw, size, type, op, mask)				\
int mpc8220_pci_##rw##_config_##size(struct pci_controller *hose,	\
	pci_dev_t dev, int offset, type val)				\
{									\
	u32 addr = 0;							\
	u16 cfg_type = 0;						\
	addr = ((offset & 0xfc) | cfg_type | (dev)  | 0x80000000);	\
	out_be32(hose->cfg_addr, addr);					\
	__asm__ __volatile__("sync");					\
	cfg_##rw(val, hose->cfg_data + (offset & mask), type, op);	\
	out_be32(hose->cfg_addr, addr & 0x7fffffff);			\
	__asm__ __volatile__("sync");					\
	return 0;							\
}

PCI_OP(read, byte, u8 *, in_8, 3)
PCI_OP(read, word, u16 *, in_le16, 2)
PCI_OP(write, byte, u8, out_8, 3)
PCI_OP(write, word, u16, out_le16, 2)
PCI_OP(write, dword, u32, out_le32, 0)

int mpc8220_pci_read_config_dword(struct pci_controller *hose, pci_dev_t dev,
	int offset, u32 *val)
{
	u32 addr;
	u32 tmpv;
	u32 mask = 2;	 /* word access */
	/* Read lower 16 bits */
	addr = ((offset & 0xfc) | (dev) | 0x80000000);
	out_be32(hose->cfg_addr, addr);
	__asm__ __volatile__("sync");
	*val = (u32) in_le16((u16 *) (hose->cfg_data + (offset & mask)));
	out_be32(hose->cfg_addr, addr & 0x7fffffff);
	__asm__ __volatile__("sync");

	/* Read upper 16 bits */
	offset += 2;
	addr = ((offset & 0xfc) | 1 | (dev) | 0x80000000);
	out_be32(hose->cfg_addr, addr);
	__asm__ __volatile__("sync");
	tmpv = (u32) in_le16((u16 *) (hose->cfg_data + (offset & mask)));
	out_be32(hose->cfg_addr, addr & 0x7fffffff);
	__asm__ __volatile__("sync");

	/* combine results into dword value */
	*val = (tmpv << 16) | *val;

	return 0;
}

void
pci_mpc8220_init(struct pci_controller *hose)
{
	u32 win0, win1, win2;
	volatile mpc8220_xcpci_t *xcpci =
		(volatile mpc8220_xcpci_t *) MMAP_XCPCI;

	volatile pcfg8220_t *portcfg = (volatile pcfg8220_t *) MMAP_PCFG;

	win0 = (u32) CONFIG_PCI_MEM_PHYS;
	win1 = (u32) CONFIG_PCI_IO_PHYS;
	win2 = (u32) CONFIG_PCI_CFG_PHYS;

	/* Assert PCI reset */
	out_be32 (&xcpci->glb_stat_ctl, PCI_GLB_STAT_CTRL_PR);

	/* Disable prefetching but read-multiples will still prefetch */
	out_be32 (&xcpci->target_ctrl, 0x00000000);

	/* Initiator windows */
	out_be32 (&xcpci->init_win0,  (win0 >> 16) | win0 | 0x003f0000);
	out_be32 (&xcpci->init_win1, ((win1 >> 16) | win1 ));
	out_be32 (&xcpci->init_win2, ((win2 >> 16) | win2 ));

	out_be32 (&xcpci->init_win_cfg,
		PCI_INIT_WIN_CFG_WIN0_CTRL_EN |
		PCI_INIT_WIN_CFG_WIN1_CTRL_EN | PCI_INIT_WIN_CFG_WIN1_CTRL_IO |
		PCI_INIT_WIN_CFG_WIN2_CTRL_EN | PCI_INIT_WIN_CFG_WIN2_CTRL_IO);

	out_be32 (&xcpci->init_ctrl, 0x00000000);

	/* Enable bus master and mem access */
	out_be32 (&xcpci->stat_cmd_reg, PCI_STAT_CMD_B | PCI_STAT_CMD_M);

	/* Cache line size and master latency */
	out_be32 (&xcpci->bist_htyp_lat_cshl, (0xf8 << PCI_CFG1_LT_SHIFT));

	out_be32 (&xcpci->base0, PCI_BASE_ADDR_REG0); /* 256MB - MBAR space */
	out_be32 (&xcpci->base1, PCI_BASE_ADDR_REG1); /* 1GB - SDRAM space */

	out_be32 (&xcpci->target_bar0,
		PCI_TARGET_BASE_ADDR_REG0 | PCI_TARGET_BASE_ADDR_EN);
	out_be32 (&xcpci->target_bar1,
		PCI_TARGET_BASE_ADDR_REG1 | PCI_TARGET_BASE_ADDR_EN);

	/* Deassert reset bit */
	out_be32 (&xcpci->glb_stat_ctl, 0x00000000);

	/* Enable PCI bus master support */
	/* Set PCIGNT1, PCIREQ1, PCIREQ0/PCIGNTIN, PCIGNT0/PCIREQOUT,
	       PCIREQ2, PCIGNT2 */
	out_be32((volatile u32 *)&portcfg->pcfg3,
		(in_be32((volatile u32 *)&portcfg->pcfg3) & 0xFC3FCE7F));
	out_be32((volatile u32 *)&portcfg->pcfg3,
		(in_be32((volatile u32 *)&portcfg->pcfg3) | 0x01400180));

	hose->first_busno = 0;
	hose->last_busno = 0xff;

	pci_set_region(hose->regions + 0,
		CONFIG_PCI_MEM_BUS,
		CONFIG_PCI_MEM_PHYS,
		CONFIG_PCI_MEM_SIZE,
		PCI_REGION_MEM);

	pci_set_region(hose->regions + 1,
		CONFIG_PCI_IO_BUS,
		CONFIG_PCI_IO_PHYS,
		CONFIG_PCI_IO_SIZE,
		PCI_REGION_IO);

	pci_set_region(hose->regions + 2,
		CONFIG_PCI_SYS_MEM_BUS,
		CONFIG_PCI_SYS_MEM_PHYS,
		CONFIG_PCI_SYS_MEM_SIZE,
		PCI_REGION_MEM | PCI_REGION_MEMORY);

	hose->region_count = 3;

	hose->cfg_addr = &(xcpci->cfg_adr);
	hose->cfg_data = CONFIG_PCI_CFG_BUS;

	pci_set_ops(hose,
		mpc8220_pci_read_config_byte,
		mpc8220_pci_read_config_word,
		mpc8220_pci_read_config_dword,
		mpc8220_pci_write_config_byte,
		mpc8220_pci_write_config_word,
		mpc8220_pci_write_config_dword);

	/* Hose scan */
	pci_register_hose(hose);
	hose->last_busno = pci_hose_scan(hose);

	out_be32 (&xcpci->base0, PCI_BASE_ADDR_REG0); /* 256MB - MBAR space */
	out_be32 (&xcpci->base1, PCI_BASE_ADDR_REG1); /* 1GB - SDRAM space */
}

#endif /* CONFIG_PCI */
