/*
 * Copyright (C) 2013 Boundary Devices Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef __ASM_ARCH_MX6Q_DDR_H__
#define __ASM_ARCH_MX6Q_DDR_H__

#ifndef CONFIG_MX6Q
#error "wrong CPU"
#endif

#define MX6_IOM_DRAM_DQM0	0x020e05ac
#define MX6_IOM_DRAM_DQM1	0x020e05b4
#define MX6_IOM_DRAM_DQM2	0x020e0528
#define MX6_IOM_DRAM_DQM3	0x020e0520
#define MX6_IOM_DRAM_DQM4	0x020e0514
#define MX6_IOM_DRAM_DQM5	0x020e0510
#define MX6_IOM_DRAM_DQM6	0x020e05bc
#define MX6_IOM_DRAM_DQM7	0x020e05c4

#define MX6_IOM_DRAM_CAS	0x020e056c
#define MX6_IOM_DRAM_RAS	0x020e0578
#define MX6_IOM_DRAM_RESET	0x020e057c
#define MX6_IOM_DRAM_SDCLK_0	0x020e0588
#define MX6_IOM_DRAM_SDCLK_1	0x020e0594
#define MX6_IOM_DRAM_SDBA2	0x020e058c
#define MX6_IOM_DRAM_SDCKE0	0x020e0590
#define MX6_IOM_DRAM_SDCKE1	0x020e0598
#define MX6_IOM_DRAM_SDODT0	0x020e059c
#define MX6_IOM_DRAM_SDODT1	0x020e05a0

#define MX6_IOM_DRAM_SDQS0	0x020e05a8
#define MX6_IOM_DRAM_SDQS1	0x020e05b0
#define MX6_IOM_DRAM_SDQS2	0x020e0524
#define MX6_IOM_DRAM_SDQS3	0x020e051c
#define MX6_IOM_DRAM_SDQS4	0x020e0518
#define MX6_IOM_DRAM_SDQS5	0x020e050c
#define MX6_IOM_DRAM_SDQS6	0x020e05b8
#define MX6_IOM_DRAM_SDQS7	0x020e05c0

#define MX6_IOM_GRP_B0DS	0x020e0784
#define MX6_IOM_GRP_B1DS	0x020e0788
#define MX6_IOM_GRP_B2DS	0x020e0794
#define MX6_IOM_GRP_B3DS	0x020e079c
#define MX6_IOM_GRP_B4DS	0x020e07a0
#define MX6_IOM_GRP_B5DS	0x020e07a4
#define MX6_IOM_GRP_B6DS	0x020e07a8
#define MX6_IOM_GRP_B7DS	0x020e0748
#define MX6_IOM_GRP_ADDDS	0x020e074c
#define MX6_IOM_DDRMODE_CTL	0x020e0750
#define MX6_IOM_GRP_DDRPKE	0x020e0758
#define MX6_IOM_GRP_DDRMODE	0x020e0774
#define MX6_IOM_GRP_CTLDS	0x020e078c
#define MX6_IOM_GRP_DDR_TYPE	0x020e0798

#endif	/*__ASM_ARCH_MX6Q_DDR_H__ */
