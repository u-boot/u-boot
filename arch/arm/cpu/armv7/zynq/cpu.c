/*
 * Copyright (C) 2012 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2012 Xilinx, Inc. All rights reserved.
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
#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>

void lowlevel_init(void)
{
	zynq_slcr_unlock();
	/* remap DDR to zero, FILTERSTART */
	writel(0, &scu_base->filter_start);

	/* Device config APB, unlock the PCAP */
	writel(0x757BDF0D, &devcfg_base->unlock);
	writel(0xFFFFFFFF, &devcfg_base->rom_shadow);

	/* OCM_CFG, Mask out the ROM, map ram into upper addresses */
	writel(0x1F, &slcr_base->ocm_cfg);
	/* FPGA_RST_CTRL, clear resets on AXI fabric ports */
	writel(0x0, &slcr_base->fpga_rst_ctrl);
	/* TZ_DDR_RAM, Set DDR trust zone non-secure */
	writel(0xFFFFFFFF, &slcr_base->trust_zone);
	/* Set urgent bits with register */
	writel(0x0, &slcr_base->ddr_urgent_sel);
	/* Urgent write, ports S2/S3 */
	writel(0xC, &slcr_base->ddr_urgent);

	zynq_slcr_lock();
}

void reset_cpu(ulong addr)
{
	zynq_slcr_cpu_reset();
	while (1)
		;
}
