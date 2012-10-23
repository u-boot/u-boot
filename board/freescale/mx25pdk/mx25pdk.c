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

#define CARD_DETECT		IMX_GPIO_NR(2, 1)

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{IMX_MMC_SDHC1_BASE},
};
#endif

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
