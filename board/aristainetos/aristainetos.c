// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/video.h>
#include <mmc.h>
#include <fsl_esdhc_imx.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>
#include <input.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <pwm.h>
#include <env.h>
#include <micrel.h>
#include <spi.h>
#include <video.h>
#include <../drivers/video/imx/ipu.h>
#if defined(CONFIG_VIDEO_BMP_LOGO)
	#include <bmp_logo.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED | \
		      PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL	(PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)

#define DISP_PAD_CTRL	(0x10)

#define ECSPI4_CS1		IMX_GPIO_NR(5, 2)

#define USDHC2_PAD_CTRL (PAD_CTL_SPEED_LOW |			\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#if (CONFIG_SYS_BOARD_VERSION == 2)
	/* 4.3 display controller */
	#define ECSPI1_CS0		IMX_GPIO_NR(4, 9)
	#define ECSPI4_CS0		IMX_GPIO_NR(3, 29)
#elif (CONFIG_SYS_BOARD_VERSION == 3)
	#define ECSPI1_CS0		IMX_GPIO_NR(2, 30)   /* NOR flash */
	/* 4.3 display controller */
	#define ECSPI1_CS1		IMX_GPIO_NR(4, 10)
#endif

#define SOFT_RESET_GPIO		IMX_GPIO_NR(7, 13)
#define SD2_DRIVER_ENABLE	IMX_GPIO_NR(7, 8)

struct i2c_pads_info i2c_pad_info3 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_5__I2C3_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO_5__GPIO1_IO05 | PC,
		.gp = IMX_GPIO_NR(1, 5)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_6__I2C3_SDA | PC,
		.gpio_mode = MX6_PAD_GPIO_6__GPIO1_IO06 | PC,
		.gp = IMX_GPIO_NR(1, 6)
	}
};

struct i2c_pads_info i2c_pad_info4 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_7__I2C4_SCL | PC,
		.gpio_mode = MX6_PAD_GPIO_7__GPIO1_IO07 | PC,
		.gp = IMX_GPIO_NR(1, 7)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_8__I2C4_SDA | PC,
		.gpio_mode = MX6_PAD_GPIO_8__GPIO1_IO08 | PC,
		.gp = IMX_GPIO_NR(1, 8)
	}
};

iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D19__UART1_CTS_B    | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D20__UART1_RTS_B    | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_EIM_D26__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D27__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart3_pads[] = {
	MX6_PAD_EIM_D24__UART3_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D25__UART3_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D31__UART3_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_EIM_D23__UART3_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const uart4_pads[] = {
	MX6_PAD_KEY_COL0__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_KEY_ROW0__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t const gpio_pads[] = {
	/* LED enable*/
	MX6_PAD_ENET_CRS_DV__GPIO1_IO25 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED yellow */
	MX6_PAD_NANDF_CS3__GPIO6_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED red */
#if (CONFIG_SYS_BOARD_VERSION == 2)
	MX6_PAD_EIM_EB0__GPIO2_IO28 | MUX_PAD_CTRL(NO_PAD_CTRL),
#elif (CONFIG_SYS_BOARD_VERSION == 3)
	MX6_PAD_EIM_WAIT__GPIO5_IO00 | MUX_PAD_CTRL(NO_PAD_CTRL),
#endif
	/* LED green */
	MX6_PAD_EIM_A24__GPIO5_IO04 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LED blue */
	MX6_PAD_EIM_EB1__GPIO2_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* spi flash WP protect */
	MX6_PAD_SD4_DAT7__GPIO2_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* spi CS 0 */
	MX6_PAD_EIM_D29__GPIO3_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* spi bus #2 SS driver enable */
	MX6_PAD_EIM_A23__GPIO6_IO06 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* RST_LOC# PHY reset input (has pull-down!)*/
	MX6_PAD_GPIO_18__GPIO7_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* SD 2 level shifter output enable */
	MX6_PAD_SD3_RST__GPIO7_IO08 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* SD1 card detect input */
	MX6_PAD_ENET_RXD0__GPIO1_IO27 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* SD1 write protect input */
	MX6_PAD_DI0_PIN4__GPIO4_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* SD2 card detect input */
	MX6_PAD_GPIO_19__GPIO4_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* SD2 write protect input */
	MX6_PAD_SD4_DAT2__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* Touchscreen IRQ */
	MX6_PAD_SD4_DAT1__GPIO2_IO09 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const misc_pads[] = {
	/* USB_OTG_ID = GPIO1_24*/
	MX6_PAD_ENET_RX_ER__USB_OTG_ID		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* H1 Power enable = GPIO1_0*/
	MX6_PAD_GPIO_0__USB_H1_PWR		| MUX_PAD_CTRL(NO_PAD_CTRL),
	/* OTG Power enable = GPIO4_15*/
	MX6_PAD_KEY_ROW4__USB_OTG_PWR		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
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

static iomux_v3_cfg_t const backlight_pads[] = {
	/* backlight PWM brightness control */
	MX6_PAD_GPIO_9__PWM1_OUT | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* backlight enable */
	MX6_PAD_EIM_BCLK__GPIO6_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL),
	/* LCD power enable */
	MX6_PAD_NANDF_CS2__GPIO6_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const ecspi1_pads[] = {
	MX6_PAD_EIM_D16__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D17__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_EIM_D18__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
#if (CONFIG_SYS_BOARD_VERSION == 2)
	MX6_PAD_KEY_ROW1__GPIO4_IO09 | MUX_PAD_CTRL(SPI_PAD_CTRL),
#elif (CONFIG_SYS_BOARD_VERSION == 3)
	MX6_PAD_EIM_EB2__GPIO2_IO30  | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_KEY_COL2__GPIO4_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
#endif
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));
}

#if (CONFIG_SYS_BOARD_VERSION == 2)
iomux_v3_cfg_t const ecspi4_pads[] = {
	MX6_PAD_EIM_D21__ECSPI4_SCLK | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D22__ECSPI4_MISO | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D28__ECSPI4_MOSI | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_A25__GPIO5_IO02  | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_D29__GPIO3_IO29  | MUX_PAD_CTRL(NO_PAD_CTRL),
};
#endif

static iomux_v3_cfg_t const display_pads[] = {
	MX6_PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK | MUX_PAD_CTRL(DISP_PAD_CTRL),
	MX6_PAD_DI0_PIN15__IPU1_DI0_PIN15,
	MX6_PAD_DI0_PIN2__IPU1_DI0_PIN02,
	MX6_PAD_DI0_PIN3__IPU1_DI0_PIN03,
	MX6_PAD_DISP0_DAT0__IPU1_DISP0_DATA00,
	MX6_PAD_DISP0_DAT1__IPU1_DISP0_DATA01,
	MX6_PAD_DISP0_DAT2__IPU1_DISP0_DATA02,
	MX6_PAD_DISP0_DAT3__IPU1_DISP0_DATA03,
	MX6_PAD_DISP0_DAT4__IPU1_DISP0_DATA04,
	MX6_PAD_DISP0_DAT5__IPU1_DISP0_DATA05,
	MX6_PAD_DISP0_DAT6__IPU1_DISP0_DATA06,
	MX6_PAD_DISP0_DAT7__IPU1_DISP0_DATA07,
	MX6_PAD_DISP0_DAT8__IPU1_DISP0_DATA08,
	MX6_PAD_DISP0_DAT9__IPU1_DISP0_DATA09,
	MX6_PAD_DISP0_DAT10__IPU1_DISP0_DATA10,
	MX6_PAD_DISP0_DAT11__IPU1_DISP0_DATA11,
	MX6_PAD_DISP0_DAT12__IPU1_DISP0_DATA12,
	MX6_PAD_DISP0_DAT13__IPU1_DISP0_DATA13,
	MX6_PAD_DISP0_DAT14__IPU1_DISP0_DATA14,
	MX6_PAD_DISP0_DAT15__IPU1_DISP0_DATA15,
	MX6_PAD_DISP0_DAT16__IPU1_DISP0_DATA16,
	MX6_PAD_DISP0_DAT17__IPU1_DISP0_DATA17,
	MX6_PAD_DISP0_DAT18__IPU1_DISP0_DATA18,
	MX6_PAD_DISP0_DAT19__IPU1_DISP0_DATA19,
	MX6_PAD_DISP0_DAT20__IPU1_DISP0_DATA20,
	MX6_PAD_DISP0_DAT21__IPU1_DISP0_DATA21,
	MX6_PAD_DISP0_DAT22__IPU1_DISP0_DATA22,
	MX6_PAD_DISP0_DAT23__IPU1_DISP0_DATA23,
};

int board_spi_cs_gpio(unsigned int bus, unsigned int cs)
{
	if (bus == CONFIG_SF_DEFAULT_BUS && cs == CONFIG_SF_DEFAULT_CS)
#if (CONFIG_SYS_BOARD_VERSION == 2)
		return IMX_GPIO_NR(5, 2);

	if (bus == 0 && cs == 0)
		return IMX_GPIO_NR(4, 9);
#elif (CONFIG_SYS_BOARD_VERSION == 3)
		return ECSPI1_CS0;

	if (bus == 0 && cs == 1)
		return ECSPI1_CS1;
#endif
	return -1;
}

static void setup_spi(void)
{
	int i;

	imx_iomux_v3_setup_multiple_pads(ecspi1_pads, ARRAY_SIZE(ecspi1_pads));

#if (CONFIG_SYS_BOARD_VERSION == 2)
	imx_iomux_v3_setup_multiple_pads(ecspi4_pads, ARRAY_SIZE(ecspi4_pads));
#endif

	for (i = 0; i < 4; i++)
		enable_spi_clk(true, i);

	gpio_direction_output(ECSPI1_CS0, 1);
#if (CONFIG_SYS_BOARD_VERSION == 2)
	gpio_direction_output(ECSPI4_CS1, 0);
	/* set cs0 to high (second device on spi bus #4) */
	gpio_direction_output(ECSPI4_CS0, 1);
#elif (CONFIG_SYS_BOARD_VERSION == 3)
	gpio_direction_output(ECSPI1_CS1, 1);
#endif
}

static void setup_iomux_uart(void)
{
	switch (CONFIG_MXC_UART_BASE) {
	case UART1_BASE:
		imx_iomux_v3_setup_multiple_pads(uart1_pads,
						 ARRAY_SIZE(uart1_pads));
		break;
	case UART2_BASE:
		imx_iomux_v3_setup_multiple_pads(uart2_pads,
						 ARRAY_SIZE(uart2_pads));
		break;
	case UART3_BASE:
		imx_iomux_v3_setup_multiple_pads(uart3_pads,
						 ARRAY_SIZE(uart3_pads));
		break;
	case UART4_BASE:
		imx_iomux_v3_setup_multiple_pads(uart4_pads,
						 ARRAY_SIZE(uart4_pads));
		break;
	}
}

int board_phy_config(struct phy_device *phydev)
{
	/* control data pad skew - devaddr = 0x02, register = 0x04 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* rx data pad skew - devaddr = 0x02, register = 0x05 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* tx data pad skew - devaddr = 0x02, register = 0x06 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x0000);
	/* gtx and rx clock pad skew - devaddr = 0x02, register = 0x08 */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC, 0x03FF);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();
	return cpu_eth_init(bis);
}

static int rotate_logo_one(unsigned char *out, unsigned char *in)
{
	int   i, j;

	for (i = 0; i < BMP_LOGO_WIDTH; i++)
		for (j = 0; j < BMP_LOGO_HEIGHT; j++)
			out[j * BMP_LOGO_WIDTH + BMP_LOGO_HEIGHT - 1 - i] =
			in[i * BMP_LOGO_WIDTH + j];
	return 0;
}

/*
 * Rotate the BMP_LOGO (only)
 * Will only work, if the logo is square, as
 * BMP_LOGO_HEIGHT and BMP_LOGO_WIDTH are defines, not variables
 */
void rotate_logo(int rotations)
{
	unsigned char out_logo[BMP_LOGO_WIDTH * BMP_LOGO_HEIGHT];
	unsigned char *in_logo;
	int   i, j;

	if (BMP_LOGO_WIDTH != BMP_LOGO_HEIGHT)
		return;

	in_logo = bmp_logo_bitmap;

	/* one 90 degree rotation */
	if (rotations == 1  ||  rotations == 2  ||  rotations == 3)
		rotate_logo_one(out_logo, in_logo);

	/* second 90 degree rotation */
	if (rotations == 2  ||  rotations == 3)
		rotate_logo_one(in_logo, out_logo);

	/* third 90 degree rotation */
	if (rotations == 3)
		rotate_logo_one(out_logo, in_logo);

	/* copy result back to original array */
	if (rotations == 1  ||  rotations == 3)
		for (i = 0; i < BMP_LOGO_WIDTH; i++)
			for (j = 0; j < BMP_LOGO_HEIGHT; j++)
				in_logo[i * BMP_LOGO_WIDTH + j] =
				out_logo[i * BMP_LOGO_WIDTH + j];
}

static void enable_display_power(void)
{
	imx_iomux_v3_setup_multiple_pads(backlight_pads,
					 ARRAY_SIZE(backlight_pads));

	/* backlight enable */
	gpio_direction_output(IMX_GPIO_NR(6, 31), 1);
	/* LCD power enable */
	gpio_direction_output(IMX_GPIO_NR(6, 15), 1);

	/* enable backlight PWM 1 */
	if (pwm_init(0, 0, 0))
		goto error;
	/* duty cycle 500ns, period: 3000ns */
	if (pwm_config(0, 50000, 300000))
		goto error;
	if (pwm_enable(0))
		goto error;
	return;

error:
	puts("error init pwm for backlight\n");
}

static void enable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;
	s32 timeout = 100000;

	/* set PLL5 clock */
	reg = readl(&ccm->analog_pll_video);
	reg |= BM_ANADIG_PLL_VIDEO_POWERDOWN;
	writel(reg, &ccm->analog_pll_video);

	/* set PLL5 to 232720000Hz */
	reg &= ~BM_ANADIG_PLL_VIDEO_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_DIV_SELECT(0x26);
	reg &= ~BM_ANADIG_PLL_VIDEO_POST_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(0);
	writel(reg, &ccm->analog_pll_video);

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(0xC0238),
	       &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(0xF4240),
	       &ccm->analog_pll_video_denom);

	reg &= ~BM_ANADIG_PLL_VIDEO_POWERDOWN;
	writel(reg, &ccm->analog_pll_video);

	while (timeout--)
		if (readl(&ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK)
			break;
	if (timeout < 0)
		printf("Warning: video pll lock timeout!\n");

	reg = readl(&ccm->analog_pll_video);
	reg |= BM_ANADIG_PLL_VIDEO_ENABLE;
	reg &= ~BM_ANADIG_PLL_VIDEO_BYPASS;
	writel(reg, &ccm->analog_pll_video);

	/* set LDB0, LDB1 clk select to 000/000 (PLL5 clock) */
	reg = readl(&ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 | MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (0 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
		| (0 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &ccm->cs2cdr);

	reg = readl(&ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &ccm->cscmr2);

	reg = readl(&ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	writel(reg, &ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	      | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
	      | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_HIGH
	      | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	      | IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT
	      | IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	      | IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~IOMUXC_GPR3_LVDS0_MUX_CTL_MASK)
	       | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
		  << IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}

static void enable_spi_display(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;
	s32 timeout = 100000;

#if defined(CONFIG_VIDEO_BMP_LOGO)
	rotate_logo(3);  /* portrait display in landscape mode */
#endif

	/*
	 * set ldb clock to 28341000 Hz calculated through the formula:
	 * (XRES + LEFT_M + RIGHT_M + HSYNC_LEN) *
	 * (YRES + UPPER_M + LOWER_M + VSYNC_LEN) * REFRESH)
	 * see:
	 * https://community.freescale.com/thread/308170
	 */
	ipu_set_ldb_clock(28341000);

	reg = readl(&ccm->cs2cdr);

	/* select pll 5 clock */
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		| MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	writel(reg, &ccm->cs2cdr);

	/* set PLL5 to 197994996Hz */
	reg &= ~BM_ANADIG_PLL_VIDEO_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_DIV_SELECT(0x21);
	reg &= ~BM_ANADIG_PLL_VIDEO_POST_DIV_SELECT;
	reg |= BF_ANADIG_PLL_VIDEO_POST_DIV_SELECT(0);
	writel(reg, &ccm->analog_pll_video);

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(0xfbf4),
	       &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(0xf4240),
	       &ccm->analog_pll_video_denom);

	reg &= ~BM_ANADIG_PLL_VIDEO_POWERDOWN;
	writel(reg, &ccm->analog_pll_video);

	while (timeout--)
		if (readl(&ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK)
			break;
	if (timeout < 0)
		printf("Warning: video pll lock timeout!\n");

	reg = readl(&ccm->analog_pll_video);
	reg |= BM_ANADIG_PLL_VIDEO_ENABLE;
	reg &= ~BM_ANADIG_PLL_VIDEO_BYPASS;
	writel(reg, &ccm->analog_pll_video);

	/* set LDB0, LDB1 clk select to 000/000 (PLL5 clock) */
	reg = readl(&ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 | MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (0 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
		| (0 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &ccm->cs2cdr);

	reg = readl(&ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV;
	writel(reg, &ccm->cscmr2);

	reg = readl(&ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	reg &= ~MXC_CCM_CHSCCDR_IPU1_DI0_PODF_MASK;
	reg |= (2 << MXC_CCM_CHSCCDR_IPU1_DI0_PODF_OFFSET);
	reg &= ~MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_MASK;
	reg |= (2 << MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_OFFSET);
	writel(reg, &ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	      | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH
	      | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_HIGH
	      | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	      | IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT
	      | IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED
	      | IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~IOMUXC_GPR3_LVDS0_MUX_CTL_MASK)
	       | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
		  << IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);

	imx_iomux_v3_setup_multiple_pads(display_pads,
					 ARRAY_SIZE(display_pads));
}

static void setup_display(void)
{
	enable_ipu_clock();
	enable_display_power();
}

static void setup_iomux_gpio(void)
{
	imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));
}

static void set_gpr_register(void)
{
	struct iomuxc *iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;

	writel(IOMUXC_GPR1_APP_CLK_REQ_N | IOMUXC_GPR1_PCIE_RDY_L23 |
	       IOMUXC_GPR1_EXC_MON_SLVE |
	       (2 << IOMUXC_GPR1_ADDRS0_OFFSET) |
	       IOMUXC_GPR1_ACT_CS0,
	       &iomuxc_regs->gpr[1]);
	writel(0x0, &iomuxc_regs->gpr[8]);
	writel(IOMUXC_GPR12_ARMP_IPG_CLK_EN | IOMUXC_GPR12_ARMP_AHB_CLK_EN |
	       IOMUXC_GPR12_ARMP_ATB_CLK_EN | IOMUXC_GPR12_ARMP_APB_CLK_EN,
	       &iomuxc_regs->gpr[12]);
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_gpio();

	gpio_direction_output(SOFT_RESET_GPIO, 1);
	gpio_direction_output(SD2_DRIVER_ENABLE, 1);
	setup_display();
	set_gpr_register();
	return 0;
}

static void setup_i2c4(void)
{
	setup_i2c(3, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
		  &i2c_pad_info4);
}

static void setup_board_gpio(void)
{
	/* enable all LEDs */
	gpio_request(IMX_GPIO_NR(2, 13), "LED ena"); /* 25 */
	gpio_direction_output(IMX_GPIO_NR(1, 25), 0);

	/* switch off Status LEDs */
#if (CONFIG_SYS_BOARD_VERSION == 2)
	gpio_request(IMX_GPIO_NR(6, 16), "LED yellow"); /* 176 */
	gpio_direction_output(IMX_GPIO_NR(6, 16), 1);
	gpio_request(IMX_GPIO_NR(2, 28), "LED red"); /* 60 */
	gpio_direction_output(IMX_GPIO_NR(2, 28), 1);
	gpio_request(IMX_GPIO_NR(5, 4), "LED green"); /* 132 */
	gpio_direction_output(IMX_GPIO_NR(5, 4), 1);
	gpio_request(IMX_GPIO_NR(2, 29), "LED blue"); /* 61 */
	gpio_direction_output(IMX_GPIO_NR(2, 29), 1);
#elif (CONFIG_SYS_BOARD_VERSION == 3)
	gpio_request(IMX_GPIO_NR(6, 16), "LED yellow"); /* 176 */
	gpio_direction_output(IMX_GPIO_NR(6, 16), 0);
	gpio_request(IMX_GPIO_NR(5, 0), "LED red"); /* 128 */
	gpio_direction_output(IMX_GPIO_NR(5, 0), 0);
	gpio_request(IMX_GPIO_NR(5, 4), "LED green"); /* 132 */
	gpio_direction_output(IMX_GPIO_NR(5, 4), 0);
	gpio_request(IMX_GPIO_NR(2, 29), "LED blue"); /* 61 */
	gpio_direction_output(IMX_GPIO_NR(2, 29), 0);
#endif
}

static void setup_board_spi(void)
{
	/* enable spi bus #2 SS drivers (and spi bus #4 SS1 for rev2b) */
	gpio_direction_output(IMX_GPIO_NR(6, 6), 1);
}

int board_late_init(void)
{
	char *my_bootdelay;
	char bootmode = 0;
	char const *panel = env_get("panel");

	/*
	 * Check the boot-source. If booting from NOR Flash,
	 * disable bootdelay
	 */
	gpio_request(IMX_GPIO_NR(7, 6), "bootsel0");
	gpio_direction_input(IMX_GPIO_NR(7, 6));
	gpio_request(IMX_GPIO_NR(7, 7), "bootsel1");
	gpio_direction_input(IMX_GPIO_NR(7, 7));
	gpio_request(IMX_GPIO_NR(7, 1), "bootsel2");
	gpio_direction_input(IMX_GPIO_NR(7, 1));
	bootmode |= (gpio_get_value(IMX_GPIO_NR(7, 6)) ? 1 : 0) << 0;
	bootmode |= (gpio_get_value(IMX_GPIO_NR(7, 7)) ? 1 : 0) << 1;
	bootmode |= (gpio_get_value(IMX_GPIO_NR(7, 1)) ? 1 : 0) << 2;

	if (bootmode == 7) {
		my_bootdelay = env_get("nor_bootdelay");
		if (my_bootdelay != NULL)
			env_set("bootdelay", my_bootdelay);
		else
			env_set("bootdelay", "-2");
	}

	/* if we have the lg panel, we can initialze it now */
	if (panel)
		if (!strcmp(panel, displays[1].mode.name))
			lg4573_spi_startup(CONFIG_LG4573_BUS,
					   CONFIG_LG4573_CS,
					   10000000, SPI_MODE_0);

	return 0;
}

struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_CSI0_DAT9__I2C1_SCL | PC,
		.gpio_mode = MX6_PAD_CSI0_DAT9__GPIO5_IO27 | PC,
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = MX6_PAD_CSI0_DAT8__I2C1_SDA | PC,
		.gpio_mode = MX6_PAD_CSI0_DAT8__GPIO5_IO26 | PC,
		.gp = IMX_GPIO_NR(5, 26)
	}
};

struct i2c_pads_info i2c_pad_info2 = {
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

iomux_v3_cfg_t const usdhc1_pads[] = {
	MX6_PAD_SD1_CLK__SD1_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__SD1_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT0__SD1_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT1__SD1_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT2__SD1_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DAT3__SD1_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

#ifdef CONFIG_FSL_ESDHC_IMX
struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC1_BASE_ADDR},
	{USDHC2_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	return 1;
}

int board_mmc_init(bd_t *bis)
{
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	imx_iomux_v3_setup_multiple_pads(usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
#if (CONFIG_SYS_BOARD_VERSION == 2)
	/*
	 * usdhc2 has a levelshifter on the carrier board Rev. DV1,
	 * that will automatically detect the driving direction.
	 * During initialisation this isn't working correctly,
	 * which causes DAT3 to be driven low towards the SD-card.
	 * This causes a SD-card enetring the SPI-Mode
	 * and therefore getting inaccessible until next power cycle.
	 * As workaround we drive the DAT3 line as GPIO and set it high.
	 * This makes usdhc2 unusable in u-boot, but works for the
	 * initialisation in Linux
	 */
	imx_iomux_v3_setup_pad(MX6_PAD_SD2_DAT3__GPIO1_IO12 |
			       MUX_PAD_CTRL(NO_PAD_CTRL));
	gpio_direction_output(IMX_GPIO_NR(1, 12) , 1);
#endif
	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
}
#endif

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

struct display_info_t const displays[] = {
	{
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= enable_lvds,
		.mode	= {
			.name           = "lb07wv8",
			.refresh        = 60,
			.xres           = 800,
			.yres           = 480,
			.pixclock       = 30066,
			.left_margin    = 88,
			.right_margin   = 88,
			.upper_margin   = 20,
			.lower_margin   = 20,
			.hsync_len      = 80,
			.vsync_len      = 5,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	}
#if ((CONFIG_SYS_BOARD_VERSION == 2) || (CONFIG_SYS_BOARD_VERSION == 3))
	, {
		.bus	= -1,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= enable_spi_display,
		.mode	= {
			.name           = "lg4573",
			.refresh        = 57,
			.xres           = 480,
			.yres           = 800,
			.pixclock       = 37037,
			.left_margin    = 59,
			.right_margin   = 10,
			.upper_margin   = 15,
			.lower_margin   = 15,
			.hsync_len      = 10,
			.vsync_len      = 15,
			.sync           = FB_SYNC_EXT | FB_SYNC_HOR_HIGH_ACT |
					  FB_SYNC_VERT_HIGH_ACT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	}
#endif
};
size_t display_count = ARRAY_SIZE(displays);

/* no console on this board */
int board_cfb_skip(void)
{
	return 1;
}

iomux_v3_cfg_t nfc_pads[] = {
	MX6_PAD_NANDF_CLE__NAND_CLE		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_ALE__NAND_ALE		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_WP_B__NAND_WP_B	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_RB0__NAND_READY_B	| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_CS0__NAND_CE0_B		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_CMD__NAND_RE_B		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_CLK__NAND_WE_B		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D0__NAND_DATA00		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D1__NAND_DATA01		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D2__NAND_DATA02		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D3__NAND_DATA03		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D4__NAND_DATA04		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D5__NAND_DATA05		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D6__NAND_DATA06		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_D7__NAND_DATA07		| MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_DAT0__NAND_DQS		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* config gpmi nand iomux */
	imx_iomux_v3_setup_multiple_pads(nfc_pads,
					 ARRAY_SIZE(nfc_pads));

	/* gate ENFC_CLK_ROOT clock first,before clk source switch */
	clrbits_le32(&mxc_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

	/* config gpmi and bch clock to 100 MHz */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL_MASK,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL(3));

	/* enable ENFC_CLK_ROOT clock */
	setbits_le32(&mxc_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

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

int board_init(void)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_spi();

	setup_i2c(0, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
		  &i2c_pad_info1);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
		  &i2c_pad_info2);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE,
		  &i2c_pad_info3);
	setup_i2c4();

	/* SPI NOR Flash read only */
	gpio_request(CONFIG_GPIO_ENABLE_SPI_FLASH, "ena_spi_nor");
	gpio_direction_output(CONFIG_GPIO_ENABLE_SPI_FLASH, 0);
	gpio_free(CONFIG_GPIO_ENABLE_SPI_FLASH);

	setup_board_gpio();
	setup_gpmi_nand();
	setup_board_spi();

	/* GPIO_1 for USB_OTG_ID */
	clrsetbits_le32(&iomux->gpr[1], IOMUXC_GPR1_USB_OTG_ID_SEL_MASK, 0);
	imx_iomux_v3_setup_multiple_pads(misc_pads, ARRAY_SIZE(misc_pads));
	return 0;
}

int checkboard(void)
{
	printf("Board: %s\n", CONFIG_BOARDNAME);
	return 0;
}

#ifdef CONFIG_USB_EHCI_MX6
int board_ehci_hcd_init(int port)
{
	int ret;

	ret = gpio_request(ARISTAINETOS_USB_H1_PWR, "usb-h1-pwr");
	if (!ret)
		gpio_direction_output(ARISTAINETOS_USB_H1_PWR, 1);
	ret = gpio_request(ARISTAINETOS_USB_OTG_PWR, "usb-OTG-pwr");
	if (!ret)
		gpio_direction_output(ARISTAINETOS_USB_OTG_PWR, 1);
	return 0;
}

int board_ehci_power(int port, int on)
{
	if (port)
		gpio_set_value(ARISTAINETOS_USB_OTG_PWR, on);
	else
		gpio_set_value(ARISTAINETOS_USB_H1_PWR, on);
	return 0;
}
#endif
