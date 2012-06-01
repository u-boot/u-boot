/*
 * Maintainer : Prafulla Wadaskar <prafulla@marvell.com>
 *
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/kirkwood.h>
#include <asm/arch/mpp.h>
#include "mv88f6281gtw_ge.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	kw_config_gpio(MV88F6281GTW_GE_OE_VAL_LOW,
			MV88F6281GTW_GE_OE_VAL_HIGH,
			MV88F6281GTW_GE_OE_LOW, MV88F6281GTW_GE_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	u32 kwmpp_config[] = {
		MPP0_SPI_SCn,
		MPP1_SPI_MOSI,
		MPP2_SPI_SCK,
		MPP3_SPI_MISO,
		MPP4_GPIO,
		MPP5_GPO,
		MPP6_SYSRST_OUTn,
		MPP7_SPI_SCn,
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_GPO,
		MPP13_GPIO,
		MPP14_GPIO,
		MPP15_GPIO,
		MPP16_GPIO,
		MPP17_GPIO,
		MPP18_GPO,
		MPP19_GPO,
		MPP20_GPIO,
		MPP21_GPIO,
		MPP22_GPIO,
		MPP23_GPIO,
		MPP24_GPIO,
		MPP25_GPIO,
		MPP26_GPIO,
		MPP27_GPIO,
		MPP28_GPIO,
		MPP29_GPIO,
		MPP30_GPIO,
		MPP31_GPIO,
		MPP32_GPIO,
		MPP33_GPIO,
		MPP34_GPIO,
		MPP35_GPIO,
		MPP36_GPIO,
		MPP37_GPIO,
		MPP38_GPIO,
		MPP39_GPIO,
		MPP40_GPIO,
		MPP41_GPIO,
		MPP42_GPIO,
		MPP43_GPIO,
		MPP44_GPIO,
		MPP45_GPIO,
		MPP46_GPIO,
		MPP47_GPIO,
		MPP48_GPIO,
		MPP49_GPIO,
		0
	};
	kirkwood_mpp_conf(kwmpp_config, NULL);
	return 0;
}

int board_init(void)
{
	/*
	 * arch number of board
	 */
	gd->bd->bi_arch_number = MACH_TYPE_MV88F6281GTW_GE;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = kw_sdram_bar(0) + 0x100;

	return 0;
}

#ifdef CONFIG_MV88E61XX_SWITCH
void reset_phy(void)
{
	/* configure and initialize switch */
	struct mv88e61xx_config swcfg = {
		.name = "egiga0",
		.vlancfg = MV88E61XX_VLANCFG_ROUTER,
		.rgmii_delay = MV88E61XX_RGMII_DELAY_EN,
		.led_init = MV88E61XX_LED_INIT_EN,
		.mdip = MV88E61XX_MDIP_REVERSE,
		.portstate = MV88E61XX_PORTSTT_FORWARDING,
		.cpuport = (1 << 5),
		.ports_enabled = 0x3f
	};

	mv88e61xx_switch_initialize(&swcfg);
}
#endif /* CONFIG_MV88E61XX_SWITCH */
