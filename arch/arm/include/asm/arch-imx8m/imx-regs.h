/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017 NXP
 */

#ifndef __ASM_ARCH_IMX8M_REGS_H__
#define __ASM_ARCH_IMX8M_REGS_H__

#define ARCH_MXC

#include <asm/mach-imx/regs-lcdif.h>

#define ROM_VERSION_A0		IS_ENABLED(CONFIG_IMX8MQ) ? 0x800 : 0x800
#define ROM_VERSION_B0		IS_ENABLED(CONFIG_IMX8MQ) ? 0x83C : 0x800

#define M4_BOOTROM_BASE_ADDR   0x007E0000

#define GPIO1_BASE_ADDR		0X30200000
#define GPIO2_BASE_ADDR		0x30210000
#define GPIO3_BASE_ADDR		0x30220000
#define GPIO4_BASE_ADDR		0x30230000
#define GPIO5_BASE_ADDR		0x30240000
#define WDOG1_BASE_ADDR		0x30280000
#define WDOG2_BASE_ADDR		0x30290000
#define WDOG3_BASE_ADDR		0x302A0000
#define IOMUXC_BASE_ADDR	0x30330000
#define IOMUXC_GPR_BASE_ADDR	0x30340000
#define OCOTP_BASE_ADDR		0x30350000
#define ANATOP_BASE_ADDR	0x30360000
#define CCM_BASE_ADDR		0x30380000
#define SRC_BASE_ADDR		0x30390000
#define GPC_BASE_ADDR		0x303A0000

#define SYSCNT_RD_BASE_ADDR	0x306A0000
#define SYSCNT_CMP_BASE_ADDR	0x306B0000
#define SYSCNT_CTRL_BASE_ADDR	0x306C0000

#define UART1_BASE_ADDR		0x30860000
#define UART3_BASE_ADDR		0x30880000
#define UART2_BASE_ADDR		0x30890000
#define I2C1_BASE_ADDR		0x30A20000
#define I2C2_BASE_ADDR		0x30A30000
#define I2C3_BASE_ADDR		0x30A40000
#define I2C4_BASE_ADDR		0x30A50000
#define UART4_BASE_ADDR		0x30A60000
#define USDHC1_BASE_ADDR	0x30B40000
#define USDHC2_BASE_ADDR	0x30B50000
#ifdef CONFIG_IMX8MM
#define USDHC3_BASE_ADDR	0x30B60000
#endif

#define TZASC_BASE_ADDR		0x32F80000

#define MXS_LCDIF_BASE		IS_ENABLED(CONFIG_IMX8MQ) ? \
					0x30320000 : 0x32e00000

#define SRC_IPS_BASE_ADDR	0x30390000
#define SRC_DDRC_RCR_ADDR	0x30391000
#define SRC_DDRC2_RCR_ADDR	0x30391004

#define DDRC_DDR_SS_GPR0	0x3d000000
#define DDRC_IPS_BASE_ADDR(X)	(0x3d400000 + ((X) * 0x2000000))
#define DDR_CSD1_BASE_ADDR	0x40000000

#if !defined(__ASSEMBLY__)
#include <asm/types.h>
#include <linux/bitops.h>
#include <stdbool.h>

#define GPR_TZASC_EN		BIT(0)
#define GPR_TZASC_EN_LOCK	BIT(16)

#define SRC_SCR_M4_ENABLE_OFFSET	3
#define SRC_SCR_M4_ENABLE_MASK		BIT(3)
#define SRC_SCR_M4C_NON_SCLR_RST_OFFSET	0
#define SRC_SCR_M4C_NON_SCLR_RST_MASK	BIT(0)
#define SRC_DDR1_ENABLE_MASK		0x8F000000UL
#define SRC_DDR2_ENABLE_MASK		0x8F000000UL
#define SRC_DDR1_RCR_PHY_PWROKIN_N_MASK	BIT(3)
#define SRC_DDR1_RCR_PHY_RESET_MASK	BIT(2)
#define SRC_DDR1_RCR_CORE_RESET_N_MASK	BIT(1)
#define SRC_DDR1_RCR_PRESET_N_MASK	BIT(0)

struct iomuxc_gpr_base_regs {
	u32 gpr[47];
};

struct ocotp_regs {
	u32	ctrl;
	u32	ctrl_set;
	u32     ctrl_clr;
	u32	ctrl_tog;
	u32	timing;
	u32     rsvd0[3];
	u32     data;
	u32     rsvd1[3];
	u32     read_ctrl;
	u32     rsvd2[3];
	u32	read_fuse_data;
	u32     rsvd3[3];
	u32	sw_sticky;
	u32     rsvd4[3];
	u32     scs;
	u32     scs_set;
	u32     scs_clr;
	u32     scs_tog;
	u32     crc_addr;
	u32     rsvd5[3];
	u32     crc_value;
	u32     rsvd6[3];
	u32     version;
	u32     rsvd7[0xdb];

	/* fuse banks */
	struct fuse_bank {
		u32	fuse_regs[0x10];
	} bank[0];
};

struct fuse_bank0_regs {
	u32 lock;
	u32 rsvd0[3];
	u32 uid_low;
	u32 rsvd1[3];
	u32 uid_high;
	u32 rsvd2[7];
};

struct fuse_bank1_regs {
	u32 tester3;
	u32 rsvd0[3];
	u32 tester4;
	u32 rsvd1[3];
	u32 tester5;
	u32 rsvd2[3];
	u32 cfg0;
	u32 rsvd3[3];
};

#ifdef CONFIG_IMX8MQ
struct anamix_pll {
	u32 audio_pll1_cfg0;
	u32 audio_pll1_cfg1;
	u32 audio_pll2_cfg0;
	u32 audio_pll2_cfg1;
	u32 video_pll_cfg0;
	u32 video_pll_cfg1;
	u32 gpu_pll_cfg0;
	u32 gpu_pll_cfg1;
	u32 vpu_pll_cfg0;
	u32 vpu_pll_cfg1;
	u32 arm_pll_cfg0;
	u32 arm_pll_cfg1;
	u32 sys_pll1_cfg0;
	u32 sys_pll1_cfg1;
	u32 sys_pll1_cfg2;
	u32 sys_pll2_cfg0;
	u32 sys_pll2_cfg1;
	u32 sys_pll2_cfg2;
	u32 sys_pll3_cfg0;
	u32 sys_pll3_cfg1;
	u32 sys_pll3_cfg2;
	u32 video_pll2_cfg0;
	u32 video_pll2_cfg1;
	u32 video_pll2_cfg2;
	u32 dram_pll_cfg0;
	u32 dram_pll_cfg1;
	u32 dram_pll_cfg2;
	u32 digprog;
	u32 osc_misc_cfg;
	u32 pllout_monitor_cfg;
	u32 frac_pllout_div_cfg;
	u32 sscg_pllout_div_cfg;
};
#else
struct anamix_pll {
	u32 audio_pll1_gnrl_ctl;
	u32 audio_pll1_fdiv_ctl0;
	u32 audio_pll1_fdiv_ctl1;
	u32 audio_pll1_sscg_ctl;
	u32 audio_pll1_mnit_ctl;
	u32 audio_pll2_gnrl_ctl;
	u32 audio_pll2_fdiv_ctl0;
	u32 audio_pll2_fdiv_ctl1;
	u32 audio_pll2_sscg_ctl;
	u32 audio_pll2_mnit_ctl;
	u32 video_pll1_gnrl_ctl;
	u32 video_pll1_fdiv_ctl0;
	u32 video_pll1_fdiv_ctl1;
	u32 video_pll1_sscg_ctl;
	u32 video_pll1_mnit_ctl;
	u32 reserved[5];
	u32 dram_pll_gnrl_ctl;
	u32 dram_pll_fdiv_ctl0;
	u32 dram_pll_fdiv_ctl1;
	u32 dram_pll_sscg_ctl;
	u32 dram_pll_mnit_ctl;
	u32 gpu_pll_gnrl_ctl;
	u32 gpu_pll_div_ctl;
	u32 gpu_pll_locked_ctl1;
	u32 gpu_pll_mnit_ctl;
	u32 vpu_pll_gnrl_ctl;
	u32 vpu_pll_div_ctl;
	u32 vpu_pll_locked_ctl1;
	u32 vpu_pll_mnit_ctl;
	u32 arm_pll_gnrl_ctl;
	u32 arm_pll_div_ctl;
	u32 arm_pll_locked_ctl1;
	u32 arm_pll_mnit_ctl;
	u32 sys_pll1_gnrl_ctl;
	u32 sys_pll1_div_ctl;
	u32 sys_pll1_locked_ctl1;
	u32 reserved2[24];
	u32 sys_pll1_mnit_ctl;
	u32 sys_pll2_gnrl_ctl;
	u32 sys_pll2_div_ctl;
	u32 sys_pll2_locked_ctl1;
	u32 sys_pll2_mnit_ctl;
	u32 sys_pll3_gnrl_ctl;
	u32 sys_pll3_div_ctl;
	u32 sys_pll3_locked_ctl1;
	u32 sys_pll3_mnit_ctl;
	u32 anamix_misc_ctl;
	u32 anamix_clk_mnit_ctl;
	u32 reserved3[437];
	u32 digprog;
};
#endif

struct fuse_bank9_regs {
	u32 mac_addr0;
	u32 rsvd0[3];
	u32 mac_addr1;
	u32 rsvd1[11];
};

/* System Reset Controller (SRC) */
struct src {
	u32 scr;
	u32 a53rcr;
	u32 a53rcr1;
	u32 m4rcr;
	u32 reserved1[4];
	u32 usbophy1_rcr;
	u32 usbophy2_rcr;
	u32 mipiphy_rcr;
	u32 pciephy_rcr;
	u32 hdmi_rcr;
	u32 disp_rcr;
	u32 reserved2[2];
	u32 gpu_rcr;
	u32 vpu_rcr;
	u32 pcie2_rcr;
	u32 mipiphy1_rcr;
	u32 mipiphy2_rcr;
	u32 reserved3;
	u32 sbmr1;
	u32 srsr;
	u32 reserved4[2];
	u32 sisr;
	u32 simr;
	u32 sbmr2;
	u32 gpr1;
	u32 gpr2;
	u32 gpr3;
	u32 gpr4;
	u32 gpr5;
	u32 gpr6;
	u32 gpr7;
	u32 gpr8;
	u32 gpr9;
	u32 gpr10;
	u32 reserved5[985];
	u32 ddr1_rcr;
	u32 ddr2_rcr;
};

#define WDOG_WDT_MASK	BIT(3)
#define WDOG_WDZST_MASK	BIT(0)
struct wdog_regs {
	u16	wcr;	/* Control */
	u16	wsr;	/* Service */
	u16	wrsr;	/* Reset Status */
	u16	wicr;	/* Interrupt Control */
	u16	wmcr;	/* Miscellaneous Control */
};

struct bootrom_sw_info {
	u8 reserved_1;
	u8 boot_dev_instance;
	u8 boot_dev_type;
	u8 reserved_2;
	u32 core_freq;
	u32 axi_freq;
	u32 ddr_freq;
	u32 tick_freq;
	u32 reserved_3[3];
};

#define ROM_SW_INFO_ADDR_B0	(IS_ENABLED(CONFIG_IMX8MQ) ? 0x00000968 :\
				 0x000009e8)
#define ROM_SW_INFO_ADDR_A0	0x000009e8

#define ROM_SW_INFO_ADDR is_soc_rev(CHIP_REV_1_0) ? \
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR_A0 : \
		(struct bootrom_sw_info **)ROM_SW_INFO_ADDR_B0
#endif
#endif
