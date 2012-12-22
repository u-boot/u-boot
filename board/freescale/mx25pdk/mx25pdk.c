/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
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
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/imx25-pinmux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <mc34704.h>

#define FEC_RESET_B		IMX_GPIO_NR(2, 3)
#define FEC_ENABLE_B		IMX_GPIO_NR(4, 8)
#define CARD_DETECT		IMX_GPIO_NR(2, 1)

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{IMX_MMC_SDHC1_BASE},
};
#endif

static void mx25pdk_fec_init(void)
{
	struct iomuxc_mux_ctl *muxctl;
	struct iomuxc_pad_ctl *padctl;
	u32 gpio_mux_mode = MX25_PIN_MUX_MODE(5);
	u32 gpio_mux_mode0_sion = MX25_PIN_MUX_MODE(0) | MX25_PIN_MUX_SION;

	/* FEC pin init is generic */
	mx25_fec_init_pins();

	muxctl = (struct iomuxc_mux_ctl *)IMX_IOPADMUX_BASE;
	padctl = (struct iomuxc_pad_ctl *)IMX_IOPADCTL_BASE;
	/*
	 * Set up FEC_RESET_B and FEC_ENABLE_B
	 *
	 * FEC_RESET_B: gpio2_3 is ALT 5 mode of pin D12
	 * FEC_ENABLE_B: gpio4_8 is ALT 5 mode of pin A17
	 */
	writel(gpio_mux_mode, &muxctl->pad_d12);
	writel(gpio_mux_mode, &muxctl->pad_a17);

	writel(0x0, &padctl->pad_d12);
	writel(0x0, &padctl->pad_a17);

	/* Assert RESET and ENABLE low */
	gpio_direction_output(FEC_RESET_B, 0);
	gpio_direction_output(FEC_ENABLE_B, 0);

	udelay(10);

	/* Deassert RESET and ENABLE */
	gpio_set_value(FEC_RESET_B, 1);
	gpio_set_value(FEC_ENABLE_B, 1);

	/* Setup I2C pins so that PMIC can turn on PHY supply */
	writel(gpio_mux_mode0_sion, &muxctl->pad_i2c1_clk);
	writel(gpio_mux_mode0_sion, &muxctl->pad_i2c1_dat);
	writel(0x1E8, &padctl->pad_i2c1_clk);
	writel(0x1E8, &padctl->pad_i2c1_dat);
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

int board_early_init_f(void)
{
	mx25_uart1_init_pins();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int board_late_init(void)
{
	struct pmic *p;
	int ret;

	mx25pdk_fec_init();

	ret = pmic_init(I2C_PMIC);
	if (ret)
		return ret;

	p = pmic_get("FSL_PMIC");
	if (!p)
		return -ENODEV;

	/* Turn on Ethernet PHY supply */
	pmic_reg_write(p, MC34704_GENERAL2_REG, ONOFFE);

	return 0;
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_getcd(struct mmc *mmc)
{
	struct iomuxc_mux_ctl *muxctl;
	struct iomuxc_pad_ctl *padctl;
	u32 gpio_mux_mode = MX25_PIN_MUX_MODE(5);

	/*
	 * Set up the Card Detect pin.
	 *
	 * SD1_GPIO_CD: gpio2_1 is ALT 5 mode of pin A15
	 *
	 */
	muxctl = (struct iomuxc_mux_ctl *)IMX_IOPADMUX_BASE;
	padctl = (struct iomuxc_pad_ctl *)IMX_IOPADCTL_BASE;

	writel(gpio_mux_mode, &muxctl->pad_a15);
	writel(0x0, &padctl->pad_a15);

	gpio_direction_input(CARD_DETECT);
	return !gpio_get_value(CARD_DETECT);
}

int board_mmc_init(bd_t *bis)
{
	struct iomuxc_mux_ctl *muxctl;
	u32 sdhc1_mux_mode = MX25_PIN_MUX_MODE(0) | MX25_PIN_MUX_SION;

	muxctl = (struct iomuxc_mux_ctl *)IMX_IOPADMUX_BASE;
	writel(sdhc1_mux_mode, &muxctl->pad_sd1_cmd);
	writel(sdhc1_mux_mode, &muxctl->pad_sd1_clk);
	writel(sdhc1_mux_mode, &muxctl->pad_sd1_data0);
	writel(sdhc1_mux_mode, &muxctl->pad_sd1_data1);
	writel(sdhc1_mux_mode, &muxctl->pad_sd1_data2);
	writel(sdhc1_mux_mode, &muxctl->pad_sd1_data3);

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC1_CLK);
	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif

int checkboard(void)
{
	puts("Board: MX25PDK\n");

	return 0;
}
