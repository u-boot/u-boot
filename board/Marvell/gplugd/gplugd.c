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

#ifdef CONFIG_ARMADA100_FEC
#include <net.h>
#include <netdev.h>
#endif /* CONFIG_ARMADA100_FEC */

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

		/* Ethernet PHY Interface */
		MFP086_ETH_TXCLK,
		MFP087_ETH_TXEN,
		MFP088_ETH_TXDQ3,
		MFP089_ETH_TXDQ2,
		MFP090_ETH_TXDQ1,
		MFP091_ETH_TXDQ0,
		MFP092_ETH_CRS,
		MFP093_ETH_COL,
		MFP094_ETH_RXCLK,
		MFP095_ETH_RXER,
		MFP096_ETH_RXDQ3,
		MFP097_ETH_RXDQ2,
		MFP098_ETH_RXDQ1,
		MFP099_ETH_RXDQ0,
		MFP100_ETH_MDC,
		MFP101_ETH_MDIO,
		MFP103_ETH_RXDV,

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

#ifdef CONFIG_ARMADA100_FEC
int board_eth_init(bd_t *bis)
{
	struct armd1apmu_registers *apmu_regs =
		(struct armd1apmu_registers *)ARMD1_APMU_BASE;

	/* Enable clock of ethernet controller */
	writel(FE_CLK_RST | FE_CLK_ENA, &apmu_regs->fecrc);

	return armada100_fec_register(ARMD1_FEC_BASE);
}
#endif /* CONFIG_ARMADA100_FEC */
