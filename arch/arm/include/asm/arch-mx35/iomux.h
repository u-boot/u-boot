/*
 * (C) Copyright 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
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

#ifndef __MACH_MX35_IOMUX_H__
#define __MACH_MX35_IOMUX_H__

#include <asm/arch/imx-regs.h>

/*
 * various IOMUX functions
 */
typedef enum iomux_pin_config {
	MUX_CONFIG_FUNC = 0,	/* used as function */
	MUX_CONFIG_ALT1,	/* used as alternate function 1 */
	MUX_CONFIG_ALT2,	/* used as alternate function 2 */
	MUX_CONFIG_ALT3,	/* used as alternate function 3 */
	MUX_CONFIG_ALT4,	/* used as alternate function 4 */
	MUX_CONFIG_ALT5,	/* used as alternate function 5 */
	MUX_CONFIG_ALT6,	/* used as alternate function 6 */
	MUX_CONFIG_ALT7,	/* used as alternate function 7 */
	MUX_CONFIG_SION = 0x1 << 4,	/* used as LOOPBACK:MUX SION bit */
	MUX_CONFIG_GPIO = MUX_CONFIG_ALT5,	/* used as GPIO */
} iomux_pin_cfg_t;

/*
 * various IOMUX pad functions
 */
typedef enum iomux_pad_config {
	PAD_CTL_DRV_3_3V = 0x0 << 13,
	PAD_CTL_DRV_1_8V = 0x1 << 13,
	PAD_CTL_HYS_CMOS = 0x0 << 8,
	PAD_CTL_HYS_SCHMITZ = 0x1 << 8,
	PAD_CTL_PKE_NONE = 0x0 << 7,
	PAD_CTL_PKE_ENABLE = 0x1 << 7,
	PAD_CTL_PUE_KEEPER = 0x0 << 6,
	PAD_CTL_PUE_PUD = 0x1 << 6,
	PAD_CTL_100K_PD = 0x0 << 4,
	PAD_CTL_47K_PU = 0x1 << 4,
	PAD_CTL_100K_PU = 0x2 << 4,
	PAD_CTL_22K_PU = 0x3 << 4,
	PAD_CTL_ODE_CMOS = 0x0 << 3,
	PAD_CTL_ODE_OpenDrain = 0x1 << 3,
	PAD_CTL_DRV_NORMAL = 0x0 << 1,
	PAD_CTL_DRV_HIGH = 0x1 << 1,
	PAD_CTL_DRV_MAX = 0x2 << 1,
	PAD_CTL_SRE_SLOW = 0x0 << 0,
	PAD_CTL_SRE_FAST = 0x1 << 0
} iomux_pad_config_t;

/*
 * various IOMUX general purpose functions
 */
typedef enum iomux_gp_func {
	MUX_SDCTL_CSD0_SEL = 0x1 << 0,
	MUX_SDCTL_CSD1_SEL = 0x1 << 1,
	MUX_TAMPER_DETECT_EN = 0x1 << 2,
} iomux_gp_func_t;

/*
 * various IOMUX input select register index
 */
typedef enum iomux_input_select {
	MUX_IN_AMX_P5_RXCLK = 0,
	MUX_IN_AMX_P5_RXFS,
	MUX_IN_AMX_P6_DA,
	MUX_IN_AMX_P6_DB,
	MUX_IN_AMX_P6_RXCLK,
	MUX_IN_AMX_P6_RXFS,
	MUX_IN_AMX_P6_TXCLK,
	MUX_IN_AMX_P6_TXFS,
	MUX_IN_CAN1_CANRX,
	MUX_IN_CAN2_CANRX,
	MUX_IN_CCM_32K_MUXED,
	MUX_IN_CCM_PMIC_RDY,
	MUX_IN_CSPI1_SS2_B,
	MUX_IN_CSPI1_SS3_B,
	MUX_IN_CSPI2_CLK_IN,
	MUX_IN_CSPI2_DATAREADY_B,
	MUX_IN_CSPI2_MISO,
	MUX_IN_CSPI2_MOSI,
	MUX_IN_CSPI2_SS0_B,
	MUX_IN_CSPI2_SS1_B,
	MUX_IN_CSPI2_SS2_B,
	MUX_IN_CSPI2_SS3_B,
	MUX_IN_EMI_WEIM_DTACK_B,
	MUX_IN_ESDHC1_DAT4_IN,
	MUX_IN_ESDHC1_DAT5_IN,
	MUX_IN_ESDHC1_DAT6_IN,
	MUX_IN_ESDHC1_DAT7_IN,
	MUX_IN_ESDHC3_CARD_CLK_IN,
	MUX_IN_ESDHC3_CMD_IN,
	MUX_IN_ESDHC3_DAT0,
	MUX_IN_ESDHC3_DAT1,
	MUX_IN_ESDHC3_DAT2,
	MUX_IN_ESDHC3_DAT3,
	MUX_IN_GPIO1_IN_0,
	MUX_IN_GPIO1_IN_10,
	MUX_IN_GPIO1_IN_11,
	MUX_IN_GPIO1_IN_1,
	MUX_IN_GPIO1_IN_20,
	MUX_IN_GPIO1_IN_21,
	MUX_IN_GPIO1_IN_22,
	MUX_IN_GPIO1_IN_2,
	MUX_IN_GPIO1_IN_3,
	MUX_IN_GPIO1_IN_4,
	MUX_IN_GPIO1_IN_5,
	MUX_IN_GPIO1_IN_6,
	MUX_IN_GPIO1_IN_7,
	MUX_IN_GPIO1_IN_8,
	MUX_IN_GPIO1_IN_9,
	MUX_IN_GPIO2_IN_0,
	MUX_IN_GPIO2_IN_10,
	MUX_IN_GPIO2_IN_11,
	MUX_IN_GPIO2_IN_12,
	MUX_IN_GPIO2_IN_13,
	MUX_IN_GPIO2_IN_14,
	MUX_IN_GPIO2_IN_15,
	MUX_IN_GPIO2_IN_16,
	MUX_IN_GPIO2_IN_17,
	MUX_IN_GPIO2_IN_18,
	MUX_IN_GPIO2_IN_19,
	MUX_IN_GPIO2_IN_20,
	MUX_IN_GPIO2_IN_21,
	MUX_IN_GPIO2_IN_22,
	MUX_IN_GPIO2_IN_23,
	MUX_IN_GPIO2_IN_24,
	MUX_IN_GPIO2_IN_25,
	MUX_IN_GPIO2_IN_26,
	MUX_IN_GPIO2_IN_27,
	MUX_IN_GPIO2_IN_28,
	MUX_IN_GPIO2_IN_29,
	MUX_IN_GPIO2_IN_2,
	MUX_IN_GPIO2_IN_30,
	MUX_IN_GPIO2_IN_31,
	MUX_IN_GPIO2_IN_3,
	MUX_IN_GPIO2_IN_4,
	MUX_IN_GPIO2_IN_5,
	MUX_IN_GPIO2_IN_6,
	MUX_IN_GPIO2_IN_7,
	MUX_IN_GPIO2_IN_8,
	MUX_IN_GPIO2_IN_9,
	MUX_IN_GPIO3_IN_0,
	MUX_IN_GPIO3_IN_10,
	MUX_IN_GPIO3_IN_11,
	MUX_IN_GPIO3_IN_12,
	MUX_IN_GPIO3_IN_13,
	MUX_IN_GPIO3_IN_14,
	MUX_IN_GPIO3_IN_15,
	MUX_IN_GPIO3_IN_4,
	MUX_IN_GPIO3_IN_5,
	MUX_IN_GPIO3_IN_6,
	MUX_IN_GPIO3_IN_7,
	MUX_IN_GPIO3_IN_8,
	MUX_IN_GPIO3_IN_9,
	MUX_IN_I2C3_SCL_IN,
	MUX_IN_I2C3_SDA_IN,
	MUX_IN_IPU_DISPB_D0_VSYNC,
	MUX_IN_IPU_DISPB_D12_VSYNC,
	MUX_IN_IPU_DISPB_SD_D,
	MUX_IN_IPU_SENSB_DATA_0,
	MUX_IN_IPU_SENSB_DATA_1,
	MUX_IN_IPU_SENSB_DATA_2,
	MUX_IN_IPU_SENSB_DATA_3,
	MUX_IN_IPU_SENSB_DATA_4,
	MUX_IN_IPU_SENSB_DATA_5,
	MUX_IN_IPU_SENSB_DATA_6,
	MUX_IN_IPU_SENSB_DATA_7,
	MUX_IN_KPP_COL_0,
	MUX_IN_KPP_COL_1,
	MUX_IN_KPP_COL_2,
	MUX_IN_KPP_COL_3,
	MUX_IN_KPP_COL_4,
	MUX_IN_KPP_COL_5,
	MUX_IN_KPP_COL_6,
	MUX_IN_KPP_COL_7,
	MUX_IN_KPP_ROW_0,
	MUX_IN_KPP_ROW_1,
	MUX_IN_KPP_ROW_2,
	MUX_IN_KPP_ROW_3,
	MUX_IN_KPP_ROW_4,
	MUX_IN_KPP_ROW_5,
	MUX_IN_KPP_ROW_6,
	MUX_IN_KPP_ROW_7,
	MUX_IN_OWIRE_BATTERY_LINE,
	MUX_IN_SPDIF_HCKT_CLK2,
	MUX_IN_SPDIF_SPDIF_IN1,
	MUX_IN_UART3_UART_RTS_B,
	MUX_IN_UART3_UART_RXD_MUX,
	MUX_IN_USB_OTG_DATA_0,
	MUX_IN_USB_OTG_DATA_1,
	MUX_IN_USB_OTG_DATA_2,
	MUX_IN_USB_OTG_DATA_3,
	MUX_IN_USB_OTG_DATA_4,
	MUX_IN_USB_OTG_DATA_5,
	MUX_IN_USB_OTG_DATA_6,
	MUX_IN_USB_OTG_DATA_7,
	MUX_IN_USB_OTG_DIR,
	MUX_IN_USB_OTG_NXT,
	MUX_IN_USB_UH2_DATA_0,
	MUX_IN_USB_UH2_DATA_1,
	MUX_IN_USB_UH2_DATA_2,
	MUX_IN_USB_UH2_DATA_3,
	MUX_IN_USB_UH2_DATA_4,
	MUX_IN_USB_UH2_DATA_5,
	MUX_IN_USB_UH2_DATA_6,
	MUX_IN_USB_UH2_DATA_7,
	MUX_IN_USB_UH2_DIR,
	MUX_IN_USB_UH2_NXT,
	MUX_IN_USB_UH2_USB_OC,
} iomux_input_select_t;

/*
 * various IOMUX input functions
 */
typedef enum iomux_input_config {
	INPUT_CTL_PATH0 = 0x0,
	INPUT_CTL_PATH1,
	INPUT_CTL_PATH2,
	INPUT_CTL_PATH3,
	INPUT_CTL_PATH4,
	INPUT_CTL_PATH5,
	INPUT_CTL_PATH6,
	INPUT_CTL_PATH7,
} iomux_input_cfg_t;

/*
 * Request ownership for an IO pin. This function has to be the first one
 * being called before that pin is used. The caller has to check the
 * return value to make sure it returns 0.
 *
 * @param  pin		a name defined by iomux_pin_name_t
 * @param  cfg		an input function as defined in iomux_pin_cfg_t
 *
 * @return		0 if successful; Non-zero otherwise
 */
void mxc_request_iomux(iomux_pin_name_t pin, iomux_pin_cfg_t cfg);

/*
 * Release ownership for an IO pin
 *
 * @param  pin		a name defined by iomux_pin_name_t
 * @param  cfg		an input function as defined in iomux_pin_cfg_t
 */
void mxc_free_iomux(iomux_pin_name_t pin, iomux_pin_cfg_t cfg);

/*
 * This function enables/disables the general purpose function for a particular
 * signal.
 *
 * @param  gp   one signal as defined in iomux_gp_func_t
 * @param  en   1 to enable; 0 to disable
 */
void mxc_iomux_set_gpr(iomux_gp_func_t gp, int en);

/*
 * This function configures the pad value for a IOMUX pin.
 *
 * @param  pin          a pin number as defined in iomux_pin_name_t
 * @param  config       the ORed value of elements defined in
 *				iomux_pad_config_t
 */
void mxc_iomux_set_pad(iomux_pin_name_t pin, u32 config);

/*
 * This function configures input path.
 *
 * @param  input        index of input select register as defined in
 *				iomux_input_select_t
 * @param  config       the binary value of elements defined in
 *				iomux_input_cfg_t
 */
void mxc_iomux_set_input(iomux_input_select_t input, u32 config);
#endif
