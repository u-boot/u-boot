

/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
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

#ifndef _PCI_I386_H_
#define _PCI_I386_H_	1

void pci_setup_type1(struct pci_controller* hose, u32 cfg_addr, u32 cfg_data);
int pci_enable_legacy_video_ports(struct pci_controller* hose);
int pci_shadow_rom(pci_dev_t dev, unsigned char *dest);
void pci_remove_rom_window(struct pci_controller* hose, u32 addr);
u32 pci_get_rom_window(struct pci_controller* hose, int size);

#endif
