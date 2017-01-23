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
	{UART3_RXD, (FSC | IEN | PDIS | PTU | M0)}, /* UART3_RXD */
	{UART3_TXD, (FSC | IEN | PDIS | PTU | M0)}, /* UART3_TXD */
};

/* PMIC I2C */
static const struct pad_conf_entry cl_som_am57x_padconf_pmic[] = {
	{MCASP1_ACLKR, (IEN | PEN | M10)}, /* MCASP1_ACLKR.I2C4_SDA */
	{MCASP1_FSR,   (IEN | PEN | M10)}, /* MCASP1_FSR.I2C4_SCL */
};

/* Green GPIO led */
static const struct pad_conf_entry cl_som_am57x_padconf_green_led[] = {
	{GPMC_A15, (IDIS | PDIS | PTD | M14)}, /* GPMC_A15.GPIO2_5 */
};

/* MMC/SD Card */
static const struct pad_conf_entry cl_som_am57x_padconf_sd_card[] = {
	{MMC1_CLK,  (IEN | PDIS | PTU | M0) }, /* MMC1_CLK */
	{MMC1_CMD,  (IEN | PDIS | PTU | M0) }, /* MMC1_CMD */
	{MMC1_DAT0, (IEN | PDIS | PTU | M0) }, /* MMC1_DAT0 */
	{MMC1_DAT1, (IEN | PDIS | PTU | M0) }, /* MMC1_DAT1 */
	{MMC1_DAT2, (IEN | PDIS | PTU | M0) }, /* MMC1_DAT2 */
	{MMC1_DAT3, (IEN | PDIS | PTU | M0) }, /* MMC1_DAT3 */
	{MMC1_SDCD, (IEN | PEN  |	M14)}, /* MMC1_SDCD */
	{MMC1_SDWP, (IEN | PEN  |	M14)}, /* MMC1_SDWP */
};

/* WiFi - must be in the safe mode on boot */
static const struct pad_conf_entry cl_som_am57x_padconf_wifi[] = {
	{UART1_CTSN, (IEN | M15)}, /* UART1_CTSN */
	{UART1_RTSN, (IEN | M15)}, /* UART1_RTSN */
	{UART2_RXD,  (IEN | M15)}, /* UART2_RXD */
	{UART2_TXD,  (IEN | M15)}, /* UART2_TXD */
	{UART2_CTSN, (IEN | M15)}, /* UART2_CTSN */
	{UART2_RTSN, (IEN | M15)}, /* UART2_RTSN */
};

/* QSPI */
static const struct pad_conf_entry cl_som_am57x_padconf_qspi[] = {
	{GPMC_A13, (IEN | PEN  |       M1)}, /* GPMC_A13.QSPI1_RTCLK */
	{GPMC_A18, (IEN | PEN  |       M1)}, /* GPMC_A18.QSPI1_SCLK */
	{GPMC_A16, (IEN | PEN  |       M1)}, /* GPMC_A16.QSPI1_D0 */
	{GPMC_A17, (IEN | PEN  |       M1)}, /* GPMC_A17.QSPI1_D1 */
	{GPMC_CS2, (IEN | PDIS | PTU | M1)}, /* GPMC_CS2.QSPI1_CS0 */
};

/* GPIO Expander I2C */
static const struct pad_conf_entry cl_som_am57x_padconf_i2c_gpio[] = {
	{MCASP1_AXR0, (IEN | PEN | M10)}, /* MCASP1_AXR0.I2C5_SDA */
	{MCASP1_AXR1, (IEN | PEN | M10)}, /* MCASP1_AXR1.I2C5_SCL */
};

/* eMMC internal storage */
static const struct pad_conf_entry cl_som_am57x_padconf_emmc[] = {
	{GPMC_A19, (IEN | PDIS | PTU | M1)}, /* GPMC_A19.MMC2_DAT4 */
	{GPMC_A20, (IEN | PDIS | PTU | M1)}, /* GPMC_A20.MMC2_DAT5 */
	{GPMC_A21, (IEN | PDIS | PTU | M1)}, /* GPMC_A21.MMC2_DAT6 */
	{GPMC_A22, (IEN | PDIS | PTU | M1)}, /* GPMC_A22.MMC2_DAT7 */
	{GPMC_A23, (IEN | PDIS | PTU | M1)}, /* GPMC_A23.MMC2_CLK */
	{GPMC_A24, (IEN | PDIS | PTU | M1)}, /* GPMC_A24.MMC2_DAT0 */
	{GPMC_A25, (IEN | PDIS | PTU | M1)}, /* GPMC_A25.MMC2_DAT1 */
	{GPMC_A26, (IEN | PDIS | PTU | M1)}, /* GPMC_A26.MMC2_DAT2 */
	{GPMC_A27, (IEN | PDIS | PTU | M1)}, /* GPMC_A27.MMC2_DAT3 */
	{GPMC_CS1, (IEN | PDIS | PTU | M1)}, /* GPMC_CS1.MMC2_CMD */
};

/* usb1_drvvbus */
static const struct pad_conf_entry cl_som_am57x_padconf_usb[] = {
	{USB1_DRVVBUS, (M0 | FSC) }, /* USB1_DRVVBUS.USB1_DRVVBUS */
};

/* Ethernet */
static const struct pad_conf_entry cl_som_am57x_padconf_ethernet[] = {
	/* MDIO bus */
	{VIN2A_D10,  (PDIS | PTU  |	  M3) }, /* VIN2A_D10.MDIO_MCLK  */
	{VIN2A_D11,  (IEN  | PDIS | PTU | M3) }, /* VIN2A_D11.MDIO_D  */
	/* EMAC Slave 1 at addr 0x1 - Default interface */
	{VIN2A_D12,  (IDIS | PEN  |	  M3) }, /* VIN2A_D12.RGMII1_TXC */
	{VIN2A_D13,  (IDIS | PEN  |	  M3) }, /* VIN2A_D13.RGMII1_TXCTL */
	{VIN2A_D14,  (IDIS | PEN  |	  M3) }, /* VIN2A_D14.RGMII1_TXD3 */
	{VIN2A_D15,  (IDIS | PEN  |	  M3) }, /* VIN2A_D15.RGMII1_TXD2 */
	{VIN2A_D16,  (IDIS | PEN  |	  M3) }, /* VIN2A_D16.RGMII1_TXD1 */
	{VIN2A_D17,  (IDIS | PEN  |	  M3) }, /* VIN2A_D17.RGMII1_TXD0 */
	{VIN2A_D18,  (IEN  | PDIS | PTD | M3) }, /* VIN2A_D18.RGMII1_RXC */
	{VIN2A_D19,  (IEN  | PDIS | PTD | M3) }, /* VIN2A_D19.RGMII1_RXCTL */
	{VIN2A_D20,  (IEN  | PDIS | PTD | M3) }, /* VIN2A_D20.RGMII1_RXD3 */
	{VIN2A_D21,  (IEN  | PDIS | PTD | M3) }, /* VIN2A_D21.RGMII1_RXD2 */
	{VIN2A_D22,  (IEN  | PDIS | PTD | M3) }, /* VIN2A_D22.RGMII1_RXD1 */
	{VIN2A_D23,  (IEN  | PDIS | PTD | M3) }, /* VIN2A_D23.RGMII1_RXD0 */
	/* Eth PHY1 reset GPIOs*/
	{VIN2A_CLK0, (IDIS | PDIS | PTD | M14)}, /* VIN2A_CLK0.GPIO3_28 */
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
