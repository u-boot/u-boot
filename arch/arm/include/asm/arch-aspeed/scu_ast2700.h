/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) Aspeed Technology Inc.
 */
#ifndef _ASM_ARCH_SCU_AST2700_H
#define _ASM_ARCH_SCU_AST2700_H

#include <linux/types.h>

/* SoC0 SCU Register */
#define SCU_CPU_REVISION_ID_HW			GENMASK(23, 16)
#define SCU_CPU_REVISION_ID_EFUSE		GENMASK(15, 8)

#define SCU_CPU_HWSTRAP_DIS_RVAS		BIT(30)
#define SCU_CPU_HWSTRAP_DP_SRC			BIT(29)
#define SCU_CPU_HWSTRAP_DAC_SRC			BIT(28)
#define SCU_CPU_HWSTRAP_VRAM_SIZE		GENMASK(11, 10)
#define SCU_CPU_HWSTRAP_DIS_CPU			BIT(0)

#define SCU_CPU_MISC_DP_RESET_SRC		BIT(11)
#define SCU_CPU_MISC_XDMA_CLIENT_EN		BIT(4)
#define SCU_CPU_MISC_2D_CLIENT_EN		BIT(3)

#define SCU_CPU_RST_SSP				BIT(30)
#define SCU_CPU_RST_DPMCU			BIT(29)
#define SCU_CPU_RST_DP				BIT(28)
#define SCU_CPU_RST_XDMA1			BIT(26)
#define SCU_CPU_RST_XDMA0			BIT(25)
#define SCU_CPU_RST_EMMC			BIT(17)
#define SCU_CPU_RST_EN_DP_PCI			BIT(15)
#define SCU_CPU_RST_CRT				BIT(13)
#define SCU_CPU_RST_RVAS1			BIT(10)
#define SCU_CPU_RST_RVAS0			BIT(9)
#define SCU_CPU_RST_2D				BIT(7)
#define SCU_CPU_RST_VIDEO			BIT(6)
#define SCU_CPU_RST_SOC				BIT(5)
#define SCU_CPU_RST_DDRPHY			BIT(1)

#define SCU_CPU_RST2_VGA			BIT(12)
#define SCU_CPU_RST2_E2M1			BIT(11)
#define SCU_CPU_RST2_E2M0			BIT(10)
#define SCU_CPU_RST2_TSP			BIT(9)

#define SCU_CPU_VGA_FUNC_DAC_OUTPUT		GENMASK(11, 10)
#define SCU_CPU_VGA_FUNC_DP_OUTPUT		GENMASK(9, 8)
#define SCU_CPU_VGA_FUNC_DAC_DISABLE		BIT(7)

#define SCU_CPU_PCI_MISC0C_FB_SIZE		GENMASK(4, 0)

#define SCU_CPU_PCI_MISC70_EN_XHCI		BIT(3)
#define SCU_CPU_PCI_MISC70_EN_EHCI		BIT(2)
#define SCU_CPU_PCI_MISC70_EN_IPMI		BIT(1)
#define SCU_CPU_PCI_MISC70_EN_VGA		BIT(0)

#define SCU_CPU_HPLL_P				GENMASK(22, 19)
#define SCU_CPU_HPLL_N				GENMASK(18, 13)
#define SCU_CPU_HPLL_M				GENMASK(12, 0)

#define SCU_CPU_HPLL2_LOCK			BIT(31)
#define SCU_CPU_HPLL2_BWADJ			GENMASK(11, 0)

#define SCU_CPU_SSP_TSP_RESET_STS		BIT(8)
#define SCU_CPU_SSP_TSP_SRAM_SD			BIT(7)
#define SCU_CPU_SSP_TSP_SRAM_DSLP		BIT(6)
#define SCU_CPU_SSP_TSP_SRAM_SLP		BIT(5)
#define SCU_CPU_SSP_TSP_NIDEN			BIT(4)
#define SCU_CPU_SSP_TSP_DBGEN			BIT(3)
#define SCU_CPU_SSP_TSP_DBG_ENABLE		BIT(2)
#define SCU_CPU_SSP_TSP_RESET			BIT(1)
#define SCU_CPU_SSP_TSP_ENABLE			BIT(0)

/* SoC1 SCU Register */
#define SCU_IO_HWSTRAP_UFS			BIT(23)
#define SCU_IO_HWSTRAP_EMMC			BIT(11)
#define SCU_IO_HWSTRAP_SECBOOT			BIT(5)
#define SCU_IO_HWSTRAP_LTPI0_EN			BIT(3)
#define SCU_IO_HWSTRAP_LTPI1_EN			BIT(1)

/* CLK information */
#define CLKIN_25M 25000000UL

#define SCU_CPU_CLKGATE1_RVAS1			BIT(28)
#define SCU_CPU_CLKGATE1_RVAS0			BIT(25)
#define SCU_CPU_CLKGATE1_E2M1			BIT(19)
#define SCU_CPU_CLKGATE1_DP			BIT(18)
#define SCU_CPU_CLKGATE1_DAC			BIT(17)
#define SCU_CPU_CLKGATE1_E2M0			BIT(12)
#define SCU_CPU_CLKGATE1_VGA1			BIT(10)
#define SCU_CPU_CLKGATE1_VGA0			BIT(5)

/*
 * Clock divider/multiplier configuration struct.
 * For H-PLL and M-PLL the formula is
 * (Output Frequency) = CLKIN * ((M + 1) / (N + 1)) / (P + 1)
 * M - Numerator
 * N - Denumerator
 * P - Post Divider
 * They have the same layout in their control register.
 *
 */
union ast2700_pll_reg {
	u32 w;
	struct {
		uint16_t m : 13;		/* bit[12:0]	*/
		uint8_t n : 6;			/* bit[18:13]	*/
		uint8_t p : 4;			/* bit[22:19]	*/
		uint8_t off : 1;		/* bit[23]	*/
		uint8_t bypass : 1;		/* bit[24]	*/
		uint8_t reset : 1;		/* bit[25]	*/
		uint8_t reserved : 6;		/* bit[31:26]	*/
	} b;
};

struct ast2700_pll_cfg {
	union ast2700_pll_reg reg;
	unsigned int ext_reg;
};

struct ast2700_pll_desc {
	u32 in;
	u32 out;
	struct ast2700_pll_cfg cfg;
};

struct aspeed_clks {
	ulong id;
	const char *name;
};

#ifndef __ASSEMBLY__
struct ast2700_scu0 {
	u32 chip_id1;		/* 0x000 */
	u32 rsv_0x04[3];		/* 0x004 ~ 0x00C */
	u32 hwstrap1;		/* 0x010 */
	u32 hwstrap1_clr;		/* 0x014 */
	u32 rsv_0x18[2];		/* 0x018 ~ 0x01C */
	u32 hwstrap1_lock;		/* 0x020 */
	u32 hwstrap1_sec1;	/* 0x024 */
	u32 hwstrap1_sec2;	/* 0x028 */
	u32 hwstrap1_sec3;	/* 0x02C */
	u32 rsv_0x30[8];		/* 0x030 ~ 0x4C */
	u32 sysrest_log1;		/* 0x050 */
	u32 sysrest_log1_sec1;	/* 0x054 */
	u32 sysrest_log1_sec2;	/* 0x058 */
	u32 sysrest_log1_sec3;	/* 0x05C */
	u32 sysrest_log2;		/* 0x060 */
	u32 sysrest_log2_sec1;	/* 0x064 */
	u32 sysrest_log2_sec2;	/* 0x068 */
	u32 sysrest_log2_sec3;	/* 0x06C */
	u32 sysrest_log3;		/* 0x070 */
	u32 sysrest_log3_sec1; /* 0x074 */
	u32 sysrest_log3_sec2; /* 0x078 */
	u32 sysrest_log3_sec3; /* 0x07C */
	u32 rsv_0x80[8];		/* 0x080 ~ 0x9C */
	u32 probe_sig_select;	/* 0x0A0 */
	u32 probe_sig_enable1;	/* 0x0A4 */
	u32 probe_sig_enable2; /* 0x0A8 */
	u32 uart_dbg_rate;		/* 0x0AC */
	u32 rsv_0xB0[4];		/* 0x0B0 ~ 0xBC*/
	u32 misc;			/* 0x0C0 */
	u32 rsv_0xC4;		/* 0x0C4 */
	u32 debug_ctrl;		/* 0x0C8 */
	u32 rsv_0xCC[5];		/* 0x0CC ~ 0x0DC */
	u32 free_counter_read_low;		/* 0x0E0 */
	u32 free_counter_read_high;	/* 0x0E4 */
	u32 rsv_0xE8[2];		/* 0x0E8 ~ 0x0EC */
	u32 random_num_ctrl;	/* 0x0F0 */
	u32 random_num_data;	/* 0x0F4 */
	u32 rsv_0xF8[10];		/* 0x0F8 ~ 0x11C */
	u32 ssp_ctrl_1;		/* 0x120 */
	u32 ssp_ctrl_2;		/* 0x124 */
	u32 ssp_ctrl_3;		/* 0x128 */
	u32 ssp_ctrl_4;		/* 0x12C */
	u32 ssp_ctrl_5;		/* 0x130 */
	u32 ssp_ctrl_6;		/* 0x134 */
	u32 ssp_ctrl_7;		/* 0x138 */
	u32 rsv_0x13c[1];		/* 0x13C */
	u32 ssp_remap0_base;	/* 0x140 */
	u32 ssp_remap0_size;	/* 0x144 */
	u32 ssp_remap1_base;	/* 0x148 */
	u32 ssp_remap1_size;	/* 0x14c */
	u32 ssp_remap2_base;	/* 0x150 */
	u32 ssp_remap2_size;	/* 0x154 */
	u32 rsv_0x158[2];		/* 0x158 ~ 0x15C */
	u32 tsp_ctrl_1;		/* 0x160 */
	u32 rsv_0x164[1];		/* 0x164 */
	u32 tsp_ctrl_3;		/* 0x168 */
	u32 tsp_ctrl_4;		/* 0x16C */
	u32 tsp_ctrl_5;		/* 0x170 */
	u32 tsp_ctrl_6;		/* 0x174 */
	u32 tsp_ctrl_7;		/* 0x178 */
	u32 rsv_0x17c[6];		/* 0x17C ~ 0x190 */
	u32 tsp_remap_size;	/* 0x194 */
	u32 rsv_0x198[26];		/* 0x198 ~ 0x1FC */
	u32 modrst1_ctrl;		/* 0x200 */
	u32 modrst1_clr;		/* 0x204 */
	u32 rsv_0x208[2];		/* 0x208 ~ 0x20C */
	u32 modrst1_lock;		/* 0x210 */
	u32 modrst1_prot1;		/* 0x214 */
	u32 modrst1_prot2;		/* 0x218 */
	u32 modrst1_prot3;		/* 0x21C */
	u32 modrst2_ctrl;		/* 0x220 */
	u32 modrst2_clr;		/* 0x224 */
	u32 rsv_0x228[2];		/* 0x228 ~ 0x22C */
	u32 modrst2_lock;		/* 0x230 */
	u32 modrst2_prot1;		/* 0x234 */
	u32 modrst2_prot2;		/* 0x238 */
	u32 modrst2_prot3;		/* 0x23C */
	u32 clkgate_ctrl;		/* 0x240 */
	u32 clkgate_clr;		/* 0x244 */
	u32 rsv_0x248[2];		/* 0x248 */
	u32 clkgate_lock;		/* 0x250 */
	u32 clkgate_secure1;	/* 0x254 */
	u32 clkgate_secure2;	/* 0x258 */
	u32 clkgate_secure3;	/* 0x25c */
	u32 rsv_0x260[8];		/* 0x260 */
	u32 clk_sel1;		/* 0x280 */
	u32 clk_sel2;		/* 0x284 */
	u32 clk_sel3;		/* 0x288 */
	u32 rsv_0x28c;		/* 0x28c */
	u32 clk_sel1_lock;		/* 0x290 */
	u32 clk_sel2_lock;		/* 0x294 */
	u32 clk_sel3_lock;		/* 0x298 */
	u32 rsv_0x29c;		/* 0x29c */
	u32 clk_sel1_secure1;	/* 0x2a0 */
	u32 clk_sel1_secure2;	/* 0x2a4 */
	u32 clk_sel1_secure3;	/* 0x2a8 */
	u32 rsv_0x2ac;		/* 0x2ac */
	u32 clk_sel2_secure1;	/* 0x2b0 */
	u32 clk_sel2_secure2;	/* 0x2b4 */
	u32 clk_sel2_secure3;	/* 0x2b8 */
	u32 rsv_0x2bc;		/* 0x2bc */
	u32 clk_sel3_secure1;	/* 0x2c0 */
	u32 clk_sel3_secure2;	/* 0x2c4 */
	u32 clk_sel3_secure3;	/* 0x2c8 */
	u32 rsv_0x2cc[9];		/* 0x2cc */
	u32 extrst_sel;		/* 0x2f0 */
	u32 rsv_0x2f4[3];		/* 0x2f4 */
	u32 hpll;			/* 0x300 */
	u32 hpll_ext;		/* 0x304 */
	u32 dpll;			/* 0x308 */
	u32 dpll_ext;		/* 0x30C */
	u32 mpll;			/* 0x310 */
	u32 mpll_ext;		/* 0x314 */
	u32 rsv_0x318[2];		/* 0x318 ~ 0x31C */
	u32 d1clk_para;		/* 0x320 */
	u32 rsv_0x324[3];		/* 0x324 ~ 0x32C */
	u32 d2clk_para;		/* 0x330 */
	u32 rsv_0x334[3];		/* 0x334 ~ 0x33C */
	u32 crt1clk_para;		/* 0x340 */
	u32 rsv_0x344[3];		/* 0x344 ~ 0x34C */
	u32 crt2clk_para;		/* 0x350 */
	u32 rsv_0x354[3];		/* 0x354 ~ 0x35C */
	u32 mphyclk_para;		/* 0x360 */
	u32 rsv_0x364[7];		/* 0x364 ~ 0x37C */
	u32 clkduty_meas_ctrl;	/* 0x380 */
	u32 clkduty1;		/* 0x384 */
	u32 clkduty2;		/* 0x368 */
	u32 clkduty_meas_res;	/* 0x38c */
	u32 rsv_0x390[4];		/* 0x390 ~ 0x39C */
	u32 freq_counter_ctrl;	/* 0x3a0 */
	u32 freq_counter_cmp;	/* 0x3a4 */
	u32 prog_delay_ring_ctrl0;	/* 0x3a8 */
	u32 prog_delay_ring_ctrl1;	/* 0x3ac */
	u32 freq_counter_readback;	/* 0x3b0 */
	u32 rsv_0x3b4[19];		/* 0x3b4 */
	u32 pinmux1;		/* 0x400 */
	u32 pinmux2;		/* 0x404 */
	u32 pinmux3;		/* 0x408 */
	u32 rsv_0x40c;		/* 0x40C */
	u32 pinmux4;		/* 0x410 */
	u32 vga_func_ctrl;		/* 0x414 */
	u32 rsv_0x418[2];	/* 0x418 */
	u32 pinmux_lock0;	/* 0x420 */
	u32 pinmux_lock1;	/* 0x424 */
	u32 pinmux_lock2;	/* 0x428 */
	u32 rsv_0x42c;
	u32 pinmux_lock3;	/* 0x430 */
	u32 pinmux_lock4;	/* 0x434 */
	u32 rsv_0x438[18];
	u32 gpio18d0_ioctrl;	/* 0x480 */
	u32 gpio18d1_ioctrl;	/* 0x484 */
	u32 gpio18d2_ioctrl;	/* 0x488 */
	u32 gpio18d3_ioctrl;	/* 0x48c */
	u32 gpio18d4_ioctrl;	/* 0x490 */
	u32 gpio18d5_ioctrl;	/* 0x494 */
	u32 gpio18d6_ioctrl;	/* 0x498 */
	u32 gpio18d7_ioctrl;	/* 0x49c */
	u32 gpio18e0_ioctrl;	/* 0x4a0 */
	u32 gpio18e1_ioctrl;	/* 0x4a4 */
	u32 gpio18e2_ioctrl;	/* 0x4a8 */
	u32 gpio18e3_ioctrl;	/* 0x4ac */
	u32 jtag_ioctrl;	/* 0x4b0 */
	u32 uart_ioctrl;	/* 0x4b4 */
	u32 misc_ioctrl;	/* 0x4b8 */
	u32 rsv_0x4bc[17];		/* 0x4bc ~ 0x4fc */
	u32 pinmux_seucre0_0;	/* 0x500 */
	u32 pinmux_seucre0_1;	/* 0x504 */
	u32 pinmux_seucre0_2;	/* 0x508 */
	u32 rsv_0x50c;
	u32 pinmux_seucre0_3;	/* 0x510 */
	u32 pinmux_seucre0_4;	/* 0x514 */
	u32 rsv_0x518[58];
	u32 pinmux_seucre1_0;	/* 0x600 */
	u32 pinmux_seucre1_1;	/* 0x604 */
	u32 pinmux_seucre1_2;	/* 0x608 */
	u32 rsv_0x60c;
	u32 pinmux_seucre1_3;	/* 0x610 */
	u32 pinmux_seucre1_4;	/* 0x614 */
	u32 rsv_0x618[58];
	u32 pinmux_seucre2_0;	/* 0x700 */
	u32 pinmux_seucre2_1;	/* 0x704 */
	u32 pinmux_seucre2_2;	/* 0x708 */
	u32 rsv_0x70c;
	u32 pinmux_seucre2_3;	/* 0x710 */
	u32 pinmux_seucre2s_4;	/* 0x714 */
	u32 rsv_0x718[26];
	u32 cpu_scratch[96];	/* 0x780 ~ 0x8FC */
	u32 vga0_scratch1[4];	/* 0x900 ~ 0x90C */
	u32 vga1_scratch1[4];	/* 0x910 ~ 0x91C */
	u32 vga0_scratch2[8];	/* 0x920 ~ 0x93C */
	u32 vga1_scratch2[8];	/* 0x940 ~ 0x95C */
	u32 pci_cfg1[3];		/* 0x960 ~ 0x968 */
	u32 rsv_0x96c;		/* 0x96C */
	u32 pcie_cfg1;		/* 0x970 */
	u32 mmio_decode1;		/* 0x974 */
	u32 reloc_ctrl_decode1[2];	/* 0x978 ~ 0x97C */
	u32 rsv_0x980[4];		/* 0x980 ~ 0x98C */
	u32 mbox_decode1;		/* 0x990 */
	u32 shared_sram_decode1[2];/* 0x994 ~ 0x998 */
	u32 rsv_0x99c;		/* 0x99C */
	u32 pci_cfg2[3];		/* 0x9A0 ~ 0x9A8 */
	u32 rsv_0x9ac;		/* 0x9AC */
	u32 pcie_cfg2;		/* 0x9B0 */
	u32 mmio_decode2;		/* 0x9B4 */
	u32 reloc_ctrl_decode2[2];	/* 0x9B8 ~ 0x9BC */
	u32 rsv_0x9c0[4];		/* 0x9C0 ~ 0x9CC */
	u32 mbox_decode2;		/* 0x9D0 */
	u32 shared_sram_decode2[2];/* 0x9D4 ~ 0x9D8 */
	u32 rsv_0x9dc[9];		/* 0x9DC ~ 0x9FC */
	u32 pci0_misc[32];		/* 0xA00 ~ 0xA7C */
	u32 pci1_misc[32];		/* 0xA80 ~ 0xAFC */
};

struct ast2700_scu1 {
	u32 chip_id1;		/* 0x000 */
	u32 rsv_0x04[3];		/* 0x004 ~ 0x00C */
	u32 hwstrap1;		/* 0x010 */
	u32 hwstrap1_clr;		/* 0x014 */
	u32 rsv_0x18[2];		/* 0x018 ~ 0x01C */
	u32 hwstrap1_lock;		/* 0x020 */
	u32 hwstrap1_sec1;	/* 0x024 */
	u32 hwstrap1_sec2;	/* 0x028 */
	u32 hwstrap1_sec3;	/* 0x02C */
	u32 hwstrap2;		/* 0x030 */
	u32 hwstrap2_clr;		/* 0x034 */
	u32 rsv_0x38[2];		/* 0x038 ~ 0x03C */
	u32 hwstrap2_lock;		/* 0x040 */
	u32 hwstrap2_sec1;	/* 0x044 */
	u32 hwstrap2_sec2;	/* 0x048 */
	u32 hwstrap2_sec3;	/* 0x04C */
	u32 sysrest_log1;		/* 0x050 */
	u32 sysrest_log1_sec1;	/* 0x054 */
	u32 sysrest_log1_sec2;	/* 0x058 */
	u32 sysrest_log1_sec3;	/* 0x05C */
	u32 sysrest_log2;		/* 0x060 */
	u32 sysrest_log2_sec1;	/* 0x064 */
	u32 sysrest_log2_sec2;	/* 0x068 */
	u32 sysrest_log2_sec3;	/* 0x06C */
	u32 sysrest_log3;		/* 0x070 */
	u32 sysrest_log3_sec1; /* 0x074 */
	u32 sysrest_log3_sec2; /* 0x078 */
	u32 sysrest_log3_sec3; /* 0x07C */
	u32 sysrest_log4;		/* 0x080 */
	u32 sysrest_log4_sec1; /* 0x084 */
	u32 sysrest_log4_sec2; /* 0x088 */
	u32 sysrest_log4_sec3; /* 0x08C */
	u32 rsv_0x90[7];		/* 0x090 ~ 0xA8 */
	u32 uart_dbg_rate;		/* 0x0AC */
	u32 rsv_0xB0[4];		/* 0x0B0 ~ 0xBC*/
	u32 misc;			/* 0x0C0 */
	u32 rsv_0xC4;		/* 0x0C4 */
	u32 debug_ctrl;		/* 0x0C8 */
	u32 rsv_0xCC;		/* 0x0CC */
	u32 dac_ctrl;		/* 0x0D0 */
	u32 dac_crc_ctrl;		/* 0x0D4 */
	u32 rsv_0xD8[2];		/* 0x0D8 ~ 0x0DC */
	u32 video_input_ctrl;		/* 0x0E0 */
	u32 rsv_0xE4[3];		/* 0x0E4 ~ 0x0EC */
	u32 random_num_ctrl;	/* 0x0F0 */
	u32 random_num_data;	/* 0x0F4 */
	u32 rsv_0xF0[2];		/* 0x0F8 ~ 0x0FC */
	u32 rsv_0x100[32];		/* 0x100 ~ 0x17C */
	u32 scratch[32];		/* 0x180 ~ 0x1FC */
	u32 modrst1_ctrl;		/* 0x200 */
	u32 modrst1_clr;		/* 0x204 */
	u32 rsv_0x208[2];		/* 0x208 ~ 0x20C */
	u32 modrst_lock1;		/* 0x210 */
	u32 modrst1_sec1;		/* 0x214 */
	u32 modrst1_sec2;		/* 0x218 */
	u32 modrst1_sec3;		/* 0x21C */
	u32 modrst2_ctrl;		/* 0x220 */
	u32 modrst2_clr;		/* 0x224 */
	u32 rsv_0x228[2];		/* 0x228 ~ 0x22C */
	u32 modrst2_lock;		/* 0x230 */
	u32 modrst2_prot1;		/* 0x234 */
	u32 modrst2_prot2;		/* 0x238 */
	u32 modrst2_prot3;		/* 0x23C */
	u32 clkgate_ctrl1;		/* 0x240 */
	u32 clkgate_clr1;		/* 0x244 */
	u32 rsv_0x248[2];		/* 0x248 */
	u32 clkgate_lock1;		/* 0x250 */
	u32 clkgate_secure11;		/* 0x254 */
	u32 clkgate_secure12;		/* 0x258 */
	u32 clkgate_secure13;		/* 0x25c */
	u32 clkgate_ctrl2;		/* 0x260 */
	u32 clkgate_clr2;		/* 0x264 */
	u32 rsv_0x268[2];		/* 0x268 */
	u32 clkgate_lock2;		/* 0x270 */
	u32 clkgate_secure21;		/* 0x274 */
	u32 clkgate_secure22;		/* 0x278 */
	u32 clkgate_secure23;		/* 0x27c */
	u32 clk_sel1;		/* 0x280 */
	u32 clk_sel2;		/* 0x284 */
	u32 rsv_0x288[2];		/* 0x288 */
	u32 clk_sel1_lock;		/* 0x290 */
	u32 clk_sel2_lock;		/* 0x294 */
	u32 rsv_0x298[2];		/* 0x298 */
	u32 clk_sel1_secure1;		/* 0x2a0 */
	u32 clk_sel1_secure2;		/* 0x2a4 */
	u32 rsv_0x2a8[2];		/* 0x2a8 */
	u32 clk_sel2_secure1;		/* 0x2b0 */
	u32 clk_sel2_secure2;		/* 0x2b4 */
	u32 rsv_0x2b8[2];		/* 0x2b8 */
	u32 clk_sel3_secure1;		/* 0x2c0 */
	u32 clk_sel3_secure2;		/* 0x2c4 */
	u32 rsv_0x2c8[10];		/* 0x2c8 */
	u32 extrst_sel1;		/* 0x2f0 */
	u32 extrst_sel2;		/* 0x2f4 */
	u32 rsv_0x2f8[2];		/* 0x2f8 */
	u32 hpll;			/* 0x300 */
	u32 hpll_ext;		/* 0x304 */
	u32 rsv_0x308[2];		/* 0x308 ~ 0x30C */
	u32 apll;			/* 0x310 */
	u32 apll_ext;		/* 0x314 */
	u32 rsv_0x318[2];		/* 0x318 ~ 0x31C */
	u32 dpll;			/* 0x320 */
	u32 dpll_ext;		/* 0x324 */
	u32 rsv_0x328[2];		/* 0x328 ~ 0x32C */
	u32 uxclk_ctrl;		/* 0x330 */
	u32 huxclk_ctrl;		/* 0x334 */
	u32 rsv_0x338[18];		/* 0x338 ~ 0x37C */
	u32 clkduty_meas_ctrl;	/* 0x380 */
	u32 clkduty1;		/* 0x384 */
	u32 clkduty2;		/* 0x388 */
	u32 rsv_0x38c;		/* 0x38c */
	u32 mac_delay;		/* 0x390 */
	u32 mac_100m_delay;		/* 0x394 */
	u32 mac_10m_delay;		/* 0x398 */
	u32 rsv_0x39c;		/* 0x39c */
	u32 freq_counter_ctrl;	/* 0x3a0 */
	u32 freq_counter_cmp;	/* 0x3a4 */
	u32 rsv_0x3a8[2];		/* 0x3a8 ~ 0x3aC */
	u32 usb_ctrl;		/* 0x3b0 */
	u32 usb_lock;		/* 0x3b4 */
	u32 usb_secure1;	/* 0x3b8 */
	u32 usb_secure2;	/* 0x3bc */
	u32 usb_secure3;	/* 0x3c0 */
	u32 rsv_0x3c4[15];	/* 0x3c4 ~ 0x3fc */
	u32 pinumx1;		/* 0x400 */
	u32 pinumx2;		/* 0x404 */
	u32 pinumx3;		/* 0x408 */
	u32 pinumx4;		/* 0x40c */
	u32 pinumx5;		/* 0x410 */
	u32 pinumx6;		/* 0x414 */
	u32 pinumx7;		/* 0x418 */
	u32 pinumx8;		/* 0x41c */
	u32 pinumx9;		/* 0x420 */
	u32 pinumx10;		/* 0x424 */
	u32 pinumx11;		/* 0x428 */
	u32 pinumx12;		/* 0x42c */
	u32 pinumx13;		/* 0x430 */
	u32 pinumx14;		/* 0x434 */
	u32 pinumx15;		/* 0x438 */
	u32 pinumx16;		/* 0x43c */
	u32 pinumx17;		/* 0x440 */
	u32 pinumx18;		/* 0x444 */
	u32 pinumx19;		/* 0x448 */
	u32 pinumx20;		/* 0x44c */
	u32 pinumx21;		/* 0x450 */
	u32 pinumx22;		/* 0x454 */
	u32 pinumx23;		/* 0x458 */
	u32 pinumx24;		/* 0x45c */
	u32 pinumx25;		/* 0x460 */
	u32 pinumx26;		/* 0x464 */
	u32 pinumx27;		/* 0x468 */
	u32 rsv_0x46c[4];	/* 0x46c ~ 0x478 */
	u32 pinumx31;		/* 0x47c */
	u32 pull_down_dis[8];	/* 0x480 ~ 0x49c */
	u32 pin_conf;		/* 0x4a0 */
	u32 rsv_0x4a4[7];	/* 0x4a4 ~ 0x4bc */
	u32 io_driving0;	/* 0x4c0 */
	u32 io_driving1;	/* 0x4c4 */
	u32 io_driving2;	/* 0x4c8 */
	u32 io_driving3;	/* 0x4cc */
	u32 io_driving4;	/* 0x4d0 */
	u32 io_driving5;	/* 0x4d4 */
	u32 io_driving6;	/* 0x4d8 */
	u32 io_driving7;	/* 0x4dc */
	u32 io_driving8;	/* 0x4e0 */
};

#endif
#endif
