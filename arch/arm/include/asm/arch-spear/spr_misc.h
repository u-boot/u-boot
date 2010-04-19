/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
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

#ifndef _SPR_MISC_H
#define _SPR_MISC_H

struct misc_regs {
	u32 auto_cfg_reg;	/* 0x0 */
	u32 armdbg_ctr_reg;	/* 0x4 */
	u32 pll1_cntl;		/* 0x8 */
	u32 pll1_frq;		/* 0xc */
	u32 pll1_mod;		/* 0x10 */
	u32 pll2_cntl;		/* 0x14 */
	u32 pll2_frq;		/* 0x18 */
	u32 pll2_mod;		/* 0x1C */
	u32 pll_ctr_reg;	/* 0x20 */
	u32 amba_clk_cfg;	/* 0x24 */
	u32 periph_clk_cfg;	/* 0x28 */
	u32 periph1_clken;	/* 0x2C */
	u32 periph2_clken;	/* 0x30 */
	u32 ras_clken;		/* 0x34 */
	u32 periph1_rst;	/* 0x38 */
	u32 periph2_rst;	/* 0x3C */
	u32 ras_rst;		/* 0x40 */
	u32 prsc1_clk_cfg;	/* 0x44 */
	u32 prsc2_clk_cfg;	/* 0x48 */
	u32 prsc3_clk_cfg;	/* 0x4C */
	u32 amem_cfg_ctrl;	/* 0x50 */
	u32 port_cfg_ctrl;	/* 0x54 */
	u32 reserved_1;		/* 0x58 */
	u32 clcd_synth_clk;	/* 0x5C */
	u32 irda_synth_clk;	/* 0x60 */
	u32 uart_synth_clk;	/* 0x64 */
	u32 gmac_synth_clk;	/* 0x68 */
	u32 ras_synth1_clk;	/* 0x6C */
	u32 ras_synth2_clk;	/* 0x70 */
	u32 ras_synth3_clk;	/* 0x74 */
	u32 ras_synth4_clk;	/* 0x78 */
	u32 arb_icm_ml1;	/* 0x7C */
	u32 arb_icm_ml2;	/* 0x80 */
	u32 arb_icm_ml3;	/* 0x84 */
	u32 arb_icm_ml4;	/* 0x88 */
	u32 arb_icm_ml5;	/* 0x8C */
	u32 arb_icm_ml6;	/* 0x90 */
	u32 arb_icm_ml7;	/* 0x94 */
	u32 arb_icm_ml8;	/* 0x98 */
	u32 arb_icm_ml9;	/* 0x9C */
	u32 dma_src_sel;	/* 0xA0 */
	u32 uphy_ctr_reg;	/* 0xA4 */
	u32 gmac_ctr_reg;	/* 0xA8 */
	u32 port_bridge_ctrl;	/* 0xAC */
	u32 reserved_2[4];	/* 0xB0--0xBC */
	u32 prc1_ilck_ctrl_reg;	/* 0xC0 */
	u32 prc2_ilck_ctrl_reg;	/* 0xC4 */
	u32 prc3_ilck_ctrl_reg;	/* 0xC8 */
	u32 prc4_ilck_ctrl_reg;	/* 0xCC */
	u32 prc1_intr_ctrl_reg;	/* 0xD0 */
	u32 prc2_intr_ctrl_reg;	/* 0xD4 */
	u32 prc3_intr_ctrl_reg;	/* 0xD8 */
	u32 prc4_intr_ctrl_reg;	/* 0xDC */
	u32 powerdown_cfg_reg;	/* 0xE0 */
	u32 ddr_1v8_compensation;	/* 0xE4  */
	u32 ddr_2v5_compensation;	/* 0xE8 */
	u32 core_3v3_compensation;	/* 0xEC */
	u32 ddr_pad;		/* 0xF0 */
	u32 bist1_ctr_reg;	/* 0xF4 */
	u32 bist2_ctr_reg;	/* 0xF8 */
	u32 bist3_ctr_reg;	/* 0xFC */
	u32 bist4_ctr_reg;	/* 0x100 */
	u32 bist5_ctr_reg;	/* 0x104 */
	u32 bist1_rslt_reg;	/* 0x108 */
	u32 bist2_rslt_reg;	/* 0x10C */
	u32 bist3_rslt_reg;	/* 0x110 */
	u32 bist4_rslt_reg;	/* 0x114 */
	u32 bist5_rslt_reg;	/* 0x118 */
	u32 syst_error_reg;	/* 0x11C */
	u32 reserved_3[0x1FB8];	/* 0x120--0x7FFC */
	u32 ras_gpp1_in;	/* 0x8000 */
	u32 ras_gpp2_in;	/* 0x8004 */
	u32 ras_gpp1_out;	/* 0x8008 */
	u32 ras_gpp2_out;	/* 0x800C */
};

/* AUTO_CFG_REG value */
#define MISC_SOCCFGMSK                  0x0000003F
#define MISC_SOCCFG30                   0x0000000C
#define MISC_SOCCFG31                   0x0000000D
#define MISC_NANDDIS			0x00020000

/* PERIPH_CLK_CFG value */
#define MISC_GPT3SYNTH			0x00000400
#define MISC_GPT4SYNTH			0x00000800

/* PRSC_CLK_CFG value */
/*
 * Fout = Fin / (2^(N+1) * (M + 1))
 */
#define MISC_PRSC_N_1			0x00001000
#define MISC_PRSC_M_9			0x00000009
#define MISC_PRSC_N_4			0x00004000
#define MISC_PRSC_M_399			0x0000018F
#define MISC_PRSC_N_6			0x00006000
#define MISC_PRSC_M_2593		0x00000A21
#define MISC_PRSC_M_124			0x0000007C
#define MISC_PRSC_CFG			(MISC_PRSC_N_1 | MISC_PRSC_M_9)

/* PERIPH1_CLKEN, PERIPH1_RST value */
#define MISC_USBDENB			0x01000000

#endif
