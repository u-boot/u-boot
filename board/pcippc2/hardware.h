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

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include "cpc710.h"
#include "cpc710_pci.h"
#include "pcippc2_fpga.h"
#include "ns16550.h"

#define REG(r, x)	(HW_PHYS_##r + HW_##r##_##x)

  /* Address map:
   *
   * 0x00000000-0x20000000	SDRAM
   * 0x40000000-0x00008000	Init RAM in the CPU DCache
   * 0xf0000000-0xf8000000	CPCI MEM
   * 0xf8000000-0xfc000000	Local PCI MEM
   * 0xfc000000-0xfe000000	CPCI I/O
   * 0xfe000000-0xff000000	Local PCI I/O
   * 0xff000000-0xff201000	System configuration space
   * 0xff400000-0xff500000	Local PCI bridge space
   * 0xff500000-0xff600000	CPCI bridge space
   * 0xfff00000-0xfff80000	Boot Flash
   */

#endif
