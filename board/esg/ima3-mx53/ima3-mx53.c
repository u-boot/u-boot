/*
 * (C) Copyright 2012, Stefano Babic <sbabic@denx.de>
 *
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
 * Foundation, Inc.
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx5x_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/errno.h>
#include <netdev.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/gpio.h>

/* NOR flash configuration */
#define IMA3_MX53_CS0GCR1	(CSEN | DSZ(2))
#define IMA3_MX53_CS0GCR2	0
#define IMA3_MX53_CS0RCR1	(RCSN(2) | OEN(1) | RWSC(15))
#define IMA3_MX53_CS0RCR2	0
#define IMA3_MX53_CS0WCR1	(WBED1 | WCSN(2) | WEN(1) | WWSC(15))
#define IMA3_MX53_CS0WCR2	0

DECLARE_GLOBAL_DATA_PTR;

static void weim_nor_settings(void)
{
	struct weim *weim_regs = (struct weim *)WEIM_BASE_ADDR;

	writel(IMA3_MX53_CS0GCR1, &weim_regs->cs0gcr1);
	writel(IMA3_MX53_CS0GCR2, &weim_regs->cs0gcr2);
	writel(IMA3_MX53_CS0RCR1, &weim_regs->cs0rcr1);
	writel(IMA3_MX53_CS0RCR2, &weim_regs->cs0rcr2);
	writel(IMA3_MX53_CS0WCR1, &weim_regs->cs0wcr1);
	writel(IMA3_MX53_CS0WCR2, &weim_regs->cs0wcr2);
	writel(0x0, &weim_regs->wcr);

	set_chipselect_size(CS0_128);
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *) CONFIG_SYS_SDRAM_BASE,
			PHYS_SDRAM_1_SIZE);
	return 0;
}

static void setup_iomux_uart(void)
{
	/* UART4 RXD */
	mxc_request_iomux(MX53_PIN_CSI0_D13, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D13,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
		PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE |
		PAD_CTL_100K_PU | PAD_CTL_ODE_OPENDRAIN_ENABLE);
	mxc_iomux_set_input(MX53_UART4_IPP_UART_RXD_MUX_SELECT_INPUT, 0x3);

	/* UART4 TXD */
	mxc_request_iomux(MX53_PIN_CSI0_D12, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D12,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
		PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE |
		PAD_CTL_100K_PU | PAD_CTL_ODE_OPENDRAIN_ENABLE);
}

static void setup_iomux_fec(void)
{
	/*FEC_MDIO*/
	mxc_request_iomux(MX53_PIN_FEC_MDIO, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_MDIO,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
		PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE | PAD_CTL_22K_PU |
		PAD_CTL_ODE_OPENDRAIN_ENABLE);
	mxc_iomux_set_input(MX53_FEC_FEC_MDI_SELECT_INPUT, 0x1);

	/*FEC_MDC*/
	mxc_request_iomux(MX53_PIN_FEC_MDC, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_MDC, PAD_CTL_DRV_HIGH);

	/* FEC RXD3 */
	mxc_request_iomux(MX53_PIN_KEY_COL0, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX53_PIN_KEY_COL0, PAD_CTL_HYS_ENABLE |
		PAD_CTL_PKE_ENABLE);

	/* FEC RXD2 */
	mxc_request_iomux(MX53_PIN_KEY_COL2, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX53_PIN_KEY_COL2, PAD_CTL_HYS_ENABLE |
		PAD_CTL_PKE_ENABLE);

	/* FEC RXD1 */
	mxc_request_iomux(MX53_PIN_FEC_RXD1, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_RXD1, PAD_CTL_HYS_ENABLE |
		PAD_CTL_PKE_ENABLE);

	/* FEC RXD0 */
	mxc_request_iomux(MX53_PIN_FEC_RXD0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_RXD0, PAD_CTL_HYS_ENABLE |
		PAD_CTL_PKE_ENABLE);

	/* FEC TXD3 */
	mxc_request_iomux(MX53_PIN_GPIO_19, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX53_PIN_GPIO_19, PAD_CTL_DRV_HIGH);

	/* FEC TXD2 */
	mxc_request_iomux(MX53_PIN_KEY_ROW2, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX53_PIN_KEY_ROW2, PAD_CTL_DRV_HIGH);

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
	mxc_iomux_set_pad(MX53_PIN_FEC_REF_CLK, PAD_CTL_HYS_ENABLE |
		PAD_CTL_PKE_ENABLE);

	/* FEC RX_ER */
	mxc_request_iomux(MX53_PIN_FEC_RX_ER, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_RX_ER, PAD_CTL_HYS_ENABLE |
		PAD_CTL_PKE_ENABLE);

	/* FEC RX_DV */
	mxc_request_iomux(MX53_PIN_FEC_CRS_DV, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX53_PIN_FEC_CRS_DV, PAD_CTL_HYS_ENABLE |
		PAD_CTL_PKE_ENABLE);

	/* FEC CRS */
	mxc_request_iomux(MX53_PIN_KEY_COL3, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX53_PIN_KEY_COL3, PAD_CTL_HYS_ENABLE |
		PAD_CTL_PKE_ENABLE);

	/* FEC COL */
	mxc_request_iomux(MX53_PIN_KEY_ROW1, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX53_PIN_KEY_ROW1, PAD_CTL_HYS_ENABLE |
		PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_input(MX53_FEC_FEC_COL_SELECT_INPUT, 0x0);

	/* FEC RX_CLK */
	mxc_request_iomux(MX53_PIN_KEY_COL1, IOMUX_CONFIG_ALT6);
	mxc_iomux_set_pad(MX53_PIN_KEY_COL1, PAD_CTL_HYS_ENABLE |
		PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_input(MX53_FEC_FEC_RX_CLK_SELECT_INPUT, 0x0);
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg = { MMC_SDHC1_BASE_ADDR };

int board_mmc_getcd(struct mmc *mmc)
{
	int ret;

	ret = !gpio_get_value(IOMUX_TO_GPIO(MX53_PIN_GPIO_1));

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	mxc_request_iomux(MX53_PIN_SD1_CMD, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_SD1_CLK, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_SD1_DATA0, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_SD1_DATA1, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_SD1_DATA2, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_SD1_DATA3, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_GPIO_1, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX53_PIN_GPIO_1,
		PAD_CTL_DRV_HIGH | PAD_CTL_HYS_ENABLE |
		PAD_CTL_PUE_KEEPER | PAD_CTL_100K_PU |
		PAD_CTL_ODE_OPENDRAIN_NONE | PAD_CTL_PKE_ENABLE);
	gpio_direction_input(IOMUX_TO_GPIO(MX53_PIN_GPIO_1));

	mxc_iomux_set_pad(MX53_PIN_SD1_CMD,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH | PAD_CTL_PUE_PULL |
		PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PU);
	mxc_iomux_set_pad(MX53_PIN_SD1_CLK,
		PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE |
		PAD_CTL_47K_PU | PAD_CTL_DRV_HIGH);
	mxc_iomux_set_pad(MX53_PIN_SD1_DATA0,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH | PAD_CTL_PUE_PULL |
		PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
	mxc_iomux_set_pad(MX53_PIN_SD1_DATA1,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH | PAD_CTL_PUE_PULL |
		PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
	mxc_iomux_set_pad(MX53_PIN_SD1_DATA2,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH | PAD_CTL_PUE_PULL |
		PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
	mxc_iomux_set_pad(MX53_PIN_SD1_DATA3,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH | PAD_CTL_PUE_PULL |
		PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);

	esdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	return fsl_esdhc_initialize(bis, &esdhc_cfg);
}
#endif

static void setup_iomux_spi(void)
{
	/* SCLK */
	mxc_request_iomux(MX53_PIN_CSI0_D8, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D8,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH | PAD_CTL_PUE_PULL |
		PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
	mxc_iomux_set_input(MX53_ECSPI2_IPP_CSPI_CLK_IN_SELECT_INPUT, 0x1);
	/* MOSI */
	mxc_request_iomux(MX53_PIN_CSI0_D9, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D9,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH | PAD_CTL_PUE_PULL |
		PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
	mxc_iomux_set_input(MX53_ECSPI2_IPP_IND_MOSI_SELECT_INPUT, 0x1);
	/* MISO */
	mxc_request_iomux(MX53_PIN_CSI0_D10, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D10,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH | PAD_CTL_PUE_PULL |
		PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
	mxc_iomux_set_input(MX53_ECSPI2_IPP_IND_MISO_SELECT_INPUT, 0x1);
	/* SSEL 0 */
	mxc_request_iomux(MX53_PIN_CSI0_D11, IOMUX_CONFIG_GPIO);
	mxc_iomux_set_pad(MX53_PIN_CSI0_D11,
		PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH | PAD_CTL_PUE_PULL |
		PAD_CTL_PKE_ENABLE | PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU);
	gpio_direction_output(IOMUX_TO_GPIO(MX53_PIN_CSI0_D11), 1);
}

int board_early_init_f(void)
{
	/* configure I/O pads */
	setup_iomux_uart();
	setup_iomux_fec();

	weim_nor_settings();

	/* configure spi */
	setup_iomux_spi();

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	mxc_set_sata_internal_clock();

	return 0;
}

#if defined(CONFIG_RESET_PHY_R)
#include <miiphy.h>

void reset_phy(void)
{
	unsigned short reg;

	/* reset the phy */
	miiphy_reset("FEC", CONFIG_PHY_ADDR);

	/* set hard link to 100Mbit, full-duplex */
	miiphy_read("FEC", CONFIG_PHY_ADDR, MII_BMCR, &reg);
	reg &= ~BMCR_ANENABLE;
	reg |= (BMCR_SPEED100 | BMCR_FULLDPLX);
	miiphy_write("FEC", CONFIG_PHY_ADDR, MII_BMCR, reg);

	miiphy_read("FEC", CONFIG_PHY_ADDR, 0x16, &reg);
	reg |= (1 << 5);
	miiphy_write("FEC", CONFIG_PHY_ADDR, 0x16, reg);
}
#endif

int checkboard(void)
{
	puts("Board: IMA3_MX53\n");

	return 0;
}
