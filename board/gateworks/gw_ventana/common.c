/*
 * Copyright (C) 2013 Gateworks Corporation
 *
 * Author: Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/imx-common/mxc_i2c.h>
#include <hwconfig.h>
#include <power/pmic.h>
#include <power/ltc3676_pmic.h>
#include <power/pfuze100_pmic.h>

#include "common.h"

/* UART1: Function varies per baseboard */
static iomux_v3_cfg_t const uart1_pads[] = {
	IOMUX_PADS(PAD_SD3_DAT6__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

/* UART2: Serial Console */
static iomux_v3_cfg_t const uart2_pads[] = {
	IOMUX_PADS(PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
	SETUP_IOMUX_PADS(uart2_pads);
}

/* I2C1: GSC */
static struct i2c_pads_info mx6q_i2c_pad_info0 = {
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
static struct i2c_pads_info mx6dl_i2c_pad_info0 = {
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
static struct i2c_pads_info mx6q_i2c_pad_info1 = {
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
static struct i2c_pads_info mx6dl_i2c_pad_info1 = {
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
static struct i2c_pads_info mx6q_i2c_pad_info2 = {
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
static struct i2c_pads_info mx6dl_i2c_pad_info2 = {
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

void setup_ventana_i2c(void)
{
	if (is_cpu_type(MXC_CPU_MX6Q)) {
		setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info0);
		setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info1);
		setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info2);
	} else {
		setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c_pad_info0);
		setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c_pad_info1);
		setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c_pad_info2);
	}
}

/*
 * Baseboard specific GPIO
 */

/* common to add baseboards */
static iomux_v3_cfg_t const gw_gpio_pads[] = {
	/* SD3_VSELECT */
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14 | DIO_PAD_CFG),
};

/* prototype */
static iomux_v3_cfg_t const gwproto_gpio_pads[] = {
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
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
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
	/* MSATA_EN */
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | DIO_PAD_CFG),
	/* PANLEDG# */
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | DIO_PAD_CFG),
	/* PANLEDR# */
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* CAN_STBY */
	IOMUX_PADS(PAD_GPIO_9__GPIO1_IO09 | DIO_PAD_CFG),
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
	/* PCI_RST# (GW522x) */
	IOMUX_PADS(PAD_EIM_D23__GPIO3_IO23 | DIO_PAD_CFG),
	/* RS485_EN */
	IOMUX_PADS(PAD_SD3_DAT4__GPIO7_IO01 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw53xx_gpio_pads[] = {
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
	/* MSATA_EN */
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | DIO_PAD_CFG),
	/* CAN_STBY */
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02 | DIO_PAD_CFG),
	/* USB_HUBRST# */
	IOMUX_PADS(PAD_GPIO_9__GPIO1_IO09 | DIO_PAD_CFG),
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
	/* RS485_EN */
	IOMUX_PADS(PAD_SD3_DAT4__GPIO7_IO01 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw54xx_gpio_pads[] = {
	/* RS232_EN# */
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 | DIO_PAD_CFG),
	/* MSATA_EN */
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | DIO_PAD_CFG),
	/* CAN_STBY */
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02 | DIO_PAD_CFG),
	/* PANLEDG# */
	IOMUX_PADS(PAD_KEY_COL0__GPIO4_IO06 | DIO_PAD_CFG),
	/* PANLEDR# */
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07 | DIO_PAD_CFG),
	/* MX6_LOCLED# */
	IOMUX_PADS(PAD_KEY_ROW4__GPIO4_IO15 | DIO_PAD_CFG),
	/* USB_HUBRST# */
	IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16 | DIO_PAD_CFG),
	/* MIPI_DIO */
	IOMUX_PADS(PAD_SD1_DAT3__GPIO1_IO21 | DIO_PAD_CFG),
	/* RS485_EN */
	IOMUX_PADS(PAD_EIM_D24__GPIO3_IO24 | DIO_PAD_CFG),
	/* IOEXP_PWREN# */
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19 | DIO_PAD_CFG),
	/* IOEXP_IRQ# */
	IOMUX_PADS(PAD_EIM_A20__GPIO2_IO18 | MUX_PAD_CTRL(IRQ_PAD_CTRL)),
	/* DIOI2C_DIS# */
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05 | DIO_PAD_CFG),
	/* PCI_RST# */
	IOMUX_PADS(PAD_ENET_TXD1__GPIO1_IO29 | DIO_PAD_CFG),
	/* VID_EN */
	IOMUX_PADS(PAD_EIM_D31__GPIO3_IO31 | DIO_PAD_CFG),
	/* RS485_EN */
	IOMUX_PADS(PAD_SD3_DAT4__GPIO7_IO01 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_DISP0_DAT23__GPIO5_IO17 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw551x_gpio_pads[] = {
	/* CAN_STBY */
	IOMUX_PADS(PAD_GPIO_9__GPIO1_IO09 | DIO_PAD_CFG),
	/* PANLED# */
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07 | DIO_PAD_CFG),
	/* PCI_RST# */
	IOMUX_PADS(PAD_GPIO_0__GPIO1_IO00 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

static iomux_v3_cfg_t const gw552x_gpio_pads[] = {
	/* MSATA_EN */
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 | DIO_PAD_CFG),
	/* USBOTG_SEL */
	IOMUX_PADS(PAD_GPIO_7__GPIO1_IO07 | DIO_PAD_CFG),
	/* USB_HUBRST# */
	IOMUX_PADS(PAD_GPIO_9__GPIO1_IO09 | DIO_PAD_CFG),
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

static iomux_v3_cfg_t const gw553x_gpio_pads[] = {
	/* PANLEDG# */
	IOMUX_PADS(PAD_KEY_COL2__GPIO4_IO10 | DIO_PAD_CFG),
	/* PANLEDR# */
	IOMUX_PADS(PAD_KEY_ROW2__GPIO4_IO11 | DIO_PAD_CFG),

	/* VID_PWR */
	IOMUX_PADS(PAD_CSI0_DATA_EN__GPIO5_IO20 | DIO_PAD_CFG),
	/* PCI_RST# */
	IOMUX_PADS(PAD_GPIO_0__GPIO1_IO00 | DIO_PAD_CFG),
	/* PCIESKT_WDIS# */
	IOMUX_PADS(PAD_GPIO_17__GPIO7_IO12 | DIO_PAD_CFG),
};

/* Digital I/O */
struct dio_cfg gw51xx_dio[] = {
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
};

struct dio_cfg gw52xx_dio[] = {
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
};

struct dio_cfg gw53xx_dio[] = {
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
};

struct dio_cfg gw54xx_dio[] = {
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
};

struct dio_cfg gw551x_dio[] = {
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
};

struct dio_cfg gw552x_dio[] = {
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
	{
		{IOMUX_PADS(PAD_CSI0_PIXCLK__GPIO5_IO18) },
		IMX_GPIO_NR(5, 18),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_DATA_EN__GPIO5_IO20) },
		IMX_GPIO_NR(5, 20),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_VSYNC__GPIO5_IO21) },
		IMX_GPIO_NR(5, 21),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_DAT4__GPIO5_IO22) },
		IMX_GPIO_NR(5, 22),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_DAT5__GPIO5_IO23) },
		IMX_GPIO_NR(5, 23),
		{ 0, 0 },
		0
	},
	{
		{IOMUX_PADS(PAD_CSI0_DAT7__GPIO5_IO25) },
		IMX_GPIO_NR(5, 25),
		{ 0, 0 },
		0
	},
};

struct dio_cfg gw553x_dio[] = {
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
};

/*
 * Board Specific GPIO
 */
struct ventana gpio_cfg[GW_UNKNOWN] = {
	/* GW5400proto */
	{
		.gpio_pads = gw54xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw54xx_gpio_pads)/2,
		.dio_cfg = gw54xx_dio,
		.dio_num = ARRAY_SIZE(gw54xx_dio),
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
		.dio_cfg = gw51xx_dio,
		.dio_num = ARRAY_SIZE(gw51xx_dio),
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
		.dio_cfg = gw52xx_dio,
		.dio_num = ARRAY_SIZE(gw52xx_dio),
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
		.msata_en = GP_MSATA_SEL,
		.rs232_en = GP_RS232_EN,
	},

	/* GW53xx */
	{
		.gpio_pads = gw53xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw53xx_gpio_pads)/2,
		.dio_cfg = gw53xx_dio,
		.dio_num = ARRAY_SIZE(gw53xx_dio),
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
		.msata_en = GP_MSATA_SEL,
		.rs232_en = GP_RS232_EN,
	},

	/* GW54xx */
	{
		.gpio_pads = gw54xx_gpio_pads,
		.num_pads = ARRAY_SIZE(gw54xx_gpio_pads)/2,
		.dio_cfg = gw54xx_dio,
		.dio_num = ARRAY_SIZE(gw54xx_dio),
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
		.msata_en = GP_MSATA_SEL,
		.rs232_en = GP_RS232_EN,
	},

	/* GW551x */
	{
		.gpio_pads = gw551x_gpio_pads,
		.num_pads = ARRAY_SIZE(gw551x_gpio_pads)/2,
		.dio_cfg = gw551x_dio,
		.dio_num = ARRAY_SIZE(gw551x_dio),
		.leds = {
			IMX_GPIO_NR(4, 7),
		},
		.pcie_rst = IMX_GPIO_NR(1, 0),
		.wdis = IMX_GPIO_NR(7, 12),
	},

	/* GW552x */
	{
		.gpio_pads = gw552x_gpio_pads,
		.num_pads = ARRAY_SIZE(gw552x_gpio_pads)/2,
		.dio_cfg = gw552x_dio,
		.dio_num = ARRAY_SIZE(gw552x_dio),
		.leds = {
			IMX_GPIO_NR(4, 6),
			IMX_GPIO_NR(4, 7),
			IMX_GPIO_NR(4, 15),
		},
		.pcie_rst = IMX_GPIO_NR(1, 29),
		.usb_sel = IMX_GPIO_NR(1, 7),
		.wdis = IMX_GPIO_NR(7, 12),
		.msata_en = GP_MSATA_SEL,
	},

	/* GW553x */
	{
		.gpio_pads = gw553x_gpio_pads,
		.num_pads = ARRAY_SIZE(gw553x_gpio_pads)/2,
		.dio_cfg = gw553x_dio,
		.dio_num = ARRAY_SIZE(gw553x_dio),
		.leds = {
			IMX_GPIO_NR(4, 10),
			IMX_GPIO_NR(4, 11),
		},
		.pcie_rst = IMX_GPIO_NR(1, 0),
		.vidin_en = IMX_GPIO_NR(5, 20),
		.wdis = IMX_GPIO_NR(7, 12),
	},
};

void setup_iomux_gpio(int board, struct ventana_board_info *info)
{
	int i;

	/* iomux common to all Ventana boards */
	SETUP_IOMUX_PADS(gw_gpio_pads);

	/* OTG power off */
	gpio_request(GP_USB_OTG_PWR, "usbotg_pwr");
	gpio_direction_output(GP_USB_OTG_PWR, 0);

	if (board >= GW_UNKNOWN)
		return;

	/* board specific iomux */
	imx_iomux_v3_setup_multiple_pads(gpio_cfg[board].gpio_pads,
					 gpio_cfg[board].num_pads);

	/* RS232_EN# */
	if (gpio_cfg[board].rs232_en) {
		gpio_request(gpio_cfg[board].rs232_en, "rs232_en");
		gpio_direction_output(gpio_cfg[board].rs232_en, 0);
	}

	/* GW522x Uses GPIO3_IO23 for PCIE_RST# */
	if (board == GW52xx && info->model[4] == '2')
		gpio_cfg[board].pcie_rst = IMX_GPIO_NR(3, 23);

	/* assert PCI_RST# */
	gpio_request(gpio_cfg[board].pcie_rst, "pci_rst#");
	gpio_direction_output(gpio_cfg[board].pcie_rst, 0);

	/* turn off (active-high) user LED's */
	for (i = 0; i < ARRAY_SIZE(gpio_cfg[board].leds); i++) {
		char name[16];
		if (gpio_cfg[board].leds[i]) {
			sprintf(name, "led_user%d", i);
			gpio_request(gpio_cfg[board].leds[i], name);
			gpio_direction_output(gpio_cfg[board].leds[i], 1);
		}
	}

	/* MSATA Enable - default to PCI */
	if (gpio_cfg[board].msata_en) {
		gpio_request(gpio_cfg[board].msata_en, "msata_en");
		gpio_direction_output(gpio_cfg[board].msata_en, 0);
	}

	/* Expansion Mezzanine IO */
	if (gpio_cfg[board].mezz_pwren) {
		gpio_request(gpio_cfg[board].mezz_pwren, "mezz_pwr");
		gpio_direction_output(gpio_cfg[board].mezz_pwren, 0);
	}
	if (gpio_cfg[board].mezz_irq) {
		gpio_request(gpio_cfg[board].mezz_irq, "mezz_irq#");
		gpio_direction_input(gpio_cfg[board].mezz_irq);
	}

	/* RS485 Transmit Enable */
	if (gpio_cfg[board].rs485en) {
		gpio_request(gpio_cfg[board].rs485en, "rs485_en");
		gpio_direction_output(gpio_cfg[board].rs485en, 0);
	}

	/* GPS_SHDN */
	if (gpio_cfg[board].gps_shdn) {
		gpio_request(gpio_cfg[board].gps_shdn, "gps_shdn");
		gpio_direction_output(gpio_cfg[board].gps_shdn, 1);
	}

	/* Analog video codec power enable */
	if (gpio_cfg[board].vidin_en) {
		gpio_request(gpio_cfg[board].vidin_en, "anavidin_en");
		gpio_direction_output(gpio_cfg[board].vidin_en, 1);
	}

	/* DIOI2C_DIS# */
	if (gpio_cfg[board].dioi2c_en) {
		gpio_request(gpio_cfg[board].dioi2c_en, "dioi2c_dis#");
		gpio_direction_output(gpio_cfg[board].dioi2c_en, 0);
	}

	/* PCICK_SSON: disable spread-spectrum clock */
	if (gpio_cfg[board].pcie_sson) {
		gpio_request(gpio_cfg[board].pcie_sson, "pci_sson");
		gpio_direction_output(gpio_cfg[board].pcie_sson, 0);
	}

	/* USBOTG mux routing */
	if (gpio_cfg[board].usb_sel) {
		gpio_request(gpio_cfg[board].usb_sel, "usb_pcisel");
		gpio_direction_output(gpio_cfg[board].usb_sel, 0);
	}

	/* PCISKT_WDIS# (Wireless disable GPIO to miniPCIe sockets) */
	if (gpio_cfg[board].wdis) {
		gpio_request(gpio_cfg[board].wdis, "wlan_dis");
		gpio_direction_output(gpio_cfg[board].wdis, 1);
	}

	/* sense vselect pin to see if we support uhs-i */
	gpio_request(GP_SD3_VSELECT, "sd3_vselect");
	gpio_direction_input(GP_SD3_VSELECT);
	gpio_cfg[board].usd_vsel = !gpio_get_value(GP_SD3_VSELECT);
}

/* setup GPIO pinmux and default configuration per baseboard and env */
void setup_board_gpio(int board, struct ventana_board_info *info)
{
	const char *s;
	char arg[10];
	size_t len;
	int i;
	int quiet = simple_strtol(getenv("quiet"), NULL, 10);

	if (board >= GW_UNKNOWN)
		return;

	/* RS232_EN# */
	if (gpio_cfg[board].rs232_en) {
		gpio_direction_output(gpio_cfg[board].rs232_en,
				      (hwconfig("rs232")) ? 0 : 1);
	}

	/* MSATA Enable */
	if (gpio_cfg[board].msata_en && is_cpu_type(MXC_CPU_MX6Q)) {
		gpio_direction_output(GP_MSATA_SEL,
				      (hwconfig("msata")) ? 1 : 0);
	}

	/* USBOTG Select (PCISKT or FrontPanel) */
	if (gpio_cfg[board].usb_sel) {
		gpio_direction_output(gpio_cfg[board].usb_sel,
				      (hwconfig("usb_pcisel")) ? 1 : 0);
	}

	/*
	 * Configure DIO pinmux/padctl registers
	 * see IMX6DQRM/IMX6SDLRM IOMUXC_SW_PAD_CTL_PAD_* register definitions
	 */
	for (i = 0; i < gpio_cfg[board].dio_num; i++) {
		struct dio_cfg *cfg = &gpio_cfg[board].dio_cfg[i];
		iomux_v3_cfg_t ctrl = DIO_PAD_CFG;
		unsigned cputype = is_cpu_type(MXC_CPU_MX6Q) ? 0 : 1;

		if (!cfg->gpio_padmux[0] && !cfg->gpio_padmux[1])
			continue;
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
			gpio_requestf(cfg->gpio_param, "dio%d", i);
			gpio_direction_input(cfg->gpio_param);
		} else if (hwconfig_subarg_cmp(arg, "mode", "pwm") &&
			   cfg->pwm_padmux) {
			if (!cfg->pwm_param) {
				printf("DIO%d:  Error: pwm config invalid\n",
					i);
				continue;
			}
			if (!quiet)
				printf("DIO%d:  pwm%d\n", i, cfg->pwm_param);
			imx_iomux_v3_setup_pad(cfg->pwm_padmux[cputype] |
					       MUX_PAD_CTRL(ctrl));
		}
	}

	if (!quiet) {
		if (gpio_cfg[board].msata_en && is_cpu_type(MXC_CPU_MX6Q)) {
			printf("MSATA: %s\n", (hwconfig("msata") ?
			       "enabled" : "disabled"));
		}
		if (gpio_cfg[board].rs232_en) {
			printf("RS232: %s\n", (hwconfig("rs232")) ?
			       "enabled" : "disabled");
		}
	}
}

/* setup board specific PMIC */
void setup_pmic(void)
{
	struct pmic *p;
	u32 reg;

	i2c_set_bus_num(CONFIG_I2C_PMIC);

	/* configure PFUZE100 PMIC */
	if (!i2c_probe(CONFIG_POWER_PFUZE100_I2C_ADDR)) {
		debug("probed PFUZE100@0x%x\n", CONFIG_POWER_PFUZE100_I2C_ADDR);
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
			reg |= (SWBST_5_00V | (SWBST_MODE_AUTO << SWBST_MODE_SHIFT));
			pmic_reg_write(p, PFUZE100_SWBSTCON1, reg);
		}
	}

	/* configure LTC3676 PMIC */
	else if (!i2c_probe(CONFIG_POWER_LTC3676_I2C_ADDR)) {
		debug("probed LTC3676@0x%x\n", CONFIG_POWER_LTC3676_I2C_ADDR);
		power_ltc3676_init(CONFIG_I2C_PMIC);
		p = pmic_get("LTC3676_PMIC");
		if (p && !pmic_probe(p)) {
			puts("PMIC:  LTC3676\n");
			/*
			 * set board-specific scalar for max CPU frequency
			 * per CPU based on the LDO enabled Operating Ranges
			 * defined in the respective IMX6DQ and IMX6SDL
			 * datasheets. The voltage resulting from the R1/R2
			 * feedback inputs on Ventana is 1308mV. Note that this
			 * is a bit shy of the Vmin of 1350mV in the datasheet
			 * for LDO enabled mode but is as high as we can go.
			 *
			 * We will rely on an OS kernel driver to properly
			 * regulate these per CPU operating point and use LDO
			 * bypass mode when using the higher frequency
			 * operating points to compensate as LDO bypass mode
			 * allows the rails be 125mV lower.
			 */
			/* mask PGOOD during SW1 transition */
			pmic_reg_write(p, LTC3676_DVB1B,
				       0x1f | LTC3676_PGOOD_MASK);
			/* set SW1 (VDD_SOC) */
			pmic_reg_write(p, LTC3676_DVB1A, 0x1f);

			/* mask PGOOD during SW3 transition */
			pmic_reg_write(p, LTC3676_DVB3B,
				       0x1f | LTC3676_PGOOD_MASK);
			/* set SW3 (VDD_ARM) */
			pmic_reg_write(p, LTC3676_DVB3A, 0x1f);
		}
	}
}
