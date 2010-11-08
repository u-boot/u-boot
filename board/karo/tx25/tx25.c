/*
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
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/imx25-pinmux.h>

static void mdelay(int n)
{
	while (n-- > 0)
		udelay(1000);
}

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FEC_MXC
void tx25_fec_init(void)
{
	struct iomuxc_mux_ctl *muxctl;
	struct iomuxc_pad_ctl *padctl;
	u32 val;
	u32 gpio_mux_mode = MX25_PIN_MUX_MODE(5);
	struct gpio_regs *gpio4 = (struct gpio_regs *)IMX_GPIO4_BASE;
	struct gpio_regs *gpio3 = (struct gpio_regs *)IMX_GPIO3_BASE;
	u32 saved_rdata0_mode, saved_rdata1_mode, saved_rx_dv_mode;

	debug("tx25_fec_init\n");
	/*
	 * fec pin init is generic
	 */
	mx25_fec_init_pins();

	/*
	 * Set up the FEC_RESET_B and FEC_ENABLE GPIO pins.
	 *
	 * FEC_RESET_B: gpio4[7] is ALT 5 mode of pin D13
	 * FEC_ENABLE_B: gpio4[9] is ALT 5 mode of pin D11
	 */
	muxctl = (struct iomuxc_mux_ctl *)IMX_IOPADMUX_BASE;
	padctl = (struct iomuxc_pad_ctl *)IMX_IOPADCTL_BASE;

	writel(gpio_mux_mode, &muxctl->pad_d13);
	writel(gpio_mux_mode, &muxctl->pad_d11);

	writel(0x0, &padctl->pad_d13);
	writel(0x0, &padctl->pad_d11);

	/* drop PHY power and assert reset (low) */
	val = readl(&gpio4->dr) & ~((1 << 7) | (1 << 9));
	writel(val, &gpio4->dr);
	val = readl(&gpio4->dir) | (1 << 7) | (1 << 9);
	writel(val, &gpio4->dir);

	mdelay(5);

	debug("resetting phy\n");

	/* turn on PHY power leaving reset asserted */
	val = readl(&gpio4->dr) | 1 << 9;
	writel(val, &gpio4->dr);

	mdelay(10);

	/*
	 * Setup some strapping pins that are latched by the PHY
	 * as reset goes high.
	 *
	 * Set PHY mode to 111
	 *  mode0 comes from FEC_RDATA0 which is GPIO 3_10 in mux mode 5
	 *  mode1 comes from FEC_RDATA1 which is GPIO 3_11 in mux mode 5
	 *  mode2 is tied high so nothing to do
	 *
	 * Turn on RMII mode
	 *  RMII mode is selected by FEC_RX_DV which is GPIO 3_12 in mux mode
	 */
	/*
	 * save three current mux modes and set each to gpio mode
	 */
	saved_rdata0_mode = readl(&muxctl->pad_fec_rdata0);
	saved_rdata1_mode = readl(&muxctl->pad_fec_rdata1);
	saved_rx_dv_mode = readl(&muxctl->pad_fec_rx_dv);

	writel(gpio_mux_mode, &muxctl->pad_fec_rdata0);
	writel(gpio_mux_mode, &muxctl->pad_fec_rdata1);
	writel(gpio_mux_mode, &muxctl->pad_fec_rx_dv);

	/*
	 * set each to 1 and make each an output
	 */
	val = readl(&gpio3->dr) | (1 << 10) | (1 << 11) | (1 << 12);
	writel(val, &gpio3->dr);
	val = readl(&gpio3->dir) | (1 << 10) | (1 << 11) | (1 << 12);
	writel(val, &gpio3->dir);

	mdelay(22);		/* this value came from RedBoot */

	/*
	 * deassert PHY reset
	 */
	val = readl(&gpio4->dr) | 1 << 7;
	writel(val, &gpio4->dr);
	writel(val, &gpio4->dr);

	mdelay(5);

	/*
	 * set FEC pins back
	 */
	writel(saved_rdata0_mode, &muxctl->pad_fec_rdata0);
	writel(saved_rdata1_mode, &muxctl->pad_fec_rdata1);
	writel(saved_rx_dv_mode, &muxctl->pad_fec_rx_dv);
}
#else
#define tx25_fec_init()
#endif

int board_init()
{
#ifdef CONFIG_MXC_UART
	extern void mx25_uart_init_pins(void);

	mx25_uart_init_pins();
#endif
	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_TX25;
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
	return 0;
}

int board_late_init(void)
{
	tx25_fec_init();
	return 0;
}

int dram_init (void)
{

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((volatile void *)PHYS_SDRAM_1,
			PHYS_SDRAM_1_SIZE);
#if CONFIG_NR_DRAM_BANKS > 1
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((volatile void *)PHYS_SDRAM_2,
			PHYS_SDRAM_2_SIZE);
#endif

	return 0;
}

int checkboard(void)
{
	printf("KARO TX25\n");
	return 0;
}
