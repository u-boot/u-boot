/*
 * Copyright (C) 2010 Samsung Electronics
 * Naveen Krishna Ch <ch.naveen@samsung.com>
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
#include <asm/io.h>
#include <asm/arch/sromc.h>

/*
 * s5p_config_sromc() - select the proper SROMC Bank and configure the
 * band width control and bank control registers
 * srom_bank	- SROM
 * srom_bw_conf  - SMC Band witdh reg configuration value
 * srom_bc_conf  - SMC Bank Control reg configuration value
 */
void s5p_config_sromc(u32 srom_bank, u32 srom_bw_conf, u32 srom_bc_conf)
{
	u32 tmp;
	struct s5p_sromc *srom =
		(struct s5p_sromc *)samsung_get_base_sromc();

	/* Configure SMC_BW register to handle proper SROMC bank */
	tmp = srom->bw;
	tmp &= ~(0xF << (srom_bank * 4));
	tmp |= srom_bw_conf;
	srom->bw = tmp;

	/* Configure SMC_BC register */
	srom->bc[srom_bank] = srom_bc_conf;
}
