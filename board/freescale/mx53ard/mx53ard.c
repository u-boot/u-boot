/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc.
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
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/gpio.h>

#define ETHERNET_INT		IMX_GPIO_NR(2, 31)

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	u32 size1, size2;

	size1 = get_ram_size((void *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	size2 = get_ram_size((void *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE);

	gd->ram_size = size1 + size2;

	return 0;
}
void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
}

static void setup_iomux_uart(void)
{
	/* UART1 RXD */
	mxc_request_iomux(MX53_PIN_ATA_DMACK, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX53_PIN_ATA_DMACK,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU |
				PAD_CTL_ODE_OPENDRAIN_ENABLE);
	mxc_iomux_set_input(MX53_UART1_IPP_UART_RXD_MUX_SELECT_INPUT, 0x3);

	/* UART1 TXD */
	mxc_request_iomux(MX53_PIN_ATA_DIOW, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX53_PIN_ATA_DIOW,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU |
				PAD_CTL_ODE_OPENDRAIN_ENABLE);
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC2_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	mxc_request_iomux(MX53_PIN_GPIO_1, IOMUX_CONFIG_ALT1);
	gpio_direction_input(IMX_GPIO_NR(1, 1));
	mxc_request_iomux(MX53_PIN_GPIO_4, IOMUX_CONFIG_ALT1);
	gpio_direction_input(IMX_GPIO_NR(1, 4));

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		ret = !gpio_get_value(IMX_GPIO_NR(1, 1));
	else
		ret = !gpio_get_value(IMX_GPIO_NR(1, 4));

	return ret;
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

			mxc_iomux_set_pad(MX53_PIN_SD1_CMD, 0x1E4);
			mxc_iomux_set_pad(MX53_PIN_SD1_CLK, 0xD4);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA0, 0x1D4);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA1, 0x1D4);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA2, 0x1D4);
			mxc_iomux_set_pad(MX53_PIN_SD1_DATA3, 0x1D4);
			break;
		case 1:
			mxc_request_iomux(MX53_PIN_SD2_CMD,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX53_PIN_SD2_CLK,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX53_PIN_SD2_DATA0,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD2_DATA1,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD2_DATA2,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_SD2_DATA3,
						IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX53_PIN_ATA_DATA12,
						IOMUX_CONFIG_ALT2);
			mxc_request_iomux(MX53_PIN_ATA_DATA13,
						IOMUX_CONFIG_ALT2);
			mxc_request_iomux(MX53_PIN_ATA_DATA14,
						IOMUX_CONFIG_ALT2);
			mxc_request_iomux(MX53_PIN_ATA_DATA15,
						IOMUX_CONFIG_ALT2);

			mxc_iomux_set_pad(MX53_PIN_SD2_CMD, 0x1E4);
			mxc_iomux_set_pad(MX53_PIN_SD2_CLK, 0xD4);
			mxc_iomux_set_pad(MX53_PIN_SD2_DATA0, 0x1D4);
			mxc_iomux_set_pad(MX53_PIN_SD2_DATA1, 0x1D4);
			mxc_iomux_set_pad(MX53_PIN_SD2_DATA2, 0x1D4);
			mxc_iomux_set_pad(MX53_PIN_SD2_DATA3, 0x1D4);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA12, 0x1D4);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA13, 0x1D4);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA14, 0x1D4);
			mxc_iomux_set_pad(MX53_PIN_ATA_DATA15, 0x1D4);
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

static void weim_smc911x_iomux(void)
{
	/* ETHERNET_INT as GPIO2_31 */
	mxc_request_iomux(MX53_PIN_EIM_EB3, IOMUX_CONFIG_ALT1);
	gpio_direction_input(ETHERNET_INT);

	/* Data bus */
	mxc_request_iomux(MX53_PIN_EIM_D16, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D16, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D17, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D17, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D18, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D18, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D19, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D19, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D20, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D20, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D21, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D21, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D22, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D22, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D23, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D23, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D24, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D24, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D25, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D25, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D26, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D26, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D27, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D27, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D28, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D28, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D29, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D29, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D30, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D30, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_D31, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_D31, 0xA4);

	/* Address lines */
	mxc_request_iomux(MX53_PIN_EIM_DA0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_DA0, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_DA1, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_DA1, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_DA2, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_DA2, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_DA3, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_DA3, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_DA4, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_DA4, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_DA5, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_DA5, 0xA4);

	mxc_request_iomux(MX53_PIN_EIM_DA6, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_EIM_DA6, 0xA4);

	/* other EIM signals for ethernet */
	mxc_request_iomux(MX53_PIN_EIM_OE, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_EIM_RW, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_EIM_CS1, IOMUX_CONFIG_ALT0);
}

static void weim_cs1_settings(void)
{
	struct weim *weim_regs = (struct weim *)WEIM_BASE_ADDR;

	writel(MX53ARD_CS1GCR1, &weim_regs->cs1gcr1);
	writel(0x0, &weim_regs->cs1gcr2);
	writel(MX53ARD_CS1RCR1, &weim_regs->cs1rcr1);
	writel(MX53ARD_CS1RCR2, &weim_regs->cs1rcr2);
	writel(MX53ARD_CS1WCR1, &weim_regs->cs1wcr1);
	writel(0x0, &weim_regs->cs1wcr2);
	writel(0x0, &weim_regs->wcr);

	set_chipselect_size(CS0_64M_CS1_64M);
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

	weim_smc911x_iomux();
	weim_cs1_settings();

#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

int checkboard(void)
{
	puts("Board: MX53ARD\n");

	return 0;
}
