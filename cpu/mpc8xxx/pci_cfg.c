/*
 * Copyright 2009 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <pci.h>

struct pci_info {
	u16	cfg;
};

/* The cfg field is a bit mask in which each bit represents the value of
 * cfg_IO_ports[] signal and the bit is set if the interface would be
 * enabled based on the value of cfg_IO_ports[] signal
 *
 * On MPC86xx/PQ3 based systems:
 *   we extract cfg_IO_ports from GUTS register PORDEVSR
 *
 * cfg_IO_ports only exist on systems w/PCIe (we set cfg 0 for systems
 * without PCIe)
 */

#if defined(CONFIG_MPC8540) || defined(CONFIG_MPC8560)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCI] = {
		.cfg =   0,
	},
};
#elif defined(CONFIG_MPC8541) || defined(CONFIG_MPC8555)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCI] = {
		.cfg =   0,
	},
};
#elif defined(CONFIG_MPC8536)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCI] = {
		.cfg =   0,
	},
	[LAW_TRGT_IF_PCIE_1] = {
		.cfg =   (1 << 2) | (1 << 3) | (1 << 5) | (1 << 7),
	},
	[LAW_TRGT_IF_PCIE_2] = {
		.cfg =   (1 << 5) | (1 << 7),
	},
	[LAW_TRGT_IF_PCIE_3] = {
		.cfg =   (1 << 7),
	},
};
#elif defined(CONFIG_MPC8544)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCI] = {
		.cfg =   0,
	},
	[LAW_TRGT_IF_PCIE_1] = {
		.cfg =   (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) |
			 (1 << 6) | (1 << 7),
	},
	[LAW_TRGT_IF_PCIE_2] = {
		.cfg =   (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7),
	},
	[LAW_TRGT_IF_PCIE_3] = {
		.cfg =   (1 << 6) | (1 << 7),
	},
};
#elif defined(CONFIG_MPC8548)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCI_1] = {
		.cfg =   0,
	},
	[LAW_TRGT_IF_PCI_2] = {
		.cfg =   0,
	},
	/* PCI_2 is always host and we dont use iosel to determine enable/disable */
	[LAW_TRGT_IF_PCIE_1] = {
		.cfg =   (1 << 3) | (1 << 4) | (1 << 7),
	},
};
#elif defined(CONFIG_MPC8568)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCI] = {
		.cfg =   0,
	},
	[LAW_TRGT_IF_PCIE_1] = {
		.cfg =   (1 << 3) | (1 << 4) | (1 << 7),
	},
};
#elif defined(CONFIG_MPC8569)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCIE_1] = {
		.cfg =   (1 << 0) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) |
			 (1 << 8) | (1 << 0xc) | (1 << 0xf),
	},
};
#elif defined(CONFIG_MPC8572)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCIE_1] = {
		.cfg =   (1 << 2) | (1 << 3) | (1 << 7) |
			 (1 << 0xb) | (1 << 0xc) | (1 << 0xf),
	},
	[LAW_TRGT_IF_PCIE_2] = {
		.cfg =   (1 << 3) | (1 << 7),
	},
	[LAW_TRGT_IF_PCIE_3] = {
		.cfg =   (1 << 7),
	},
};
#elif defined(CONFIG_MPC8610)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCI_1] = {
		.cfg =   0,
	},
	[LAW_TRGT_IF_PCIE_1] = {
		.cfg =   (1 << 1) | (1 << 4),
	},
	[LAW_TRGT_IF_PCIE_2] = {
		.cfg =   (1 << 0) | (1 << 4),
	},
};
#elif defined(CONFIG_MPC8641)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCIE_1] = {
		.cfg =   (1 << 2) | (1 << 3) | (1 << 5) | (1 << 6) |
			 (1 << 7) | (1 << 0xe) | (1 << 0xf),
	},
};
#elif defined(CONFIG_P1011) || defined(CONFIG_P1020)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCIE_1] = {
		.cfg =   (1 << 0) | (1 << 6) | (1 << 0xe) | (1 << 0xf),
	},
	[LAW_TRGT_IF_PCIE_2] = {
		.cfg =   (1 << 0xe),
	},
};
#elif defined(CONFIG_P2010) || defined(CONFIG_P2020)
static struct pci_info pci_config_info[] =
{
	[LAW_TRGT_IF_PCIE_1] = {
		.cfg =   (1 << 0) | (1 << 2) | (1 << 4) | (1 << 6) |
			 (1 << 0xd) | (1 << 0xe) | (1 << 0xf),
	},
	[LAW_TRGT_IF_PCIE_2] = {
		.cfg =   (1 << 2) | (1 << 0xe),
	},
	[LAW_TRGT_IF_PCIE_3] = {
		.cfg =   (1 << 2) | (1 << 4),
	},
};
#elif defined(CONFIG_FSL_CORENET)
#else
#error Need to define pci_config_info for processor
#endif

#ifndef CONFIG_FSL_CORENET
int is_fsl_pci_cfg(enum law_trgt_if trgt, u32 io_sel)
{
	return ((1 << io_sel) & pci_config_info[trgt].cfg);
}
#endif
