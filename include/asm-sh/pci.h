/*
 * SH4 PCI Controller (PCIC) for U-Boot.
 * (C) Dustin McIntire (dustin@sensoria.com)
 * (C) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * (C) 2008 Yusuke Goda <goda.yusuke@renesas.com>
 *
 * u-boot/include/asm-sh/pci.h
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
#ifndef _ASM_PCI_H_
#define _ASM_PCI_H_

#include <pci.h>
#if defined(CONFIG_SH7751_PCI)
int pci_sh7751_init(struct pci_controller *hose);
#elif defined(CONFIG_SH7780_PCI)
int pci_sh7780_init(struct pci_controller *hose);
#else
#error "Not support PCI."
#endif

int pci_sh4_init(struct pci_controller *hose);
/* PCI dword read for sh4 */
int pci_sh4_read_config_dword(struct pci_controller *hose,
		pci_dev_t dev, int offset, u32 *value);

/* PCI dword write for sh4 */
int pci_sh4_write_config_dword(struct pci_controller *hose,
		pci_dev_t dev, int offset, u32 value);

#endif	/* _ASM_PCI_H_ */
