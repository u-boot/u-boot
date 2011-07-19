/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
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

#ifndef _CLK_RST_H_
#define _CLK_RST_H_

/* Clock/Reset Controller (CLK_RST_CONTROLLER_) regs */
struct clk_rst_ctlr {
	uint crc_rst_src;		/* _RST_SOURCE_0,	0x00 */
	uint crc_rst_dev_l;		/* _RST_DEVICES_L_0,	0x04 */
	uint crc_rst_dev_h;		/* _RST_DEVICES_H_0,	0x08 */
	uint crc_rst_dev_u;		/* _RST_DEVICES_U_0,	0x0C */
	uint crc_clk_out_enb_l;		/* _CLK_OUT_ENB_L_0,	0x10 */
	uint crc_clk_out_enb_h;		/* _CLK_OUT_ENB_H_0,	0x14 */
	uint crc_clk_out_enb_u;		/* _CLK_OUT_ENB_U_0,	0x18 */
	uint crc_reserved0;		/* reserved_0,		0x1C */
	uint crc_cclk_brst_pol;		/* _CCLK_BURST_POLICY_0,0x20 */
	uint crc_super_cclk_div;	/* _SUPER_CCLK_DIVIDER_0,0x24 */
	uint crc_sclk_brst_pol;		/* _SCLK_BURST_POLICY_0, 0x28 */
	uint crc_super_sclk_div;	/* _SUPER_SCLK_DIVIDER_0,0x2C */
	uint crc_clk_sys_rate;		/* _CLK_SYSTEM_RATE_0,	0x30 */
	uint crc_prog_dly_clk;		/* _PROG_DLY_CLK_0,	0x34 */
	uint crc_aud_sync_clk_rate;	/* _AUDIO_SYNC_CLK_RATE_0,0x38 */
	uint crc_reserved1;		/* reserved_1,		0x3C */
	uint crc_cop_clk_skip_plcy;	/* _COP_CLK_SKIP_POLICY_0,0x40 */
	uint crc_clk_mask_arm;		/* _CLK_MASK_ARM_0,	0x44 */
	uint crc_misc_clk_enb;		/* _MISC_CLK_ENB_0,	0x48 */
	uint crc_clk_cpu_cmplx;		/* _CLK_CPU_CMPLX_0,	0x4C */
	uint crc_osc_ctrl;		/* _OSC_CTRL_0,		0x50 */
	uint crc_pll_lfsr;		/* _PLL_LFSR_0,		0x54 */
	uint crc_osc_freq_det;		/* _OSC_FREQ_DET_0,	0x58 */
	uint crc_osc_freq_det_stat;	/* _OSC_FREQ_DET_STATUS_0,0x5C */
	uint crc_reserved2[8];		/* reserved_2[8],	0x60-7C */

	uint crc_pllc_base;		/* _PLLC_BASE_0,	0x80 */
	uint crc_pllc_out;		/* _PLLC_OUT_0,		0x84 */
	uint crc_reserved3;		/* reserved_3,		0x88 */
	uint crc_pllc_misc;		/* _PLLC_MISC_0,	0x8C */

	uint crc_pllm_base;		/* _PLLM_BASE_0,	0x90 */
	uint crc_pllm_out;		/* _PLLM_OUT_0,		0x94 */
	uint crc_reserved4;		/* reserved_4,		0x98 */
	uint crc_pllm_misc;		/* _PLLM_MISC_0,	0x9C */

	uint crc_pllp_base;		/* _PLLP_BASE_0,	0xA0 */
	uint crc_pllp_outa;		/* _PLLP_OUTA_0,	0xA4 */
	uint crc_pllp_outb;		/* _PLLP_OUTB_0,	0xA8 */
	uint crc_pllp_misc;		/* _PLLP_MISC_0,	0xAC */

	uint crc_plla_base;		/* _PLLA_BASE_0,	0xB0 */
	uint crc_plla_out;		/* _PLLA_OUT_0,		0xB4 */
	uint crc_reserved5;		/* reserved_5,		0xB8 */
	uint crc_plla_misc;		/* _PLLA_MISC_0,	0xBC */

	uint crc_pllu_base;		/* _PLLU_BASE_0,	0xC0 */
	uint crc_reserved6;		/* _reserved_6,		0xC4 */
	uint crc_reserved7;		/* _reserved_7,		0xC8 */
	uint crc_pllu_misc;		/* _PLLU_MISC_0,	0xCC */

	uint crc_plld_base;		/* _PLLD_BASE_0,	0xD0 */
	uint crc_reserved8;		/* _reserved_8,		0xD4 */
	uint crc_reserved9;		/* _reserved_9,		0xD8 */
	uint crc_plld_misc;		/* _PLLD_MISC_0,	0xDC */

	uint crc_pllx_base;		/* _PLLX_BASE_0,	0xE0 */
	uint crc_pllx_misc;		/* _PLLX_MISC_0,	0xE4 */

	uint crc_plle_base;		/* _PLLE_BASE_0,	0xE8 */
	uint crc_plle_misc;		/* _PLLE_MISC_0,	0xEC */

	uint crc_plls_base;		/* _PLLS_BASE_0,	0xF0 */
	uint crc_plls_misc;		/* _PLLS_MISC_0,	0xF4 */
	uint crc_reserved10;		/* _reserved_10,	0xF8 */
	uint crc_reserved11;		/* _reserved_11,	0xFC */

	uint crc_clk_src_i2s1;		/*_I2S1_0,		0x100 */
	uint crc_clk_src_i2s2;		/*_I2S2_0,		0x104 */
	uint crc_clk_src_spdif_out;	/*_SPDIF_OUT_0,		0x108 */
	uint crc_clk_src_spdif_in;	/*_SPDIF_IN_0,		0x10C */
	uint crc_clk_src_pwm;		/*_PWM_0,		0x110 */
	uint crc_clk_src_spi1;		/*_SPI1_0,		0x114 */
	uint crc_clk_src_sbc2;		/*_SBC2_0,		0x118 */
	uint crc_clk_src_sbc3;		/*_SBC3_0,		0x11C */
	uint crc_clk_src_xio;		/*_XIO_0,		0x120 */
	uint crc_clk_src_i2c1;		/*_I2C1_0,		0x124 */
	uint crc_clk_src_dvc_i2c;	/*_DVC_I2C_0,		0x128 */
	uint crc_clk_src_twc;		/*_TWC_0,		0x12C */
	uint crc_reserved12;		/*			0x130 */
	uint crc_clk_src_sbc1;		/*_SBC1_0,		0x134 */
	uint crc_clk_src_disp1;		/*_DISP1_0,		0x138 */
	uint crc_clk_src_disp2;		/*_DISP2_0,		0x13C */
	uint crc_clk_src_cve;		/*_CVE_0,		0x140 */
	uint crc_clk_src_ide;		/*_IDE_0,		0x144 */
	uint crc_clk_src_vi;		/*_VI_0,		0x148 */
	uint crc_reserved13;		/*			0x14C */
	uint crc_clk_src_sdmmc1;	/*_SDMMC1_0,		0x150 */
	uint crc_clk_src_sdmmc2;	/*_SDMMC2_0,		0x154 */
	uint crc_clk_src_g3d;		/*_G3D_0,		0x158 */
	uint crc_clk_src_g2d;		/*_G2D_0,		0x15C */
	uint crc_clk_src_ndflash;	/*_NDFLASH_0,		0x160 */
	uint crc_clk_src_sdmmc4;	/*_SDMMC4_0,		0x164 */
	uint crc_clk_src_vfir;		/*_VFIR_0,		0x168 */
	uint crc_clk_src_epp;		/*_EPP_0,		0x16C */
	uint crc_clk_src_mp3;		/*_MPE_0,		0x170 */
	uint crc_clk_src_mipi;		/*_MIPI_0,		0x174 */
	uint crc_clk_src_uarta;		/*_UARTA_0,		0x178 */
	uint crc_clk_src_uartb;		/*_UARTB_0,		0x17C */
	uint crc_clk_src_host1x;	/*_HOST1X_0,		0x180 */
	uint crc_reserved14;		/*			0x184 */
	uint crc_clk_src_tvo;		/*_TVO_0,		0x188 */
	uint crc_clk_src_hdmi;		/*_HDMI_0,		0x18C */
	uint crc_reserved15;		/*			0x190 */
	uint crc_clk_src_tvdac;		/*_TVDAC_0,		0x194 */
	uint crc_clk_src_i2c2;		/*_I2C2_0,		0x198 */
	uint crc_clk_src_emc;		/*_EMC_0,		0x19C */
	uint crc_clk_src_uartc;		/*_UARTC_0,		0x1A0 */
	uint crc_reserved16;		/*			0x1A4 */
	uint crc_clk_src_vi_sensor;	/*_VI_SENSOR_0,		0x1A8 */
	uint crc_reserved17;		/*			0x1AC */
	uint crc_reserved18;		/*			0x1B0 */
	uint crc_clk_src_sbc4;		/*_SBC4_0,		0x1B4 */
	uint crc_clk_src_i2c3;		/*_I2C3_0,		0x1B8 */
	uint crc_clk_src_sdmmc3;	/*_SDMMC3_0,		0x1BC */
	uint crc_clk_src_uartd;		/*_UARTD_0,		0x1C0 */
	uint crc_clk_src_uarte;		/*_UARTE_0,		0x1C4 */
	uint crc_clk_src_vde;		/*_VDE_0,		0x1C8 */
	uint crc_clk_src_owr;		/*_OWR_0,		0x1CC */
	uint crc_clk_src_nor;		/*_NOR_0,		0x1D0 */
	uint crc_clk_src_csite;		/*_CSITE_0,		0x1D4 */
	uint crc_reserved19[9];		/*			0x1D8-1F8 */
	uint crc_clk_src_osc;		/*_OSC_0,		0x1FC */
	uint crc_reserved20[80];	/*			0x200-33C */
	uint crc_cpu_cmplx_set;		/* _CPU_CMPLX_SET_0,	0x340 */
	uint crc_cpu_cmplx_clr;		/* _CPU_CMPLX_CLR_0,	0x344 */
};

#define PLL_BYPASS		(1 << 31)
#define PLL_ENABLE		(1 << 30)
#define PLL_BASE_OVRRIDE	(1 << 28)
#define PLL_DIVP		(1 << 20)	/* post divider, b22:20 */
#define PLL_DIVM		0x0C		/* input divider, b4:0 */

#define SWR_UARTD_RST		(1 << 1)
#define CLK_ENB_UARTD		(1 << 1)
#define SWR_UARTA_RST		(1 << 6)
#define CLK_ENB_UARTA		(1 << 6)

#define SWR_CPU_RST		(1 << 0)
#define CLK_ENB_CPU		(1 << 0)
#define SWR_CSITE_RST		(1 << 9)
#define CLK_ENB_CSITE		(1 << 9)

#define SET_CPURESET0		(1 << 0)
#define SET_DERESET0		(1 << 4)
#define SET_DBGRESET0		(1 << 12)

#define SET_CPURESET1		(1 << 1)
#define SET_DERESET1		(1 << 5)
#define SET_DBGRESET1		(1 << 13)

#define CLR_CPURESET0		(1 << 0)
#define CLR_DERESET0		(1 << 4)
#define CLR_DBGRESET0		(1 << 12)

#define CLR_CPURESET1		(1 << 1)
#define CLR_DERESET1		(1 << 5)
#define CLR_DBGRESET1		(1 << 13)

#define CPU0_CLK_STP		(1 << 8)
#define CPU1_CLK_STP		(1 << 9)

#define CPCON			(1 << 8)

#define SWR_SDMMC4_RST		(1 << 15)
#define CLK_ENB_SDMMC4		(1 << 15)
#define SWR_SDMMC3_RST		(1 << 5)
#define CLK_ENB_SDMMC3		(1 << 5)

#endif	/* CLK_RST_H */
