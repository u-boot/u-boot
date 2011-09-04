/*
 * (C) Copyright 2010 Freescale Semiconductor, Inc.
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
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx5x_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/errno.h>
#include <netdev.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <fsl_pmic.h>
#include <asm/gpio.h>
#include <mc13892.h>

DECLARE_GLOBAL_DATA_PTR;

u32 get_board_rev(void)
{
	return get_cpu_rev();
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

static void setup_iomux_uart(void)
{
	/* UART1 RXD */
	mxc_request_iomux(MX53_PIN_CSI0_D11, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D11,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU |
				PAD_CTL_ODE_OPENDRAIN_ENABLE);
	mxc_iomux_set_input(MX53_UART1_IPP_UART_RXD_MUX_SELECT_INPUT, 0x1);

	/* UART1 TXD */
	mxc_request_iomux(MX53_PIN_CSI0_D10, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D10,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU |
				PAD_CTL_ODE_OPENDRAIN_ENABLE);
}

static void setup_i2c(unsigned int port_number)
{
	switch (port_number) {
	case 0:
		/* i2c1 SDA */
		mxc_request_iomux(MX53_PIN_CSI0_D8,
				IOMUX_CONFIG_ALT5 | IOMUX_CONFIG_SION);
		mxc_iomux_set_input(MX53_I2C1_IPP_SDA_IN_SELECT_INPUT,
				INPUT_CTL_PATH0);
		mxc_iomux_set_pad(MX53_PIN_CSI0_D8,
				PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |
				PAD_CTL_100K_PU | PAD_CTL_HYS_ENABLE |
				PAD_CTL_ODE_OPENDRAIN_ENABLE);
		/* i2c1 SCL */
		mxc_request_iomux(MX53_PIN_CSI0_D9,
				IOMUX_CONFIG_ALT5 | IOMUX_CONFIG_SION);
		mxc_iomux_set_input(MX53_I2C1_IPP_SCL_IN_SELECT_INPUT,
				INPUT_CTL_PATH0);
		mxc_iomux_set_pad(MX53_PIN_CSI0_D9,
				PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |
				PAD_CTL_100K_PU | PAD_CTL_HYS_ENABLE |
				PAD_CTL_ODE_OPENDRAIN_ENABLE);
		break;
	case 1:
		/* i2c2 SDA */
		mxc_request_iomux(MX53_PIN_KEY_ROW3,
				IOMUX_CONFIG_ALT4 | IOMUX_CONFIG_SION);
		mxc_iomux_set_input(MX53_I2C2_IPP_SDA_IN_SELECT_INPUT,
				INPUT_CTL_PATH0);
		mxc_iomux_set_pad(MX53_PIN_KEY_ROW3,
				PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |
				PAD_CTL_100K_PU | PAD_CTL_HYS_ENABLE |
				PAD_CTL_ODE_OPENDRAIN_ENABLE);

		/* i2c2 SCL */
		mxc_request_iomux(MX53_PIN_KEY_COL3,
				IOMUX_CONFIG_ALT4 | IOMUX_CONFIG_SION);
		mxc_iomux_set_input(MX53_I2C2_IPP_SCL_IN_SELECT_INPUT,
				INPUT_CTL_PATH0);
		mxc_iomux_set_pad(MX53_PIN_KEY_COL3,
				PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |
				PAD_CTL_100K_PU | PAD_CTL_HYS_ENABLE |
				PAD_CTL_ODE_OPENDRAIN_ENABLE);
		break;
	default:
		printf("Warning: Wrong I2C port number\n");
		break;
	}
}

void power_init(void)
{
	unsigned int val;

	/* Set VDDA to 1.25V */
	val = pmic_reg_read(REG_SW_2);
	val &= ~SWX_OUT_MASK;
	val |= SWX_OUT_1_25;
	pmic_reg_write(REG_SW_2, val);

	/*
	 * Need increase VCC and VDDA to 1.3V
	 * according to MX53 IC TO2 datasheet.
	 */
	if (is_soc_rev(CHIP_REV_2_0) == 0) {
		/* Set VCC to 1.3V for TO2 */
		val = pmic_reg_read(REG_SW_1);
		val &= ~SWX_OUT_MASK;
		val |= SWX_OUT_1_30;
		pmic_reg_write(REG_SW_1, val);

		/* Set VDDA to 1.3V for TO2 */
		val = pmic_reg_read(REG_SW_2);
		val &= ~SWX_OUT_MASK;
		val |= SWX_OUT_1_30;
		pmic_reg_write(REG_SW_2, val);
	}
}

static void setup_iomux_fec(void)
{
	/*FEC_MDIO*/
	mxc_request_iomux(MX53_PIN_FEC_MDIO, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_MDIO,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_22K_PU | PAD_CTL_ODE_OPENDRAIN_ENABLE);
	mxc_iomux_set_input(MX53_FEC_FEC_MDI_SELECT_INPUT, 0x1);

	/*FEC_MDC*/
	mxc_request_iomux(MX53_PIN_FEC_MDC, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_MDC, PAD_CTL_DRV_HIGH);

	/* FEC RXD1 */
	mxc_request_iomux(MX53_PIN_FEC_RXD1, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_RXD1,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);

	/* FEC RXD0 */
	mxc_request_iomux(MX53_PIN_FEC_RXD0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_RXD0,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);

	 /* FEC TXD1 */
	mxc_request_iomux(MX53_PIN_FEC_TXD1, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_TXD1, PAD_CTL_DRV_HIGH);

	/* FEC TXD0 */
	mxc_request_iomux(MX53_PIN_FEC_TXD0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_TXD0, PAD_CTL_DRV_HIGH);

	/* FEC TX_EN */
	mxc_request_iomux(MX53_PIN_FEC_TX_EN, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_TX_EN, PAD_CTL_DRV_HIGH);

	/* FEC TX_CLK */
	mxc_request_iomux(MX53_PIN_FEC_REF_CLK, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_REF_CLK,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);

	/* FEC RX_ER */
	mxc_request_iomux(MX53_PIN_FEC_RX_ER, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_RX_ER,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);

	/* FEC CRS */
	mxc_request_iomux(MX53_PIN_FEC_CRS_DV, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_CRS_DV,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR, 1},
	{MMC_SDHC3_BASE_ADDR, 1},
};

int board_mmc_getcd(u8 *cd, struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		*cd = gpio_get_value(77); /*GPIO3_13*/
	else
		*cd = gpio_get_value(75); /*GPIO3_11*/

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	u32 index;
	s32 status = 0;

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM; index++) {
		switch (index) {
		case 0:
			mxc_request_iomux(MX53_PIN_SD1_CMD, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD1_CLK, IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD1_DATA0,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD1_DATA1,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD1_DATA2,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD1_DATA3,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_EIM_DA13,
						IOMUX_CONFIG_ALT1);

			mxc_iomux_set_pad(MX53_PIN_SD1_CMD,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU);
			mxc_iomux_set_pad(MX53_PIN_SD1_CLK,
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_DRV_HIGH);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA0,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA1,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA2,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA3,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			break;
		case 1:
			mxc_request_iomux(MX53_PIN_ATA_RESET_B,
						IOMUX_CONFIG_ALT2);
			mxc_request_iomux(MX53_PIN_ATA_IORDY,
						IOMUX_CONFIG_ALT2);
			mxc_request_iomux(MX53_PIN_ATA_DATA8,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA9,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA10,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA11,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA0,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA1,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA2,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_ATA_DATA3,
						IOMUX_CONFIG_ALT4);
			mxc_request_iomux(MX53_PIN_EIM_DA11,
						IOMUX_CONFIG_ALT1);

			mxc_iomux_set_pad(MX53_PIN_ATA_RESET_B,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_IORDY,
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_DRV_HIGH);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA8,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA9,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA10,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA11,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA0,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA1,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA2,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA3,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);

			break;
		default:
			printf("Warning: you configured more ESDHC controller"
				"(%d) as supported by the board(2)\n",
				CONFIG_SYS_FSL_ESDHC_NUM);
			return status;
		}
		status |= fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
	}

	return status;
}
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_fec();

	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_MX53_EVK;
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int board_late_init(void)
{
	setup_i2c(1);
	power_init();

	return 0;
}

int checkboard(void)
{
	puts("Board: MX53EVK\n");

	return 0;
}
