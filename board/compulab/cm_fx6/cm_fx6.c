/*
 * Board functions for Compulab CM-FX6 board
 *
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Nikita Kiryanov <nikita@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <fdt_support.h>
#include <sata.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/iomux.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/sata.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include "common.h"
#include "../common/eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_DWC_AHSATA
static int cm_fx6_issd_gpios[] = {
	/* The order of the GPIOs in the array is important! */
	CM_FX6_SATA_PHY_SLP,
	CM_FX6_SATA_NRSTDLY,
	CM_FX6_SATA_PWREN,
	CM_FX6_SATA_NSTANDBY1,
	CM_FX6_SATA_NSTANDBY2,
	CM_FX6_SATA_LDO_EN,
};

static void cm_fx6_sata_power(int on)
{
	int i;

	if (!on) { /* tell the iSSD that the power will be removed */
		gpio_direction_output(CM_FX6_SATA_PWLOSS_INT, 1);
		mdelay(10);
	}

	for (i = 0; i < ARRAY_SIZE(cm_fx6_issd_gpios); i++) {
		gpio_direction_output(cm_fx6_issd_gpios[i], on);
		udelay(100);
	}

	if (!on) /* for compatibility lower the power loss interrupt */
		gpio_direction_output(CM_FX6_SATA_PWLOSS_INT, 0);
}

static iomux_v3_cfg_t const sata_pads[] = {
	/* SATA PWR */
	IOMUX_PADS(PAD_ENET_TX_EN__GPIO1_IO28 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A22__GPIO2_IO16    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D20__GPIO3_IO20    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A25__GPIO5_IO02    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	/* SATA CTRL */
	IOMUX_PADS(PAD_ENET_TXD0__GPIO1_IO30  | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D23__GPIO3_IO23    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D29__GPIO3_IO29    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A23__GPIO6_IO06    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_BCLK__GPIO6_IO31   | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void cm_fx6_setup_issd(void)
{
	SETUP_IOMUX_PADS(sata_pads);
	/* Make sure this gpio has logical 0 value */
	gpio_direction_output(CM_FX6_SATA_PWLOSS_INT, 0);
	udelay(100);

	cm_fx6_sata_power(0);
	mdelay(250);
	cm_fx6_sata_power(1);
}

#define CM_FX6_SATA_INIT_RETRIES	10
int sata_initialize(void)
{
	int err, i;

	cm_fx6_setup_issd();
	for (i = 0; i < CM_FX6_SATA_INIT_RETRIES; i++) {
		err = setup_sata();
		if (err) {
			printf("SATA setup failed: %d\n", err);
			return err;
		}

		udelay(100);

		err = __sata_initialize();
		if (!err)
			break;

		/* There is no device on the SATA port */
		if (sata_port_status(0, 0) == 0)
			break;

		/* There's a device, but link not established. Retry */
	}

	return err;
}
#endif

#ifdef CONFIG_SYS_I2C_MXC
#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
			PAD_CTL_DSE_40ohm | PAD_CTL_HYS | \
			PAD_CTL_ODE | PAD_CTL_SRE_FAST)

I2C_PADS(i2c0_pads,
	 PAD_EIM_D21__I2C1_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_EIM_D21__GPIO3_IO21 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(3, 21),
	 PAD_EIM_D28__I2C1_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_EIM_D28__GPIO3_IO28 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(3, 28));

I2C_PADS(i2c1_pads,
	 PAD_KEY_COL3__I2C2_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_KEY_COL3__GPIO4_IO12 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(4, 12),
	 PAD_KEY_ROW3__I2C2_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_KEY_ROW3__GPIO4_IO13 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(4, 13));

I2C_PADS(i2c2_pads,
	 PAD_GPIO_3__I2C3_SCL | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_GPIO_3__GPIO1_IO03 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(1, 3),
	 PAD_GPIO_6__I2C3_SDA | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 PAD_GPIO_6__GPIO1_IO06 | MUX_PAD_CTRL(I2C_PAD_CTRL),
	 IMX_GPIO_NR(1, 6));


static void cm_fx6_setup_i2c(void)
{
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, I2C_PADS_INFO(i2c0_pads));
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, I2C_PADS_INFO(i2c1_pads));
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, I2C_PADS_INFO(i2c2_pads));
}
#else
static void cm_fx6_setup_i2c(void) { }
#endif

#ifdef CONFIG_USB_EHCI_MX6
#define WEAK_PULLDOWN	(PAD_CTL_PUS_100K_DOWN |		\
			PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
			PAD_CTL_HYS | PAD_CTL_SRE_SLOW)

static int cm_fx6_usb_hub_reset(void)
{
	int err;

	err = gpio_request(CM_FX6_USB_HUB_RST, "usb hub rst");
	if (err) {
		printf("USB hub rst gpio request failed: %d\n", err);
		return -1;
	}

	SETUP_IOMUX_PAD(PAD_SD3_RST__GPIO7_IO08 | MUX_PAD_CTRL(NO_PAD_CTRL));
	gpio_direction_output(CM_FX6_USB_HUB_RST, 0);
	udelay(10);
	gpio_direction_output(CM_FX6_USB_HUB_RST, 1);
	mdelay(1);

	return 0;
}

static int cm_fx6_init_usb_otg(void)
{
	int ret;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	ret = gpio_request(SB_FX6_USB_OTG_PWR, "usb-pwr");
	if (ret) {
		printf("USB OTG pwr gpio request failed: %d\n", ret);
		return ret;
	}

	SETUP_IOMUX_PAD(PAD_EIM_D22__GPIO3_IO22 | MUX_PAD_CTRL(NO_PAD_CTRL));
	SETUP_IOMUX_PAD(PAD_ENET_RX_ER__USB_OTG_ID |
						MUX_PAD_CTRL(WEAK_PULLDOWN));
	clrbits_le32(&iomux->gpr[1], IOMUXC_GPR1_OTG_ID_MASK);
	/* disable ext. charger detect, or it'll affect signal quality at dp. */
	return gpio_direction_output(SB_FX6_USB_OTG_PWR, 0);
}

#define MX6_USBNC_BASEADDR	0x2184800
#define USBNC_USB_H1_PWR_POL	(1 << 9)
int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_uh1_ctrl = (u32 *)(MX6_USBNC_BASEADDR + 4);

	switch (port) {
	case 0:
		return cm_fx6_init_usb_otg();
	case 1:
		SETUP_IOMUX_PAD(PAD_GPIO_0__USB_H1_PWR |
				MUX_PAD_CTRL(NO_PAD_CTRL));

		/* Set PWR polarity to match power switch's enable polarity */
		setbits_le32(usbnc_usb_uh1_ctrl, USBNC_USB_H1_PWR_POL);
		return cm_fx6_usb_hub_reset();
	default:
		break;
	}

	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port == 0)
		return gpio_direction_output(SB_FX6_USB_OTG_PWR, on);

	return 0;
}
#endif

#ifdef CONFIG_FEC_MXC
#define ENET_PAD_CTRL		(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
				 PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

static int mx6_rgmii_rework(struct phy_device *phydev)
{
	unsigned short val;

	/* Ar8031 phy SmartEEE feature cause link status generates glitch,
	 * which cause ethernet link down/up issue, so disable SmartEEE
	 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x3);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x805d);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4003);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= ~(0x1 << 8);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= 0xffe3;
	val |= 0x18;
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);

	if (phydev->drv->config)
		return phydev->drv->config(phydev);

	return 0;
}

static iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC   | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3 | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_0__CCM_CLKO1    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_3__CCM_CLKO2    | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | MUX_PAD_CTRL(0x84)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK  |
						MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL |
						MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL |
						MUX_PAD_CTRL(ENET_PAD_CTRL)),
};

static int handle_mac_address(void)
{
	unsigned char enetaddr[6];
	int rc;

	rc = eth_getenv_enetaddr("ethaddr", enetaddr);
	if (rc)
		return 0;

	rc = cl_eeprom_read_mac_addr(enetaddr);
	if (rc)
		return rc;

	if (!is_valid_ether_addr(enetaddr))
		return -1;

	return eth_setenv_enetaddr("ethaddr", enetaddr);
}

int board_eth_init(bd_t *bis)
{
	int res = handle_mac_address();
	if (res)
		puts("No MAC address found\n");

	SETUP_IOMUX_PADS(enet_pads);
	/* phy reset */
	gpio_direction_output(CM_FX6_ENET_NRST, 0);
	udelay(500);
	gpio_set_value(CM_FX6_ENET_NRST, 1);
	enable_enet_clk(1);
	return cpu_eth_init(bis);
}
#endif

#ifdef CONFIG_NAND_MXS
static iomux_v3_cfg_t const nand_pads[] = {
	IOMUX_PADS(PAD_NANDF_CLE__NAND_CLE     | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_ALE__NAND_ALE     | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS0__NAND_CE0_B   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_RB0__NAND_READY_B | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D0__NAND_DATA00   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D1__NAND_DATA01   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D2__NAND_DATA02   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D3__NAND_DATA03   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D4__NAND_DATA04   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D5__NAND_DATA05   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D6__NAND_DATA06   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D7__NAND_DATA07   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CMD__NAND_RE_B      | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CLK__NAND_WE_B      | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void cm_fx6_setup_gpmi_nand(void)
{
	SETUP_IOMUX_PADS(nand_pads);
	/* Enable clock roots */
	enable_usdhc_clk(1, 3);
	enable_usdhc_clk(1, 4);

	setup_gpmi_io_clk(MXC_CCM_CS2CDR_ENFC_CLK_PODF(0xf) |
			  MXC_CCM_CS2CDR_ENFC_CLK_PRED(1)   |
			  MXC_CCM_CS2CDR_ENFC_CLK_SEL(0));
}
#else
static void cm_fx6_setup_gpmi_nand(void) {}
#endif

#ifdef CONFIG_FSL_ESDHC
static struct fsl_esdhc_cfg usdhc_cfg[3] = {
	{USDHC1_BASE_ADDR},
	{USDHC2_BASE_ADDR},
	{USDHC3_BASE_ADDR},
};

static enum mxc_clock usdhc_clk[3] = {
	MXC_ESDHC_CLK,
	MXC_ESDHC2_CLK,
	MXC_ESDHC3_CLK,
};

int board_mmc_init(bd_t *bis)
{
	int i;

	cm_fx6_set_usdhc_iomux();
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		usdhc_cfg[i].sdhc_clk = mxc_get_clock(usdhc_clk[i]);
		usdhc_cfg[i].max_bus_width = 4;
		fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		enable_usdhc_clk(1, i);
	}

	return 0;
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
void ft_board_setup(void *blob, bd_t *bd)
{
	uint8_t enetaddr[6];

	/* MAC addr */
	if (eth_getenv_enetaddr("ethaddr", enetaddr)) {
		fdt_find_and_setprop(blob, "/fec", "local-mac-address",
				     enetaddr, 6, 1);
	}
}
#endif

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;
	cm_fx6_setup_gpmi_nand();
	cm_fx6_setup_i2c();

	return 0;
}

int checkboard(void)
{
	puts("Board: CM-FX6\n");
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;

	switch (gd->ram_size) {
	case 0x10000000: /* DDR_16BIT_256MB */
		gd->bd->bi_dram[0].size = 0x10000000;
		gd->bd->bi_dram[1].size = 0;
		break;
	case 0x20000000: /* DDR_32BIT_512MB */
		gd->bd->bi_dram[0].size = 0x20000000;
		gd->bd->bi_dram[1].size = 0;
		break;
	case 0x40000000:
		if (is_cpu_type(MXC_CPU_MX6SOLO)) { /* DDR_32BIT_1GB */
			gd->bd->bi_dram[0].size = 0x20000000;
			gd->bd->bi_dram[1].size = 0x20000000;
		} else { /* DDR_64BIT_1GB */
			gd->bd->bi_dram[0].size = 0x40000000;
			gd->bd->bi_dram[1].size = 0;
		}
		break;
	case 0x80000000: /* DDR_64BIT_2GB */
		gd->bd->bi_dram[0].size = 0x40000000;
		gd->bd->bi_dram[1].size = 0x40000000;
		break;
	case 0xEFF00000: /* DDR_64BIT_4GB */
		gd->bd->bi_dram[0].size = 0x70000000;
		gd->bd->bi_dram[1].size = 0x7FF00000;
		break;
	}
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	switch (gd->ram_size) {
	case 0x10000000:
	case 0x20000000:
	case 0x40000000:
	case 0x80000000:
		break;
	case 0xF0000000:
		gd->ram_size -= 0x100000;
		break;
	default:
		printf("ERROR: Unsupported DRAM size 0x%lx\n", gd->ram_size);
		return -1;
	}

	return 0;
}

u32 get_board_rev(void)
{
	return cl_eeprom_get_board_rev();
}

