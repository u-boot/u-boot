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
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx53.h>
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

#define UART_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | \
			 PAD_CTL_PUS_100K_UP | PAD_CTL_ODE)

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT13__UART4_RXD_MUX, UART_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT12__UART4_TXD_MUX, UART_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

static void setup_iomux_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_FEC_MDIO__FEC_MDIO, PAD_CTL_HYS |
			PAD_CTL_DSE_HIGH | PAD_CTL_PUS_22K_UP | PAD_CTL_ODE),
		NEW_PAD_CTRL(MX53_PAD_FEC_MDC__FEC_MDC, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL0__FEC_RDATA_3,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL2__FEC_RDATA_2,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RXD1__FEC_RDATA_1,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RXD0__FEC_RDATA_0,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_GPIO_19__FEC_TDATA_3, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_KEY_ROW2__FEC_TDATA_2, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_TXD1__FEC_TDATA_1, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_TXD0__FEC_TDATA_0, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_TX_EN__FEC_TX_EN, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_REF_CLK__FEC_TX_CLK,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RX_ER__FEC_RX_ER,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_CRS_DV__FEC_RX_DV,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL3__FEC_CRS,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_ROW1__FEC_COL,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL1__FEC_RX_CLK,
				PAD_CTL_HYS | PAD_CTL_PKE),
	};

	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg = { MMC_SDHC1_BASE_ADDR };

int board_mmc_getcd(struct mmc *mmc)
{
	int ret;

	ret = !gpio_get_value(IMX_GPIO_NR(1, 1));

	return ret;
}

#define SD_CMD_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | \
				 PAD_CTL_PUS_100K_UP)
#define SD_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | \
				 PAD_CTL_DSE_HIGH)
#define SD_CD_PAD_CTRL		(PAD_CTL_DSE_HIGH | PAD_CTL_HYS | PAD_CTL_PKE)

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sd1_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_SD1_CMD__ESDHC1_CMD, SD_CMD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_CLK__ESDHC1_CLK, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA0__ESDHC1_DAT0, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA1__ESDHC1_DAT1, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA2__ESDHC1_DAT2, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA3__ESDHC1_DAT3, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_GPIO_1__GPIO1_1, SD_CD_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(sd1_pads, ARRAY_SIZE(sd1_pads));
	gpio_direction_input(IMX_GPIO_NR(1, 1));

	esdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	return fsl_esdhc_initialize(bis, &esdhc_cfg);
}
#endif

#define SPI_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | PAD_CTL_PUS_47K_UP)

static void setup_iomux_spi(void)
{
	static const iomux_v3_cfg_t spi_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT8__ECSPI2_SCLK, SPI_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT9__ECSPI2_MOSI, SPI_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT10__ECSPI2_MISO, SPI_PAD_CTRL),
		/* SSEL 0 */
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT11__GPIO5_29, SPI_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(spi_pads, ARRAY_SIZE(spi_pads));
	gpio_direction_output(IMX_GPIO_NR(5, 29), 1);
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
