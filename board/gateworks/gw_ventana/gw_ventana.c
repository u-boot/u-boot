/*
 * Copyright (C) 2013 Gateworks Corporation
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/sata.h>
#include <jffs2/load_kernel.h>
#include <hwconfig.h>
#include <i2c.h>
#include <linux/ctype.h>
#include <fdt_support.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <mmc.h>
#include <mtd_node.h>
#include <netdev.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include <i2c.h>
#include <fdt_support.h>
#include <jffs2/load_kernel.h>
#include <spi_flash.h>

#include "gsc.h"
#include "ventana_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

/* GPIO's common to all baseboards */
#define GP_PHY_RST	IMX_GPIO_NR(1, 30)
#define GP_USB_OTG_PWR	IMX_GPIO_NR(3, 22)
#define GP_SD3_CD	IMX_GPIO_NR(7, 0)
#define GP_RS232_EN	IMX_GPIO_NR(2, 11)
#define GP_MSATA_SEL	IMX_GPIO_NR(2, 8)

/* I2C bus numbers */
#define I2C_GSC		0
#define I2C_PMIC	1

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED	  |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm     | PAD_CTL_SRE_FAST)

#define DIO_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_34ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

/*
 * EEPROM board info struct populated by read_eeprom so that we only have to
 * read it once.
 */
static struct ventana_board_info ventana_info;

enum {
	GW54proto, /* original GW5400-A prototype */
	GW51xx,
	GW52xx,
	GW53xx,
	GW54xx,
	GW_UNKNOWN,
};

int board_type;

/* UART1: Function varies per baseboard */
iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_SD3_DAT6__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD3_DAT7__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

/* UART2: Serial Console */
iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

/* I2C1: GSC */
struct i2c_pads_info i2c_pad_info0 = {
	.scl = {
		.i2c_mode = MX6_PAD_EIM_D21__I2C1_SCL | PC,
		.gpio_mode = MX6_PAD_EIM_D21__GPIO3_IO21 | PC,
		.gp = IMX_GPIO_NR(3, 21)
	},
	.sda = {
		.i2c_mode = MX6_PAD_EIM_D28__I2C1_SDA | PC,
		.gpio_mode = MX6_PAD_EIM_D28__GPIO3_IO28 | PC,
		.gp = IMX_GPIO_NR(3, 28)
	}
};

/* I2C2: PMIC/PCIe Switch/PCIe Clock/Mezz */
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

/* I2C3: Misc/Expansion */
struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_3__I2C3_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO_3__GPIO1_IO03 | PC,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_6__I2C3_SDA | PC,
		.gpio_mode = MX6_PAD_GPIO_6__GPIO1_IO06 | PC,
		.gp = IMX_GPIO_NR(1, 6)
	}
};

/* MMC */
iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT5__GPIO7_IO00  | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

/* ENET */
iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__RGMII_RXC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	/* PHY nRST */
	MX6_PAD_ENET_TXD0__GPIO1_IO30		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

/* NAND */
iomux_v3_cfg_t const nfc_pads[] = {
	MX6_PAD_NANDF_CLE__NAND_CLE     | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_ALE__NAND_ALE     | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_WP_B__NAND_WP_B   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_RB0__NAND_READY_B | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_CS0__NAND_CE0_B   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_CMD__NAND_RE_B      | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_CLK__NAND_WE_B      | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D0__NAND_DATA00   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D1__NAND_DATA01   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D2__NAND_DATA02   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D3__NAND_DATA03   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D4__NAND_DATA04   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D5__NAND_DATA05   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D6__NAND_DATA06   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D7__NAND_DATA07   | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#ifdef CONFIG_CMD_NAND
static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	imx_iomux_v3_setup_multiple_pads(nfc_pads, ARRAY_SIZE(nfc_pads));

	/* config gpmi and bch clock to 100 MHz */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL_MASK,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL(3));

	/* enable gpmi and bch clock gating */
	setbits_le32(&mxc_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_OFFSET);

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#endif

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));

	/* toggle PHY_RST# */
	gpio_direction_output(GP_PHY_RST, 0);
	mdelay(2);
	gpio_set_value(GP_PHY_RST, 1);
}

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
}

#ifdef CONFIG_USB_EHCI_MX6
iomux_v3_cfg_t const usb_pads[] = {
	MX6_PAD_GPIO_1__USB_OTG_ID   | MUX_PAD_CTRL(DIO_PAD_CTRL),
	MX6_PAD_KEY_COL4__USB_OTG_OC | MUX_PAD_CTRL(DIO_PAD_CTRL),
	MX6_PAD_EIM_D22__GPIO3_IO22  | MUX_PAD_CTRL(DIO_PAD_CTRL), /* OTG PWR */
};

int board_ehci_hcd_init(int port)
{
	struct ventana_board_info *info = &ventana_info;

	imx_iomux_v3_setup_multiple_pads(usb_pads, ARRAY_SIZE(usb_pads));

	/* Reset USB HUB (present on GW54xx/GW53xx) */
	switch (info->model[3]) {
	case '3': /* GW53xx */
		imx_iomux_v3_setup_pad(MX6_PAD_GPIO_9__GPIO1_IO09|
				       MUX_PAD_CTRL(NO_PAD_CTRL));
		gpio_direction_output(IMX_GPIO_NR(1, 9), 0);
		mdelay(2);
		gpio_set_value(IMX_GPIO_NR(1, 9), 1);
		break;
	case '4': /* GW54xx */
		imx_iomux_v3_setup_pad(MX6_PAD_SD1_DAT0__GPIO1_IO16 |
				       MUX_PAD_CTRL(NO_PAD_CTRL));
		gpio_direction_output(IMX_GPIO_NR(1, 16), 0);
		mdelay(2);
		gpio_set_value(IMX_GPIO_NR(1, 16), 1);
		break;
	}

	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port)
		return 0;
	gpio_set_value(GP_USB_OTG_PWR, on);
	return 0;
}
#endif /* CONFIG_USB_EHCI_MX6 */

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg usdhc_cfg = { USDHC3_BASE_ADDR };

int board_mmc_getcd(struct mmc *mmc)
{
	/* Card Detect */
	gpio_direction_input(GP_SD3_CD);
	return !gpio_get_value(GP_SD3_CD);
}

int board_mmc_init(bd_t *bis)
{
	/* Only one USDHC controller on Ventana */
	imx_iomux_v3_setup_multiple_pads(usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg.max_bus_width = 4;

	return fsl_esdhc_initialize(bis, &usdhc_cfg);
}
#endif /* CONFIG_FSL_ESDHC */

#ifdef CONFIG_MXC_SPI
iomux_v3_cfg_t const ecspi1_pads[] = {
	/* SS1 */
	MX6_PAD_EIM_D19__GPIO3_IO19  | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
};

static void setup_spi(void)
{
	gpio_direction_output(CONFIG_SF_DEFAULT_CS, 1);
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads,
					 ARRAY_SIZE(ecspi1_pads));
}
#endif

/* configure eth0 PHY board-specific LED behavior */
int board_phy_config(struct phy_device *phydev)
{
	unsigned short val;

	/* Marvel 88E1510 */
	if (phydev->phy_id == 0x1410dd1) {
		/*
		 * Page 3, Register 16: LED[2:0] Function Control Register
		 * LED[0] (SPD:Amber) R16_3.3:0 to 0111: on-GbE link
		 * LED[1] (LNK:Green) R16_3.7:4 to 0001: on-link, blink-activity
		 */
		phy_write(phydev, MDIO_DEVAD_NONE, 22, 3);
		val = phy_read(phydev, MDIO_DEVAD_NONE, 16);
		val &= 0xff00;
		val |= 0x0017;
		phy_write(phydev, MDIO_DEVAD_NONE, 16, val);
		phy_write(phydev, MDIO_DEVAD_NONE, 22, 0);
	}

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();

#ifdef CONFIG_FEC_MXC
	cpu_eth_init(bis);
#endif

#ifdef CONFIG_CI_UDC
	/* For otg ethernet*/
	usb_eth_initialize(bis);
#endif

	return 0;
}

/* read ventana EEPROM, check for validity, and return baseboard type */
static int
read_eeprom(void)
{
	int i;
	int chksum;
	char baseboard;
	int type;
	struct ventana_board_info *info = &ventana_info;
	unsigned char *buf = (unsigned char *)&ventana_info;

	memset(info, 0, sizeof(ventana_info));

	/*
	 * On a board with a missing/depleted backup battery for GSC, the
	 * board may be ready to probe the GSC before its firmware is
	 * running.  We will wait here indefinately for the GSC/EEPROM.
	 */
	while (1) {
		if (0 == i2c_set_bus_num(I2C_GSC) &&
		    0 == i2c_probe(GSC_EEPROM_ADDR))
			break;
		mdelay(1);
	}

	/* read eeprom config section */
	if (gsc_i2c_read(GSC_EEPROM_ADDR, 0x00, 1, buf, sizeof(ventana_info))) {
		puts("EEPROM: Failed to read EEPROM\n");
		info->model[0] = 0;
		return GW_UNKNOWN;
	}

	/* sanity checks */
	if (info->model[0] != 'G' || info->model[1] != 'W') {
		puts("EEPROM: Invalid Model in EEPROM\n");
		info->model[0] = 0;
		return GW_UNKNOWN;
	}

	/* validate checksum */
	for (chksum = 0, i = 0; i < sizeof(*info)-2; i++)
		chksum += buf[i];
	if ((info->chksum[0] != chksum>>8) ||
	    (info->chksum[1] != (chksum&0xff))) {
		puts("EEPROM: Failed EEPROM checksum\n");
		info->model[0] = 0;
		return GW_UNKNOWN;
	}

	/* original GW5400-A prototype */
	baseboard = info->model[3];
	if (strncasecmp((const char *)info->model, "GW5400-A", 8) == 0)
		baseboard = '0';

	switch (baseboard) {
	case '0': /* original GW5400-A prototype */
		type = GW54proto;
		break;
	case '1':
		type = GW51xx;
		break;
	case '2':
		type = GW52xx;
		break;
	case '3':
		type = GW53xx;
		break;
	case '4':
		type = GW54xx;
		break;
	default:
		printf("EEPROM: Unknown model in EEPROM: %s\n", info->model);
		type = GW_UNKNOWN;
		break;
	}
	return type;
}

/*
 * Baseboard specific GPIO
 */

/* common to add baseboards */
static iomux_v3_cfg_t const gw_gpio_pads[] = {
	/* MSATA_EN */
	MX6_PAD_SD4_DAT0__GPIO2_IO08 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* RS232_EN# */
	MX6_PAD_SD4_DAT3__GPIO2_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

/* prototype */
static iomux_v3_cfg_t const gwproto_gpio_pads[] = {
	/* PANLEDG# */
	MX6_PAD_KEY_COL0__GPIO4_IO06 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PANLEDR# */
	MX6_PAD_KEY_ROW0__GPIO4_IO07 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LOCLED# */
	MX6_PAD_KEY_ROW4__GPIO4_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* RS485_EN */
	MX6_PAD_SD3_DAT4__GPIO7_IO01 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* IOEXP_PWREN# */
	MX6_PAD_EIM_A19__GPIO2_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* IOEXP_IRQ# */
	MX6_PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* VID_EN */
	MX6_PAD_EIM_D31__GPIO3_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* DIOI2C_DIS# */
	MX6_PAD_GPIO_19__GPIO4_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PCICK_SSON */
	MX6_PAD_SD1_CLK__GPIO1_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PCI_RST# */
	MX6_PAD_ENET_TXD1__GPIO1_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const gw51xx_gpio_pads[] = {
	/* PANLEDG# */
	MX6_PAD_KEY_COL0__GPIO4_IO06 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PANLEDR# */
	MX6_PAD_KEY_ROW0__GPIO4_IO07 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* IOEXP_PWREN# */
	MX6_PAD_EIM_A19__GPIO2_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* IOEXP_IRQ# */
	MX6_PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(NO_PAD_CTRL),

	/* GPS_SHDN */
	MX6_PAD_GPIO_2__GPIO1_IO02 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* VID_PWR */
	MX6_PAD_CSI0_DATA_EN__GPIO5_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PCI_RST# */
	MX6_PAD_GPIO_0__GPIO1_IO00 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const gw52xx_gpio_pads[] = {
	/* PANLEDG# */
	MX6_PAD_KEY_COL0__GPIO4_IO06 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PANLEDR# */
	MX6_PAD_KEY_ROW0__GPIO4_IO07 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* IOEXP_PWREN# */
	MX6_PAD_EIM_A19__GPIO2_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* IOEXP_IRQ# */
	MX6_PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(NO_PAD_CTRL),

	/* MX6_LOCLED# */
	MX6_PAD_KEY_ROW4__GPIO4_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* GPS_SHDN */
	MX6_PAD_ENET_RXD0__GPIO1_IO27 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* USBOTG_SEL */
	MX6_PAD_GPIO_2__GPIO1_IO02 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* VID_PWR */
	MX6_PAD_EIM_D31__GPIO3_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PCI_RST# */
	MX6_PAD_ENET_TXD1__GPIO1_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const gw53xx_gpio_pads[] = {
	/* PANLEDG# */
	MX6_PAD_KEY_COL0__GPIO4_IO06 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PANLEDR# */
	MX6_PAD_KEY_ROW0__GPIO4_IO07 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* IOEXP_PWREN# */
	MX6_PAD_EIM_A19__GPIO2_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* IOEXP_IRQ# */
	MX6_PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(NO_PAD_CTRL),

	/* MX6_LOCLED# */
	MX6_PAD_KEY_ROW4__GPIO4_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* GPS_SHDN */
	MX6_PAD_ENET_RXD0__GPIO1_IO27 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* VID_EN */
	MX6_PAD_EIM_D31__GPIO3_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PCI_RST# */
	MX6_PAD_ENET_TXD1__GPIO1_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const gw54xx_gpio_pads[] = {
	/* PANLEDG# */
	MX6_PAD_KEY_COL0__GPIO4_IO06 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PANLEDR# */
	MX6_PAD_KEY_COL2__GPIO4_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* MX6_LOCLED# */
	MX6_PAD_KEY_ROW4__GPIO4_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* MIPI_DIO */
	MX6_PAD_SD1_DAT3__GPIO1_IO21 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* RS485_EN */
	MX6_PAD_EIM_D24__GPIO3_IO24 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* IOEXP_PWREN# */
	MX6_PAD_KEY_ROW0__GPIO4_IO07 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* IOEXP_IRQ# */
	MX6_PAD_KEY_ROW1__GPIO4_IO09 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* DIOI2C_DIS# */
	MX6_PAD_GPIO_19__GPIO4_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* DIOI2C_DIS# */
	MX6_PAD_GPIO_19__GPIO4_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PCICK_SSON */
	MX6_PAD_SD1_CLK__GPIO1_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* PCI_RST# */
	MX6_PAD_ENET_TXD1__GPIO1_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

/*
 * each baseboard has 4 user configurable Digital IO lines which can
 * be pinmuxed as a GPIO or in some cases a PWM
 */
struct dio_cfg {
	iomux_v3_cfg_t gpio_padmux;
	unsigned gpio_param;
	iomux_v3_cfg_t pwm_padmux;
	unsigned pwm_param;
};

struct ventana {
	/* pinmux */
	iomux_v3_cfg_t const *gpio_pads;
	int num_pads;
	/* DIO pinmux/val */
	struct dio_cfg dio_cfg[4];
	/* various gpios (0 if non-existent) */
	int leds[3];
	int pcie_rst;
	int mezz_pwren;
	int mezz_irq;
	int rs485en;
	int gps_shdn;
	int vidin_en;
	int dioi2c_en;
	int pcie_sson;
	int usb_sel;
};

struct ventana gpio_cfg[] = {
	/* GW5400proto */
	{
		.gpio_pads = gw54xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw54xx_gpio_pads),
		.dio_cfg = {
			{ MX6_PAD_GPIO_9__GPIO1_IO09, IMX_GPIO_NR(1, 9),
			  MX6_PAD_GPIO_9__PWM1_OUT, 1 },
			{ MX6_PAD_SD1_DAT2__GPIO1_IO19, IMX_GPIO_NR(1, 19),
			  MX6_PAD_SD1_DAT2__PWM2_OUT, 2 },
			{ MX6_PAD_SD4_DAT1__GPIO2_IO09, IMX_GPIO_NR(2, 9),
			  MX6_PAD_SD4_DAT1__PWM3_OUT, 3 },
			{ MX6_PAD_SD4_DAT2__GPIO2_IO10, IMX_GPIO_NR(2, 10),
			  MX6_PAD_SD4_DAT2__PWM4_OUT, 4 },
		},
		.leds = {
			IMX_GPIO_NR(4, 6),
			IMX_GPIO_NR(4, 10),
			IMX_GPIO_NR(4, 15),
		},
		.pcie_rst = IMX_GPIO_NR(1, 29),
		.mezz_pwren = IMX_GPIO_NR(4, 7),
		.mezz_irq = IMX_GPIO_NR(4, 9),
		.rs485en = IMX_GPIO_NR(3, 24),
		.dioi2c_en = IMX_GPIO_NR(4,  5),
		.pcie_sson = IMX_GPIO_NR(1, 20),
	},

	/* GW51xx */
	{
		.gpio_pads = gw51xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw51xx_gpio_pads),
		.dio_cfg = {
			{ MX6_PAD_SD1_DAT0__GPIO1_IO16, IMX_GPIO_NR(1, 16),
			  0, 0 },
			{ MX6_PAD_SD1_DAT2__GPIO1_IO19, IMX_GPIO_NR(1, 19),
			  MX6_PAD_SD1_DAT2__PWM2_OUT, 2 },
			{ MX6_PAD_SD1_DAT1__GPIO1_IO17, IMX_GPIO_NR(1, 17),
			  MX6_PAD_SD1_DAT1__PWM3_OUT, 3 },
			{ MX6_PAD_SD1_CMD__GPIO1_IO18, IMX_GPIO_NR(1, 18),
			  MX6_PAD_SD1_CMD__PWM4_OUT, 4 },
		},
		.leds = {
			IMX_GPIO_NR(4, 6),
			IMX_GPIO_NR(4, 10),
		},
		.pcie_rst = IMX_GPIO_NR(1, 0),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.gps_shdn = IMX_GPIO_NR(1, 2),
		.vidin_en = IMX_GPIO_NR(5, 20),
	},

	/* GW52xx */
	{
		.gpio_pads = gw52xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw52xx_gpio_pads),
		.dio_cfg = {
			{ MX6_PAD_SD1_DAT0__GPIO1_IO16, IMX_GPIO_NR(1, 16),
			  0, 0 },
			{ MX6_PAD_SD1_DAT2__GPIO1_IO19, IMX_GPIO_NR(1, 19),
			  MX6_PAD_SD1_DAT2__PWM2_OUT, 2 },
			{ MX6_PAD_SD1_DAT1__GPIO1_IO17, IMX_GPIO_NR(1, 17),
			  MX6_PAD_SD1_DAT1__PWM3_OUT, 3 },
			{ MX6_PAD_SD1_CLK__GPIO1_IO20, IMX_GPIO_NR(1, 20),
			  0, 0 },
		},
		.leds = {
			IMX_GPIO_NR(4, 6),
			IMX_GPIO_NR(4, 7),
			IMX_GPIO_NR(4, 15),
		},
		.pcie_rst = IMX_GPIO_NR(1, 29),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.gps_shdn = IMX_GPIO_NR(1, 27),
		.vidin_en = IMX_GPIO_NR(3, 31),
		.usb_sel = IMX_GPIO_NR(1, 2),
	},

	/* GW53xx */
	{
		.gpio_pads = gw53xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw53xx_gpio_pads),
		.dio_cfg = {
			{ MX6_PAD_SD1_DAT0__GPIO1_IO16, IMX_GPIO_NR(1, 16),
			  0, 0 },
			{ MX6_PAD_SD1_DAT2__GPIO1_IO19, IMX_GPIO_NR(1, 19),
			  MX6_PAD_SD1_DAT2__PWM2_OUT, 2 },
			{ MX6_PAD_SD1_DAT1__GPIO1_IO17, IMX_GPIO_NR(1, 17),
			  MX6_PAD_SD1_DAT1__PWM3_OUT, 3 },
			{ MX6_PAD_SD1_CLK__GPIO1_IO20, IMX_GPIO_NR(1, 20),
			  0, 0 },
		},
		.leds = {
			IMX_GPIO_NR(4, 6),
			IMX_GPIO_NR(4, 7),
			IMX_GPIO_NR(4, 15),
		},
		.pcie_rst = IMX_GPIO_NR(1, 29),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.gps_shdn = IMX_GPIO_NR(1, 27),
		.vidin_en = IMX_GPIO_NR(3, 31),
	},

	/* GW54xx */
	{
		.gpio_pads = gw54xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw54xx_gpio_pads),
		.dio_cfg = {
			{ MX6_PAD_GPIO_9__GPIO1_IO09, IMX_GPIO_NR(1, 9),
			  MX6_PAD_GPIO_9__PWM1_OUT, 1 },
			{ MX6_PAD_SD1_DAT2__GPIO1_IO19, IMX_GPIO_NR(1, 19),
			  MX6_PAD_SD1_DAT2__PWM2_OUT, 2 },
			{ MX6_PAD_SD4_DAT1__GPIO2_IO09, IMX_GPIO_NR(2, 9),
			  MX6_PAD_SD4_DAT1__PWM3_OUT, 3 },
			{ MX6_PAD_SD4_DAT2__GPIO2_IO10, IMX_GPIO_NR(2, 10),
			  MX6_PAD_SD4_DAT2__PWM4_OUT, 4 },
		},
		.leds = {
			IMX_GPIO_NR(4, 6),
			IMX_GPIO_NR(4, 7),
			IMX_GPIO_NR(4, 15),
		},
		.pcie_rst = IMX_GPIO_NR(1, 29),
		.mezz_pwren = IMX_GPIO_NR(2, 19),
		.mezz_irq = IMX_GPIO_NR(2, 18),
		.rs485en = IMX_GPIO_NR(7, 1),
		.vidin_en = IMX_GPIO_NR(3, 31),
		.dioi2c_en = IMX_GPIO_NR(4,  5),
		.pcie_sson = IMX_GPIO_NR(1, 20),
	},
};

/* setup GPIO pinmux and default configuration per baseboard */
static void setup_board_gpio(int board)
{
	struct ventana_board_info *info = &ventana_info;
	const char *s;
	char arg[10];
	size_t len;
	int i;
	int quiet = simple_strtol(getenv("quiet"), NULL, 10);

	if (board >= GW_UNKNOWN)
		return;

	/* RS232_EN# */
	gpio_direction_output(GP_RS232_EN, (hwconfig("rs232")) ? 0 : 1);

	/* MSATA Enable */
	if (is_cpu_type(MXC_CPU_MX6Q) &&
	    test_bit(EECONFIG_SATA, info->config)) {
		gpio_direction_output(GP_MSATA_SEL,
				      (hwconfig("msata")) ?  1 : 0);
	} else {
		gpio_direction_output(GP_MSATA_SEL, 0);
	}

	/*
	 * assert PCI_RST# (released by OS when clock is valid)
	 * TODO: figure out why leaving this de-asserted from PCI scan on boot
	 *       causes linux pcie driver to hang during enumeration
	 */
	gpio_direction_output(gpio_cfg[board].pcie_rst, 0);

	/* turn off (active-high) user LED's */
	for (i = 0; i < 4; i++) {
		if (gpio_cfg[board].leds[i])
			gpio_direction_output(gpio_cfg[board].leds[i], 1);
	}

	/* Expansion Mezzanine IO */
	gpio_direction_output(gpio_cfg[board].mezz_pwren, 0);
	gpio_direction_input(gpio_cfg[board].mezz_irq);

	/* RS485 Transmit Enable */
	if (gpio_cfg[board].rs485en)
		gpio_direction_output(gpio_cfg[board].rs485en, 0);

	/* GPS_SHDN */
	if (gpio_cfg[board].gps_shdn)
		gpio_direction_output(gpio_cfg[board].gps_shdn, 1);

	/* Analog video codec power enable */
	if (gpio_cfg[board].vidin_en)
		gpio_direction_output(gpio_cfg[board].vidin_en, 1);

	/* DIOI2C_DIS# */
	if (gpio_cfg[board].dioi2c_en)
		gpio_direction_output(gpio_cfg[board].dioi2c_en, 0);

	/* PCICK_SSON: disable spread-spectrum clock */
	if (gpio_cfg[board].pcie_sson)
		gpio_direction_output(gpio_cfg[board].pcie_sson, 0);

	/* USBOTG Select (PCISKT or FrontPanel) */
	if (gpio_cfg[board].usb_sel)
		gpio_direction_output(gpio_cfg[board].usb_sel, 0);

	/*
	 * Configure DIO pinmux/padctl registers
	 * see IMX6DQRM/IMX6SDLRM IOMUXC_SW_PAD_CTL_PAD_* register definitions
	 */
	for (i = 0; i < 4; i++) {
		struct dio_cfg *cfg = &gpio_cfg[board].dio_cfg[i];
		unsigned ctrl = DIO_PAD_CTRL;

		sprintf(arg, "dio%d", i);
		if (!hwconfig(arg))
			continue;
		s = hwconfig_subarg(arg, "padctrl", &len);
		if (s)
			ctrl = simple_strtoul(s, NULL, 16) & 0x3ffff;
		if (hwconfig_subarg_cmp(arg, "mode", "gpio")) {
			if (!quiet) {
				printf("DIO%d:  GPIO%d_IO%02d (gpio-%d)\n", i,
				       (cfg->gpio_param/32)+1,
				       cfg->gpio_param%32,
				       cfg->gpio_param);
			}
			imx_iomux_v3_setup_pad(cfg->gpio_padmux |
					       MUX_PAD_CTRL(ctrl));
			gpio_direction_input(cfg->gpio_param);
		} else if (hwconfig_subarg_cmp("dio2", "mode", "pwm") &&
			   cfg->pwm_padmux) {
			if (!quiet)
				printf("DIO%d:  pwm%d\n", i, cfg->pwm_param);
			imx_iomux_v3_setup_pad(cfg->pwm_padmux |
					       MUX_PAD_CTRL(ctrl));
		}
	}

	if (!quiet) {
		if (is_cpu_type(MXC_CPU_MX6Q) &&
		    (test_bit(EECONFIG_SATA, info->config))) {
			printf("MSATA: %s\n", (hwconfig("msata") ?
			       "enabled" : "disabled"));
		}
		printf("RS232: %s\n", (hwconfig("rs232")) ?
		       "enabled" : "disabled");
	}
}

#if defined(CONFIG_CMD_PCI)
int imx6_pcie_toggle_reset(void)
{
	if (board_type < GW_UNKNOWN) {
		gpio_direction_output(gpio_cfg[board_type].pcie_rst, 0);
		mdelay(50);
		gpio_direction_output(gpio_cfg[board_type].pcie_rst, 1);
	}
	return 0;
}
#endif /* CONFIG_CMD_PCI */

#ifdef CONFIG_SERIAL_TAG
/*
 * called when setting up ATAGS before booting kernel
 * populate serialnum from the following (in order of priority):
 *   serial# env var
 *   eeprom
 */
void get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial = getenv("serial#");

	if (serial) {
		serialnr->high = 0;
		serialnr->low = simple_strtoul(serial, NULL, 10);
	} else if (ventana_info.model[0]) {
		serialnr->high = 0;
		serialnr->low = ventana_info.serial;
	} else {
		serialnr->high = 0;
		serialnr->low = 0;
	}
}
#endif

/*
 * Board Support
 */

int board_early_init_f(void)
{
	setup_iomux_uart();
	gpio_direction_output(GP_USB_OTG_PWR, 0); /* OTG power off */

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM,
				    CONFIG_DDR_MB*1024*1024);

	return 0;
}

int board_init(void)
{
	struct iomuxc_base_regs *const iomuxc_regs
		= (struct iomuxc_base_regs *)IOMUXC_BASE_ADDR;

	clrsetbits_le32(&iomuxc_regs->gpr[1],
			IOMUXC_GPR1_OTG_ID_MASK,
			IOMUXC_GPR1_OTG_ID_GPIO1);

	/* address of linux boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_CMD_NAND
	setup_gpmi_nand();
#endif
#ifdef CONFIG_MXC_SPI
	setup_spi();
#endif
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info0);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);

#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif
	/* read Gateworks EEPROM into global struct (used later) */
	board_type = read_eeprom();

	/* board-specifc GPIO iomux */
	if (board_type < GW_UNKNOWN) {
		imx_iomux_v3_setup_multiple_pads(gw_gpio_pads,
						 ARRAY_SIZE(gw_gpio_pads));
		imx_iomux_v3_setup_multiple_pads(gpio_cfg[board_type].gpio_pads,
						 gpio_cfg[board_type].num_pads);
	}

	return 0;
}

#if defined(CONFIG_DISPLAY_BOARDINFO_LATE)
/*
 * called during late init (after relocation and after board_init())
 * by virtue of CONFIG_DISPLAY_BOARDINFO_LATE as we needed i2c initialized and
 * EEPROM read.
 */
int checkboard(void)
{
	struct ventana_board_info *info = &ventana_info;
	unsigned char buf[4];
	const char *p;
	int quiet; /* Quiet or minimal output mode */

	quiet = 0;
	p = getenv("quiet");
	if (p)
		quiet = simple_strtol(p, NULL, 10);
	else
		setenv("quiet", "0");

	puts("\nGateworks Corporation Copyright 2014\n");
	if (info->model[0]) {
		printf("Model: %s\n", info->model);
		printf("MFGDate: %02x-%02x-%02x%02x\n",
		       info->mfgdate[0], info->mfgdate[1],
		       info->mfgdate[2], info->mfgdate[3]);
		printf("Serial:%d\n", info->serial);
	} else {
		puts("Invalid EEPROM - board will not function fully\n");
	}
	if (quiet)
		return 0;

	/* Display GSC firmware revision/CRC/status */
	i2c_set_bus_num(I2C_GSC);
	if (!gsc_i2c_read(GSC_SC_ADDR, GSC_SC_FWVER, 1, buf, 1)) {
		printf("GSC:   v%d", buf[0]);
		if (!gsc_i2c_read(GSC_SC_ADDR, GSC_SC_STATUS, 1, buf, 4)) {
			printf(" 0x%04x", buf[2] | buf[3]<<8); /* CRC */
			printf(" 0x%02x", buf[0]); /* irq status */
		}
		puts("\n");
	}
	/* Display RTC */
	if (!gsc_i2c_read(GSC_RTC_ADDR, 0x00, 1, buf, 4)) {
		printf("RTC:   %d\n",
		       buf[0] | buf[1]<<8 | buf[2]<<16 | buf[3]<<24);
	}

	return 0;
}
#endif

#ifdef CONFIG_CMD_BMODE
/*
 * BOOT_CFG1, BOOT_CFG2, BOOT_CFG3, BOOT_CFG4
 * see Table 8-11 and Table 5-9
 *  BOOT_CFG1[7] = 1 (boot from NAND)
 *  BOOT_CFG1[5] = 0 - raw NAND
 *  BOOT_CFG1[4] = 0 - default pad settings
 *  BOOT_CFG1[3:2] = 00 - devices = 1
 *  BOOT_CFG1[1:0] = 00 - Row Address Cycles = 3
 *  BOOT_CFG2[4:3] = 00 - Boot Search Count = 2
 *  BOOT_CFG2[2:1] = 01 - Pages In Block = 64
 *  BOOT_CFG2[0] = 0 - Reset time 12ms
 */
static const struct boot_mode board_boot_modes[] = {
	/* NAND: 64pages per block, 3 row addr cycles, 2 copies of FCB/DBBT */
	{ "nand", MAKE_CFGVAL(0x80, 0x02, 0x00, 0x00) },
	{ NULL, 0 },
};
#endif

/* late init */
int misc_init_r(void)
{
	struct ventana_board_info *info = &ventana_info;
	unsigned char reg;

	/* set env vars based on EEPROM data */
	if (ventana_info.model[0]) {
		char str[16], fdt[36];
		char *p;
		const char *cputype = "";
		int i;

		/*
		 * FDT name will be prefixed with CPU type.  Three versions
		 * will be created each increasingly generic and bootloader
		 * env scripts will try loading each from most specific to
		 * least.
		 */
		if (is_cpu_type(MXC_CPU_MX6Q))
			cputype = "imx6q";
		else if (is_cpu_type(MXC_CPU_MX6DL))
			cputype = "imx6dl";
		memset(str, 0, sizeof(str));
		for (i = 0; i < (sizeof(str)-1) && info->model[i]; i++)
			str[i] = tolower(info->model[i]);
		if (!getenv("model"))
			setenv("model", str);
		if (!getenv("fdt_file")) {
			sprintf(fdt, "%s-%s.dtb", cputype, str);
			setenv("fdt_file", fdt);
		}
		p = strchr(str, '-');
		if (p) {
			*p++ = 0;

			setenv("model_base", str);
			if (!getenv("fdt_file1")) {
				sprintf(fdt, "%s-%s.dtb", cputype, str);
				setenv("fdt_file1", fdt);
			}
			str[4] = 'x';
			str[5] = 'x';
			str[6] = 0;
			if (!getenv("fdt_file2")) {
				sprintf(fdt, "%s-%s.dtb", cputype, str);
				setenv("fdt_file2", fdt);
			}
		}

		/* initialize env from EEPROM */
		if (test_bit(EECONFIG_ETH0, info->config) &&
		    !getenv("ethaddr")) {
			eth_setenv_enetaddr("ethaddr", info->mac0);
		}
		if (test_bit(EECONFIG_ETH1, info->config) &&
		    !getenv("eth1addr")) {
			eth_setenv_enetaddr("eth1addr", info->mac1);
		}

		/* board serial-number */
		sprintf(str, "%6d", info->serial);
		setenv("serial#", str);
	}

	/* configure PFUZE100 PMIC (not used on all Ventana baseboards) */
	if ((board_type == GW54xx || board_type == GW54proto) &&
	    !pmic_init(I2C_PMIC)) {
		struct pmic *p = pmic_get("PFUZE100_PMIC");
		u32 reg;
		if (p && !pmic_probe(p)) {
			pmic_reg_read(p, PFUZE100_DEVICEID, &reg);
			printf("PMIC:  PFUZE100 ID=0x%02x\n", reg);

			/* Set VGEN1 to 1.5V and enable */
			pmic_reg_read(p, PFUZE100_VGEN1VOL, &reg);
			reg &= ~(LDO_VOL_MASK);
			reg |= (LDOA_1_50V | LDO_EN);
			pmic_reg_write(p, PFUZE100_VGEN1VOL, reg);

			/* Set SWBST to 5.0V and enable */
			pmic_reg_read(p, PFUZE100_SWBSTCON1, &reg);
			reg &= ~(SWBST_MODE_MASK | SWBST_VOL_MASK);
			reg |= (SWBST_5_00V | SWBST_MODE_AUTO);
			pmic_reg_write(p, PFUZE100_SWBSTCON1, reg);
		}
	}

	/* setup baseboard specific GPIO pinmux and config */
	setup_board_gpio(board_type);

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

	/*
	 *  The Gateworks System Controller implements a boot
	 *  watchdog (always enabled) as a workaround for IMX6 boot related
	 *  errata such as:
	 *    ERR005768 - no fix
	 *    ERR006282 - fixed in silicon r1.3
	 *    ERR007117 - fixed in silicon r1.3
	 *    ERR007220 - fixed in silicon r1.3
	 *  see http://cache.freescale.com/files/32bit/doc/errata/IMX6DQCE.pdf
	 *
	 * Disable the boot watchdog and display/clear the timeout flag if set
	 */
	i2c_set_bus_num(I2C_GSC);
	if (!gsc_i2c_read(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1)) {
		reg |= (1 << GSC_SC_CTRL1_WDDIS);
		if (gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			puts("Error: could not disable GSC Watchdog\n");
	} else {
		puts("Error: could not disable GSC Watchdog\n");
	}
	if (!gsc_i2c_read(GSC_SC_ADDR, GSC_SC_STATUS, 1, &reg, 1)) {
		if (reg & (1 << GSC_SC_IRQ_WATCHDOG)) { /* watchdog timeout */
			puts("GSC boot watchdog timeout detected");
			reg &= ~(1 << GSC_SC_IRQ_WATCHDOG); /* clear flag */
			gsc_i2c_write(GSC_SC_ADDR, GSC_SC_STATUS, 1, &reg, 1);
		}
	}

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)

/* FDT aliases associated with EEPROM config bits */
const char *fdt_aliases[] = {
	"ethernet0",
	"ethernet1",
	"hdmi_out",
	"ahci0",
	"pcie",
	"ssi0",
	"ssi1",
	"lcd0",
	"lvds0",
	"lvds1",
	"usb0",
	"usb1",
	"mmc0",
	"mmc1",
	"mmc2",
	"mmc3",
	"uart0",
	"uart1",
	"uart2",
	"uart3",
	"uart4",
	"ipu0",
	"ipu1",
	"can0",
	"mipi_dsi",
	"mipi_csi",
	"tzasc0",
	"tzasc1",
	"i2c0",
	"i2c1",
	"i2c2",
	"vpu",
	"csi0",
	"csi1",
	"caam",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"spi0",
	"spi1",
	"spi2",
	"spi3",
	"spi4",
	"spi5",
	NULL,
	NULL,
	"pps",
	NULL,
	NULL,
	NULL,
	"hdmi_in",
	"cvbs_out",
	"cvbs_in",
	"nand",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

/*
 * called prior to booting kernel or by 'fdt boardsetup' command
 *
 * unless 'fdt_noauto' env var is set we will update the following in the DTB:
 *  - mtd partitions based on mtdparts/mtdids env
 *  - system-serial (board serial num from EEPROM)
 *  - board (full model from EEPROM)
 *  - peripherals removed from DTB if not loaded on board (per EEPROM config)
 */
void ft_board_setup(void *blob, bd_t *bd)
{
	int bit;
	struct ventana_board_info *info = &ventana_info;
	struct node_info nodes[] = {
		{ "sst,w25q256",          MTD_DEV_TYPE_NOR, },  /* SPI flash */
		{ "fsl,imx6q-gpmi-nand",  MTD_DEV_TYPE_NAND, }, /* NAND flash */
	};
	const char *model = getenv("model");

	if (getenv("fdt_noauto")) {
		puts("   Skiping ft_board_setup (fdt_noauto defined)\n");
		return;
	}

	/* Update partition nodes using info from mtdparts env var */
	puts("   Updating MTD partitions...\n");
	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));

	if (!model) {
		puts("invalid board info: Leaving FDT fully enabled\n");
		return;
	}
	printf("   Adjusting FDT per EEPROM for %s...\n", model);

	/* board serial number */
	fdt_setprop(blob, 0, "system-serial", getenv("serial#"),
		    strlen(getenv("serial#") + 1));

	/* board (model contains model from device-tree) */
	fdt_setprop(blob, 0, "board", info->model,
		    strlen((const char *)info->model) + 1);

	/*
	 * Peripheral Config:
	 *  remove nodes by alias path if EEPROM config tells us the
	 *  peripheral is not loaded on the board.
	 */
	for (bit = 0; bit < 64; bit++) {
		if (!test_bit(bit, info->config))
			fdt_del_node_and_alias(blob, fdt_aliases[bit]);
	}
}
#endif /* defined(CONFIG_OF_FLAT_TREE) && defined(CONFIG_OF_BOARD_SETUP) */

