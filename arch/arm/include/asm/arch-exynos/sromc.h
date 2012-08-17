/*
 * (C) Copyright 2010 Samsung Electronics
 * Naveen Krishna Ch <ch.naveen@samsung.com>
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
 *
 * Note: This file contains the register description for SROMC
 *
 */

#ifndef __ASM_ARCH_SROMC_H_
#define __ASM_ARCH_SROMC_H_

#define SROMC_DATA16_WIDTH(x)    (1<<((x*4)+0))
#define SROMC_BYTE_ADDR_MODE(x)  (1<<((x*4)+1))  /* 0-> Half-word base address*/
						/* 1-> Byte base address*/
#define SROMC_WAIT_ENABLE(x)     (1<<((x*4)+2))
#define SROMC_BYTE_ENABLE(x)     (1<<((x*4)+3))

#define SROMC_BC_TACS(x) (x << 28) /* address set-up */
#define SROMC_BC_TCOS(x) (x << 24) /* chip selection set-up */
#define SROMC_BC_TACC(x) (x << 16) /* access cycle */
#define SROMC_BC_TCOH(x) (x << 12) /* chip selection hold */
#define SROMC_BC_TAH(x)  (x << 8)  /* address holding time */
#define SROMC_BC_TACP(x) (x << 4)  /* page mode access cycle */
#define SROMC_BC_PMC(x)  (x << 0)  /* normal(1data)page mode configuration */

#ifndef __ASSEMBLY__
struct s5p_sromc {
	unsigned int	bw;
	unsigned int	bc[4];
};
#endif	/* __ASSEMBLY__ */

/* Configure the Band Width and Bank Control Regs for required SROMC Bank */
void s5p_config_sromc(u32 srom_bank, u32 srom_bw_conf, u32 srom_bc_conf);

#endif /* __ASM_ARCH_SROMC_H_ */
