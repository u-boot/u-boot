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

#ifndef _PCIPPC2_H_
#define _PCIPPC2_H_

#include <config.h>
#include <common.h>

#include "hardware.h"

#define FPGA(r, p)	(pcippc2_fpga0_phys + HW_FPGA0_##r##_##p)
#define UART(r)		(pcippc2_fpga0_phys + HW_FPGA0_UART1 + NS16550_##r * 4)
#define RTC(r)		(pcippc2_fpga1_phys + HW_FPGA1_RTC + r)

extern u32		pcippc2_fpga0_phys;
extern u32		pcippc2_fpga1_phys;

extern u32	pcippc2_sdram_size		(void);

extern void 	pcippc2_fpga_init		(void);

extern void 	pcippc2_cpci3264_init	(void);

extern void 	cpc710_pci_init			(void);
extern void	cpc710_pci_enable_timeout	(void);

extern unsigned long
		cpc710_ram_init			(void);

#endif
