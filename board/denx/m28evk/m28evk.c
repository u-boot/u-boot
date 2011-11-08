/*
 * DENX M28 module
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
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
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx28.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <linux/mii.h>
#include <miiphy.h>
#include <netdev.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Functions
 */
int board_early_init_f(void)
{
	/* IO0 clock at 480MHz */
	mx28_set_ioclk(MXC_IOCLK0, 480000);
	/* IO1 clock at 480MHz */
	mx28_set_ioclk(MXC_IOCLK1, 480000);

	/* SSP0 clock at 96MHz */
	mx28_set_sspclk(MXC_SSPCLK0, 96000, 0);
	/* SSP2 clock at 96MHz */
	mx28_set_sspclk(MXC_SSPCLK2, 96000, 0);

#ifdef	CONFIG_CMD_USB
	mxs_iomux_setup_pad(MX28_PAD_SSP2_SS1__USB1_OVERCURRENT);
	mxs_iomux_setup_pad(MX28_PAD_AUART3_TX__GPIO_3_13 |
			MXS_PAD_12MA | MXS_PAD_3V3 | MXS_PAD_PULLUP);
	gpio_direction_output(MX28_PAD_AUART3_TX__GPIO_3_13, 0);
#endif

	return 0;
}

int board_init(void)
{
	/* Adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

#define	HW_DIGCTRL_SCRATCH0	0x8001c280
#define	HW_DIGCTRL_SCRATCH1	0x8001c290
int dram_init(void)
{
	uint32_t sz[2];

	sz[0] = readl(HW_DIGCTRL_SCRATCH0);
	sz[1] = readl(HW_DIGCTRL_SCRATCH1);

	if (sz[0] != sz[1]) {
		printf("MX28:\n"
			"Error, the RAM size in HW_DIGCTRL_SCRATCH0 and\n"
			"HW_DIGCTRL_SCRATCH1 is not the same. Please\n"
			"verify these two registers contain valid RAM size!\n");
		hang();
	}

	gd->ram_size = sz[0];
	return 0;
}

#ifdef	CONFIG_CMD_MMC
static int m28_mmc_wp(int id)
{
	if (id != 0) {
		printf("MXS MMC: Invalid card selected (card id = %d)\n", id);
		return 1;
	}

	return gpio_get_value(MX28_PAD_AUART2_CTS__GPIO_3_10);
}

int board_mmc_init(bd_t *bis)
{
	/* Configure WP as output */
	gpio_direction_input(MX28_PAD_AUART2_CTS__GPIO_3_10);

	return mxsmmc_initialize(bis, 0, m28_mmc_wp);
}
#endif

#ifdef	CONFIG_CMD_NET

#define	MII_OPMODE_STRAP_OVERRIDE	0x16
#define	MII_PHY_CTRL1			0x1e
#define	MII_PHY_CTRL2			0x1f

int fecmxc_mii_postcall(int phy)
{
	miiphy_write("FEC1", phy, MII_BMCR, 0x9000);
	miiphy_write("FEC1", phy, MII_OPMODE_STRAP_OVERRIDE, 0x0202);
	if (phy == 3)
		miiphy_write("FEC1", 3, MII_PHY_CTRL2, 0x8180);
	return 0;
}

int board_eth_init(bd_t *bis)
{
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;
	struct eth_device *dev;
	int ret;

	ret = cpu_eth_init(bis);

	clrsetbits_le32(&clkctrl_regs->hw_clkctrl_enet,
		CLKCTRL_ENET_TIME_SEL_MASK | CLKCTRL_ENET_CLK_OUT_EN,
		CLKCTRL_ENET_TIME_SEL_RMII_CLK);

	ret = fecmxc_initialize_multi(bis, 0, 0, MXS_ENET0_BASE);
	if (ret) {
		printf("FEC MXS: Unable to init FEC0\n");
		return ret;
	}

	ret = fecmxc_initialize_multi(bis, 1, 3, MXS_ENET1_BASE);
	if (ret) {
		printf("FEC MXS: Unable to init FEC1\n");
		return ret;
	}

	dev = eth_get_dev_by_name("FEC0");
	if (!dev) {
		printf("FEC MXS: Unable to get FEC0 device entry\n");
		return -EINVAL;
	}

	ret = fecmxc_register_mii_postcall(dev, fecmxc_mii_postcall);
	if (ret) {
		printf("FEC MXS: Unable to register FEC0 mii postcall\n");
		return ret;
	}

	dev = eth_get_dev_by_name("FEC1");
	if (!dev) {
		printf("FEC MXS: Unable to get FEC1 device entry\n");
		return -EINVAL;
	}

	ret = fecmxc_register_mii_postcall(dev, fecmxc_mii_postcall);
	if (ret) {
		printf("FEC MXS: Unable to register FEC1 mii postcall\n");
		return ret;
	}

	return ret;
}

#ifdef	CONFIG_M28_FEC_MAC_IN_OCOTP

#define	MXS_OCOTP_MAX_TIMEOUT	1000000
void imx_get_mac_from_fuse(char *mac)
{
	struct mx28_ocotp_regs *ocotp_regs =
		(struct mx28_ocotp_regs *)MXS_OCOTP_BASE;
	uint32_t data;

	memset(mac, 0, 6);

	writel(OCOTP_CTRL_RD_BANK_OPEN, &ocotp_regs->hw_ocotp_ctrl_set);

	if (mx28_wait_mask_clr(&ocotp_regs->hw_ocotp_ctrl_reg, OCOTP_CTRL_BUSY,
				MXS_OCOTP_MAX_TIMEOUT)) {
		printf("MXS FEC: Can't get MAC from OCOTP\n");
		return;
	}

	data = readl(&ocotp_regs->hw_ocotp_cust0);

	mac[0] = 0x00;
	mac[1] = 0x04;
	mac[2] = (data >> 24) & 0xff;
	mac[3] = (data >> 16) & 0xff;
	mac[4] = (data >> 8) & 0xff;
	mac[5] = data & 0xff;
}
#else
void imx_get_mac_from_fuse(char *mac)
{
	memset(mac, 0, 6);
}
#endif

#endif
