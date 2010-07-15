/*
 * (C) Copyright 2010
 * Texas Instruments Incorporated, <www.ti.com>
 *
 *	Balaji Krishnamoorthy	<balajitk@ti.com>
 *	Aneesh V		<aneesh@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _SDP_H_
#define _SDP_H_

#include <asm/io.h>
#include <asm/arch/mux_omap4.h>

const struct pad_conf_entry core_padconf_array[] = {
	{GPMC_AD0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* sdmmc2_dat0 */
	{GPMC_AD1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, 	/* sdmmc2_dat1 */
	{GPMC_AD2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* sdmmc2_dat2 */
	{GPMC_AD3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* sdmmc2_dat3 */
	{GPMC_AD4, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* sdmmc2_dat4 */
	{GPMC_AD5, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* sdmmc2_dat5 */
	{GPMC_AD6, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* sdmmc2_dat6 */
	{GPMC_AD7, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* sdmmc2_dat7 */
	{GPMC_AD8, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M3)},	/* gpio_32 */
	{GPMC_AD9, (PTU | IEN | M3)},					/* gpio_33 */
	{GPMC_AD10, (PTU | IEN | M3)},					/* gpio_34 */
	{GPMC_AD11, (PTU | IEN | M3)},					/* gpio_35 */
	{GPMC_AD12, (PTU | IEN | M3)},					/* gpio_36 */
	{GPMC_AD13, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3)},	/* gpio_37 */
	{GPMC_AD14, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3)},	/* gpio_38 */
	{GPMC_AD15, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3)},	/* gpio_39 */
	{GPMC_A16, (M3)},						/* gpio_40 */
	{GPMC_A17, (PTD | M3)},						/* gpio_41 */
	{GPMC_A18, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row6 */
	{GPMC_A19, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row7 */
	{GPMC_A20, (IEN | M3)},						/* gpio_44 */
	{GPMC_A21, (M3)},						/* gpio_45 */
	{GPMC_A22, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col6 */
	{GPMC_A23, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col7 */
	{GPMC_A24, (PTD | M3)},						/* gpio_48 */
	{GPMC_A25, (PTD | M3)},						/* gpio_49 */
	{GPMC_NCS0, (M3)},						/* gpio_50 */
	{GPMC_NCS1, (IEN | M3)},					/* gpio_51 */
	{GPMC_NCS2, (IEN | M3)},					/* gpio_52 */
	{GPMC_NCS3, (IEN | M3)},					/* gpio_53 */
	{GPMC_NWP, (M3)},						/* gpio_54 */
	{GPMC_CLK, (PTD | M3)},						/* gpio_55 */
	{GPMC_NADV_ALE, (M3)},						/* gpio_56 */
	{GPMC_NOE, (PTU | IEN | OFF_EN | OFF_OUT_PTD | M1)},		/* sdmmc2_clk */
	{GPMC_NWE, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* sdmmc2_cmd */
	{GPMC_NBE0_CLE, (M3)},						/* gpio_59 */
	{GPMC_NBE1, (PTD | M3)},					/* gpio_60 */
	{GPMC_WAIT0, (PTU | IEN | M3)},					/* gpio_61 */
	{GPMC_WAIT1, (IEN | M3)},					/* gpio_62 */
	{C2C_DATA11, (PTD | M3)},					/* gpio_100 */
	{C2C_DATA12, (M1)},						/* dsi1_te0 */
	{C2C_DATA13, (PTD | M3)},					/* gpio_102 */
	{C2C_DATA14, (M1)},						/* dsi2_te0 */
	{C2C_DATA15, (PTD | M3)},					/* gpio_104 */
	{HDMI_HPD, (M0)},						/* hdmi_hpd */
	{HDMI_CEC, (M0)},						/* hdmi_cec */
	{HDMI_DDC_SCL, (PTU | M0)},					/* hdmi_ddc_scl */
	{HDMI_DDC_SDA, (PTU | IEN | M0)},				/* hdmi_ddc_sda */
	{CSI21_DX0, (IEN | M0)},					/* csi21_dx0 */
	{CSI21_DY0, (IEN | M0)},					/* csi21_dy0 */
	{CSI21_DX1, (IEN | M0)},					/* csi21_dx1 */
	{CSI21_DY1, (IEN | M0)},					/* csi21_dy1 */
	{CSI21_DX2, (IEN | M0)},					/* csi21_dx2 */
	{CSI21_DY2, (IEN | M0)},					/* csi21_dy2 */
	{CSI21_DX3, (PTD | M7)},					/* csi21_dx3 */
	{CSI21_DY3, (PTD | M7)},					/* csi21_dy3 */
	{CSI21_DX4, (PTD | OFF_EN | OFF_PD | OFF_IN | M7)},		/* csi21_dx4 */
	{CSI21_DY4, (PTD | OFF_EN | OFF_PD | OFF_IN | M7)},		/* csi21_dy4 */
	{CSI22_DX0, (IEN | M0)},					/* csi22_dx0 */
	{CSI22_DY0, (IEN | M0)},					/* csi22_dy0 */
	{CSI22_DX1, (IEN | M0)},					/* csi22_dx1 */
	{CSI22_DY1, (IEN | M0)},					/* csi22_dy1 */
	{CAM_SHUTTER, (OFF_EN | OFF_PD | OFF_OUT_PTD | M0)},		/* cam_shutter */
	{CAM_STROBE, (OFF_EN | OFF_PD | OFF_OUT_PTD | M0)},		/* cam_strobe */
	{CAM_GLOBALRESET, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M3)},	/* gpio_83 */
	{USBB1_ULPITLL_CLK, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M4)},/* usbb1_ulpiphy_clk */
	{USBB1_ULPITLL_STP, (OFF_EN | OFF_OUT_PTD | M4)},		/* usbb1_ulpiphy_stp */
	{USBB1_ULPITLL_DIR, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dir */
	{USBB1_ULPITLL_NXT, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_nxt */
	{USBB1_ULPITLL_DAT0, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat0 */
	{USBB1_ULPITLL_DAT1, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat1 */
	{USBB1_ULPITLL_DAT2, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat2 */
	{USBB1_ULPITLL_DAT3, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat3 */
	{USBB1_ULPITLL_DAT4, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat4 */
	{USBB1_ULPITLL_DAT5, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat5 */
	{USBB1_ULPITLL_DAT6, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat6 */
	{USBB1_ULPITLL_DAT7, (IEN | OFF_EN | OFF_PD | OFF_IN | M4)},	/* usbb1_ulpiphy_dat7 */
	{USBB1_HSIC_DATA, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* usbb1_hsic_data */
	{USBB1_HSIC_STROBE, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* usbb1_hsic_strobe */
	{USBC1_ICUSB_DP, (IEN | M0)},					/* usbc1_icusb_dp */
	{USBC1_ICUSB_DM, (IEN | M0)},					/* usbc1_icusb_dm */
	{SDMMC1_CLK, (PTU | OFF_EN | OFF_OUT_PTD | M0)},		/* sdmmc1_clk */
	{SDMMC1_CMD, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc1_cmd */
	{SDMMC1_DAT0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc1_dat0 */
	{SDMMC1_DAT1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc1_dat1 */
	{SDMMC1_DAT2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc1_dat2 */
	{SDMMC1_DAT3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc1_dat3 */
	{SDMMC1_DAT4, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc1_dat4 */
	{SDMMC1_DAT5, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc1_dat5 */
	{SDMMC1_DAT6, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc1_dat6 */
	{SDMMC1_DAT7, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc1_dat7 */
	{ABE_MCBSP2_CLKX, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_mcbsp2_clkx */
	{ABE_MCBSP2_DR, (IEN | OFF_EN | OFF_OUT_PTD | M0)},		/* abe_mcbsp2_dr */
	{ABE_MCBSP2_DX, (OFF_EN | OFF_OUT_PTD | M0)},			/* abe_mcbsp2_dx */
	{ABE_MCBSP2_FSX, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_mcbsp2_fsx */
	{ABE_MCBSP1_CLKX, (IEN | M1)},					/* abe_slimbus1_clock */
	{ABE_MCBSP1_DR, (IEN | M1)},					/* abe_slimbus1_data */
	{ABE_MCBSP1_DX, (OFF_EN | OFF_OUT_PTD | M0)},			/* abe_mcbsp1_dx */
	{ABE_MCBSP1_FSX, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_mcbsp1_fsx */
	{ABE_PDM_UL_DATA, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_ul_data */
	{ABE_PDM_DL_DATA, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_dl_data */
	{ABE_PDM_FRAME, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_frame */
	{ABE_PDM_LB_CLK, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_pdm_lb_clk */
	{ABE_CLKS, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* abe_clks */
	{ABE_DMIC_CLK1, (M0)},						/* abe_dmic_clk1 */
	{ABE_DMIC_DIN1, (IEN | M0)},					/* abe_dmic_din1 */
	{ABE_DMIC_DIN2, (IEN | M0)},					/* abe_dmic_din2 */
	{ABE_DMIC_DIN3, (IEN | M0)},					/* abe_dmic_din3 */
	{UART2_CTS, (PTU | IEN | M0)},					/* uart2_cts */
	{UART2_RTS, (M0)},						/* uart2_rts */
	{UART2_RX, (PTU | IEN | M0)},					/* uart2_rx */
	{UART2_TX, (M0)},						/* uart2_tx */
	{HDQ_SIO, (M3)},						/* gpio_127 */
	{I2C1_SCL, (PTU | IEN | M0)},					/* i2c1_scl */
	{I2C1_SDA, (PTU | IEN | M0)},					/* i2c1_sda */
	{I2C2_SCL, (PTU | IEN | M0)},					/* i2c2_scl */
	{I2C2_SDA, (PTU | IEN | M0)},					/* i2c2_sda */
	{I2C3_SCL, (PTU | IEN | M0)},					/* i2c3_scl */
	{I2C3_SDA, (PTU | IEN | M0)},					/* i2c3_sda */
	{I2C4_SCL, (PTU | IEN | M0)},					/* i2c4_scl */
	{I2C4_SDA, (PTU | IEN | M0)},					/* i2c4_sda */
	{MCSPI1_CLK, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi1_clk */
	{MCSPI1_SOMI, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi1_somi */
	{MCSPI1_SIMO, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi1_simo */
	{MCSPI1_CS0, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* mcspi1_cs0 */
	{MCSPI1_CS1, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M3)},	/* mcspi1_cs1 */
	{MCSPI1_CS2, (PTU | OFF_EN | OFF_OUT_PTU | M3)},		/* gpio_139 */
	{MCSPI1_CS3, (PTU | IEN | M3)},					/* gpio_140 */
	{UART3_CTS_RCTX, (PTU | IEN | M0)},				/* uart3_tx */
	{UART3_RTS_SD, (M0)},						/* uart3_rts_sd */
	{UART3_RX_IRRX, (IEN | M0)},					/* uart3_rx */
	{UART3_TX_IRTX, (M0)},						/* uart3_tx */
	{SDMMC5_CLK, (PTU | IEN | OFF_EN | OFF_OUT_PTD | M0)},		/* sdmmc5_clk */
	{SDMMC5_CMD, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_cmd */
	{SDMMC5_DAT0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat0 */
	{SDMMC5_DAT1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat1 */
	{SDMMC5_DAT2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat2 */
	{SDMMC5_DAT3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* sdmmc5_dat3 */
	{MCSPI4_CLK, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi4_clk */
	{MCSPI4_SIMO, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi4_simo */
	{MCSPI4_SOMI, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* mcspi4_somi */
	{MCSPI4_CS0, (PTD | IEN | OFF_EN | OFF_PD | OFF_IN | M0)},	/* mcspi4_cs0 */
	{UART4_RX, (IEN | M0)},						/* uart4_rx */
	{UART4_TX, (M0)},						/* uart4_tx */
	{USBB2_ULPITLL_CLK, (IEN | M3)},				/* gpio_157 */
	{USBB2_ULPITLL_STP, (IEN | M5)},				/* dispc2_data23 */
	{USBB2_ULPITLL_DIR, (IEN | M5)},				/* dispc2_data22 */
	{USBB2_ULPITLL_NXT, (IEN | M5)},				/* dispc2_data21 */
	{USBB2_ULPITLL_DAT0, (IEN | M5)},				/* dispc2_data20 */
	{USBB2_ULPITLL_DAT1, (IEN | M5)},				/* dispc2_data19 */
	{USBB2_ULPITLL_DAT2, (IEN | M5)},				/* dispc2_data18 */
	{USBB2_ULPITLL_DAT3, (IEN | M5)},				/* dispc2_data15 */
	{USBB2_ULPITLL_DAT4, (IEN | M5)},				/* dispc2_data14 */
	{USBB2_ULPITLL_DAT5, (IEN | M5)},				/* dispc2_data13 */
	{USBB2_ULPITLL_DAT6, (IEN | M5)},				/* dispc2_data12 */
	{USBB2_ULPITLL_DAT7, (IEN | M5)},				/* dispc2_data11 */
	{USBB2_HSIC_DATA, (PTD | OFF_EN | OFF_OUT_PTU | M3)},		/* gpio_169 */
	{USBB2_HSIC_STROBE, (PTD | OFF_EN | OFF_OUT_PTU | M3)},		/* gpio_170 */
	{UNIPRO_TX0, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col0 */
	{UNIPRO_TY0, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col1 */
	{UNIPRO_TX1, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col2 */
	{UNIPRO_TY1, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col3 */
	{UNIPRO_TX2, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col4 */
	{UNIPRO_TY2, (OFF_EN | OFF_PD | OFF_IN | M1)},			/* kpd_col5 */
	{UNIPRO_RX0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row0 */
	{UNIPRO_RY0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row1 */
	{UNIPRO_RX1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row2 */
	{UNIPRO_RY1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row3 */
	{UNIPRO_RX2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row4 */
	{UNIPRO_RY2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)},	/* kpd_row5 */
	{USBA0_OTG_CE, (PTD | OFF_EN | OFF_PD | OFF_OUT_PTD | M0)},	/* usba0_otg_ce */
	{USBA0_OTG_DP, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* usba0_otg_dp */
	{USBA0_OTG_DM, (IEN | OFF_EN | OFF_PD | OFF_IN | M0)},		/* usba0_otg_dm */
	{FREF_CLK1_OUT, (M0)},						/* fref_clk1_out */
	{FREF_CLK2_OUT, (M0)},						/* fref_clk2_out */
	{SYS_NIRQ1, (PTU | IEN | M0)},					/* sys_nirq1 */
	{SYS_NIRQ2, (PTU | IEN | M0)},					/* sys_nirq2 */
	{SYS_BOOT0, (PTU | IEN | M3)},					/* gpio_184 */
	{SYS_BOOT1, (M3)},						/* gpio_185 */
	{SYS_BOOT2, (PTD | IEN | M3)},					/* gpio_186 */
	{SYS_BOOT3, (M3)},						/* gpio_187 */
	{SYS_BOOT4, (M3)},						/* gpio_188 */
	{SYS_BOOT5, (PTD | IEN | M3)},					/* gpio_189 */
	{DPM_EMU0, (IEN | M0)},						/* dpm_emu0 */
	{DPM_EMU1, (IEN | M0)},						/* dpm_emu1 */
	{DPM_EMU2, (IEN | M0)},						/* dpm_emu2 */
	{DPM_EMU3, (IEN | M5)},						/* dispc2_data10 */
	{DPM_EMU4, (IEN | M5)},						/* dispc2_data9 */
	{DPM_EMU5, (IEN | M5)},						/* dispc2_data16 */
	{DPM_EMU6, (IEN | M5)},						/* dispc2_data17 */
	{DPM_EMU7, (IEN | M5)},						/* dispc2_hsync */
	{DPM_EMU8, (IEN | M5)},						/* dispc2_pclk */
	{DPM_EMU9, (IEN | M5)},						/* dispc2_vsync */
	{DPM_EMU10, (IEN | M5)},					/* dispc2_de */
	{DPM_EMU11, (IEN | M5)},					/* dispc2_data8 */
	{DPM_EMU12, (IEN | M5)},					/* dispc2_data7 */
	{DPM_EMU13, (IEN | M5)},					/* dispc2_data6 */
	{DPM_EMU14, (IEN | M5)},					/* dispc2_data5 */
	{DPM_EMU15, (IEN | M5)},					/* dispc2_data4 */
	{DPM_EMU16, (M3)},						/* gpio_27 */
	{DPM_EMU17, (IEN | M5)},					/* dispc2_data2 */
	{DPM_EMU18, (IEN | M5)},					/* dispc2_data1 */
	{DPM_EMU19, (IEN | M5)},					/* dispc2_data0 */
};

const struct pad_conf_entry wkup_padconf_array[] = {
	{PAD0_SIM_IO, (IEN | M0)},		/* sim_io */
	{PAD1_SIM_CLK, (M0)},			/* sim_clk */
	{PAD0_SIM_RESET, (M0)},			/* sim_reset */
	{PAD1_SIM_CD, (PTU | IEN | M0)},	/* sim_cd */
	{PAD0_SIM_PWRCTRL, (M0)},		/* sim_pwrctrl */
	{PAD1_SR_SCL, (PTU | IEN | M0)},	/* sr_scl */
	{PAD0_SR_SDA, (PTU | IEN | M0)},	/* sr_sda */
	{PAD1_FREF_XTAL_IN, (M0)},		/* # */
	{PAD0_FREF_SLICER_IN, (M0)},		/* fref_slicer_in */
	{PAD1_FREF_CLK_IOREQ, (M0)},		/* fref_clk_ioreq */
	{PAD0_FREF_CLK0_OUT, (M2)},		/* sys_drm_msecure */
	{PAD1_FREF_CLK3_REQ, (PTU | IEN | M0)},	/* # */
	{PAD0_FREF_CLK3_OUT, (M0)},		/* fref_clk3_out */
	{PAD1_FREF_CLK4_REQ, (PTU | IEN | M0)},	/* # */
	{PAD0_FREF_CLK4_OUT, (M0)},		/* # */
	{PAD1_SYS_32K, (IEN | M0)},		/* sys_32k */
	{PAD0_SYS_NRESPWRON, (M0)},		/* sys_nrespwron */
	{PAD1_SYS_NRESWARM, (M0)},		/* sys_nreswarm */
	{PAD0_SYS_PWR_REQ, (PTU | M0)},		/* sys_pwr_req */
	{PAD1_SYS_PWRON_RESET, (M3)},		/* gpio_wk29 */
	{PAD0_SYS_BOOT6, (IEN | M3)},		/* gpio_wk9 */
	{PAD1_SYS_BOOT7, (IEN | M3)},		/* gpio_wk10 */
};

#endif

