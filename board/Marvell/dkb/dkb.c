/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
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
#include <i2c.h>
#include <asm/arch/mfp.h>
#include <asm/arch/cpu.h>
#ifdef CONFIG_GENERIC_MMC
#include <sdhci.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	u32 mfp_cfg[] = {
		/* Enable Console on UART2 */
		MFP47_UART2_RXD,
		MFP48_UART2_TXD,

		/* I2C */
		MFP53_CI2C_SCL,
		MFP54_CI2C_SDA,

		/* MMC1 */
		MFP_MMC1_DAT7,
		MFP_MMC1_DAT6,
		MFP_MMC1_DAT5,
		MFP_MMC1_DAT4,
		MFP_MMC1_DAT3,
		MFP_MMC1_DAT2,
		MFP_MMC1_DAT1,
		MFP_MMC1_DAT0,
		MFP_MMC1_CMD,
		MFP_MMC1_CLK,
		MFP_MMC1_CD,
		MFP_MMC1_WP,

		MFP_EOC		/*End of configureation*/
	};
	/* configure MFP's */
	mfp_config(mfp_cfg);

	return 0;
}

int board_init(void)
{
	/* arch number of Board */
	gd->bd->bi_arch_number = MACH_TYPE_TTC_DKB;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = panth_sdram_base(0) + 0x100;
	return 0;
}

#ifdef CONFIG_GENERIC_MMC
#define I2C_SLAVE_ADDR	0x34
#define LDO13_REG	0x28
#define LDO_V30		0x6
#define LDO_VOLTAGE(x)	((x & 0x7) << 1)
#define LDO_EN		0x1
int board_mmc_init(bd_t *bd)
{
	ulong mmc_base_address[CONFIG_SYS_MMC_NUM] = CONFIG_SYS_MMC_BASE;
	u8 i, data;

	/* set LDO 13 to 3.0v */
	data = LDO_VOLTAGE(LDO_V30) | LDO_EN;
	i2c_write(I2C_SLAVE_ADDR, LDO13_REG, 1, &data, 1);

	for (i = 0; i < CONFIG_SYS_MMC_NUM; i++) {
		if (mv_sdh_init(mmc_base_address[i], 0, 0,
				SDHCI_QUIRK_32BIT_DMA_ADDR))
			return 1;
	}

	return 0;
}
#endif
