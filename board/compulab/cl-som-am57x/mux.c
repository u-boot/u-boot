/*
 * Pinmux configuration for CompuLab CL-SOM-AM57x board
 *
 * (C) Copyright 2016 CompuLab, Ltd. http://compulab.co.il/
 *
 * Author: Dmitry Lifshitz <lifshitz@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <asm/arch/sys_proto.h>
#include <asm/arch/mux_dra7xx.h>

/* Serial console */
static const struct pad_conf_entry cl_som_am57x_padconf_console[] = {
	{UART3_RXD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)}, /* UART3_RXD */
	{UART3_TXD, (M0 | PIN_INPUT_PULLUP | SLEWCONTROL)}, /* UART3_TXD */
};

/* PMIC I2C */
static const struct pad_conf_entry cl_som_am57x_padconf_pmic[] = {
	{MCASP1_ACLKR, (M10 | PIN_INPUT)}, /* MCASP1_ACLKR.I2C4_SDA */
	{MCASP1_FSR,   (M10 | PIN_INPUT)}, /* MCASP1_FSR.I2C4_SCL */
};

/* Green GPIO led */
static const struct pad_conf_entry cl_som_am57x_padconf_green_led[] = {
	{GPMC_A15, (M14 | PIN_OUTPUT_PULLDOWN)}, /* GPMC_A15.GPIO2_5 */
};

/* MMC/SD Card */
static const struct pad_conf_entry cl_som_am57x_padconf_sd_card[] = {
	{MMC1_CLK,  (M0  | PIN_INPUT_PULLUP)}, /* MMC1_CLK */
	{MMC1_CMD,  (M0  | PIN_INPUT_PULLUP)}, /* MMC1_CMD */
	{MMC1_DAT0, (M0  | PIN_INPUT_PULLUP)}, /* MMC1_DAT0 */
	{MMC1_DAT1, (M0  | PIN_INPUT_PULLUP)}, /* MMC1_DAT1 */
	{MMC1_DAT2, (M0  | PIN_INPUT_PULLUP)}, /* MMC1_DAT2 */
	{MMC1_DAT3, (M0  | PIN_INPUT_PULLUP)}, /* MMC1_DAT3 */
	{MMC1_SDCD, (M14 | PIN_INPUT)       }, /* MMC1_SDCD */
	{MMC1_SDWP, (M14 | PIN_INPUT)       }, /* MMC1_SDWP */
};

/* WiFi - must be in the safe mode on boot */
static const struct pad_conf_entry cl_som_am57x_padconf_wifi[] = {
	{UART1_CTSN, (M15 | PIN_INPUT_PULLDOWN)}, /* UART1_CTSN */
	{UART1_RTSN, (M15 | PIN_INPUT_PULLDOWN)}, /* UART1_RTSN */
	{UART2_RXD,  (M15 | PIN_INPUT_PULLDOWN)}, /* UART2_RXD */
	{UART2_TXD,  (M15 | PIN_INPUT_PULLDOWN)}, /* UART2_TXD */
	{UART2_CTSN, (M15 | PIN_INPUT_PULLDOWN)}, /* UART2_CTSN */
	{UART2_RTSN, (M15 | PIN_INPUT_PULLDOWN)}, /* UART2_RTSN */
};

/* QSPI */
static const struct pad_conf_entry cl_som_am57x_padconf_qspi[] = {
	{GPMC_A13, (M1 | PIN_INPUT)       }, /* GPMC_A13.QSPI1_RTCLK */
	{GPMC_A18, (M1 | PIN_INPUT)       }, /* GPMC_A18.QSPI1_SCLK */
	{GPMC_A16, (M1 | PIN_INPUT)       }, /* GPMC_A16.QSPI1_D0 */
	{GPMC_A17, (M1 | PIN_INPUT)       }, /* GPMC_A17.QSPI1_D1 */
	{GPMC_CS2, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_CS2.QSPI1_CS0 */
};

/* GPIO Expander I2C */
static const struct pad_conf_entry cl_som_am57x_padconf_i2c_gpio[] = {
	{MCASP1_AXR0, (M10 | PIN_INPUT)}, /* MCASP1_AXR0.I2C5_SDA */
	{MCASP1_AXR1, (M10 | PIN_INPUT)}, /* MCASP1_AXR1.I2C5_SCL */
};

/* eMMC internal storage */
static const struct pad_conf_entry cl_som_am57x_padconf_emmc[] = {
	{GPMC_A19, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_A19.MMC2_DAT4 */
	{GPMC_A20, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_A20.MMC2_DAT5 */
	{GPMC_A21, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_A21.MMC2_DAT6 */
	{GPMC_A22, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_A22.MMC2_DAT7 */
	{GPMC_A23, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_A23.MMC2_CLK */
	{GPMC_A24, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_A24.MMC2_DAT0 */
	{GPMC_A25, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_A25.MMC2_DAT1 */
	{GPMC_A26, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_A26.MMC2_DAT2 */
	{GPMC_A27, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_A27.MMC2_DAT3 */
	{GPMC_CS1, (M1 | PIN_INPUT_PULLUP)}, /* GPMC_CS1.MMC2_CMD */
};

/* usb1_drvvbus */
static const struct pad_conf_entry cl_som_am57x_padconf_usb[] = {
	/* USB1_DRVVBUS.USB1_DRVVBUS */
	{USB1_DRVVBUS, (M0 | PIN_OUTPUT_PULLDOWN | SLEWCONTROL) },
};

/* Ethernet */
static const struct pad_conf_entry cl_som_am57x_padconf_ethernet[] = {
	/* MDIO bus */
	{VIN2A_D10,  (M3  | PIN_OUTPUT_PULLUP) }, /* VIN2A_D10.MDIO_MCLK  */
	{VIN2A_D11,  (M3  | PIN_INPUT_PULLUP)  }, /* VIN2A_D11.MDIO_D  */
	/* EMAC Slave 1 at addr 0x1 - Default interface */
	{VIN2A_D12,  (M3  | PIN_OUTPUT)         }, /* VIN2A_D12.RGMII1_TXC */
	{VIN2A_D13,  (M3  | PIN_OUTPUT)         }, /* VIN2A_D13.RGMII1_TXCTL */
	{VIN2A_D14,  (M3  | PIN_OUTPUT)         }, /* VIN2A_D14.RGMII1_TXD3 */
	{VIN2A_D15,  (M3  | PIN_OUTPUT)         }, /* VIN2A_D15.RGMII1_TXD2 */
	{VIN2A_D16,  (M3  | PIN_OUTPUT)         }, /* VIN2A_D16.RGMII1_TXD1 */
	{VIN2A_D17,  (M3  | PIN_OUTPUT)         }, /* VIN2A_D17.RGMII1_TXD0 */
	{VIN2A_D18,  (M3  | PIN_INPUT_PULLDOWN) }, /* VIN2A_D18.RGMII1_RXC */
	{VIN2A_D19,  (M3  | PIN_INPUT_PULLDOWN) }, /* VIN2A_D19.RGMII1_RXCTL */
	{VIN2A_D20,  (M3  | PIN_INPUT_PULLDOWN) }, /* VIN2A_D20.RGMII1_RXD3 */
	{VIN2A_D21,  (M3  | PIN_INPUT_PULLDOWN) }, /* VIN2A_D21.RGMII1_RXD2 */
	{VIN2A_D22,  (M3  | PIN_INPUT_PULLDOWN) }, /* VIN2A_D22.RGMII1_RXD1 */
	{VIN2A_D23,  (M3  | PIN_INPUT_PULLDOWN) }, /* VIN2A_D23.RGMII1_RXD0 */
	/* Eth PHY1 reset GPIOs*/
	{VIN2A_CLK0, (M14 | PIN_OUTPUT_PULLDOWN)}, /* VIN2A_CLK0.GPIO3_28 */
};

#define SET_MUX(mux_array) do_set_mux32((*ctrl)->control_padconf_core_base, \
					mux_array, ARRAY_SIZE(mux_array))

void set_muxconf_regs(void)
{
	SET_MUX(cl_som_am57x_padconf_console);
	SET_MUX(cl_som_am57x_padconf_pmic);
	SET_MUX(cl_som_am57x_padconf_green_led);
	SET_MUX(cl_som_am57x_padconf_sd_card);
	SET_MUX(cl_som_am57x_padconf_wifi);
	SET_MUX(cl_som_am57x_padconf_qspi);
	SET_MUX(cl_som_am57x_padconf_i2c_gpio);
	SET_MUX(cl_som_am57x_padconf_emmc);
	SET_MUX(cl_som_am57x_padconf_usb);
	SET_MUX(cl_som_am57x_padconf_ethernet);
}
