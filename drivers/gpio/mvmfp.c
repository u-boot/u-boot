/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>,
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <asm/io.h>
#include <mvmfp.h>
#include <asm/arch/mfp.h>

/*
 * mfp_config
 *
 * On most of Marvell SoCs (ex. ARMADA100) there is Multi-Funtion-Pin
 * configuration registers to configure each GPIO/Function pin on the
 * SoC.
 *
 * This function reads the array of values for
 * MFPR_X registers and programms them into respective
 * Multi-Function Pin registers.
 * It supports - Alternate Function Selection programming.
 *
 * Whereas,
 * The Configureation value is constructed using MFP()
 * array consists of 32bit values as defined in MFP(xx,xx..) macro
 */
void mfp_config(u32 *mfp_cfgs)
{
	u32 *p_mfpr = NULL;
	u32 cfg_val, val;

	do {
		cfg_val = *mfp_cfgs++;
		/* exit if End of configuration table detected */
		if (cfg_val == MFP_EOC)
			break;

		p_mfpr = (u32 *)(MV_MFPR_BASE
				+ MFP_REG_GET_OFFSET(cfg_val));

		/* Write a mfg register as per configuration */
		val = 0;
		if (cfg_val & MFP_AF_FLAG)
			/* Abstract and program Afternate-Func Selection */
			val |= cfg_val & MFP_AF_MASK;
		if (cfg_val & MFP_EDGE_FLAG)
			/* Abstract and program Edge configuration */
			val |= cfg_val & MFP_LPM_EDGE_MASK;
		if (cfg_val & MFP_DRIVE_FLAG)
			/* Abstract and program Drive configuration */
			val |= cfg_val & MFP_DRIVE_MASK;
		if (cfg_val & MFP_PULL_FLAG)
			/* Abstract and program Pullup/down configuration */
			val |= cfg_val & MFP_PULL_MASK;

		writel(val, p_mfpr);
	} while (1);
	/*
	 * perform a read-back of any MFPR register to make sure the
	 * previous writings are finished
	 */
	readl(p_mfpr);
}
