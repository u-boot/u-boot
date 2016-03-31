/*
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 *
 * Author: Ye Li <ye.li@nxp.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/io.h>
#include <asm/imx-common/mxc_i2c.h>
#include <linux/sizes.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <i2c.h>
#include <miiphy.h>
#include <netdev.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include "../common/pfuze.h"
#include <usb.h>
#include <usb/ehci-ci.h>
#include <pca953x.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define I2C_PAD_CTRL    (PAD_CTL_PKE | PAD_CTL_PUE |            \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_SPEED_HIGH   |                                   \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST)

#define ENET_CLK_PAD_CTRL  (PAD_CTL_SPEED_MED | \
	PAD_CTL_DSE_120ohm   | PAD_CTL_SRE_FAST)

#define ENET_RX_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |          \
	PAD_CTL_SPEED_HIGH   | PAD_CTL_SRE_FAST)

#define I2C_PMIC	1

#define GPMI_PAD_CTRL0 (PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_100K_UP)
#define GPMI_PAD_CTRL1 (PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED | \
			PAD_CTL_SRE_FAST)
#define GPMI_PAD_CTRL2 (GPMI_PAD_CTRL0 | GPMI_PAD_CTRL1)

/*Define for building port exp gpio, pin starts from 0*/
#define PORTEXP_IO_NR(chip, pin) \
	((chip << 5) + pin)

/*Get the chip addr from a ioexp gpio*/
#define PORTEXP_IO_TO_CHIP(gpio_nr) \
	(gpio_nr >> 5)

/*Get the pin number from a ioexp gpio*/
#define PORTEXP_IO_TO_PIN(gpio_nr) \
	(gpio_nr & 0x1f)

#define CPU_PER_RST_B	PORTEXP_IO_NR(0x30, 4)
#define STEER_ENET		PORTEXP_IO_NR(0x32, 2)

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

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_SIZE;

	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_GPIO1_IO04__UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_GPIO1_IO05__UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__USDHC3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__USDHC3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DATA0__USDHC3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DATA1__USDHC3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DATA2__USDHC3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DATA3__USDHC3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DATA4__USDHC3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DATA5__USDHC3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DATA6__USDHC3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DATA7__USDHC3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	/* CD pin */
	MX6_PAD_USB_H_DATA__GPIO7_IO_10 | MUX_PAD_CTRL(NO_PAD_CTRL),

	/* RST_B, used for power reset cycle */
	MX6_PAD_KEY_COL1__GPIO2_IO_11 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc4_pads[] = {
	MX6_PAD_SD4_CLK__USDHC4_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_CMD__USDHC4_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA0__USDHC4_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA1__USDHC4_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA2__USDHC4_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA3__USDHC4_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA4__USDHC4_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA5__USDHC4_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA6__USDHC4_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DATA7__USDHC4_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),

	/* CD pin */
	MX6_PAD_USB_H_STROBE__GPIO7_IO_11 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const fec2_pads[] = {
	MX6_PAD_ENET1_MDC__ENET2_MDC | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_MDIO__ENET2_MDIO | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII2_RX_CTL__ENET2_RX_EN | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII2_RD0__ENET2_RX_DATA_0 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII2_RD1__ENET2_RX_DATA_1 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII2_RD2__ENET2_RX_DATA_2 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII2_RD3__ENET2_RX_DATA_3 | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII2_RXC__ENET2_RX_CLK | MUX_PAD_CTRL(ENET_RX_PAD_CTRL),
	MX6_PAD_RGMII2_TX_CTL__ENET2_TX_EN | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII2_TD0__ENET2_TX_DATA_0 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII2_TD1__ENET2_TX_DATA_1 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII2_TD2__ENET2_TX_DATA_2 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII2_TD3__ENET2_TX_DATA_3 | MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII2_TXC__ENET2_RGMII_TXC | MUX_PAD_CTRL(ENET_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

static int setup_fec(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* Use 125MHz anatop loopback REF_CLK1 for ENET2 */
	clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC2_MASK, 0);

	return enable_fec_anatop_clock(1, ENET_125MHZ);
}

int board_eth_init(bd_t *bis)
{
	int ret;

	imx_iomux_v3_setup_multiple_pads(fec2_pads, ARRAY_SIZE(fec2_pads));
	setup_fec();

	ret = fecmxc_initialize_multi(bis, 1,
		CONFIG_FEC_MXC_PHYADDR, IMX_FEC_BASE);
	if (ret)
		printf("FEC%d MXC: %s:failed\n", 1, __func__);

	return ret;
}

int board_phy_config(struct phy_device *phydev)
{
	/*
	 * Enable 1.8V(SEL_1P5_1P8_POS_REG) on
	 * Phy control debug reg 0
	 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

	/* rgmii tx clock delay enable */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
/* I2C2 for PMIC */
struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO1_IO02__I2C2_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO1_IO02__GPIO1_IO_2 | PC,
		.gp = IMX_GPIO_NR(1, 2),
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO1_IO03__I2C2_SDA | PC,
		.gpio_mode = MX6_PAD_GPIO1_IO03__GPIO1_IO_3 | PC,
		.gp = IMX_GPIO_NR(1, 3),
	},
};

/* I2C3 for IO Expander */
struct i2c_pads_info i2c_pad_info3 = {
	.scl = {
		.i2c_mode = MX6_PAD_KEY_COL4__I2C3_SCL | PC,
		.gpio_mode = MX6_PAD_KEY_COL4__GPIO2_IO_14 | PC,
		.gp = IMX_GPIO_NR(2, 14),
	},
	.sda = {
		.i2c_mode = MX6_PAD_KEY_ROW4__I2C3_SDA | PC,
		.gpio_mode = MX6_PAD_KEY_ROW4__GPIO2_IO_19 | PC,
		.gp = IMX_GPIO_NR(2, 19),
	},
};

int power_init_board(void)
{
	struct pmic *p;

	p = pfuze_common_init(I2C_PMIC);
	if (!p)
		return -ENODEV;

	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		(1 << 9)

static iomux_v3_cfg_t const usb_otg_pads[] = {
	/* OGT1 */
	MX6_PAD_GPIO1_IO09__USB_OTG1_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_GPIO1_IO10__ANATOP_OTG1_ID | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* OTG2 */
	MX6_PAD_GPIO1_IO12__USB_OTG2_PWR | MUX_PAD_CTRL(NO_PAD_CTRL)
};

static void setup_usb(void)
{
	imx_iomux_v3_setup_multiple_pads(usb_otg_pads,
					 ARRAY_SIZE(usb_otg_pads));
}

int board_usb_phy_mode(int port)
{
	if (port == 1)
		return USB_INIT_HOST;
	else
		return usb_phy_mode(port);
}

int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	if (port > 1)
		return -EINVAL;

	usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
				 port * 4);

	/* Set Power polarity */
	setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

	return 0;
}
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

static struct fsl_esdhc_cfg usdhc_cfg[3] = {
	{USDHC3_BASE_ADDR},
	{USDHC4_BASE_ADDR},
};

#define USDHC3_CD_GPIO	IMX_GPIO_NR(7, 10)
#define USDHC3_RST_GPIO	IMX_GPIO_NR(2, 11)
#define USDHC4_CD_GPIO	IMX_GPIO_NR(7, 11)

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC3_BASE_ADDR:
		ret = !gpio_get_value(USDHC3_CD_GPIO);
		break;
	case USDHC4_BASE_ADDR:
		ret = !gpio_get_value(USDHC4_CD_GPIO);
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	int i, ret;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC3
	 * mmc1                    USDHC4
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(
				usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
			gpio_direction_input(USDHC3_CD_GPIO);

			/* This starts a power cycle for UHS-I. Need to set steer to B0 to A*/
			gpio_direction_output(USDHC3_RST_GPIO, 0);
			udelay(1000); /* need 1ms at least */
			gpio_direction_output(USDHC3_RST_GPIO, 1);

			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			break;
		case 1:
			imx_iomux_v3_setup_multiple_pads(
				usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
			gpio_direction_input(USDHC4_CD_GPIO);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) than supported by the board\n", i + 1);
			return -EINVAL;
			}

			ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
			if (ret) {
				printf("Warning: failed to initialize mmc dev %d\n", i);
				return ret;
			}
	}

	return 0;
}

#ifdef CONFIG_FSL_QSPI

#define QSPI_PAD_CTRL1	\
	(PAD_CTL_SRE_FAST | PAD_CTL_SPEED_HIGH | \
	 PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_47K_UP | PAD_CTL_DSE_40ohm)

static iomux_v3_cfg_t const quadspi_pads[] = {
	MX6_PAD_QSPI1A_SS0_B__QSPI1_A_SS0_B   | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1A_SCLK__QSPI1_A_SCLK     | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1A_DATA0__QSPI1_A_DATA_0  | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1A_DATA1__QSPI1_A_DATA_1  | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1A_DATA2__QSPI1_A_DATA_2  | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1A_DATA3__QSPI1_A_DATA_3  | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1B_SS0_B__QSPI1_B_SS0_B   | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1B_SCLK__QSPI1_B_SCLK     | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1B_DATA0__QSPI1_B_DATA_0  | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1B_DATA1__QSPI1_B_DATA_1  | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1B_DATA2__QSPI1_B_DATA_2  | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
	MX6_PAD_QSPI1B_DATA3__QSPI1_B_DATA_3  | MUX_PAD_CTRL(QSPI_PAD_CTRL1),
};

int board_qspi_init(void)
{
	/* Set the iomux */
	imx_iomux_v3_setup_multiple_pads(quadspi_pads,
					 ARRAY_SIZE(quadspi_pads));

	/* Set the clock */
	enable_qspi_clk(0);

	return 0;
}
#endif

#ifdef CONFIG_NAND_MXS
iomux_v3_cfg_t gpmi_pads[] = {
	MX6_PAD_NAND_CLE__RAWNAND_CLE		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_ALE__RAWNAND_ALE		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_WP_B__RAWNAND_WP_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_READY_B__RAWNAND_READY_B	| MUX_PAD_CTRL(GPMI_PAD_CTRL0),
	MX6_PAD_NAND_CE0_B__RAWNAND_CE0_B		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_RE_B__RAWNAND_RE_B		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_WE_B__RAWNAND_WE_B		| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA00__RAWNAND_DATA00	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA01__RAWNAND_DATA01	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA02__RAWNAND_DATA02	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA03__RAWNAND_DATA03	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA04__RAWNAND_DATA04	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA05__RAWNAND_DATA05	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA06__RAWNAND_DATA06	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
	MX6_PAD_NAND_DATA07__RAWNAND_DATA07	| MUX_PAD_CTRL(GPMI_PAD_CTRL2),
};

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	imx_iomux_v3_setup_multiple_pads(gpmi_pads, ARRAY_SIZE(gpmi_pads));

	setup_gpmi_io_clk((MXC_CCM_CS2CDR_QSPI2_CLK_PODF(0) |
			MXC_CCM_CS2CDR_QSPI2_CLK_PRED(3) |
			MXC_CCM_CS2CDR_QSPI2_CLK_SEL(3)));

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#endif

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_SYS_I2C_MXC
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info3);
#endif

	/* Reset CPU_PER_RST_B signal for enet phy and PCIE */
	port_exp_direction_output(CPU_PER_RST_B, 0);
	udelay(500);
	port_exp_direction_output(CPU_PER_RST_B, 1);

	/* Set steering signal to L for selecting B0 */
	port_exp_direction_output(STEER_ENET, 0);

#ifdef CONFIG_USB_EHCI_MX6
	setup_usb();
#endif

#ifdef CONFIG_FSL_QSPI
	board_qspi_init();
#endif

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	{"sda", MAKE_CFGVAL(0x42, 0x30, 0x00, 0x00)},
	{"sdb", MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
	{"qspi1", MAKE_CFGVAL(0x10, 0x00, 0x00, 0x00)},
	{"nand", MAKE_CFGVAL(0x82, 0x00, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

	return 0;
}

int checkboard(void)
{
	puts("Board: MX6SX SABRE AUTO\n");

	return 0;
}
