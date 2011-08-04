/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
 *
 * Based on Aspenite:
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
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
#include <mvmfp.h>
#include <asm/arch/mfp.h>
#include <asm/arch/armada100.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	u32 mfp_cfg[] = {
		/* I2C */
		MFP105_CI2C_SDA,
		MFP106_CI2C_SCL,

		/* Enable Console on UART3 */
		MFPO8_UART3_TXD,
		MFPO9_UART3_RXD,
		MFP_EOC		/*End of configuration*/
	};
	/* configure MFP's */
	mfp_config(mfp_cfg);
	return 0;
}

int board_init(void)
{
	/* arch number of Board */
	gd->bd->bi_arch_number = MACH_TYPE_SHEEVAD;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = armd1_sdram_base(0) + 0x100;
	return 0;
}
