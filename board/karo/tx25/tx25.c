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
#include <asm/arch/iomux-mx25.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong bootflag)
{
	relocate_code(CONFIG_SPL_TEXT_BASE);
	asm volatile("ldr pc, =nand_boot");
}
#endif

#ifdef CONFIG_FEC_MXC
/*
 * FIXME: need to revisit this
 * The original code enabled PUE and 100-k pull-down without PKE, so the right
 * value here is likely:
 *	0 for no pull
 * or:
 *	PAD_CTL_PUS_100K_DOWN for 100-k pull-down
 */
#define FEC_OUT_PAD_CTRL	0

#define GPIO_FEC_RESET_B	IMX_GPIO_NR(4, 7)
#define GPIO_FEC_ENABLE_B	IMX_GPIO_NR(4, 9)

void tx25_fec_init(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		MX25_PAD_FEC_TX_CLK__FEC_TX_CLK,
		MX25_PAD_FEC_RX_DV__FEC_RX_DV,
		MX25_PAD_FEC_RDATA0__FEC_RDATA0,
		NEW_PAD_CTRL(MX25_PAD_FEC_TDATA0__FEC_TDATA0, FEC_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_FEC_TX_EN__FEC_TX_EN, FEC_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_FEC_MDC__FEC_MDC, FEC_OUT_PAD_CTRL),
		MX25_PAD_FEC_MDIO__FEC_MDIO,
		MX25_PAD_FEC_RDATA1__FEC_RDATA1,
		NEW_PAD_CTRL(MX25_PAD_FEC_TDATA1__FEC_TDATA1, FEC_OUT_PAD_CTRL),

		NEW_PAD_CTRL(MX25_PAD_D13__GPIO_4_7, 0), /* FEC_RESET_B */
		NEW_PAD_CTRL(MX25_PAD_D11__GPIO_4_9, 0), /* FEC_ENABLE_B */
	};

	static const iomux_v3_cfg_t fec_cfg_pads[] = {
		MX25_PAD_FEC_RDATA0__GPIO_3_10,
		MX25_PAD_FEC_RDATA1__GPIO_3_11,
		MX25_PAD_FEC_RX_DV__GPIO_3_12,
	};

	debug("tx25_fec_init\n");
	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));

	/* drop PHY power and assert reset (low) */
	gpio_direction_output(GPIO_FEC_RESET_B, 0);
	gpio_direction_output(GPIO_FEC_ENABLE_B, 0);

	mdelay(5);

	debug("resetting phy\n");

	/* turn on PHY power leaving reset asserted */
	gpio_set_value(GPIO_FEC_ENABLE_B, 1);

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
	 * set each mux mode to gpio mode
	 */
	imx_iomux_v3_setup_multiple_pads(fec_cfg_pads,
						ARRAY_SIZE(fec_cfg_pads));

	/*
	 * set each to 1 and make each an output
	 */
	gpio_direction_output(IMX_GPIO_NR(3, 10), 1);
	gpio_direction_output(IMX_GPIO_NR(3, 11), 1);
	gpio_direction_output(IMX_GPIO_NR(3, 12), 1);

	mdelay(22);		/* this value came from RedBoot */

	/*
	 * deassert PHY reset
	 */
	gpio_set_value(GPIO_FEC_RESET_B, 1);

	mdelay(5);

	/*
	 * set FEC pins back
	 */
	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}
#else
#define tx25_fec_init()
#endif

#ifdef CONFIG_MXC_UART
/*
 * Set up input pins with hysteresis and 100-k pull-ups
 */
#define UART1_IN_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_100K_UP)
/*
 * FIXME: need to revisit this
 * The original code enabled PUE and 100-k pull-down without PKE, so the right
 * value here is likely:
 *	0 for no pull
 * or:
 *	PAD_CTL_PUS_100K_DOWN for 100-k pull-down
 */
#define UART1_OUT_PAD_CTRL	0

static void tx25_uart1_init(void)
{
	static const iomux_v3_cfg_t uart1_pads[] = {
		NEW_PAD_CTRL(MX25_PAD_UART1_RXD__UART1_RXD, UART1_IN_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_UART1_TXD__UART1_TXD, UART1_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_UART1_RTS__UART1_RTS, UART1_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_UART1_CTS__UART1_CTS, UART1_IN_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}
#else
#define tx25_uart1_init()
#endif

int board_init()
{
	tx25_uart1_init();

	/* board id for linux */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
	return 0;
}

int board_late_init(void)
{
	tx25_fec_init();
	return 0;
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM_1,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((void *)PHYS_SDRAM_1,
			PHYS_SDRAM_1_SIZE);
#if CONFIG_NR_DRAM_BANKS > 1
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = get_ram_size((void *)PHYS_SDRAM_2,
			PHYS_SDRAM_2_SIZE);
#else

#endif
}

int checkboard(void)
{
	printf("KARO TX25\n");
	return 0;
}
