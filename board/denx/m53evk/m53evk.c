/*
 * DENX M53 module
 *
 * Copyright (C) 2012-2013 Marek Vasut <marex@denx.de>
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
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/spl.h>
#include <asm/errno.h>
#include <netdev.h>
#include <i2c.h>
#include <mmc.h>
#include <spl.h>
#include <fsl_esdhc.h>
#include <asm/gpio.h>
#include <usb/ehci-fsl.h>

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
	mxc_request_iomux(MX53_PIN_ATA_BUFFER_EN, IOMUX_CONFIG_ALT3);
	mxc_request_iomux(MX53_PIN_ATA_DMARQ, IOMUX_CONFIG_ALT3);

	mxc_iomux_set_pad(MX53_PIN_ATA_BUFFER_EN,
				PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_ATA_DMARQ,
				PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_HYS_ENABLE);

	mxc_iomux_set_input(MX53_UART2_IPP_UART_RXD_MUX_SELECT_INPUT, 0x3);
}

#ifdef CONFIG_USB_EHCI_MX5
int board_ehci_hcd_init(int port)
{
	if (port == 0) {
		/* USB OTG PWRON */
		mxc_request_iomux(MX53_PIN_GPIO_4, IOMUX_CONFIG_ALT1);
		mxc_iomux_set_pad(MX53_PIN_GPIO_4,
				PAD_CTL_PKE_ENABLE |
				PAD_CTL_100K_PD |
				PAD_CTL_DRV_HIGH
				);
		gpio_direction_output(IOMUX_TO_GPIO(MX53_PIN_GPIO_4), 0);

		/* USB OTG Over Current */
		mxc_request_iomux(MX53_PIN_GPIO_18, IOMUX_CONFIG_ALT1);
		mxc_iomux_set_input(MX53_USBOH3_IPP_IND_OTG_OC_SELECT_INPUT, 1);
	} else if (port == 1) {
		/* USB Host PWRON */
		mxc_request_iomux(MX53_PIN_GPIO_2, IOMUX_CONFIG_ALT1);
		mxc_iomux_set_pad(MX53_PIN_GPIO_2,
				PAD_CTL_PKE_ENABLE |
				PAD_CTL_100K_PD |
				PAD_CTL_DRV_HIGH
				);
		gpio_direction_output(IOMUX_TO_GPIO(MX53_PIN_GPIO_2), 0);

		/* USB Host Over Current */
		mxc_request_iomux(MX53_PIN_GPIO_3, IOMUX_CONFIG_ALT6);
		mxc_iomux_set_input(MX53_USBOH3_IPP_IND_UH1_OC_SELECT_INPUT, 1);
	}

	return 0;
}
#endif

static void setup_iomux_fec(void)
{
	/* MDIO IOMUX */
	mxc_request_iomux(MX53_PIN_FEC_MDIO, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_FEC_MDC, IOMUX_CONFIG_ALT0);

	/* FEC 0 IOMUX */
	mxc_request_iomux(MX53_PIN_FEC_CRS_DV, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_FEC_REF_CLK, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_FEC_RX_ER, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_FEC_TX_EN, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_FEC_RXD0, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_FEC_RXD1, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_FEC_TXD0, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_FEC_TXD1, IOMUX_CONFIG_ALT0);

	/* FEC 1 IOMUX */
	mxc_request_iomux(MX53_PIN_KEY_COL0, IOMUX_CONFIG_ALT6); /* RXD3 */
	mxc_request_iomux(MX53_PIN_KEY_ROW0, IOMUX_CONFIG_ALT6); /* TX_ER */
	mxc_request_iomux(MX53_PIN_KEY_COL1, IOMUX_CONFIG_ALT6); /* RX_CLK */
	mxc_request_iomux(MX53_PIN_KEY_ROW1, IOMUX_CONFIG_ALT6); /* COL */
	mxc_request_iomux(MX53_PIN_KEY_COL2, IOMUX_CONFIG_ALT6); /* RXD2 */
	mxc_request_iomux(MX53_PIN_KEY_ROW2, IOMUX_CONFIG_ALT6); /* TXD2 */
	mxc_request_iomux(MX53_PIN_KEY_COL3, IOMUX_CONFIG_ALT6); /* CRS */
	mxc_request_iomux(MX53_PIN_GPIO_19, IOMUX_CONFIG_ALT6);  /* TXD3 */

	/* MDIO PADs */
	mxc_iomux_set_pad(MX53_PIN_FEC_MDIO,
				PAD_CTL_HYS_ENABLE | PAD_CTL_DRV_HIGH |
				PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE |
				PAD_CTL_22K_PU | PAD_CTL_ODE_OPENDRAIN_ENABLE);
	mxc_iomux_set_input(MX53_FEC_FEC_MDI_SELECT_INPUT, 0x1);
	mxc_iomux_set_pad(MX53_PIN_FEC_MDC, PAD_CTL_DRV_HIGH);

	/* FEC 0 PADs */
	mxc_iomux_set_pad(MX53_PIN_FEC_CRS_DV,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_FEC_REF_CLK,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_FEC_RX_ER,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_FEC_TX_EN, PAD_CTL_DRV_HIGH);
	mxc_iomux_set_pad(MX53_PIN_FEC_RXD0,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_FEC_RXD1,
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_FEC_TXD0, PAD_CTL_DRV_HIGH);
	mxc_iomux_set_pad(MX53_PIN_FEC_TXD1, PAD_CTL_DRV_HIGH);

	/* FEC 1 PADs */
	mxc_iomux_set_pad(MX53_PIN_KEY_COL0,	/* RXD3 */
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_KEY_ROW0,	/* TX_ER */
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_KEY_COL1,	/* RX_CLK */
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_KEY_ROW1,	/* COL */
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_KEY_COL2,	/* RXD2 */
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_KEY_ROW2,	/* TXD2 */
			PAD_CTL_DRV_HIGH);
	mxc_iomux_set_pad(MX53_PIN_KEY_COL3,	/* CRS */
			PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_GPIO_19,	/* TXD3 */
			PAD_CTL_DRV_HIGH);
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg = {
	MMC_SDHC1_BASE_ADDR,
};

int board_mmc_getcd(struct mmc *mmc)
{
	mxc_request_iomux(MX53_PIN_GPIO_1, IOMUX_CONFIG_ALT1);
	gpio_direction_input(IMX_GPIO_NR(1, 1));

	return !gpio_get_value(IMX_GPIO_NR(1, 1));
}

int board_mmc_init(bd_t *bis)
{
	esdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

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

	/* GPIO 2_31 is SD power */
	mxc_request_iomux(MX53_PIN_EIM_EB3, IOMUX_CONFIG_ALT1);
	gpio_direction_output(IMX_GPIO_NR(2, 31), 0);

	return fsl_esdhc_initialize(bis, &esdhc_cfg);
}
#endif

static void setup_iomux_i2c(void)
{
	mxc_request_iomux(MX53_PIN_EIM_D16,
		IOMUX_CONFIG_ALT5 | IOMUX_CONFIG_SION);
	mxc_request_iomux(MX53_PIN_EIM_EB2,
		IOMUX_CONFIG_ALT5 | IOMUX_CONFIG_SION);

	mxc_iomux_set_pad(MX53_PIN_EIM_D16,
		PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |
		PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE |
		PAD_CTL_PUE_PULL |
		PAD_CTL_ODE_OPENDRAIN_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_EIM_EB2,
		PAD_CTL_SRE_FAST | PAD_CTL_DRV_HIGH |
		PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE |
		PAD_CTL_PUE_PULL |
		PAD_CTL_ODE_OPENDRAIN_ENABLE);

	mxc_iomux_set_input(MX53_I2C2_IPP_SDA_IN_SELECT_INPUT, 0x1);
	mxc_iomux_set_input(MX53_I2C2_IPP_SCL_IN_SELECT_INPUT, 0x1);
}

static void setup_iomux_nand(void)
{
	mxc_request_iomux(MX53_PIN_NANDF_WE_B, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_NANDF_RE_B, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_NANDF_CLE, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_NANDF_ALE, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_NANDF_WP_B, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_NANDF_RB0, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_NANDF_CS0, IOMUX_CONFIG_ALT0);
	mxc_request_iomux(MX53_PIN_ATA_DATA0, IOMUX_CONFIG_ALT3);
	mxc_request_iomux(MX53_PIN_ATA_DATA1, IOMUX_CONFIG_ALT3);
	mxc_request_iomux(MX53_PIN_ATA_DATA2, IOMUX_CONFIG_ALT3);
	mxc_request_iomux(MX53_PIN_ATA_DATA3, IOMUX_CONFIG_ALT3);
	mxc_request_iomux(MX53_PIN_ATA_DATA4, IOMUX_CONFIG_ALT3);
	mxc_request_iomux(MX53_PIN_ATA_DATA5, IOMUX_CONFIG_ALT3);
	mxc_request_iomux(MX53_PIN_ATA_DATA6, IOMUX_CONFIG_ALT3);
	mxc_request_iomux(MX53_PIN_ATA_DATA7, IOMUX_CONFIG_ALT3);

	mxc_iomux_set_pad(MX53_PIN_NANDF_WE_B, PAD_CTL_DRV_HIGH);
	mxc_iomux_set_pad(MX53_PIN_NANDF_RE_B, PAD_CTL_DRV_HIGH);
	mxc_iomux_set_pad(MX53_PIN_NANDF_CLE, PAD_CTL_DRV_HIGH);
	mxc_iomux_set_pad(MX53_PIN_NANDF_ALE, PAD_CTL_DRV_HIGH);
	mxc_iomux_set_pad(MX53_PIN_NANDF_WP_B, PAD_CTL_PUE_PULL |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_NANDF_RB0, PAD_CTL_PUE_PULL |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_NANDF_CS0, PAD_CTL_DRV_HIGH);
	mxc_iomux_set_pad(MX53_PIN_ATA_DATA0, PAD_CTL_DRV_HIGH |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_ATA_DATA1, PAD_CTL_DRV_HIGH |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_ATA_DATA2, PAD_CTL_DRV_HIGH |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_ATA_DATA3, PAD_CTL_DRV_HIGH |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_ATA_DATA4, PAD_CTL_DRV_HIGH |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_ATA_DATA5, PAD_CTL_DRV_HIGH |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_ATA_DATA6, PAD_CTL_DRV_HIGH |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
	mxc_iomux_set_pad(MX53_PIN_ATA_DATA7, PAD_CTL_DRV_HIGH |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
}

static void m53_set_clock(void)
{
	int ret;
	const uint32_t ref_clk = MXC_HCLK;
	const uint32_t dramclk = 400;
	uint32_t cpuclk;

	mxc_request_iomux(MX53_PIN_GPIO_10, IOMUX_CONFIG_GPIO);
	mxc_iomux_set_pad(MX53_PIN_GPIO_10, PAD_CTL_DRV_HIGH |
			PAD_CTL_100K_PU | PAD_CTL_PKE_ENABLE);
	gpio_direction_input(IOMUX_TO_GPIO(MX53_PIN_GPIO_10));

	/* GPIO10 selects modules' CPU speed, 1 = 1200MHz ; 0 = 800MHz */
	cpuclk = gpio_get_value(IOMUX_TO_GPIO(MX53_PIN_GPIO_10)) ? 1200 : 800;

	ret = mxc_set_clock(ref_clk, cpuclk, MXC_ARM_CLK);
	if (ret)
		printf("CPU:   Switch CPU clock to %dMHz failed\n", cpuclk);

	ret = mxc_set_clock(ref_clk, dramclk, MXC_PERIPH_CLK);
	if (ret) {
		printf("CPU:   Switch peripheral clock to %dMHz failed\n",
			dramclk);
	}

	ret = mxc_set_clock(ref_clk, dramclk, MXC_DDR_CLK);
	if (ret)
		printf("CPU:   Switch DDR clock to %dMHz failed\n", dramclk);
}

static void m53_set_nand(void)
{
	u32 i;

	/* NAND flash is muxed on ATA pins */
	setbits_le32(M4IF_BASE_ADDR + 0xc, M4IF_GENP_WEIM_MM_MASK);

	/* Wait for Grant/Ack sequence (see EIM_CSnGCR2:MUX16_BYP_GRANT) */
	for (i = 0x4; i < 0x94; i += 0x18) {
		clrbits_le32(WEIM_BASE_ADDR + i,
			     WEIM_GCR2_MUX16_BYP_GRANT_MASK);
	}

	mxc_set_clock(0, 33, MXC_NFC_CLK);
	enable_nfc_clk(1);
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_fec();
	setup_iomux_i2c();
	setup_iomux_nand();

	m53_set_clock();

	mxc_set_sata_internal_clock();

	/* NAND clock @ 33MHz */
	m53_set_nand();

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: DENX M53EVK\n");

	return 0;
}

/*
 * NAND SPL
 */
#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
	setup_iomux_nand();
	m53_set_clock();
	m53_set_nand();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NAND;
}
#endif
