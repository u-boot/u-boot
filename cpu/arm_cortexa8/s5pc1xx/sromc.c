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
#include <asm/arch/smc.h>

/*
 * s5pc1xx_config_sromc() - select the proper SROMC Bank and configure the
 * 		    band width control and bank control registers
 * srom_bank	- SROM Bank 0 to 5
 * smc_bw_conf  - SMC Band witdh reg configuration value
 * smc_bc_conf  - SMC Bank Control reg configuration value
 */
void s5pc1xx_config_sromc(u32 srom_bank, u32 smc_bw_conf, u32 smc_bc_conf)
{
	u32 tmp;
	struct s5pc1xx_smc *srom;

	if (cpu_is_s5pc100())
		srom = (struct s5pc1xx_smc *)S5PC100_SROMC_BASE;
	else
		srom = (struct s5pc1xx_smc *)S5PC110_SROMC_BASE;

	/* Configure SMC_BW register to handle proper SROMC bank */
	tmp = srom->bw;
	tmp &= ~(0xF << (srom_bank * 4));
	tmp |= smc_bw_conf;
	srom->bw = tmp;

	/* Configure SMC_BC register */
	srom->bc[srom_bank] = smc_bc_conf;
}
