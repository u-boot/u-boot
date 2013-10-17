/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/cpu.h>
#include <asm/arch/kirkwood.h>
#include <asm/arch/mpp.h>
#include "rd6281a.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	/*
	 * default gpio configuration
	 * There are maximum 64 gpios controlled through 2 sets of registers
	 * the  below configuration configures mainly initial LED status
	 */
	kw_config_gpio(RD6281A_OE_VAL_LOW,
			RD6281A_OE_VAL_HIGH,
			RD6281A_OE_LOW, RD6281A_OE_HIGH);

	/* Multi-Purpose Pins Functionality configuration */
	static const u32 kwmpp_config[] = {
		MPP0_NF_IO2,
		MPP1_NF_IO3,
		MPP2_NF_IO4,
		MPP3_NF_IO5,
		MPP4_NF_IO6,
		MPP5_NF_IO7,
		MPP6_SYSRST_OUTn,
		MPP7_GPO,
		MPP8_TW_SDA,
		MPP9_TW_SCK,
		MPP10_UART0_TXD,
		MPP11_UART0_RXD,
		MPP12_SD_CLK,
		MPP13_SD_CMD,
		MPP14_SD_D0,
		MPP15_SD_D1,
		MPP16_SD_D2,
		MPP17_SD_D3,
		MPP18_NF_IO0,
		MPP19_NF_IO1,
		MPP20_GE1_0,
		MPP21_GE1_1,
		MPP22_GE1_2,
		MPP23_GE1_3,
		MPP24_GE1_4,
		MPP25_GE1_5,
		MPP26_GE1_6,
		MPP27_GE1_7,
		MPP28_GPIO,
		MPP29_GPIO,
		MPP30_GE1_10,
		MPP31_GE1_11,
		MPP32_GE1_12,
		MPP33_GE1_13,
		MPP34_GE1_14,
		MPP35_GPIO,
		MPP36_AUDIO_SPDIFI,
		MPP37_AUDIO_SPDIFO,
		MPP38_GPIO,
		MPP39_TDM_SPI_CS0,
		MPP40_TDM_SPI_SCK,
		MPP41_TDM_SPI_MISO,
		MPP42_TDM_SPI_MOSI,
		MPP43_TDM_CODEC_INTn,
		MPP44_GPIO,
		MPP45_TDM_PCLK,
		MPP46_TDM_FS,
		MPP47_TDM_DRX,
		MPP48_TDM_DTX,
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
	gd->bd->bi_arch_number = MACH_TYPE_RD88F6281;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = kw_sdram_bar(0) + 0x100;

	return 0;
}

void mv_phy_88e1116_init(char *name)
{
	u16 reg;
	u16 devadr;

	if (miiphy_set_current_dev(name))
		return;

	/* command to read PHY dev address */
	if (miiphy_read(name, 0xEE, 0xEE, (u16 *) &devadr)) {
		printf("Err..%s could not read PHY dev address\n",
			__FUNCTION__);
		return;
	}

	/*
	 * Enable RGMII delay on Tx and Rx for CPU port
	 * Ref: sec 4.7.2 of chip datasheet
	 */
	miiphy_write(name, devadr, MV88E1116_PGADR_REG, 2);
	miiphy_read(name, devadr, MV88E1116_MAC_CTRL_REG, &reg);
	reg |= (MV88E1116_RGMII_RXTM_CTRL | MV88E1116_RGMII_TXTM_CTRL);
	miiphy_write(name, devadr, MV88E1116_MAC_CTRL_REG, reg);
	miiphy_write(name, devadr, MV88E1116_PGADR_REG, 0);

	/* reset the phy */
	if (miiphy_read (name, devadr, MII_BMCR, &reg) != 0) {
		printf("Err..(%s) PHY status read failed\n", __FUNCTION__);
		return;
	}
	if (miiphy_write (name, devadr, MII_BMCR, reg | 0x8000) != 0) {
		printf("Err..(%s) PHY reset failed\n", __FUNCTION__);
		return;
	}

	printf("88E1116 Initialized on %s\n", name);
}

/* Configure and enable Switch and PHY */
void reset_phy(void)
{
	/* configure and initialize switch */
	struct mv88e61xx_config swcfg = {
		.name = "egiga0",
		.vlancfg = MV88E61XX_VLANCFG_ROUTER,
		.rgmii_delay = MV88E61XX_RGMII_DELAY_EN,
		.led_init = MV88E61XX_LED_INIT_EN,
		.portstate = MV88E61XX_PORTSTT_FORWARDING,
		.cpuport = (1 << 5),
		.ports_enabled = 0x3f,
	};

	mv88e61xx_switch_initialize(&swcfg);

	/* configure and initialize PHY */
	mv_phy_88e1116_init("egiga1");
}
