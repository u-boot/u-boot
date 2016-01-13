/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/spi.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/imx-common/video.h>
#include <asm/arch/crm_regs.h>
#include <pca953x.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include "../common/pfuze.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define GPMI_PAD_CTRL0 (PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_100K_UP)
#define GPMI_PAD_CTRL1 (PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED | \
			PAD_CTL_SRE_FAST)
#define GPMI_PAD_CTRL2 (GPMI_PAD_CTRL0 | GPMI_PAD_CTRL1)

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

#define WEIM_NOR_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |          \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)

#define I2C_PMIC	1

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static iomux_v3_cfg_t const uart4_pads[] = {
	MX6_PAD_KEY_COL0__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_KEY_ROW0__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_KEY_COL1__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_KEY_COL2__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__RGMII_RXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
};

/* I2C2 PMIC, iPod, Tuner, Codec, Touch, HDMI EDID, MIPI CSI2 card */
static struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_EIM_EB2__I2C2_SCL | PC,
		.gpio_mode = MX6_PAD_EIM_EB2__GPIO2_IO30 | PC,
		.gp = IMX_GPIO_NR(2, 30)
	},
	.sda = {
		.i2c_mode = MX6_PAD_KEY_ROW3__I2C2_SDA | PC,
		.gpio_mode = MX6_PAD_KEY_ROW3__GPIO4_IO13 | PC,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

#ifndef CONFIG_SYS_FLASH_CFI
/*
 * I2C3 MLB, Port Expanders (A, B, C), Video ADC, Light Sensor,
 * Compass Sensor, Accelerometer, Res Touch
 */
static struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_3__I2C3_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO_3__GPIO1_IO03 | PC,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6_PAD_EIM_D18__I2C3_SDA | PC,
		.gpio_mode = MX6_PAD_EIM_D18__GPIO3_IO18 | PC,
		.gp = IMX_GPIO_NR(3, 18)
	}
};
#endif

static iomux_v3_cfg_t const i2c3_pads[] = {
	MX6_PAD_EIM_A24__GPIO5_IO04		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const port_exp[] = {
	MX6_PAD_SD2_DAT0__GPIO1_IO15		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

/*Define for building port exp gpio, pin starts from 0*/
#define PORTEXP_IO_NR(chip, pin) \
	((chip << 5) + pin)

/*Get the chip addr from a ioexp gpio*/
#define PORTEXP_IO_TO_CHIP(gpio_nr) \
	(gpio_nr >> 5)

/*Get the pin number from a ioexp gpio*/
#define PORTEXP_IO_TO_PIN(gpio_nr) \
	(gpio_nr & 0x1f)

static int port_exp_direction_output(unsigned gpio, int value)
{
	int ret;

	i2c_set_bus_num(2);
	ret = i2c_probe(PORTEXP_IO_TO_CHIP(gpio));
	if (ret)
		return ret;

	ret = pca953x_set_dir(PORTEXP_IO_TO_CHIP(gpio),
		(1 << PORTEXP_IO_TO_PIN(gpio)),
		(PCA953X_DIR_OUT << PORTEXP_IO_TO_PIN(gpio)));

	if (ret)
		return ret;

	ret = pca953x_set_val(PORTEXP_IO_TO_CHIP(gpio),
		(1 << PORTEXP_IO_TO_PIN(gpio)),
		(value << PORTEXP_IO_TO_PIN(gpio)));

	if (ret)
		return ret;

	return 0;
}

static iomux_v3_cfg_t const eimnor_pads[] = {
	MX6_PAD_EIM_D16__EIM_DATA16	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D17__EIM_DATA17	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D18__EIM_DATA18	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D19__EIM_DATA19	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D20__EIM_DATA20	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D21__EIM_DATA21	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D22__EIM_DATA22	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D23__EIM_DATA23	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D24__EIM_DATA24	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D25__EIM_DATA25	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D26__EIM_DATA26	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D27__EIM_DATA27	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D28__EIM_DATA28	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D29__EIM_DATA29	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D30__EIM_DATA30	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_D31__EIM_DATA31	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA0__EIM_AD00	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA1__EIM_AD01	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA2__EIM_AD02	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA3__EIM_AD03	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA4__EIM_AD04	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA5__EIM_AD05	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA6__EIM_AD06	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA7__EIM_AD07	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA8__EIM_AD08	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA9__EIM_AD09	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA10__EIM_AD10	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA11__EIM_AD11	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL) ,
	MX6_PAD_EIM_DA12__EIM_AD12	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA13__EIM_AD13	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA14__EIM_AD14	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_DA15__EIM_AD15	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A16__EIM_ADDR16	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A17__EIM_ADDR17	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A18__EIM_ADDR18	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A19__EIM_ADDR19	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A20__EIM_ADDR20	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A21__EIM_ADDR21	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A22__EIM_ADDR22	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_A23__EIM_ADDR23	| MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
	MX6_PAD_EIM_OE__EIM_OE_B	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_RW__EIM_RW		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_CS0__EIM_CS0_B	| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void eimnor_cs_setup(void)
{
	struct weim *weim_regs = (struct weim *)WEIM_BASE_ADDR;

	writel(0x00020181, &weim_regs->cs0gcr1);
	writel(0x00000001, &weim_regs->cs0gcr2);
	writel(0x0a020000, &weim_regs->cs0rcr1);
	writel(0x0000c000, &weim_regs->cs0rcr2);
	writel(0x0804a240, &weim_regs->cs0wcr1);
	writel(0x00000120, &weim_regs->wcr);

	set_chipselect_size(CS0_128);
}

static void setup_iomux_eimnor(void)
{
	imx_iomux_v3_setup_multiple_pads(eimnor_pads, ARRAY_SIZE(eimnor_pads));

	gpio_direction_output(IMX_GPIO_NR(5, 4), 0);

	eimnor_cs_setup();
}

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));
}

static iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT4__SD3_DATA4	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT5__SD3_DATA5	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT6__SD3_DATA6	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT7__SD3_DATA7	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_GPIO_18__SD3_VSELECT | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NANDF_CS2__GPIO6_IO15   | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
}

#ifdef CONFIG_FSL_ESDHC
static struct fsl_esdhc_cfg usdhc_cfg[1] = {
	{USDHC3_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	gpio_direction_input(IMX_GPIO_NR(6, 15));
	return !gpio_get_value(IMX_GPIO_NR(6, 15));
}

int board_mmc_init(bd_t *bis)
{
	imx_iomux_v3_setup_multiple_pads(usdhc3_pads, ARRAY_SIZE(usdhc3_pads));

	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
}
#endif

#ifdef CONFIG_NAND_MXS
static iomux_v3_cfg_t gpmi_pads[] = {
	MX6_PAD_NANDF_CLE__NAND_CLE		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_ALE__NAND_ALE		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_WP_B__NAND_WP_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_RB0__NAND_READY_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL0),
	MX6_PAD_NANDF_CS0__NAND_CE0_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_SD4_CMD__NAND_RE_B		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_SD4_CLK__NAND_WE_B		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D0__NAND_DATA00	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D1__NAND_DATA01	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D2__NAND_DATA02	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D3__NAND_DATA03	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D4__NAND_DATA04	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D5__NAND_DATA05	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D6__NAND_DATA06	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NANDF_D7__NAND_DATA07	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_SD4_DAT0__NAND_DQS		| MUX_PAD_CTRL(GPMI_PAD_CTRL1),
};

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	imx_iomux_v3_setup_multiple_pads(gpmi_pads, ARRAY_SIZE(gpmi_pads));

	setup_gpmi_io_clk((MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL(3)));

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#endif

int mx6_rgmii_rework(struct phy_device *phydev)
{
	unsigned short val;

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
		phydev->drv->config(phydev);

	return 0;
}

static void setup_fec(void)
{
	if (is_mx6dqp()) {
		/*
		 * select ENET MAC0 TX clock from PLL
		 */
		imx_iomux_set_gpr_register(5, 9, 1, 1);
		enable_fec_anatop_clock(0, ENET_125MHZ);
	}

	setup_iomux_enet();
}

int board_eth_init(bd_t *bis)
{
	setup_fec();

	return cpu_eth_init(bis);
}

#define BOARD_REV_B  0x200
#define BOARD_REV_A  0x100

static int mx6sabre_rev(void)
{
	/*
	 * Get Board ID information from OCOTP_GP1[15:8]
	 * i.MX6Q ARD RevA: 0x01
	 * i.MX6Q ARD RevB: 0x02
	 */
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[4];
	struct fuse_bank4_regs *fuse =
			(struct fuse_bank4_regs *)bank->fuse_regs;
	int reg = readl(&fuse->gp1);
	int ret;

	switch (reg >> 8 & 0x0F) {
	case 0x02:
		ret = BOARD_REV_B;
		break;
	case 0x01:
	default:
		ret = BOARD_REV_A;
		break;
	}

	return ret;
}

u32 get_board_rev(void)
{
	int rev = mx6sabre_rev();

	return (get_cpu_rev() & ~(0xF << 8)) | rev;
}

#if defined(CONFIG_VIDEO_IPUV3)
static void disable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	clrbits_le32(&iomux->gpr[2],
		     IOMUXC_GPR2_LVDS_CH0_MODE_MASK |
		     IOMUXC_GPR2_LVDS_CH1_MODE_MASK);
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
	disable_lvds(dev);
	imx_enable_hdmi_phy();
}

struct display_info_t const displays[] = {{
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.detect	= NULL,
	.enable	= NULL,
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
} }, {
	.bus	= -1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= detect_hdmi,
	.enable	= do_enable_hdmi,
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
		.vmode          = FB_VMODE_NONINTERLACED,
} } };
size_t display_count = ARRAY_SIZE(displays);

iomux_v3_cfg_t const backlight_pads[] = {
	MX6_PAD_SD4_DAT1__GPIO2_IO09 | MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static void setup_iomux_backlight(void)
{
	gpio_direction_output(IMX_GPIO_NR(2, 9), 1);
	imx_iomux_v3_setup_multiple_pads(backlight_pads,
					 ARRAY_SIZE(backlight_pads));
}

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	setup_iomux_backlight();
	enable_ipu_clock();
	imx_setup_hdmi();

	/* Turn on LDB_DI0 and LDB_DI1 clocks */
	reg = readl(&mxc_ccm->CCGR3);
	reg |= MXC_CCM_CCGR3_LDB_DI0_MASK | MXC_CCM_CCGR3_LDB_DI1_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* Set LDB_DI0 and LDB_DI1 clk select to 3b'011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK |
		 MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET) |
	       (3 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV | MXC_CCM_CSCMR2_LDB_DI1_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0 <<
		MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW |
	      IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW |
	      IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG |
	      IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT |
	      IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG |
	      IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT |
	      IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0 |
	      IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg &= ~(IOMUXC_GPR3_LVDS0_MUX_CTL_MASK |
		 IOMUXC_GPR3_HDMI_MUX_CTL_MASK);
	reg |= (IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
		IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET) |
	       (IOMUXC_GPR3_MUX_SRC_IPU1_DI0 <<
		IOMUXC_GPR3_HDMI_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}
#endif /* CONFIG_VIDEO_IPUV3 */

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

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/* I2C 2 and 3 setup - I2C 3 hw mux with EIM */
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	/* I2C 3 Steer */
	gpio_direction_output(IMX_GPIO_NR(5, 4), 1);
	imx_iomux_v3_setup_multiple_pads(i2c3_pads, ARRAY_SIZE(i2c3_pads));
#ifndef CONFIG_SYS_FLASH_CFI
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
#endif
	gpio_direction_output(IMX_GPIO_NR(1, 15), 1);
	imx_iomux_v3_setup_multiple_pads(port_exp, ARRAY_SIZE(port_exp));

#ifdef CONFIG_VIDEO_IPUV3
	setup_display();
#endif
	setup_iomux_eimnor();
	return 0;
}

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(4, 9)) : -1;
}
#endif

int power_init_board(void)
{
	struct pmic *p;
	unsigned int value;

	p = pfuze_common_init(I2C_PMIC);
	if (!p)
		return -ENODEV;

	if (is_mx6dqp()) {
		/* set SW2 staby volatage 0.975V*/
		pmic_reg_read(p, PFUZE100_SW2STBY, &value);
		value &= ~0x3f;
		value |= 0x17;
		pmic_reg_write(p, PFUZE100_SW2STBY, value);
	}

	return pfuze_mode_init(p, APS_PFM);
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0", MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	{NULL,   0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	setenv("board_name", "SABREAUTO");

	if (is_mx6dqp())
		setenv("board_rev", "MX6QP");
	else if (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D))
		setenv("board_rev", "MX6Q");
	else if (is_cpu_type(MXC_CPU_MX6DL) || is_cpu_type(MXC_CPU_MX6SOLO))
		setenv("board_rev", "MX6DL");
#endif

	return 0;
}

int checkboard(void)
{
	int rev = mx6sabre_rev();
	char *revname;

	switch (rev) {
	case BOARD_REV_B:
		revname = "B";
		break;
	case BOARD_REV_A:
	default:
		revname = "A";
		break;
	}

	printf("Board: MX6Q-Sabreauto rev%s\n", revname);

	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
#define USB_HOST1_PWR     PORTEXP_IO_NR(0x32, 7)
#define USB_OTG_PWR       PORTEXP_IO_NR(0x34, 1)

iomux_v3_cfg_t const usb_otg_pads[] = {
	MX6_PAD_ENET_RX_ER__USB_OTG_ID | MUX_PAD_CTRL(NO_PAD_CTRL),
};

int board_ehci_hcd_init(int port)
{
	switch (port) {
	case 0:
		imx_iomux_v3_setup_multiple_pads(usb_otg_pads,
			ARRAY_SIZE(usb_otg_pads));

		/*
		  * Set daisy chain for otg_pin_id on 6q.
		 *  For 6dl, this bit is reserved.
		 */
		imx_iomux_set_gpr_register(1, 13, 1, 0);
		break;
	case 1:
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return -EINVAL;
	}
	return 0;
}

int board_ehci_power(int port, int on)
{
	switch (port) {
	case 0:
		if (on)
			port_exp_direction_output(USB_OTG_PWR, 1);
		else
			port_exp_direction_output(USB_OTG_PWR, 0);
		break;
	case 1:
		if (on)
			port_exp_direction_output(USB_HOST1_PWR, 1);
		else
			port_exp_direction_output(USB_HOST1_PWR, 0);
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return -EINVAL;
	}

	return 0;
}
#endif
