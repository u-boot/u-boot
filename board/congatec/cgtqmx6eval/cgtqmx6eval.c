/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 * Based on mx6qsabrelite.c file
 * Copyright (C) 2013, Adeneo Embedded <www.adeneo-embedded.com>
 * Leo Sartre, <lsartre@adeneo-embedded.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/sata.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <malloc.h>
#include <miiphy.h>
#include <netdev.h>
#include <micrel.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define MX6Q_QMX6_PFUZE_MUX		IMX_GPIO_NR(6, 9)


#define ENET_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED   |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_SD2_CLK__SD2_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_CMD__SD2_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT0__SD2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT1__SD2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT2__SD2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT3__SD2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_GPIO_4__GPIO1_IO04      | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_RST__SD3_RESET | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc4_pads[] = {
	MX6_PAD_SD4_CLK__SD4_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_CMD__SD4_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT0__SD4_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT1__SD4_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT2__SD4_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT3__SD4_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT4__SD4_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT5__SD4_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT6__SD4_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT7__SD4_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_D6__GPIO2_IO06    | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

static iomux_v3_cfg_t const usb_otg_pads[] = {
	MX6_PAD_EIM_D22__USB_OTG_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_GPIO_1__USB_OTG_ID | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t enet_pads_ksz9031[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__GPIO6_IO30 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_RGMII_RD0__GPIO6_IO25 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_RGMII_RD1__GPIO6_IO27 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_RGMII_RD2__GPIO6_IO28 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_RGMII_RD3__GPIO6_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__GPIO6_IO24 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t enet_pads_final_ksz9031[] = {
	MX6_PAD_RGMII_RXC__RGMII_RXC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static iomux_v3_cfg_t enet_pads_ar8035[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__RGMII_RXC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL | MUX_PAD_CTRL(ENET_PAD_CTRL),
};

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_KEY_COL3__I2C2_SCL | PC,
		.gpio_mode = MX6_PAD_KEY_COL3__GPIO4_IO12 | PC,
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

#define I2C_PMIC	1	/* I2C2 port is used to connect to the PMIC */

struct interface_level {
	char *name;
	uchar value;
};

static struct interface_level mipi_levels[] = {
	{"0V0", 0x00},
	{"2V5", 0x17},
};

/* setup board specific PMIC */
int power_init_board(void)
{
	struct pmic *p;
	u32 id1, id2, i;
	int ret;
	char const *lv_mipi;

	/* configure I2C multiplexer */
	gpio_direction_output(MX6Q_QMX6_PFUZE_MUX, 1);

	power_pfuze100_init(I2C_PMIC);
	p = pmic_get("PFUZE100");
	if (!p)
		return -EINVAL;

	ret = pmic_probe(p);
	if (ret)
		return ret;

	pmic_reg_read(p, PFUZE100_DEVICEID, &id1);
	pmic_reg_read(p, PFUZE100_REVID, &id2);
	printf("PFUZE100 Rev. [%02x/%02x] detected\n", id1, id2);

	if (id2 >= 0x20)
		return 0;

	/* set level of MIPI if specified */
	lv_mipi = getenv("lv_mipi");
	if (lv_mipi)
		return 0;

	for (i = 0; i < ARRAY_SIZE(mipi_levels); i++) {
		if (!strcmp(mipi_levels[i].name, lv_mipi)) {
			printf("set MIPI level %s\n",
			       mipi_levels[i].name);
			ret = pmic_reg_write(p, PFUZE100_VGEN4VOL,
					     mipi_levels[i].value);
			if (ret)
				return ret;
		}
	}

	return 0;
}

int board_eth_init(bd_t *bis)
{
	struct phy_device *phydev;
	struct mii_dev *bus;
	unsigned short id1, id2;
	int ret;

	iomux_v3_cfg_t enet_reset = MX6_PAD_EIM_D23__GPIO3_IO23 |
				    MUX_PAD_CTRL(NO_PAD_CTRL);

	/* check whether KSZ9031 or AR8035 has to be configured */
	imx_iomux_v3_setup_multiple_pads(enet_pads_ar8035,
					 ARRAY_SIZE(enet_pads_ar8035));
	imx_iomux_v3_setup_pad(enet_reset);

	/* phy reset */
	gpio_direction_output(IMX_GPIO_NR(3, 23), 0);
	udelay(2000);
	gpio_set_value(IMX_GPIO_NR(3, 23), 1);
	udelay(500);

	bus = fec_get_miibus(IMX_FEC_BASE, -1);
	if (!bus)
		return -EINVAL;
	phydev = phy_find_by_mask(bus, (0xf << 4), PHY_INTERFACE_MODE_RGMII);
	if (!phydev) {
		printf("Error: phy device not found.\n");
		ret = -ENODEV;
		goto free_bus;
	}

	/* get the PHY id */
	id1 = phy_read(phydev, MDIO_DEVAD_NONE, 2);
	id2 = phy_read(phydev, MDIO_DEVAD_NONE, 3);

	if ((id1 == 0x22) && ((id2 & 0xFFF0) == 0x1620)) {
		/* re-configure for Micrel KSZ9031 */
		printf("configure Micrel KSZ9031 Ethernet Phy at address %d\n",
		       phydev->addr);

		/* phy reset: gpio3-23 */
		gpio_set_value(IMX_GPIO_NR(3, 23), 0);
		gpio_set_value(IMX_GPIO_NR(6, 30), (phydev->addr >> 2));
		gpio_set_value(IMX_GPIO_NR(6, 25), 1);
		gpio_set_value(IMX_GPIO_NR(6, 27), 1);
		gpio_set_value(IMX_GPIO_NR(6, 28), 1);
		gpio_set_value(IMX_GPIO_NR(6, 29), 1);
		imx_iomux_v3_setup_multiple_pads(enet_pads_ksz9031,
						 ARRAY_SIZE(enet_pads_ksz9031));
		gpio_set_value(IMX_GPIO_NR(6, 24), 1);
		udelay(500);
		gpio_set_value(IMX_GPIO_NR(3, 23), 1);
		imx_iomux_v3_setup_multiple_pads(enet_pads_final_ksz9031,
						 ARRAY_SIZE(enet_pads_final_ksz9031));
	} else if ((id1 == 0x004d) && (id2 == 0xd072)) {
		/* configure Atheros AR8035 - actually nothing to do */
		printf("configure Atheros AR8035 Ethernet Phy at address %d\n",
		       phydev->addr);
	} else {
		printf("Unknown Ethernet-Phy: 0x%04x 0x%04x\n", id1, id2);
		ret = -EINVAL;
		goto free_phydev;
	}

	ret = fec_probe(bis, -1, IMX_FEC_BASE, bus, phydev);
	if (ret)
		goto free_phydev;

	return 0;

free_phydev:
	free(phydev);
free_bus:
	free(bus);
	return ret;
}

int mx6_rgmii_rework(struct phy_device *phydev)
{
	unsigned short id1, id2;
	unsigned short val;

	/* check whether KSZ9031 or AR8035 has to be configured */
	id1 = phy_read(phydev, MDIO_DEVAD_NONE, 2);
	id2 = phy_read(phydev, MDIO_DEVAD_NONE, 3);

	if ((id1 == 0x22) && ((id2 & 0xFFF0) == 0x1620)) {
		/* finalize phy configuration for Micrel KSZ9031 */
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 4);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_POST_INC_W | 0x2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x0000);

		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 5);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_POST_INC_W | 0x2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, MII_KSZ9031_MOD_REG);

		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 6);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_POST_INC_W | 0x2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0xFFFF);

		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 8);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_POST_INC_W | 0x2);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x3FFF);

		/* fix KSZ9031 link up issue */
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 0x0);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x4);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_NO_POST_INC);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x6);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_REG);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x3);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_NO_POST_INC);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, 0x1A80);
	}

	if ((id1 == 0x004d) && (id2 == 0xd072)) {
		/* enable AR8035 ouput a 125MHz clk from CLK_25M */
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, 0x7);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, MII_KSZ9031_MOD_DATA_POST_INC_RW | 0x16);
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_CONTROL, MII_KSZ9031_MOD_DATA_NO_POST_INC | 0x7);
		val = phy_read(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA);
		val &= 0xfe63;
		val |= 0x18;
		phy_write(phydev, MDIO_DEVAD_NONE, MMD_ACCESS_REG_DATA, val);

		/* introduce tx clock delay */
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
		val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
		val |= 0x0100;
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

		/* disable hibernation */
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0xb);
		val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x3c40);
	}
	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
 
static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
}

#ifdef CONFIG_FSL_ESDHC
static struct fsl_esdhc_cfg usdhc_cfg[] = {
	{USDHC2_BASE_ADDR},
	{USDHC3_BASE_ADDR},
	{USDHC4_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		gpio_direction_input(IMX_GPIO_NR(1, 4));
		ret = !gpio_get_value(IMX_GPIO_NR(1, 4));
		break;
	case USDHC3_BASE_ADDR:
		ret = 1;	/* eMMC is always present */
		break;
	case USDHC4_BASE_ADDR:
		gpio_direction_input(IMX_GPIO_NR(2, 6));
		ret = !gpio_get_value(IMX_GPIO_NR(2, 6));
		break;
	default:
		printf("Bad USDHC interface\n");
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	s32 status = 0;
	int i;

	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);

	imx_iomux_v3_setup_multiple_pads(usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
	imx_iomux_v3_setup_multiple_pads(usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
	imx_iomux_v3_setup_multiple_pads(usdhc4_pads, ARRAY_SIZE(usdhc4_pads));

	for (i = 0; i < ARRAY_SIZE(usdhc_cfg); i++) {
		status = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (status)
			return status;
	}

	return 0;
}
#endif

int board_ehci_hcd_init(int port)
{
	switch (port) {
	case 0:
		imx_iomux_v3_setup_multiple_pads(usb_otg_pads,
						 ARRAY_SIZE(usb_otg_pads));
		/*
		 * set daisy chain for otg_pin_id on 6q.
		 * for 6dl, this bit is reserved
		 */
		imx_iomux_set_gpr_register(1, 13, 1, 1);
		break;
	case 1:
		/* nothing to do */
		break;
	default:
		printf("Invalid USB port: %d\n", port);
		return -EINVAL;
	}

	return 0;
}

int board_ehci_power(int port, int on)
{
	switch (port) {
	case 0:
		break;
	case 1:
		gpio_direction_output(IMX_GPIO_NR(5, 5), on);
		break;
	default:
		printf("Invalid USB port: %d\n", port);
		return -EINVAL;
	}

	return 0;
}

struct display_info_t {
	int bus;
	int addr;
	int pixfmt;
	int (*detect)(struct display_info_t const *dev);
	void (*enable)(struct display_info_t const *dev);
	struct fb_videomode mode;
};

static void disable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	clrbits_le32(&iomux->gpr[2], IOMUXC_GPR2_LVDS_CH0_MODE_MASK |
		     IOMUXC_GPR2_LVDS_CH1_MODE_MASK);
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
	disable_lvds(dev);
	imx_enable_hdmi_phy();
}

static struct display_info_t const displays[] = {
{
	.bus = -1,
	.addr = 0,
	.pixfmt = IPU_PIX_FMT_RGB666,
	.detect = NULL,
	.enable = NULL,
	.mode = {
		.name =
		"Hannstar-XGA",
		.refresh = 60,
		.xres = 1024,
		.yres = 768,
		.pixclock = 15385,
		.left_margin = 220,
		.right_margin = 40,
		.upper_margin = 21,
		.lower_margin = 7,
		.hsync_len = 60,
		.vsync_len = 10,
		.sync = FB_SYNC_EXT,
		.vmode = FB_VMODE_NONINTERLACED } },
{
	.bus = -1,
	.addr = 0,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = NULL,
	.enable = do_enable_hdmi,
	.mode = {
		.name = "HDMI",
		.refresh = 60,
		.xres = 1024,
		.yres = 768,
		.pixclock = 15385,
		.left_margin = 220,
		.right_margin = 40,
		.upper_margin = 21,
		.lower_margin = 7,
		.hsync_len = 60,
		.vsync_len = 10,
		.sync = FB_SYNC_EXT,
		.vmode = FB_VMODE_NONINTERLACED } }
};

int board_video_skip(void)
{
	int i;
	int ret;
	char const *panel = getenv("panel");
	if (!panel) {
		for (i = 0; i < ARRAY_SIZE(displays); i++) {
			struct display_info_t const *dev = displays + i;
			if (dev->detect && dev->detect(dev)) {
				panel = dev->mode.name;
				printf("auto-detected panel %s\n", panel);
				break;
			}
		}
		if (!panel) {
			panel = displays[0].mode.name;
			printf("No panel detected: default to %s\n", panel);
			i = 0;
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(displays); i++) {
			if (!strcmp(panel, displays[i].mode.name))
				break;
		}
	}
	if (i < ARRAY_SIZE(displays)) {
		ret = ipuv3_fb_init(&displays[i].mode, 0, displays[i].pixfmt);
		if (!ret) {
			if (displays[i].enable)
				displays[i].enable(displays + i);
			printf("Display: %s (%ux%u)\n",
			       displays[i].mode.name, displays[i].mode.xres,
			       displays[i].mode.yres);
		} else
			printf("LCD %s cannot be configured: %d\n",
			       displays[i].mode.name, ret);
	} else {
		printf("unsupported panel %s\n", panel);
		return -EINVAL;
	}

	return 0;
}

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();

	/* Turn on LDB0, LDB1, IPU,IPU DI0 clocks */
	setbits_le32(&mxc_ccm->CCGR3, MXC_CCM_CCGR3_LDB_DI0_MASK |
		     MXC_CCM_CCGR3_LDB_DI1_MASK);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK |
		 MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET) |
		(3 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	setbits_le32(&mxc_ccm->cscmr2, MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV |
		     MXC_CCM_CSCMR2_LDB_DI1_IPU_DIV);

	setbits_le32(&mxc_ccm->chsccdr, CHSCCDR_CLK_SEL_LDB_DI0 <<
		     MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET |
		     CHSCCDR_CLK_SEL_LDB_DI0 <<
		     MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
		| IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW
		| IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
		| IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
		| IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
		| IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
		| IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
		| IOMUXC_GPR2_LVDS_CH0_MODE_DISABLED
		| IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS1_MUX_CTL_MASK |
		       IOMUXC_GPR3_HDMI_MUX_CTL_MASK)) |
		(IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
		 IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_display();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif

	return 0;
}

int checkboard(void)
{
	puts("Board: Conga-QEVAL QMX6 Quad\n");

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	MAKE_CFGVAL(0x50, 0x20, 0x00, 0x00)},
	{"mmc1",	MAKE_CFGVAL(0x50, 0x38, 0x00, 0x00)},
	{NULL,		0},
};
#endif

int misc_init_r(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	return 0;
}
