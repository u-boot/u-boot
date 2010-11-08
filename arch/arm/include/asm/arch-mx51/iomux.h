/*
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __MACH_MX51_IOMUX_H__
#define __MACH_MX51_IOMUX_H__

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx51_pins.h>

typedef unsigned int iomux_pin_name_t;

/* various IOMUX output functions */
typedef enum iomux_config {
	IOMUX_CONFIG_ALT0,	/*!< used as alternate function 0 */
	IOMUX_CONFIG_ALT1,	/*!< used as alternate function 1 */
	IOMUX_CONFIG_ALT2,	/*!< used as alternate function 2 */
	IOMUX_CONFIG_ALT3,	/*!< used as alternate function 3 */
	IOMUX_CONFIG_ALT4,	/*!< used as alternate function 4 */
	IOMUX_CONFIG_ALT5,	/*!< used as alternate function 5 */
	IOMUX_CONFIG_ALT6,	/*!< used as alternate function 6 */
	IOMUX_CONFIG_ALT7,	/*!< used as alternate function 7 */
	IOMUX_CONFIG_GPIO,	/*!< added to help user use GPIO mode */
	IOMUX_CONFIG_SION = 0x1 << 4,	/*!< used as LOOPBACK:MUX SION bit */
} iomux_pin_cfg_t;

/* various IOMUX pad functions */
typedef enum iomux_pad_config {
	PAD_CTL_SRE_SLOW = 0x0 << 0,	/* Slow slew rate */
	PAD_CTL_SRE_FAST = 0x1 << 0,	/* Fast slew rate */
	PAD_CTL_DRV_LOW = 0x0 << 1,	/* Low drive strength */
	PAD_CTL_DRV_MEDIUM = 0x1 << 1,	/* Medium drive strength */
	PAD_CTL_DRV_HIGH = 0x2 << 1,	/* High drive strength */
	PAD_CTL_DRV_MAX = 0x3 << 1,	/* Max drive strength */
	PAD_CTL_ODE_OPENDRAIN_NONE = 0x0 << 3,	/* Opendrain disable */
	PAD_CTL_ODE_OPENDRAIN_ENABLE = 0x1 << 3,/* Opendrain enable */
	PAD_CTL_100K_PD = 0x0 << 4,	/* 100Kohm pulldown */
	PAD_CTL_47K_PU = 0x1 << 4,	/* 47Kohm pullup */
	PAD_CTL_100K_PU = 0x2 << 4,	/* 100Kohm pullup */
	PAD_CTL_22K_PU = 0x3 << 4,	/* 22Kohm pullup */
	PAD_CTL_PUE_KEEPER = 0x0 << 6,	/* enable pulldown */
	PAD_CTL_PUE_PULL = 0x1 << 6,	/* enable pullup */
	PAD_CTL_PKE_NONE = 0x0 << 7,	/* Disable pullup/pulldown */
	PAD_CTL_PKE_ENABLE = 0x1 << 7,	/* Enable pullup/pulldown */
	PAD_CTL_HYS_NONE = 0x0 << 8,	/* Hysteresis disabled */
	PAD_CTL_HYS_ENABLE = 0x1 << 8,	/* Hysteresis enabled */
	PAD_CTL_DDR_INPUT_CMOS = 0x0 << 9,/* DDR input CMOS */
	PAD_CTL_DDR_INPUT_DDR = 0x1 << 9,/* DDR input DDR */
	PAD_CTL_DRV_VOT_LOW = 0x0 << 13, /* Low voltage mode */
	PAD_CTL_DRV_VOT_HIGH = 0x1 << 13,/* High voltage mode */
} iomux_pad_config_t;

/* various IOMUX input select register index */
typedef enum iomux_input_select {
	MUX_IN_AUDMUX_P4_INPUT_DA_AMX_SELECT_I = 0,
	MUX_IN_AUDMUX_P4_INPUT_DB_AMX_SELECT_I,
	MUX_IN_AUDMUX_P4_INPUT_TXCLK_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P4_INPUT_TXFS_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P5_INPUT_DA_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P5_INPUT_DB_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P5_INPUT_RXCLK_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P5_INPUT_RXFS_AMX_SELECT,
	MUX_IN_AUDMUX_P5_INPUT_TXCLK_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P5_INPUT_TXFS_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P6_INPUT_DA_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P6_INPUT_DB_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P6_INPUT_RXCLK_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P6_INPUT_RXFS_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P6_INPUT_TXCLK_AMX_SELECT_INPUT,
	MUX_IN_AUDMUX_P6_INPUT_TXFS_AMX_SELECT_INPUT,
	MUX_IN_CCM_IPP_DI_CLK_SELECT_INPUT,
	/* TO2 */
	MUX_IN_CCM_IPP_DI1_CLK_SELECT_INPUT,
	MUX_IN_CCM_PLL1_BYPASS_CLK_SELECT_INPUT,
	MUX_IN_CCM_PLL2_BYPASS_CLK_SELECT_INPUT,
	MUX_IN_CSPI_IPP_CSPI_CLK_IN_SELECT_INPUT,
	MUX_IN_CSPI_IPP_IND_MISO_SELECT_INPUT,
	MUX_IN_CSPI_IPP_IND_MOSI_SELECT_INPUT,
	MUX_IN_CSPI_IPP_IND_SS_B_1_SELECT_INPUT,
	MUX_IN_CSPI_IPP_IND_SS_B_2_SELECT_INPUT,
	MUX_IN_CSPI_IPP_IND_SS_B_3_SELECT_INPUT,
	MUX_IN_DPLLIP1_L1T_TOG_EN_SELECT_INPUT,
	/* TO2 */
	MUX_IN_ECSPI2_IPP_IND_SS_B_1_SELECT_INPUT,
	MUX_IN_ECSPI2_IPP_IND_SS_B_3_SELECT_INPUT,
	MUX_IN_EMI_IPP_IND_RDY_INT_SELECT_INPUT,
	MUX_IN_ESDHC3_IPP_DAT0_IN_SELECT_INPUT,
	MUX_IN_ESDHC3_IPP_DAT1_IN_SELECT_INPUT,
	MUX_IN_ESDHC3_IPP_DAT2_IN_SELECT_INPUT,
	MUX_IN_ESDHC3_IPP_DAT3_IN_SELECT_INPUT,
	MUX_IN_FEC_FEC_COL_SELECT_INPUT,
	MUX_IN_FEC_FEC_CRS_SELECT_INPUT,
	MUX_IN_FEC_FEC_MDI_SELECT_INPUT,
	MUX_IN_FEC_FEC_RDATA_0_SELECT_INPUT,
	MUX_IN_FEC_FEC_RDATA_1_SELECT_INPUT,
	MUX_IN_FEC_FEC_RDATA_2_SELECT_INPUT,
	MUX_IN_FEC_FEC_RDATA_3_SELECT_INPUT,
	MUX_IN_FEC_FEC_RX_CLK_SELECT_INPUT,
	MUX_IN_FEC_FEC_RX_DV_SELECT_INPUT,
	MUX_IN_FEC_FEC_RX_ER_SELECT_INPUT,
	MUX_IN_FEC_FEC_TX_CLK_SELECT_INPUT,
	MUX_IN_GPIO3_IPP_IND_G_IN_1_SELECT_INPUT,
	MUX_IN_GPIO3_IPP_IND_G_IN_2_SELECT_INPUT,
	MUX_IN_GPIO3_IPP_IND_G_IN_3_SELECT_INPUT,
	MUX_IN_GPIO3_IPP_IND_G_IN_4_SELECT_INPUT,
	MUX_IN_GPIO3_IPP_IND_G_IN_5_SELECT_INPUT,
	MUX_IN_GPIO3_IPP_IND_G_IN_6_SELECT_INPUT,
	MUX_IN_GPIO3_IPP_IND_G_IN_7_SELECT_INPUT,
	MUX_IN_GPIO3_IPP_IND_G_IN_8_SELECT_INPUT,
	/* TO2 */
	MUX_IN_GPIO3_IPP_IND_G_IN_12_SELECT_INPUT,
	MUX_IN_HSC_MIPI_MIX_IPP_IND_SENS1_DATA_EN_SELECT_INPUT,
	MUX_IN_HSC_MIPI_MIX_IPP_IND_SENS2_DATA_EN_SELECT_INPUT,
	/* TO2 */
	MUX_IN_HSC_MIPI_MIX_PAR_VSYNC_SELECT_INPUT,
	/* TO2 */
	MUX_IN_HSC_MIPI_MIX_PAR_DI_WAIT_SELECT_INPUT,
	MUX_IN_HSC_MIPI_MIX_PAR_SISG_TRIG_SELECT_INPUT,
	MUX_IN_I2C1_IPP_SCL_IN_SELECT_INPUT,
	MUX_IN_I2C1_IPP_SDA_IN_SELECT_INPUT,
	MUX_IN_I2C2_IPP_SCL_IN_SELECT_INPUT,
	MUX_IN_I2C2_IPP_SDA_IN_SELECT_INPUT,

	MUX_IN_IPU_IPP_DI_0_IND_DISPB_SD_D_SELECT_INPUT,

	MUX_IN_IPU_IPP_DI_1_IND_DISPB_SD_D_SELECT_INPUT,

	MUX_IN_KPP_IPP_IND_COL_6_SELECT_INPUT,
	MUX_IN_KPP_IPP_IND_COL_7_SELECT_INPUT,
	MUX_IN_KPP_IPP_IND_ROW_4_SELECT_INPUT,
	MUX_IN_KPP_IPP_IND_ROW_5_SELECT_INPUT,
	MUX_IN_KPP_IPP_IND_ROW_6_SELECT_INPUT,
	MUX_IN_KPP_IPP_IND_ROW_7_SELECT_INPUT,
	MUX_IN_UART1_IPP_UART_RTS_B_SELECT_INPUT,
	MUX_IN_UART1_IPP_UART_RXD_MUX_SELECT_INPUT,
	MUX_IN_UART2_IPP_UART_RTS_B_SELECT_INPUT,
	MUX_IN_UART2_IPP_UART_RXD_MUX_SELECT_INPUT,
	MUX_IN_UART3_IPP_UART_RTS_B_SELECT_INPUT,
	MUX_IN_UART3_IPP_UART_RXD_MUX_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_CLK_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_DATA_0_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_DATA_1_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_DATA_2_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_DATA_3_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_DATA_4_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_DATA_5_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_DATA_6_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_DATA_7_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_DIR_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_NXT_SELECT_INPUT,
	MUX_IN_USBOH3_IPP_IND_UH3_STP_SELECT_INPUT,
	MUX_INPUT_NUM_MUX,
} iomux_input_select_t;

/* various IOMUX input functions */
typedef enum iomux_input_config {
	INPUT_CTL_PATH0 = 0x0,
	INPUT_CTL_PATH1,
	INPUT_CTL_PATH2,
	INPUT_CTL_PATH3,
	INPUT_CTL_PATH4,
	INPUT_CTL_PATH5,
	INPUT_CTL_PATH6,
	INPUT_CTL_PATH7,
} iomux_input_config_t;

void mxc_request_iomux(iomux_pin_name_t pin, iomux_pin_cfg_t config);
void mxc_free_iomux(iomux_pin_name_t pin, iomux_pin_cfg_t config);
void mxc_iomux_set_pad(iomux_pin_name_t pin, u32 config);
unsigned int mxc_iomux_get_pad(iomux_pin_name_t pin);
void mxc_iomux_set_input(iomux_input_select_t input, u32 config);

#endif				/*  __MACH_MX51_IOMUX_H__ */
