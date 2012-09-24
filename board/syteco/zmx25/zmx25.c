/*
 * (c) 2011 Graf-Syteco, Matthias Weisser
 * <weisserm@arcor.de>
 *
 * Based on tx25.c:
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *
 * Based on imx27lite.c:
 *   Copyright (C) 2008,2009 Eric Jarrige <jorasse@users.sourceforge.net>
 *   Copyright (C) 2009 Ilya Yanok <yanok@emcraft.com>
 * And:
 *   RedBoot tx25_misc.c Copyright (C) 2009 Red Hat
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
 *
 */
#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/imx25-pinmux.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init()
{
	struct iomuxc_mux_ctl *muxctl;
	struct iomuxc_pad_ctl *padctl;
	struct iomuxc_pad_input_select *inputselect;
	u32 gpio_mux_mode0_sion = MX25_PIN_MUX_MODE(0) | MX25_PIN_MUX_SION;
	u32 gpio_mux_mode1 = MX25_PIN_MUX_MODE(1);
	u32 gpio_mux_mode5 = MX25_PIN_MUX_MODE(5);
	u32 gpio_mux_mode6 = MX25_PIN_MUX_MODE(6);
	u32 input_select1 = MX25_PAD_INPUT_SELECT_DAISY(1);
	u32 input_select2 = MX25_PAD_INPUT_SELECT_DAISY(2);

	icache_enable();

	muxctl = (struct iomuxc_mux_ctl *)IMX_IOPADMUX_BASE;
	padctl = (struct iomuxc_pad_ctl *)IMX_IOPADCTL_BASE;
	inputselect = (struct iomuxc_pad_input_select *)IMX_IOPADINPUTSEL_BASE;

	/* Setup of core volatage selection pin to run at 1.4V */
	writel(gpio_mux_mode5, &muxctl->pad_ext_armclk); /* VCORE GPIO3[15] */
	gpio_direction_output(IMX_GPIO_NR(3, 15), 1);

	/* Setup of input daisy chains for SD card pins*/
	writel(gpio_mux_mode0_sion, &muxctl->pad_sd1_cmd);
	writel(gpio_mux_mode0_sion, &muxctl->pad_sd1_clk);
	writel(gpio_mux_mode0_sion, &muxctl->pad_sd1_data0);
	writel(gpio_mux_mode0_sion, &muxctl->pad_sd1_data1);
	writel(gpio_mux_mode0_sion, &muxctl->pad_sd1_data2);
	writel(gpio_mux_mode0_sion, &muxctl->pad_sd1_data3);

	/* Setup of digital output for USB power and OC */
	writel(gpio_mux_mode5, &muxctl->pad_csi_d3); /* USB Power GPIO1[28] */
	gpio_direction_output(IMX_GPIO_NR(1, 28), 1);

	writel(gpio_mux_mode5, &muxctl->pad_csi_d2); /* USB OC GPIO1[27] */
	gpio_direction_input(IMX_GPIO_NR(1, 18));

	/* Setup of digital output control pins */
	writel(gpio_mux_mode5, &muxctl->pad_csi_d8); /* Ouput 1 Ctrl GPIO1[7] */
	writel(gpio_mux_mode5, &muxctl->pad_csi_d7); /* Ouput 2 Ctrl GPIO1[6] */
	writel(gpio_mux_mode5, &muxctl->pad_csi_d6); /* Ouput 1 Stat GPIO1[31]*/
	writel(gpio_mux_mode5, &muxctl->pad_csi_d5); /* Ouput 2 Stat GPIO1[30]*/

	writel(0, &padctl->pad_csi_d6); /* Ouput 1 Stat pull up off */
	writel(0, &padctl->pad_csi_d5); /* Ouput 2 Stat pull up off */

	/* Switch both output drivers off */
	gpio_direction_output(IMX_GPIO_NR(1, 7), 0);
	gpio_direction_output(IMX_GPIO_NR(1, 6), 0);

	/* Setup of key input pin GPIO2[29]*/
	writel(gpio_mux_mode5 | MX25_PIN_MUX_SION, &muxctl->pad_kpp_row0);
	writel(0, &padctl->pad_kpp_row0); /* Key pull up off */
	gpio_direction_input(IMX_GPIO_NR(2, 29));

	/* Setup of status LED outputs */
	writel(gpio_mux_mode5, &muxctl->pad_csi_d9);	/* GPIO4[21] */
	writel(gpio_mux_mode5, &muxctl->pad_csi_d4);	/* GPIO1[29] */

	/* Switch both LEDs off */
	gpio_direction_output(IMX_GPIO_NR(4, 21), 0);
	gpio_direction_output(IMX_GPIO_NR(1, 29), 0);

	/* Setup of CAN1 and CAN2 signals */
	writel(gpio_mux_mode6, &muxctl->pad_gpio_a);	/* CAN1 TX */
	writel(gpio_mux_mode6, &muxctl->pad_gpio_b);	/* CAN1 RX */
	writel(gpio_mux_mode6, &muxctl->pad_gpio_c);	/* CAN2 TX */
	writel(gpio_mux_mode6, &muxctl->pad_gpio_d);	/* CAN2 RX */

	/* Setup of input daisy chains for CAN signals*/
	writel(input_select1, &inputselect->can1_ipp_ind_canrx); /* CAN1 RX */
	writel(input_select1, &inputselect->can2_ipp_ind_canrx); /* CAN2 RX */

	/* Setup of I2C3 signals */
	writel(gpio_mux_mode1, &muxctl->pad_cspi1_ss1);	/* I2C3 SDA */
	writel(gpio_mux_mode1, &muxctl->pad_gpio_e);	/* I2C3 SCL */

	/* Setup of input daisy chains for I2C3 signals*/
	writel(input_select1, &inputselect->i2c3_ipp_sda_in);	/* I2C3 SDA */
	writel(input_select2, &inputselect->i2c3_ipp_scl_in);	/* I2C3 SCL */

	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int board_late_init(void)
{
	const char *e;

#ifdef CONFIG_FEC_MXC
	struct iomuxc_mux_ctl *muxctl;
	u32 gpio_mux_mode2 = MX25_PIN_MUX_MODE(2);
	u32 gpio_mux_mode5 = MX25_PIN_MUX_MODE(5);

	/*
	 * fec pin init is generic
	 */
	mx25_fec_init_pins();

	/*
	 * Set up LAN-RESET and FEC_RX_ERR
	 *
	 * LAN-RESET:  GPIO3[16] is ALT 5 mode of pin U20
	 * FEC_RX_ERR: FEC_RX_ERR is ALT 2 mode of pin R2
	 */
	muxctl = (struct iomuxc_mux_ctl *)IMX_IOPADMUX_BASE;

	writel(gpio_mux_mode5, &muxctl->pad_upll_bypclk);
	writel(gpio_mux_mode2, &muxctl->pad_uart2_cts);

	/* assert PHY reset (low) */
	gpio_direction_output(IMX_GPIO_NR(3, 16), 0);

	udelay(5000);

	/* deassert PHY reset */
	gpio_set_value(IMX_GPIO_NR(3, 16), 1);

	udelay(5000);
#endif

	e = getenv("gs_base_board");
	if (e != NULL) {
		if (strcmp(e, "G283") == 0) {
			int key = gpio_get_value(IMX_GPIO_NR(2, 29));

			if (key) {
				/* Switch on both LEDs to inidcate boot mode */
				gpio_set_value(IMX_GPIO_NR(1, 29), 0);
				gpio_set_value(IMX_GPIO_NR(4, 21), 0);

				setenv("preboot", "run gs_slow_boot");
			} else
				setenv("preboot", "run gs_fast_boot");
		}
	}

	return 0;
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM,
				PHYS_SDRAM_SIZE);
	return 0;
}
