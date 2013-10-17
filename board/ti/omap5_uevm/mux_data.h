/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated, <www.ti.com>
 *
 *	Sricharan R		<r.sricharan@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _EVM5430_MUX_DATA_H
#define _EVM5430_MUX_DATA_H

#include <asm/arch/mux_omap5.h>

const struct pad_conf_entry core_padconf_array_essential[] = {

	{EMMC_CLK, (PTU | IEN | M0)}, /*  EMMC_CLK   */
	{EMMC_CMD, (PTU | IEN | M0)}, /*  EMMC_CMD   */
	{EMMC_DATA0, (PTU | IEN | M0)}, /*  EMMC_DATA0 */
	{EMMC_DATA1, (PTU | IEN | M0)}, /*  EMMC_DATA1 */
	{EMMC_DATA2, (PTU | IEN | M0)}, /*  EMMC_DATA2 */
	{EMMC_DATA3, (PTU | IEN | M0)}, /*  EMMC_DATA3 */
	{EMMC_DATA4, (PTU | IEN | M0)}, /*  EMMC_DATA4 */
	{EMMC_DATA5, (PTU | IEN | M0)}, /*  EMMC_DATA5 */
	{EMMC_DATA6, (PTU | IEN | M0)}, /*  EMMC_DATA6 */
	{EMMC_DATA7, (PTU | IEN | M0)}, /*  EMMC_DATA7 */
	{SDCARD_CLK, (PTU | IEN | M0)}, /*  SDCARD_CLK  */
	{SDCARD_CMD, (PTU | IEN | M0)}, /*  SDCARD_CMD  */
	{SDCARD_DATA0, (PTU | IEN | M0)}, /*  SDCARD_DATA0*/
	{SDCARD_DATA1, (PTU | IEN | M0)}, /*  SDCARD_DATA1*/
	{SDCARD_DATA2, (PTU | IEN | M0)}, /*  SDCARD_DATA2*/
	{SDCARD_DATA3, (PTU | IEN | M0)}, /*  SDCARD_DATA3*/
	{UART3_RX_IRRX, (PTU | IEN | M0)}, /*  UART3_RX_IRRX    */
	{UART3_TX_IRTX, (M0)},    /*  UART3_TX_IRTX    */
	{USBB1_HSIC_STROBE, (PTU | IEN | M0)},    /*  USBB1_HSIC_STROBE */
	{USBB1_HSIC_DATA, (PTU | IEN | M0)},    /*  USBB1_HSIC_DATA */
	{USBB2_HSIC_STROBE, (PTU | IEN | M0)},    /*  USBB2_HSIC_STROBE */
	{USBB2_HSIC_DATA, (PTU | IEN | M0)},    /*  USBB2_HSIC_DATA  */
	{USBB3_HSIC_STROBE, (PTU | IEN | M0)},    /*  USBB3_HSIC_STROBE*/
	{USBB3_HSIC_DATA, (PTU | IEN | M0)},    /*  USBB3_HSIC_DATA  */
	{USBD0_HS_DP, (IEN | M0)},	/*  USBD0_HS_DP */
	{USBD0_HS_DM, (IEN | M0)},	/*  USBD0_HS_DM */
	{USBD0_SS_RX, (IEN | M0)},	/*  USBD0_SS_RX */
	{I2C5_SCL, (IEN | M0)}, /* I2C5_SCL */
	{I2C5_SDA, (IEN | M0)}, /* I2C5_SDA */
	{HSI2_ACWAKE, (PTU | M6)},    /*  HSI2_ACWAKE */
	{HSI2_CAFLAG, (PTU | M6)},    /*  HSI2_CAFLAG */
};

const struct pad_conf_entry wkup_padconf_array_essential[] = {

	{SR_PMIC_SCL, (PTU | IEN | M0)}, /* SR_PMIC_SCL */
	{SR_PMIC_SDA, (PTU | IEN | M0)}, /* SR_PMIC_SDA */
	{SYS_32K, (IEN | M0)}, /*  SYS_32K     */
	{FREF_CLK1_OUT, (PTD | IEN | M0)},    /*  FREF_CLK1_OUT  */

};

const struct pad_conf_entry core_padconf_array_non_essential[] = {

	{C2C_DATAIN0, (IEN | M0)},    /*  C2C_DATAIN0   */
	{C2C_DATAIN1, (IEN | M0)},    /*  C2C_DATAIN1   */
	{C2C_DATAIN2, (IEN | M0)},    /*  C2C_DATAIN2   */
	{C2C_DATAIN3, (IEN | M0)},    /*  C2C_DATAIN3   */
	{C2C_DATAIN4, (IEN | M0)},    /*  C2C_DATAIN4   */
	{C2C_DATAIN5, (IEN | M0)},    /*  C2C_DATAIN5   */
	{C2C_DATAIN6, (IEN | M0)},    /*  C2C_DATAIN6   */
	{C2C_DATAIN7, (IEN | M0)},    /*  C2C_DATAIN7   */
	{C2C_CLKIN1,  (IEN | M0)},    /*  C2C_CLKIN1    */
	{C2C_CLKIN0,  (IEN | M0)},    /*  C2C_CLKIN0    */
	{C2C_CLKOUT0, (M0)},    /*  C2C_CLKOUT0   */
	{C2C_CLKOUT1, (M0)},    /*  C2C_CLKOUT1   */
	{C2C_DATAOUT0, (M0)},    /*  C2C_DATAOUT0  */
	{C2C_DATAOUT1, (M0)},    /*  C2C_DATAOUT1  */
	{C2C_DATAOUT2, (M0)},    /*  C2C_DATAOUT2  */
	{C2C_DATAOUT3, (M0)},    /*  C2C_DATAOUT3  */
	{C2C_DATAOUT4, (M0)},    /*  C2C_DATAOUT4  */
	{C2C_DATAOUT5, (M0)},    /*  C2C_DATAOUT5  */
	{C2C_DATAOUT6, (M0)},    /*  C2C_DATAOUT6  */
	{C2C_DATAOUT7, (M0)},    /*  C2C_DATAOUT7  */
	{C2C_DATA8, (IEN | M0)},    /*  C2C_DATA8     */
	{C2C_DATA9, (IEN | M0)},    /*  C2C_DATA9     */
	{C2C_DATA10, (IEN | M0)},    /*  C2C_DATA10    */
	{C2C_DATA11, (IEN | M0)},    /*  C2C_DATA11    */
	{C2C_DATA12, (IEN | M0)},    /*  C2C_DATA12    */
	{C2C_DATA13, (IEN | M0)},    /*  C2C_DATA13    */
	{C2C_DATA14, (IEN | M0)},    /*  C2C_DATA14    */
	{C2C_DATA15, (IEN | M0)},    /*  C2C_DATA15    */
	{LLIB_WAKEREQOUT, (PTU | IEN | M6)},    /*  GPIO2_32      */
	{LLIA_WAKEREQOUT, (M1)},    /*  C2C_WAKEREQOUT */
	{HSI1_ACREADY, (PTD | M6)},    /*  GPIO3_64  */
	{HSI1_CAREADY, (PTD | M6)},    /*  GPIO3_65  */
	{HSI1_ACWAKE,  (PTD | IEN | M6)},    /*  GPIO3_66  */
	{HSI1_CAWAKE,  (PTU | IEN | M6)},    /*  GPIO3_67  */
	{HSI1_ACFLAG,  (PTD | IEN | M6)},    /*  GPIO3_68  */
	{HSI1_ACDATA,  (PTD | M6)},    /*  GPIO3_69  */
	{HSI1_CAFLAG,  (M6)},    /*  GPIO3_70  */
	{HSI1_CADATA,  (M6)},    /*  GPIO3_71  */
	{UART1_TX, (M0)},    /*  UART1_TX  */
	{UART1_CTS, (PTU | IEN | M0)},    /*  UART1_CTS */
	{UART1_RX, (PTU | IEN | M0)},    /*  UART1_RX  */
	{UART1_RTS, (M0)},    /*  UART1_RTS */
	{HSI2_CAREADY, (IEN | M0)},    /*  HSI2_CAREADY */
	{HSI2_ACREADY, (OFF_EN | M0)},    /*  HSI2_ACREADY */
	{HSI2_CAWAKE, (IEN | PTD | M0)},    /*  HSI2_CAWAKE  */
	{HSI2_ACWAKE, (M0)},    /*  HSI2_ACWAKE  */
	{HSI2_CAFLAG, (IEN | PTD | M0)},    /*  HSI2_CAFLAG  */
	{HSI2_CADATA, (IEN | PTD | M0)},    /*  HSI2_CADATA  */
	{HSI2_ACFLAG, (M0)},    /*  HSI2_ACFLAG  */
	{HSI2_ACDATA, (M0)},    /*  HSI2_ACDATA  */
	{UART2_RTS, (IEN | M1)},    /*  MCSPI3_SOMI  */
	{UART2_CTS, (IEN | M1)},    /*  MCSPI3_CS0   */
	{UART2_RX, (IEN | M1)},    /*  MCSPI3_SIMO  */
	{UART2_TX, (IEN | M1)},    /*  MCSPI3_CLK   */
	{TIMER10_PWM_EVT, (IEN | M0)},    /*  TIMER10_PWM_EVT  */
	{DSIPORTA_TE0, (IEN | M0)},    /*  DSIPORTA_TE0     */
	{DSIPORTA_LANE0X, (IEN | M0)},    /*  DSIPORTA_LANE0X  */
	{DSIPORTA_LANE0Y, (IEN | M0)},    /*  DSIPORTA_LANE0Y  */
	{DSIPORTA_LANE1X, (IEN | M0)},    /*  DSIPORTA_LANE1X  */
	{DSIPORTA_LANE1Y, (IEN | M0)},    /*  DSIPORTA_LANE1Y  */
	{DSIPORTA_LANE2X, (IEN | M0)},    /*  DSIPORTA_LANE2X  */
	{DSIPORTA_LANE2Y, (IEN | M0)},    /*  DSIPORTA_LANE2Y  */
	{DSIPORTA_LANE3X, (IEN | M0)},    /*  DSIPORTA_LANE3X  */
	{DSIPORTA_LANE3Y, (IEN | M0)},    /*  DSIPORTA_LANE3Y  */
	{DSIPORTA_LANE4X, (IEN | M0)},    /*  DSIPORTA_LANE4X  */
	{DSIPORTA_LANE4Y, (IEN | M0)},    /*  DSIPORTA_LANE4Y  */
	{TIMER9_PWM_EVT, (IEN | M0)},    /*  TIMER9_PWM_EVT   */
	{DSIPORTC_TE0, (IEN | M0)},    /*  DSIPORTC_TE0     */
	{DSIPORTC_LANE0X, (IEN | M0)},    /*  DSIPORTC_LANE0X  */
	{DSIPORTC_LANE0Y, (IEN | M0)},    /*  DSIPORTC_LANE0Y  */
	{DSIPORTC_LANE1X, (IEN | M0)},    /*  DSIPORTC_LANE1X  */
	{DSIPORTC_LANE1Y, (IEN | M0)},    /*  DSIPORTC_LANE1Y  */
	{DSIPORTC_LANE2X, (IEN | M0)},    /*  DSIPORTC_LANE2X  */
	{DSIPORTC_LANE2Y, (IEN | M0)},    /*  DSIPORTC_LANE2Y  */
	{DSIPORTC_LANE3X, (IEN | M0)},    /*  DSIPORTC_LANE3X  */
	{DSIPORTC_LANE3Y, (IEN | M0)},    /*  DSIPORTC_LANE3Y  */
	{DSIPORTC_LANE4X, (IEN | M0)},    /*  DSIPORTC_LANE4X  */
	{DSIPORTC_LANE4Y, (IEN | M0)},    /*  DSIPORTC_LANE4Y  */
	{RFBI_HSYNC0, (M4)},    /*  KBD_COL5   */
	{RFBI_TE_VSYNC0, (PTD | M6)},    /*  GPIO6_161  */
	{RFBI_RE, (M4)},    /*  KBD_COL4   */
	{RFBI_A0, (PTD | IEN | M6)},    /*  GPIO6_165  */
	{RFBI_DATA8, (M4)},    /*  KBD_COL3   */
	{RFBI_DATA9, (PTD | M6)},    /*  GPIO6_175  */
	{RFBI_DATA10, (PTD | M6)},    /*  GPIO6_176  */
	{RFBI_DATA11, (PTD | M6)},    /*  GPIO6_177  */
	{RFBI_DATA12, (PTD | M6)},    /*  GPIO6_178  */
	{RFBI_DATA13, (PTU | IEN | M6)},    /*  GPIO6_179  */
	{RFBI_DATA14, (M4)},    /*  KBD_COL7   */
	{RFBI_DATA15, (M4)},    /*  KBD_COL6   */
	{GPIO6_182, (M6)},    /*  GPIO6_182  */
	{GPIO6_183, (PTD | M6)},    /*  GPIO6_183  */
	{GPIO6_184, (M4)},    /*  KBD_COL2   */
	{GPIO6_185, (PTD | IEN | M6)},    /*  GPIO6_185  */
	{GPIO6_186, (PTD | M6)},    /*  GPIO6_186  */
	{GPIO6_187, (PTU | IEN | M4)},    /*  KBD_ROW2   */
	{RFBI_DATA0, (PTD | M6)},    /*  GPIO6_166  */
	{RFBI_DATA1, (PTD | M6)},    /*  GPIO6_167  */
	{RFBI_DATA2, (PTD | M6)},    /*  GPIO6_168  */
	{RFBI_DATA3, (PTD | IEN | M6)},    /*  GPIO6_169  */
	{RFBI_DATA4, (IEN | M6)},    /*  GPIO6_170  */
	{RFBI_DATA5, (IEN | M6)},    /*  GPIO6_171  */
	{RFBI_DATA6, (PTD | M6)},    /*  GPIO6_172  */
	{RFBI_DATA7, (PTD | M6)},    /*  GPIO6_173  */
	{RFBI_CS0, (PTD | IEN | M6)},    /*  GPIO6_163  */
	{RFBI_WE, (PTD | M6)},    /*  GPIO6_162  */
	{MCSPI2_CS0, (M0)},    /*  MCSPI2_CS0 */
	{MCSPI2_CLK, (IEN | M0)},    /*  MCSPI2_CLK */
	{MCSPI2_SIMO, (IEN | M0)},    /*  MCSPI2_SIMO*/
	{MCSPI2_SOMI, (PTU | IEN | M0)},    /*  MCSPI2_SOMI*/
	{I2C4_SCL, (IEN | M0)},    /*  I2C4_SCL   */
	{I2C4_SDA, (IEN | M0)},    /*  I2C4_SDA   */
	{HDMI_CEC, (IEN | M0)},    /*  HDMI_CEC   */
	{HDMI_HPD, (PTD | IEN | M0)},    /*  HDMI_HPD   */
	{HDMI_DDC_SCL, (IEN | M0)},    /*  HDMI_DDC_SCL */
	{HDMI_DDC_SDA, (IEN | M0)},    /*  HDMI_DDC_SDA */
	{CSIPORTA_LANE0X, (IEN | M0)},    /*  CSIPORTA_LANE0X  */
	{CSIPORTA_LANE0Y, (IEN | M0)},    /*  CSIPORTA_LANE0Y  */
	{CSIPORTA_LANE1Y, (IEN | M0)},    /*  CSIPORTA_LANE1Y  */
	{CSIPORTA_LANE1X, (IEN | M0)},    /*  CSIPORTA_LANE1X  */
	{CSIPORTA_LANE2Y, (IEN | M0)},    /*  CSIPORTA_LANE2Y  */
	{CSIPORTA_LANE2X, (IEN | M0)},    /*  CSIPORTA_LANE2X  */
	{CSIPORTA_LANE3X, (IEN | M0)},    /*  CSIPORTA_LANE3X  */
	{CSIPORTA_LANE3Y, (IEN | M0)},    /*  CSIPORTA_LANE3Y  */
	{CSIPORTA_LANE4X, (IEN | M0)},    /*  CSIPORTA_LANE4X  */
	{CSIPORTA_LANE4Y, (IEN | M0)},    /*  CSIPORTA_LANE4Y  */
	{CSIPORTB_LANE0X, (IEN | M0)},    /*  CSIPORTB_LANE0X  */
	{CSIPORTB_LANE0Y, (IEN | M0)},    /*  CSIPORTB_LANE0Y  */
	{CSIPORTB_LANE1Y, (IEN | M0)},    /*  CSIPORTB_LANE1Y  */
	{CSIPORTB_LANE1X, (IEN | M0)},    /*  CSIPORTB_LANE1X  */
	{CSIPORTB_LANE2Y, (IEN | M0)},    /*  CSIPORTB_LANE2Y  */
	{CSIPORTB_LANE2X, (IEN | M0)},    /*  CSIPORTB_LANE2X  */
	{CSIPORTC_LANE0Y, (IEN | M0)},    /*  CSIPORTC_LANE0Y  */
	{CSIPORTC_LANE0X, (IEN | M0)},    /*  CSIPORTC_LANE0X  */
	{CSIPORTC_LANE1Y, (IEN | M0)},    /*  CSIPORTC_LANE1Y  */
	{CSIPORTC_LANE1X, (IEN | M0)},    /*  CSIPORTC_LANE1X  */
	{CAM_SHUTTER, (M0)},    /*  CAM_SHUTTER      */
	{CAM_STROBE, (M0)},    /*  CAM_STROBE       */
	{CAM_GLOBALRESET, (IEN | M0)},    /*  CAM_GLOBALRESET  */
	{TIMER11_PWM_EVT, (PTD | M6)},    /*  GPIO8_227  */
	{TIMER5_PWM_EVT, (PTD | M6)},    /*  GPIO8_228  */
	{TIMER6_PWM_EVT, (PTD | M6)},    /*  GPIO8_229  */
	{TIMER8_PWM_EVT,      (PTU | M6)},    /*  GPIO8_230  */
	{I2C3_SCL, (IEN | M0)},    /*  I2C3_SCL   */
	{I2C3_SDA, (IEN | M0)},    /*  I2C3_SDA   */
	{GPIO8_233, (IEN | M2)},    /*  TIMER8_PWM_EVT   */
	{ABE_CLKS, (IEN | M0)},    /*  ABE_CLKS  */
	{ABEDMIC_DIN1, (IEN | M0)},    /*  ABEDMIC_DIN1 */
	{ABEDMIC_DIN2, (IEN | M0)},    /*  ABEDMIC_DIN2 */
	{ABEDMIC_DIN3, (IEN | M0)},    /*  ABEDMIC_DIN3 */
	{ABEDMIC_CLK1, (M0)},    /*  ABEDMIC_CLK1 */
	{ABEDMIC_CLK2, (IEN | M1)},    /*  ABEMCBSP1_FSX */
	{ABEDMIC_CLK3, (M1)},    /*  ABEMCBSP1_DX  */
	{ABESLIMBUS1_CLOCK, (IEN | M1)},    /*  ABEMCBSP1_CLKX   */
	{ABESLIMBUS1_DATA, (IEN | M1)},    /*  ABEMCBSP1_DR */
	{ABEMCBSP2_DR, (IEN | M0)},    /*  ABEMCBSP2_DR */
	{ABEMCBSP2_DX, (M0)},    /*  ABEMCBSP2_DX */
	{ABEMCBSP2_FSX, (IEN | M0)},    /*  ABEMCBSP2_FSX  */
	{ABEMCBSP2_CLKX, (IEN | M0)},    /*  ABEMCBSP2_CLKX */
	{ABEMCPDM_UL_DATA, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},    /*  ABEMCPDM_UL_DATA */
	{ABEMCPDM_DL_DATA, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},    /*  ABEMCPDM_DL_DATA */
	{ABEMCPDM_FRAME, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},    /*  ABEMCPDM_FRAME   */
	{ABEMCPDM_LB_CLK, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},    /*  ABEMCPDM_LB_CLK  */
	{WLSDIO_CLK, (PTU | IEN | M0)},    /*  WLSDIO_CLK  */
	{WLSDIO_CMD, (PTU | IEN | M0)},    /*  WLSDIO_CMD  */
	{WLSDIO_DATA0, (PTU | IEN | M0)},    /*  WLSDIO_DATA0*/
	{WLSDIO_DATA1, (PTU | IEN | M0)},    /*  WLSDIO_DATA1*/
	{WLSDIO_DATA2, (PTU | IEN | M0)},    /*  WLSDIO_DATA2*/
	{WLSDIO_DATA3, (PTU | IEN | M0)},    /*  WLSDIO_DATA3*/
	{UART5_RX, (PTU | IEN | M0)},    /*  UART5_RX    */
	{UART5_TX, (M0)},    /*  UART5_TX    */
	{UART5_CTS, (PTU | IEN | M0)},    /*  UART5_CTS   */
	{UART5_RTS, (M0)},    /*  UART5_RTS   */
	{I2C2_SCL, (IEN | M0)},    /*  I2C2_SCL    */
	{I2C2_SDA, (IEN | M0)},    /*  I2C2_SDA    */
	{MCSPI1_CLK, (M6)},    /*  GPIO5_140   */
	{MCSPI1_SOMI, (IEN | M6)},    /*  GPIO5_141   */
	{MCSPI1_SIMO, (PTD | M6)},    /*  GPIO5_142   */
	{MCSPI1_CS0, (PTD | M6)},    /*  GPIO5_143   */
	{MCSPI1_CS1, (PTD | IEN | M6)},    /*  GPIO5_144   */
	{I2C5_SCL, (IEN | M0)},    /*  I2C5_SCL    */
	{I2C5_SDA, (IEN | M0)},    /*  I2C5_SDA    */
	{PERSLIMBUS2_CLOCK, (PTD | M6)},    /*  GPIO5_145   */
	{PERSLIMBUS2_DATA, (PTD | IEN | M6)},    /*  GPIO5_146   */
	{UART6_TX, (PTU | IEN | M6)},    /*  GPIO5_149   */
	{UART6_RX, (PTU | IEN | M6)},    /*  GPIO5_150   */
	{UART6_CTS, (PTU | IEN | M6)},    /*  GPIO5_151   */
	{UART6_RTS, (PTU | M0)},    /*  UART6_RTS   */
	{UART3_CTS_RCTX, (PTU | IEN | M6)},    /*  GPIO5_153   */
	{UART3_RTS_IRSD, (PTU | IEN | M1)},    /*  HDQ_SIO     */
	{I2C1_PMIC_SCL, (PTU | IEN | M0)},    /*  I2C1_PMIC_SCL  */
	{I2C1_PMIC_SDA, (PTU | IEN | M0)},    /*  I2C1_PMIC_SDA  */

};

const struct pad_conf_entry wkup_padconf_array_non_essential[] = {

/*
 * This pad keeps C2C Module always enabled.
 * Putting this in safe mode do not cause the issue.
 * C2C driver could enable this mux setting if needed.
 */
	{LLIA_WAKEREQIN, (M7)},    /*  SAFE MODE  */
	{LLIB_WAKEREQIN, (M7)},    /*  SAFE MODE  */
	{DRM_EMU0, (PTU | IEN | M0)},    /*  DRM_EMU0    */
	{DRM_EMU1, (PTU | IEN | M0)},    /*  DRM_EMU1    */
	{JTAG_NTRST, (IEN | M0)},    /*  JTAG_NTRST  */
	{JTAG_TCK, (IEN | M0)},    /*  JTAG_TCK    */
	{JTAG_RTCK, (M0)},    /*  JTAG_RTCK   */
	{JTAG_TMSC, (IEN | M0)},    /*  JTAG_TMSC   */
	{JTAG_TDI, (IEN | M0)},    /*  JTAG_TDI    */
	{JTAG_TDO, (M0)},    /*  JTAG_TDO    */
	{FREF_CLK_IOREQ, (IEN | M0)},    /*  FREF_CLK_IOREQ */
	{FREF_CLK0_OUT, (M0)},    /*  FREF_CLK0_OUT  */
	{FREF_CLK1_OUT, (M0)},    /*  FREF_CLK1_OUT  */
	{FREF_CLK2_OUT, (M0)},    /*  FREF_CLK2_OUT  */
	{FREF_CLK2_REQ, (PTU | IEN | M6)},    /*  GPIO1_WK9      */
	{FREF_CLK1_REQ, (PTD | IEN | M6)},    /*  GPIO1_WK8      */
	{SYS_NRESPWRON, (IEN | M0)},    /*  SYS_NRESPWRON  */
	{SYS_NRESWARM, (PTU | IEN | M0)},    /*  SYS_NRESWARM   */
	{SYS_PWR_REQ, (M0)},    /*  SYS_PWR_REQ    */
	{SYS_NIRQ1, (PTU | IEN | M0)},    /*  SYS_NIRQ1      */
	{SYS_NIRQ2, (PTU | IEN | M0)},    /*  SYS_NIRQ2      */
	{SYS_BOOT0, (IEN | M0)},    /*  SYS_BOOT0      */
	{SYS_BOOT1, (IEN | M0)},    /*  SYS_BOOT1      */
	{SYS_BOOT2, (IEN | M0)},    /*  SYS_BOOT2      */
	{SYS_BOOT3, (IEN | M0)},    /*  SYS_BOOT3      */
	{SYS_BOOT4, (IEN | M0)},    /*  SYS_BOOT4      */
	{SYS_BOOT5, (IEN | M0)},    /*  SYS_BOOT5      */

};

#endif /* _EVM4430_MUX_DATA_H */
