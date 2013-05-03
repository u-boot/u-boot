/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * Copyright (C) 2011, Stefano Babic <sbabic@denx.de>
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
#include <asm/errno.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux-mx35.h>
#include <i2c.h>
#include <linux/types.h>
#include <asm/gpio.h>
#include <asm/arch/sys_proto.h>
#include <netdev.h>

#ifndef CONFIG_BOARD_EARLY_INIT_F
#error "CONFIG_BOARD_EARLY_INIT_F must be set for this board"
#endif

#define CCM_CCMR_CONFIG		0x003F4208

#define ESDCTL_DDR2_CONFIG	0x007FFC3F
#define ESDCTL_0x92220000	0x92220000
#define ESDCTL_0xA2220000	0xA2220000
#define ESDCTL_0xB2220000	0xB2220000
#define ESDCTL_0x82228080	0x82228080
#define ESDCTL_DDR2_EMR2	0x04000000
#define ESDCTL_DDR2_EMR3	0x06000000
#define ESDCTL_PRECHARGE	0x00000400
#define ESDCTL_DDR2_EN_DLL	0x02000400
#define ESDCTL_DDR2_RESET_DLL	0x00000333
#define ESDCTL_DDR2_MR		0x00000233
#define ESDCTL_DDR2_OCD_DEFAULT 0x02000780
#define ESDCTL_DELAY_LINE5	0x00F49F00

static inline void dram_wait(unsigned int count)
{
	volatile unsigned int wait = count;

	while (wait--)
		;
}

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1,
		PHYS_SDRAM_1_SIZE);

	return 0;
}

static void board_setup_sdram_bank(u32 start_address)

{
	struct esdc_regs *esdc = (struct esdc_regs *)ESDCTL_BASE_ADDR;
	u32 *cfg_reg, *ctl_reg;
	u32 val;

	switch (start_address) {
	case CSD0_BASE_ADDR:
		cfg_reg = &esdc->esdcfg0;
		ctl_reg = &esdc->esdctl0;
		break;
	case CSD1_BASE_ADDR:
		cfg_reg = &esdc->esdcfg1;
		ctl_reg = &esdc->esdctl1;
		break;
	default:
		return;
	}

	/* Initialize MISC register for DDR2 */
	val = ESDC_MISC_RST | ESDC_MISC_MDDR_EN | ESDC_MISC_MDDR_DL_RST |
		ESDC_MISC_DDR_EN | ESDC_MISC_DDR2_EN;
	writel(val, &esdc->esdmisc);
	val &= ~(ESDC_MISC_RST | ESDC_MISC_MDDR_DL_RST);
	writel(val, &esdc->esdmisc);

	/*
	 * according to DDR2 specs, wait a while before
	 * the PRECHARGE_ALL command
	 */
	dram_wait(0x20000);

	/* Load DDR2 config and timing */
	writel(ESDCTL_DDR2_CONFIG, cfg_reg);

	/* Precharge ALL */
	writel(ESDCTL_0x92220000,
		ctl_reg);
	writel(0xda, start_address + ESDCTL_PRECHARGE);

	/* Load mode */
	writel(ESDCTL_0xB2220000,
		ctl_reg);
	writeb(0xda, start_address + ESDCTL_DDR2_EMR2); /* EMRS2 */
	writeb(0xda, start_address + ESDCTL_DDR2_EMR3); /* EMRS3 */
	writeb(0xda, start_address + ESDCTL_DDR2_EN_DLL); /* Enable DLL */
	writeb(0xda, start_address + ESDCTL_DDR2_RESET_DLL); /* Reset DLL */

	/* Precharge ALL */
	writel(ESDCTL_0x92220000,
		ctl_reg);
	writel(0xda, start_address + ESDCTL_PRECHARGE);

	/* Set mode auto refresh : at least two refresh are required */
	writel(ESDCTL_0xA2220000,
		ctl_reg);
	writel(0xda, start_address);
	writel(0xda, start_address);

	writel(ESDCTL_0xB2220000,
		ctl_reg);
	writeb(0xda, start_address + ESDCTL_DDR2_MR);
	writeb(0xda, start_address + ESDCTL_DDR2_OCD_DEFAULT);

	/* OCD mode exit */
	writeb(0xda, start_address + ESDCTL_DDR2_EN_DLL); /* Enable DLL */

	/* Set normal mode */
	writel(ESDCTL_0x82228080,
		ctl_reg);

	dram_wait(0x20000);

	/* Do not set delay lines, only for MDDR */
}

static void board_setup_sdram(void)
{
	struct esdc_regs *esdc = (struct esdc_regs *)ESDCTL_BASE_ADDR;

	/* Initialize with default values both CSD0/1 */
	writel(0x2000, &esdc->esdctl0);
	writel(0x2000, &esdc->esdctl1);

	board_setup_sdram_bank(CSD0_BASE_ADDR);
}

static void setup_iomux_uart3(void)
{
	static const iomux_v3_cfg_t uart3_pads[] = {
		MX35_PAD_RTS2__UART3_RXD_MUX,
		MX35_PAD_CTS2__UART3_TXD_MUX,
	};

	imx_iomux_v3_setup_multiple_pads(uart3_pads, ARRAY_SIZE(uart3_pads));
}

#define I2C_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_100K_DOWN | PAD_CTL_ODE)

static void setup_iomux_i2c(void)
{
	static const iomux_v3_cfg_t i2c_pads[] = {
		NEW_PAD_CTRL(MX35_PAD_I2C1_CLK__I2C1_SCL, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_I2C1_DAT__I2C1_SDA, I2C_PAD_CTRL),

		NEW_PAD_CTRL(MX35_PAD_TX3_RX2__I2C3_SCL, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX35_PAD_TX2_RX3__I2C3_SDA, I2C_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(i2c_pads, ARRAY_SIZE(i2c_pads));
}


static void setup_iomux_spi(void)
{
	static const iomux_v3_cfg_t spi_pads[] = {
		MX35_PAD_CSPI1_MOSI__CSPI1_MOSI,
		MX35_PAD_CSPI1_MISO__CSPI1_MISO,
		MX35_PAD_CSPI1_SS0__CSPI1_SS0,
		MX35_PAD_CSPI1_SS1__CSPI1_SS1,
		MX35_PAD_CSPI1_SCLK__CSPI1_SCLK,
	};

	imx_iomux_v3_setup_multiple_pads(spi_pads, ARRAY_SIZE(spi_pads));
}

static void setup_iomux_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		MX35_PAD_FEC_TX_CLK__FEC_TX_CLK,
		MX35_PAD_FEC_RX_CLK__FEC_RX_CLK,
		MX35_PAD_FEC_RX_DV__FEC_RX_DV,
		MX35_PAD_FEC_COL__FEC_COL,
		MX35_PAD_FEC_RDATA0__FEC_RDATA_0,
		MX35_PAD_FEC_TDATA0__FEC_TDATA_0,
		MX35_PAD_FEC_TX_EN__FEC_TX_EN,
		MX35_PAD_FEC_MDC__FEC_MDC,
		MX35_PAD_FEC_MDIO__FEC_MDIO,
		MX35_PAD_FEC_TX_ERR__FEC_TX_ERR,
		MX35_PAD_FEC_RX_ERR__FEC_RX_ERR,
		MX35_PAD_FEC_CRS__FEC_CRS,
		MX35_PAD_FEC_RDATA1__FEC_RDATA_1,
		MX35_PAD_FEC_TDATA1__FEC_TDATA_1,
		MX35_PAD_FEC_RDATA2__FEC_RDATA_2,
		MX35_PAD_FEC_TDATA2__FEC_TDATA_2,
		MX35_PAD_FEC_RDATA3__FEC_RDATA_3,
		MX35_PAD_FEC_TDATA3__FEC_TDATA_3,
	};

	/* setup pins for FEC */
	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

int board_early_init_f(void)
{
	struct ccm_regs *ccm =
		(struct ccm_regs *)IMX_CCM_BASE;

	/* setup GPIO3_1 to set HighVCore signal */
	imx_iomux_v3_setup_pad(MX35_PAD_ATA_DA1__GPIO3_1);
	gpio_direction_output(65, 1);

	/* initialize PLL and clock configuration */
	writel(CCM_CCMR_CONFIG, &ccm->ccmr);

	writel(CCM_MPLL_532_HZ, &ccm->mpctl);
	writel(CCM_PPLL_300_HZ, &ccm->ppctl);

	/* Set the core to run at 532 Mhz */
	writel(0x00001000, &ccm->pdr0);

	/* Set-up RAM */
	board_setup_sdram();

	/* enable clocks */
	writel(readl(&ccm->cgr0) |
		MXC_CCM_CGR0_EMI_MASK |
		MXC_CCM_CGR0_EDIO_MASK |
		MXC_CCM_CGR0_EPIT1_MASK,
		&ccm->cgr0);

	writel(readl(&ccm->cgr1) |
		MXC_CCM_CGR1_FEC_MASK |
		MXC_CCM_CGR1_GPIO1_MASK |
		MXC_CCM_CGR1_GPIO2_MASK |
		MXC_CCM_CGR1_GPIO3_MASK |
		MXC_CCM_CGR1_I2C1_MASK |
		MXC_CCM_CGR1_I2C2_MASK |
		MXC_CCM_CGR1_I2C3_MASK,
		&ccm->cgr1);

	/* Set-up NAND */
	__raw_writel(readl(&ccm->rcsr) | MXC_CCM_RCSR_NFC_FMS, &ccm->rcsr);

	/* Set pinmux for the required peripherals */
	setup_iomux_uart3();
	setup_iomux_i2c();
	setup_iomux_fec();
	setup_iomux_spi();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

u32 get_board_rev(void)
{
	int rev = 0;

	return (get_cpu_rev() & ~(0xF << 8)) | (rev & 0xF) << 8;
}
