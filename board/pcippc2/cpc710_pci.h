/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef _CPC710_PCI_H_
#define _CPC710_PCI_H_

#define PCI_MEMORY_PHYS		0x00000000
#define PCI_MEMORY_BUS		0x80000000
#define PCI_MEMORY_MAXSIZE      0x20000000

#define BRIDGE_CPCI_PHYS	0xff500000
#define BRIDGE_CPCI_MEM_SIZE	0x08000000
#define BRIDGE_CPCI_MEM_PHYS    0xf0000000
#define BRIDGE_CPCI_MEM_BUS     0x00000000
#define BRIDGE_CPCI_IO_SIZE	0x02000000
#define BRIDGE_CPCI_IO_PHYS     0xfc000000
#define BRIDGE_CPCI_IO_BUS      0x00000000

#define BRIDGE_LOCAL_PHYS	0xff400000
#define BRIDGE_LOCAL_MEM_SIZE	0x04000000
#define BRIDGE_LOCAL_MEM_PHYS   0xf8000000
#define BRIDGE_LOCAL_MEM_BUS    0x40000000
#define BRIDGE_LOCAL_IO_SIZE	0x01000000
#define BRIDGE_LOCAL_IO_PHYS    0xfe000000
#define BRIDGE_LOCAL_IO_BUS     0x04000000

#define BRIDGE(r, x)		(BRIDGE_##r##_PHYS + HW_BRIDGE_##x)

#define PCI_LATENCY_TIMER_VAL	0xff

#endif
