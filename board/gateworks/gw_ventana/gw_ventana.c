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
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/sata.h>
#include <asm/imx-common/spi.h>
#include <asm/imx-common/video.h>
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
#include <pci.h>
#include <power/pmic.h>
#include <power/ltc3676_pmic.h>
#include <power/pfuze100_pmic.h>
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

#define IRQ_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_34ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)

#define DIO_PAD_CFG   (MUX_PAD_CTRL(DIO_PAD_CTRL) | MUX_MODE_SION)


/*
 * EEPROM board info struct populated by read_eeprom so that we only have to
 * read it once.
 */
struct ventana_board_info ventana_info;

int board_type;

/* UART1: Function varies per baseboard */
iomux_v3_cfg_t const uart1_pads[] = {
	IOMUX_PADS(PAD_SD3_DAT6__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

/* UART2: Serial Console */
iomux_v3_cfg_t const uart2_pads[] = {
	IOMUX_PADS(PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

/* I2C1: GSC */
struct i2c_pads_info mx6q_i2c_pad_info0 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_EIM_D21__I2C1_SCL | PC,
		.gpio_mode = MX6Q_PAD_EIM_D21__GPIO3_IO21 | PC,
		.gp = IMX_GPIO_NR(3, 21)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_EIM_D28__I2C1_SDA | PC,
		.gpio_mode = MX6Q_PAD_EIM_D28__GPIO3_IO28 | PC,
		.gp = IMX_GPIO_NR(3, 28)
	}
};
struct i2c_pads_info mx6dl_i2c_pad_info0 = {
	.scl = {
		.i2c_mode = MX6DL_PAD_EIM_D21__I2C1_SCL | PC,
		.gpio_mode = MX6DL_PAD_EIM_D21__GPIO3_IO21 | PC,
		.gp = IMX_GPIO_NR(3, 21)
	},
	.sda = {
		.i2c_mode = MX6DL_PAD_EIM_D28__I2C1_SDA | PC,
		.gpio_mode = MX6DL_PAD_EIM_D28__GPIO3_IO28 | PC,
		.gp = IMX_GPIO_NR(3, 28)
	}
};

/* I2C2: PMIC/PCIe Switch/PCIe Clock/Mezz */
struct i2c_pads_info mx6q_i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_KEY_COL3__I2C2_SCL | PC,
		.gpio_mode = MX6Q_PAD_KEY_COL3__GPIO4_IO12 | PC,
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6Q_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};
struct i2c_pads_info mx6dl_i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6DL_PAD_KEY_COL3__I2C2_SCL | PC,
		.gpio_mode = MX6DL_PAD_KEY_COL3__GPIO4_IO12 | PC,
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6DL_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6DL_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

/* I2C3: Misc/Expansion */
struct i2c_pads_info mx6q_i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_GPIO_3__I2C3_SCL | PC,
		.gpio_mode = MX6Q_PAD_GPIO_3__GPIO1_IO03 | PC,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_GPIO_6__I2C3_SDA | PC,
		.gpio_mode = MX6Q_PAD_GPIO_6__GPIO1_IO06 | PC,
		.gp = IMX_GPIO_NR(1, 6)
	}
};
struct i2c_pads_info mx6dl_i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6DL_PAD_GPIO_3__I2C3_SCL | PC,
		.gpio_mode = MX6DL_PAD_GPIO_3__GPIO1_IO03 | PC,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6DL_PAD_GPIO_6__I2C3_SDA | PC,
		.gpio_mode = MX6DL_PAD_GPIO_6__GPIO1_IO06 | PC,
		.gp = IMX_GPIO_NR(1, 6)
	}
};

/* MMC */
iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	/* CD */
	IOMUX_PADS(PAD_SD3_DAT5__GPIO7_IO00  | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
};

/* ENET */
iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC    | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL |
		   MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK |
		   MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3  | MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL |
		   MUX_PAD_CTRL(ENET_PAD_CTRL)),
	/* PHY nRST */
	IOMUX_PADS(PAD_ENET_TXD0__GPIO1_IO30 | DIO_PAD_CFG),
};

/* NAND */
iomux_v3_cfg_t const nfc_pads[] = {
	IOMUX_PADS(PAD_NANDF_CLE__NAND_CLE     | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_ALE__NAND_ALE     | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_WP_B__NAND_WP_B   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_RB0__NAND_READY_B | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS0__NAND_CE0_B   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CMD__NAND_RE_B      | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CLK__NAND_WE_B      | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D0__NAND_DATA00   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D1__NAND_DATA01   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D2__NAND_DATA02   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D3__NAND_DATA03   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D4__NAND_DATA04   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D5__NAND_DATA05   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D6__NAND_DATA06   | MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_D7__NAND_DATA07   | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

#ifdef CONFIG_CMD_NAND
static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	SETUP_IOMUX_PADS(nfc_pads);

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
	SETUP_IOMUX_PADS(enet_pads);

	/* toggle PHY_RST# */
	gpio_direction_output(GP_PHY_RST, 0);
	mdelay(2);
	gpio_set_value(GP_PHY_RST, 1);
}

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
	SETUP_IOMUX_PADS(uart2_pads);
}

#ifdef CONFIG_USB_EHCI_MX6
iomux_v3_cfg_t const usb_pads[] = {
	IOMUX_PADS(PAD_GPIO_1__USB_OTG_ID   | DIO_PAD_CFG),
	IOMUX_PADS(PAD_KEY_COL4__USB_OTG_OC | DIO_PAD_CFG),
	/* OTG PWR */
	IOMUX_PADS(PAD_EIM_D22__GPIO3_IO22  | DIO_PAD_CFG),
};

int board_ehci_hcd_init(int port)
{
	struct ventana_board_info *info = &ventana_info;

	SETUP_IOMUX_PADS(usb_pads);

	/* Reset USB HUB (present on GW54xx/GW53xx) */
	switch (info->model[3]) {
	case '3': /* GW53xx */
	case '5': /* GW552x */
		SETUP_IOMUX_PAD(PAD_GPIO_9__GPIO1_IO09 | DIO_PAD_CFG);
		gpio_direction_output(IMX_GPIO_NR(1, 9), 0);
		mdelay(2);
		gpio_set_value(IMX_GPIO_NR(1, 9), 1);
		break;
	case '4': /* GW54xx */
		SETUP_IOMUX_PAD(PAD_SD1_DAT0__GPIO1_IO16 | DIO_PAD_CFG);
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
	SETUP_IOMUX_PADS(usdhc3_pads);
	usdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg.max_bus_width = 4;

	return fsl_esdhc_initialize(bis, &usdhc_cfg);
}
#endif /* CONFIG_FSL_ESDHC */

#ifdef CONFIG_MXC_SPI
iomux_v3_cfg_t const ecspi1_pads[] = {
	/* SS1 */
	IOMUX_PADS(PAD_EIM_D19__GPIO3_IO19  | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL)),
};

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(3, 19)) : -1;
}

static void setup_spi(void)
{
	gpio_direction_output(IMX_GPIO_NR(3, 19), 1);
	SETUP_IOMUX_PADS(ecspi1_pads);
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
	if (board_type != GW552x)
		cpu_eth_init(bis);
#endif

#ifdef CONFIG_CI_UDC
	/* For otg ethernet*/
	usb_eth_initialize(bis);
#endif

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)

static void enable_hdmi(struct display_info_t const *dev)
{
	imx_enable_hdmi_phy();
}

static int detect_i2c(struct display_info_t const *dev)
{
	return i2c_set_bus_num(dev->bus) == 0 &&
		i2c_probe(dev->addr) == 0;
}

static void enable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)
				IOMUXC_BASE_ADDR;

	/* set CH0 data width to 24bit (IOMUXC_GPR2:5 0=18bit, 1=24bit) */
	u32 reg = readl(&iomux->gpr[2]);
	reg |= IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT;
	writel(reg, &iomux->gpr[2]);

	/* Enable Backlight */
	SETUP_IOMUX_PAD(PAD_SD1_CMD__GPIO1_IO18 | DIO_PAD_CFG);
	gpio_direction_output(IMX_GPIO_NR(1, 18), 1);
}

struct display_info_t const displays[] = {{
	/* HDMI Output */
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_hdmi,
	.enable	= enable_hdmi,
	.mode	= {
		.name           = "HDMI",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	/* Freescale MXC-LVDS1: HannStar HSD100PXN1-A00 w/ egalx_ts cont */
	.bus	= 2,
	.addr	= 0x4,
	.pixfmt	= IPU_PIX_FMT_LVDS666,
	.detect	= detect_i2c,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "Hannstar-XGA",
		.refresh        = 60,
		.xres           = 1024,
		.yres           = 768,
		.pixclock       = 15385,
		.left_margin    = 220,
		.right_margin   = 40,
		.upper_margin   = 21,
		.lower_margin   = 7,
		.hsync_len      = 60,
		.vsync_len      = 10,
		.sync           = FB_SYNC_EXT,
		.vmode          = FB_VMODE_NONINTERLACED
} } };
size_t display_count = ARRAY_SIZE(displays);

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	imx_setup_hdmi();
	/* Turn on LDB0,IPU,IPU DI0 clocks */
	reg = __raw_readl(&mxc_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_LDB_DI0_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 |MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3<<MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
	      |(3<<MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<<MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	     |IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
	     |IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
	     |IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
	     |IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
	     |IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	     |IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
	     |IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	     |IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~IOMUXC_GPR3_LVDS0_MUX_CTL_MASK)
	    | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
	       <<IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);

	/* Backlight CABEN on LVDS connector */
	SETUP_IOMUX_PAD(PAD_SD2_CLK__GPIO1_IO10 | DIO_PAD_CFG);
	gpio_direction_output(IMX_GPIO_NR(1, 10), 0);
}
#endif /* CONFIG_VIDEO_IPUV3 */

/*
 * Baseboard specific GPIO
 */

/* common to add baseboards */
static iomux_v3_cfg_t const gw_gpio_pads[] = {
	/* MSATA_EN */
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | DIO_PAD_CFG),
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
};

/* prototype */
static iomux_v3_cfg_t const gwproto_gpio_pads[] = {
	/* PANLEDG# */
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | DIO_PAD_CFG),
	/* PANLEDR# */
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07 | DIO_PAD_CFG),
	/* LOCLED# */
	IOMUX_PADS(PAD_KEY_ROW4__GPIO4_IO15 | DIO_PAD_CFG),
	/* RS485_EN */
	IOMUX_PADS(PAD_SD3_DAT4__GPIO7_IO01 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* VID_EN */
	IOMUX_PADS(PAD_EIM_D31__GPIO3_IO31 | DIO_PAD_CFG),
	/* DIOI2C_DIS# */
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05 | DIO_PAD_CFG),
	/* PCICK_SSON */
	IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20 | DIO_PAD_CFG),
	/* PCI_RST# */
	IOMUX_PADS(PAD_ENET_TXD1__GPIO1_IO29 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw51xx_gpio_pads[] = {
	/* PANLEDG# */
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | DIO_PAD_CFG),
	/* PANLEDR# */
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),

	/* GPS_SHDN */
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02 | DIO_PAD_CFG),
	/* VID_PWR */
	IOMUX_PADS(PAD_CSI0_DATA_EN__GPIO5_IO20 | DIO_PAD_CFG),
	/* PCI_RST# */
	IOMUX_PADS(PAD_GPIO_0__GPIO1_IO00 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw52xx_gpio_pads[] = {
	/* PANLEDG# */
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | DIO_PAD_CFG),
	/* PANLEDR# */
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),

	/* MX6_LOCLED# */
	IOMUX_PADS(PAD_KEY_ROW4__GPIO4_IO15 | DIO_PAD_CFG),
	/* GPS_SHDN */
	IOMUX_PADS(PAD_ENET_RXD0__GPIO1_IO27 | DIO_PAD_CFG),
	/* USBOTG_SEL */
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02 | DIO_PAD_CFG),
	/* VID_PWR */
	IOMUX_PADS(PAD_EIM_D31__GPIO3_IO31 | DIO_PAD_CFG),
	/* PCI_RST# */
	IOMUX_PADS(PAD_ENET_TXD1__GPIO1_IO29 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw53xx_gpio_pads[] = {
	/* PANLEDG# */
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | DIO_PAD_CFG),
	/* PANLEDR# */
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07 | DIO_PAD_CFG),
	/* MX6_LOCLED# */
	IOMUX_PADS(PAD_KEY_ROW4__GPIO4_IO15 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* DIOI2C_DIS# */
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05 | DIO_PAD_CFG),
	/* GPS_SHDN */
	IOMUX_PADS(PAD_ENET_RXD0__GPIO1_IO27 | DIO_PAD_CFG),
	/* VID_EN */
	IOMUX_PADS(PAD_EIM_D31__GPIO3_IO31 | DIO_PAD_CFG),
	/* PCI_RST# */
	IOMUX_PADS(PAD_ENET_TXD1__GPIO1_IO29 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw54xx_gpio_pads[] = {
	/* PANLEDG# */
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | DIO_PAD_CFG),
	/* PANLEDR# */
	IOMUX_PADS(PAD_KEY_COL2__GPIO4_IO10 | DIO_PAD_CFG),
	/* MX6_LOCLED# */
	IOMUX_PADS(PAD_KEY_ROW4__GPIO4_IO15 | DIO_PAD_CFG),
	/* MIPI_DIO */
	IOMUX_PADS(PAD_SD1_DAT3__GPIO1_IO21 | DIO_PAD_CFG),
	/* RS485_EN */
	IOMUX_PADS(PAD_EIM_D24__GPIO3_IO24 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_KEY_ROW1__GPIO4_IO09 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* DIOI2C_DIS# */
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05 | DIO_PAD_CFG),
	/* PCICK_SSON */
	IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20 | DIO_PAD_CFG),
	/* PCI_RST# */
	IOMUX_PADS(PAD_ENET_TXD1__GPIO1_IO29 | DIO_PAD_CFG),
	/* VID_EN */
	IOMUX_PADS(PAD_EIM_D31__GPIO3_IO31 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_DISP0_DAT23__GPIO5_IO17 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw552x_gpio_pads[] = {
	/* PANLEDG# */
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | DIO_PAD_CFG),
	/* PANLEDR# */
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07 | DIO_PAD_CFG),
	/* MX6_LOCLED# */
	IOMUX_PADS(PAD_KEY_ROW4__GPIO4_IO15 | DIO_PAD_CFG),
	/* PCI_RST# */
	IOMUX_PADS(PAD_ENET_TXD1__GPIO1_IO29 | DIO_PAD_CFG),
	/* MX6_DIO[4:9] */
	IOMUX_PADS(PAD_CSI0_PIXCLK__GPIO5_IO18 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_CSI0_DATA_EN__GPIO5_IO20 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_CSI0_VSYNC__GPIO5_IO21 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_CSI0_DAT4__GPIO5_IO22 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_CSI0_DAT5__GPIO5_IO23 | DIO_PAD_CFG),
	IOMUX_PADS(PAD_CSI0_DAT7__GPIO5_IO25 | DIO_PAD_CFG),
	/* PCIEGBE1_OFF# */
	IOMUX_PADS(PAD_GPIO_1__GPIO1_IO01 | DIO_PAD_CFG),
	/* PCIEGBE2_OFF# */
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

/*
 * each baseboard has 4 user configurable Digital IO lines which can
 * be pinmuxed as a GPIO or in some cases a PWM
 */
struct dio_cfg {
	iomux_v3_cfg_t gpio_padmux[2];
	unsigned gpio_param;
	iomux_v3_cfg_t pwm_padmux[2];
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
	int wdis;
};

struct ventana gpio_cfg[] = {
	/* GW5400proto */
	{
		.gpio_pads = gw54xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw54xx_gpio_pads)/2,
		.dio_cfg = {
			{
				{ IOMUX_PADS(PAD_GPIO_9__GPIO1_IO09) },
				IMX_GPIO_NR(1, 9),
				{ IOMUX_PADS(PAD_GPIO_9__PWM1_OUT) },
				1
			},
			{
				{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
				IMX_GPIO_NR(1, 19),
				{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
				2
			},
			{
				{ IOMUX_PADS(PAD_SD4_DAT1__GPIO2_IO09) },
				IMX_GPIO_NR(2, 9),
				{ IOMUX_PADS(PAD_SD4_DAT1__PWM3_OUT) },
				3
			},
			{
				{ IOMUX_PADS(PAD_SD4_DAT2__GPIO2_IO10) },
				IMX_GPIO_NR(2, 10),
				{ IOMUX_PADS(PAD_SD4_DAT2__PWM4_OUT) },
				4
			},
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
		.num_pads = ARRAY_SIZE(gw51xx_gpio_pads)/2,
		.dio_cfg = {
			{
				{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
				IMX_GPIO_NR(1, 16),
				{ 0, 0 },
				0
			},
			{
				{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
				IMX_GPIO_NR(1, 19),
				{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
				2
			},
			{
				{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
				IMX_GPIO_NR(1, 17),
				{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
				3
			},
			{
				{ IOMUX_PADS(PAD_SD1_CMD__GPIO1_IO18) },
				IMX_GPIO_NR(1, 18),
				{ IOMUX_PADS(PAD_SD1_CMD__PWM4_OUT) },
				4
			},
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
		.wdis = IMX_GPIO_NR(7, 12),
	},

	/* GW52xx */
	{
		.gpio_pads = gw52xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw52xx_gpio_pads)/2,
		.dio_cfg = {
			{
				{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
				IMX_GPIO_NR(1, 16),
				{ 0, 0 },
				0
			},
			{
				{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
				IMX_GPIO_NR(1, 19),
				{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
				2
			},
			{
				{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
				IMX_GPIO_NR(1, 17),
				{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
				3
			},
			{
				{ IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20) },
				IMX_GPIO_NR(1, 20),
				{ 0, 0 },
				0
			},
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
		.wdis = IMX_GPIO_NR(7, 12),
	},

	/* GW53xx */
	{
		.gpio_pads = gw53xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw53xx_gpio_pads)/2,
		.dio_cfg = {
			{
				{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
				IMX_GPIO_NR(1, 16),
				{ 0, 0 },
				0
			},
			{
				{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
				IMX_GPIO_NR(1, 19),
				{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
				2
			},
			{
				{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
				IMX_GPIO_NR(1, 17),
				{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
				3
			},
			{
				{IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20) },
				IMX_GPIO_NR(1, 20),
				{ 0, 0 },
				0
			},
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
		.wdis = IMX_GPIO_NR(7, 12),
	},

	/* GW54xx */
	{
		.gpio_pads = gw54xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw54xx_gpio_pads)/2,
		.dio_cfg = {
			{
				{ IOMUX_PADS(PAD_GPIO_9__GPIO1_IO09) },
				IMX_GPIO_NR(1, 9),
				{ IOMUX_PADS(PAD_GPIO_9__PWM1_OUT) },
				1
			},
			{
				{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
				IMX_GPIO_NR(1, 19),
				{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
				2
			},
			{
				{ IOMUX_PADS(PAD_SD4_DAT1__GPIO2_IO09) },
				IMX_GPIO_NR(2, 9),
				{ IOMUX_PADS(PAD_SD4_DAT1__PWM3_OUT) },
				3
			},
			{
				{ IOMUX_PADS(PAD_SD4_DAT2__GPIO2_IO10) },
				IMX_GPIO_NR(2, 10),
				{ IOMUX_PADS(PAD_SD4_DAT2__PWM4_OUT) },
				4
			},
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
		.wdis = IMX_GPIO_NR(5, 17),
	},

	/* GW552x */
	{
		.gpio_pads = gw552x_gpio_pads,
		.num_pads = ARRAY_SIZE(gw552x_gpio_pads)/2,
		.dio_cfg = {
			{
				{ IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16) },
				IMX_GPIO_NR(1, 16),
				{ 0, 0 },
				0
			},
			{
				{ IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19) },
				IMX_GPIO_NR(1, 19),
				{ IOMUX_PADS(PAD_SD1_DAT2__PWM2_OUT) },
				2
			},
			{
				{ IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17) },
				IMX_GPIO_NR(1, 17),
				{ IOMUX_PADS(PAD_SD1_DAT1__PWM3_OUT) },
				3
			},
			{
				{ IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20) },
				IMX_GPIO_NR(2, 10),
				{ 0, 0 },
				0
			},
		},
		.leds = {
			IMX_GPIO_NR(4, 6),
			IMX_GPIO_NR(4, 7),
			IMX_GPIO_NR(4, 15),
		},
		.pcie_rst = IMX_GPIO_NR(1, 29),
	},
};

/* setup board specific PMIC */
int power_init_board(void)
{
	struct pmic *p;
	u32 reg;

	/* configure PFUZE100 PMIC */
	if (board_type == GW54xx || board_type == GW54proto) {
		power_pfuze100_init(CONFIG_I2C_PMIC);
		p = pmic_get("PFUZE100");
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

	/* configure LTC3676 PMIC */
	else {
		power_ltc3676_init(CONFIG_I2C_PMIC);
		p = pmic_get("LTC3676_PMIC");
		if (p && !pmic_probe(p)) {
			puts("PMIC:  LTC3676\n");
			/* set board-specific scalar to 1225mV for IMX6Q@1GHz */
			if (is_cpu_type(MXC_CPU_MX6Q)) {
				/* mask PGOOD during SW1 transition */
				reg = 0x1d | LTC3676_PGOOD_MASK;
				pmic_reg_write(p, LTC3676_DVB1B, reg);
				/* set SW1 (VDD_SOC) to 1259mV */
				reg = 0x1d;
				pmic_reg_write(p, LTC3676_DVB1A, reg);

				/* mask PGOOD during SW3 transition */
				reg = 0x1d | LTC3676_PGOOD_MASK;
				pmic_reg_write(p, LTC3676_DVB3B, reg);
				/*set SW3 (VDD_ARM) to 1259mV */
				reg = 0x1d;
				pmic_reg_write(p, LTC3676_DVB3A, reg);
			}
		}
	}

	return 0;
}

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

#if !defined(CONFIG_CMD_PCI)
	/* assert PCI_RST# (released by OS when clock is valid) */
	gpio_direction_output(gpio_cfg[board].pcie_rst, 0);
#endif

	/* turn off (active-high) user LED's */
	for (i = 0; i < ARRAY_SIZE(gpio_cfg[board].leds); i++) {
		if (gpio_cfg[board].leds[i])
			gpio_direction_output(gpio_cfg[board].leds[i], 1);
	}

	/* Expansion Mezzanine IO */
	if (gpio_cfg[board].mezz_pwren)
		gpio_direction_output(gpio_cfg[board].mezz_pwren, 0);
	if (gpio_cfg[board].mezz_irq)
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

	/* PCISKT_WDIS# (Wireless disable GPIO to miniPCIe sockets) */
	if (gpio_cfg[board].wdis)
		gpio_direction_output(gpio_cfg[board].wdis, 1);

	/*
	 * Configure DIO pinmux/padctl registers
	 * see IMX6DQRM/IMX6SDLRM IOMUXC_SW_PAD_CTL_PAD_* register definitions
	 */
	for (i = 0; i < 4; i++) {
		struct dio_cfg *cfg = &gpio_cfg[board].dio_cfg[i];
		iomux_v3_cfg_t ctrl = DIO_PAD_CFG;
		unsigned cputype = is_cpu_type(MXC_CPU_MX6Q) ? 0 : 1;

		sprintf(arg, "dio%d", i);
		if (!hwconfig(arg))
			continue;
		s = hwconfig_subarg(arg, "padctrl", &len);
		if (s) {
			ctrl = MUX_PAD_CTRL(simple_strtoul(s, NULL, 16)
					    & 0x1ffff) | MUX_MODE_SION;
		}
		if (hwconfig_subarg_cmp(arg, "mode", "gpio")) {
			if (!quiet) {
				printf("DIO%d:  GPIO%d_IO%02d (gpio-%d)\n", i,
				       (cfg->gpio_param/32)+1,
				       cfg->gpio_param%32,
				       cfg->gpio_param);
			}
			imx_iomux_v3_setup_pad(cfg->gpio_padmux[cputype] |
					       ctrl);
			gpio_direction_input(cfg->gpio_param);
		} else if (hwconfig_subarg_cmp("dio2", "mode", "pwm") &&
			   cfg->pwm_padmux) {
			if (!quiet)
				printf("DIO%d:  pwm%d\n", i, cfg->pwm_param);
			imx_iomux_v3_setup_pad(cfg->pwm_padmux[cputype] |
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
		uint pin = gpio_cfg[board_type].pcie_rst;
		gpio_direction_output(pin, 0);
		mdelay(50);
		gpio_direction_output(pin, 1);
	}
	return 0;
}

/*
 * Most Ventana boards have a PLX PEX860x PCIe switch onboard and use its
 * GPIO's as PERST# signals for its downstream ports - configure the GPIO's
 * properly and assert reset for 100ms.
 */
void board_pci_fixup_dev(struct pci_controller *hose, pci_dev_t dev,
			 unsigned short vendor, unsigned short device,
			 unsigned short class)
{
	u32 dw;

	debug("%s: %02d:%02d.%02d: %04x:%04x\n", __func__,
	      PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev), vendor, device);
	if (vendor == PCI_VENDOR_ID_PLX &&
	    (device & 0xfff0) == 0x8600 &&
	    PCI_DEV(dev) == 0 && PCI_FUNC(dev) == 0) {
		debug("configuring PLX 860X downstream PERST#\n");
		pci_hose_read_config_dword(hose, dev, 0x62c, &dw);
		dw |= 0xaaa8; /* GPIO1-7 outputs */
		pci_hose_write_config_dword(hose, dev, 0x62c, dw);

		pci_hose_read_config_dword(hose, dev, 0x644, &dw);
		dw |= 0xfe;   /* GPIO1-7 output high */
		pci_hose_write_config_dword(hose, dev, 0x644, dw);

		mdelay(100);
	}
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

/* called from SPL board_init_f() */
int board_early_init_f(void)
{
	setup_iomux_uart();
	gpio_direction_output(GP_USB_OTG_PWR, 0); /* OTG power off */

#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

int board_init(void)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

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
	if (is_cpu_type(MXC_CPU_MX6Q)) {
		setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info0);
		setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info1);
		setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info2);
	} else {
		setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c_pad_info0);
		setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c_pad_info1);
		setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c_pad_info2);
	}

#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif
	/* read Gateworks EEPROM into global struct (used later) */
	board_type = read_eeprom(CONFIG_I2C_GSC, &ventana_info);

	/* board-specifc GPIO iomux */
	SETUP_IOMUX_PADS(gw_gpio_pads);
	if (board_type < GW_UNKNOWN) {
		iomux_v3_cfg_t const *p = gpio_cfg[board_type].gpio_pads;
		int count = gpio_cfg[board_type].num_pads;

		imx_iomux_v3_setup_multiple_pads(p, count);
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
	i2c_set_bus_num(CONFIG_I2C_GSC);
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
		if (is_cpu_type(MXC_CPU_MX6Q) ||
		    is_cpu_type(MXC_CPU_MX6D))
			cputype = "imx6q";
		else if (is_cpu_type(MXC_CPU_MX6DL) ||
			 is_cpu_type(MXC_CPU_MX6SOLO))
			cputype = "imx6dl";
		setenv("soctype", cputype);
		if (8 << (ventana_info.nand_flash_size-1) >= 2048)
			setenv("flash_layout", "large");
		else
			setenv("flash_layout", "normal");
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
			if (board_type != GW552x)
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


	/* setup baseboard specific GPIO pinmux and config */
	setup_board_gpio(board_type);

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

	/*
	 *  The Gateworks System Controller implements a boot
	 *  watchdog (always enabled) as a workaround for IMX6 boot related
	 *  errata such as:
	 *    ERR005768 - no fix scheduled
	 *    ERR006282 - fixed in silicon r1.2
	 *    ERR007117 - fixed in silicon r1.3
	 *    ERR007220 - fixed in silicon r1.3
	 *    ERR007926 - no fix scheduled
	 *  see http://cache.freescale.com/files/32bit/doc/errata/IMX6DQCE.pdf
	 *
	 * Disable the boot watchdog and display/clear the timeout flag if set
	 */
	i2c_set_bus_num(CONFIG_I2C_GSC);
	if (!gsc_i2c_read(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1)) {
		reg |= (1 << GSC_SC_CTRL1_WDDIS);
		if (gsc_i2c_write(GSC_SC_ADDR, GSC_SC_CTRL1, 1, &reg, 1))
			puts("Error: could not disable GSC Watchdog\n");
	} else {
		puts("Error: could not disable GSC Watchdog\n");
	}
	if (!gsc_i2c_read(GSC_SC_ADDR, GSC_SC_STATUS, 1, &reg, 1)) {
		if (reg & (1 << GSC_SC_IRQ_WATCHDOG)) { /* watchdog timeout */
			puts("GSC boot watchdog timeout detected\n");
			reg &= ~(1 << GSC_SC_IRQ_WATCHDOG); /* clear flag */
			gsc_i2c_write(GSC_SC_ADDR, GSC_SC_STATUS, 1, &reg, 1);
		}
	}

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)

/*
 * called prior to booting kernel or by 'fdt boardsetup' command
 *
 * unless 'fdt_noauto' env var is set we will update the following in the DTB:
 *  - mtd partitions based on mtdparts/mtdids env
 *  - system-serial (board serial num from EEPROM)
 *  - board (full model from EEPROM)
 *  - peripherals removed from DTB if not loaded on board (per EEPROM config)
 */
int ft_board_setup(void *blob, bd_t *bd)
{
	struct ventana_board_info *info = &ventana_info;
	struct ventana_eeprom_config *cfg;
	struct node_info nodes[] = {
		{ "sst,w25q256",          MTD_DEV_TYPE_NOR, },  /* SPI flash */
		{ "fsl,imx6q-gpmi-nand",  MTD_DEV_TYPE_NAND, }, /* NAND flash */
	};
	const char *model = getenv("model");

	if (getenv("fdt_noauto")) {
		puts("   Skiping ft_board_setup (fdt_noauto defined)\n");
		return 0;
	}

	/* Update partition nodes using info from mtdparts env var */
	puts("   Updating MTD partitions...\n");
	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));

	if (!model) {
		puts("invalid board info: Leaving FDT fully enabled\n");
		return 0;
	}
	printf("   Adjusting FDT per EEPROM for %s...\n", model);

	/* board serial number */
	fdt_setprop(blob, 0, "system-serial", getenv("serial#"),
		    strlen(getenv("serial#")) + 1);

	/* board (model contains model from device-tree) */
	fdt_setprop(blob, 0, "board", info->model,
		    strlen((const char *)info->model) + 1);

	/*
	 * Peripheral Config:
	 *  remove nodes by alias path if EEPROM config tells us the
	 *  peripheral is not loaded on the board.
	 */
	if (getenv("fdt_noconfig")) {
		puts("   Skiping periperhal config (fdt_noconfig defined)\n");
		return 0;
	}
	cfg = econfig;
	while (cfg->name) {
		if (!test_bit(cfg->bit, info->config)) {
			fdt_del_node_and_alias(blob, cfg->dtalias ?
					       cfg->dtalias : cfg->name);
		}
		cfg++;
	}

	return 0;
}
#endif /* defined(CONFIG_OF_FLAT_TREE) && defined(CONFIG_OF_BOARD_SETUP) */

