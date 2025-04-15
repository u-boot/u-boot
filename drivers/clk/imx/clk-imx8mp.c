// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP
 * Peng Fan <peng.fan@nxp.com>
 */

#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <dt-bindings/clock/imx8mp-clock.h>

#include "clk.h"

#if CONFIG_IS_ENABLED(VIDEO)
static u32 share_count_media;
#endif

static const char * const pll_ref_sels[] = { "osc_24m", "dummy", "dummy", "dummy", };
#if CONFIG_IS_ENABLED(VIDEO)
static const char * const video_pll1_bypass_sels[] = {"video_pll1", "video_pll1_ref_sel", };
#endif
static const char * const dram_pll_bypass_sels[] = {"dram_pll", "dram_pll_ref_sel", };
static const char * const arm_pll_bypass_sels[] = {"arm_pll", "arm_pll_ref_sel", };
static const char * const sys_pll1_bypass_sels[] = {"sys_pll1", "sys_pll1_ref_sel", };
static const char * const sys_pll2_bypass_sels[] = {"sys_pll2", "sys_pll2_ref_sel", };
static const char * const sys_pll3_bypass_sels[] = {"sys_pll3", "sys_pll3_ref_sel", };

static const char * const imx8mp_arm_core_sels[] = {"arm_a53_src", "arm_pll_out", };

static const char * const imx8mp_a53_sels[] = {"osc_24m", "arm_pll_out", "sys_pll2_500m",
					       "sys_pll2_1000m", "sys_pll1_800m", "sys_pll1_400m",
					       "audio_pll1_out", "sys_pll3_out", };

static const char * const imx8mp_hsio_axi_sels[] = {"osc_24m", "sys_pll2_500m", "sys_pll1_800m",
						    "sys_pll2_100m", "sys_pll2_200m", "clk_ext2",
						    "clk_ext4", "audio_pll2_out", };

#if CONFIG_IS_ENABLED(VIDEO)
static const char * const imx8mp_media_isp_sels[] = {"osc_24m", "sys_pll2_1000m", "sys_pll1_800m",
						     "sys_pll3_out", "sys_pll1_400m", "audio_pll2_out",
						     "clk_ext1", "sys_pll2_500m", };
#endif

static const char * const imx8mp_main_axi_sels[] = {"osc_24m", "sys_pll2_333m", "sys_pll1_800m",
						    "sys_pll2_250m", "sys_pll2_1000m", "audio_pll1_out",
						    "video_pll1_out", "sys_pll1_100m",};

static const char * const imx8mp_enet_axi_sels[] = {"osc_24m", "sys_pll1_266m", "sys_pll1_800m",
						    "sys_pll2_250m", "sys_pll2_200m", "audio_pll1_out",
						    "video_pll1_out", "sys_pll3_out", };

static const char * const imx8mp_nand_usdhc_sels[] = {"osc_24m", "sys_pll1_266m", "sys_pll1_800m",
						      "sys_pll2_200m", "sys_pll1_133m", "sys_pll3_out",
						      "sys_pll2_250m", "audio_pll1_out", };

#if CONFIG_IS_ENABLED(VIDEO)
static const char * const imx8mp_media_axi_sels[] = {"osc_24m", "sys_pll2_1000m", "sys_pll1_800m",
						     "sys_pll3_out", "sys_pll1_40m", "audio_pll2_out",
						     "clk_ext1", "sys_pll2_500m", };

static const char * const imx8mp_media_apb_sels[] = {"osc_24m", "sys_pll2_125m", "sys_pll1_800m",
					      "sys_pll3_out", "sys_pll1_40m", "audio_pll2_out",
					      "clk_ext1", "sys_pll1_133m", };
#endif

static const char * const imx8mp_noc_sels[] = {"osc_24m", "sys_pll1_800m", "sys_pll3_out",
					       "sys_pll2_1000m", "sys_pll2_500m", "audio_pll1_out",
					       "video_pll1_out", "audio_pll2_out", };

static const char * const imx8mp_noc_io_sels[] = {"osc_24m", "sys_pll1_800m", "sys_pll3_out",
						  "sys_pll2_1000m", "sys_pll2_500m", "audio_pll1_out",
						  "video_pll1_out", "audio_pll2_out", };

static const char * const imx8mp_ahb_sels[] = {"osc_24m", "sys_pll1_133m", "sys_pll1_800m",
					       "sys_pll1_400m", "sys_pll2_125m", "sys_pll3_out",
					       "audio_pll1_out", "video_pll1_out", };

static const char * const imx8mp_dram_alt_sels[] = {"osc_24m", "sys_pll1_800m", "sys_pll1_100m",
						    "sys_pll2_500m", "sys_pll2_1000m", "sys_pll3_out",
						    "audio_pll1_out", "sys_pll1_266m", };

static const char * const imx8mp_dram_apb_sels[] = {"osc_24m", "sys_pll2_200m", "sys_pll1_40m",
						    "sys_pll1_160m", "sys_pll1_800m", "sys_pll3_out",
						    "sys_pll2_250m", "audio_pll2_out", };

static const char * const imx8mp_pcie_aux_sels[] = {"osc_24m", "sys_pll2_200m", "sys_pll2_50m",
						    "sys_pll3_out", "sys_pll2_100m", "sys_pll1_80m",
						    "sys_pll1_160m", "sys_pll1_200m", };

static const char * const imx8mp_i2c5_sels[] = {"osc_24m", "sys_pll1_160m", "sys_pll2_50m",
						"sys_pll3_out", "audio_pll1_out", "video_pll1_out",
						"audio_pll2_out", "sys_pll1_133m", };

static const char * const imx8mp_i2c6_sels[] = {"osc_24m", "sys_pll1_160m", "sys_pll2_50m",
						"sys_pll3_out", "audio_pll1_out", "video_pll1_out",
						"audio_pll2_out", "sys_pll1_133m", };

static const char * const imx8mp_enet_qos_sels[] = {"osc_24m", "sys_pll2_125m", "sys_pll2_50m",
						    "sys_pll2_100m", "sys_pll1_160m", "audio_pll1_out",
						    "video_pll1_out", "clk_ext4", };

static const char * const imx8mp_enet_qos_timer_sels[] = {"osc_24m", "sys_pll2_100m", "audio_pll1_out",
							  "clk_ext1", "clk_ext2", "clk_ext3",
							  "clk_ext4", "video_pll1_out", };

static const char * const imx8mp_usdhc1_sels[] = {"osc_24m", "sys_pll1_400m", "sys_pll1_800m",
						  "sys_pll2_500m", "sys_pll3_out", "sys_pll1_266m",
						  "audio_pll2_out", "sys_pll1_100m", };

static const char * const imx8mp_usdhc2_sels[] = {"osc_24m", "sys_pll1_400m", "sys_pll1_800m",
						  "sys_pll2_500m", "sys_pll3_out", "sys_pll1_266m",
						  "audio_pll2_out", "sys_pll1_100m", };

static const char * const imx8mp_i2c1_sels[] = {"osc_24m", "sys_pll1_160m", "sys_pll2_50m",
						"sys_pll3_out", "audio_pll1_out", "video_pll1_out",
						"audio_pll2_out", "sys_pll1_133m", };

static const char * const imx8mp_i2c2_sels[] = {"osc_24m", "sys_pll1_160m", "sys_pll2_50m",
						"sys_pll3_out", "audio_pll1_out", "video_pll1_out",
						"audio_pll2_out", "sys_pll1_133m", };

static const char * const imx8mp_i2c3_sels[] = {"osc_24m", "sys_pll1_160m", "sys_pll2_50m",
						"sys_pll3_out", "audio_pll1_out", "video_pll1_out",
						"audio_pll2_out", "sys_pll1_133m", };

static const char * const imx8mp_i2c4_sels[] = {"osc_24m", "sys_pll1_160m", "sys_pll2_50m",
						"sys_pll3_out", "audio_pll1_out", "video_pll1_out",
						"audio_pll2_out", "sys_pll1_133m", };

static const char * const imx8mp_uart1_sels[] = {"osc_24m", "sys_pll1_80m", "sys_pll2_200m",
						 "sys_pll2_100m", "sys_pll3_out", "clk_ext2",
						 "clk_ext4", "audio_pll2_out", };

static const char * const imx8mp_uart2_sels[] = {"osc_24m", "sys_pll1_80m", "sys_pll2_200m",
						 "sys_pll2_100m", "sys_pll3_out", "clk_ext2",
						 "clk_ext3", "audio_pll2_out", };

static const char * const imx8mp_uart3_sels[] = {"osc_24m", "sys_pll1_80m", "sys_pll2_200m",
						 "sys_pll2_100m", "sys_pll3_out", "clk_ext2",
						 "clk_ext4", "audio_pll2_out", };

static const char * const imx8mp_uart4_sels[] = {"osc_24m", "sys_pll1_80m", "sys_pll2_200m",
						 "sys_pll2_100m", "sys_pll3_out", "clk_ext2",
						 "clk_ext3", "audio_pll2_out", };

static const char * const imx8mp_usb_core_ref_sels[] = {"osc_24m", "sys_pll1_100m", "sys_pll1_40m",
							"sys_pll2_100m", "sys_pll2_200m", "clk_ext2",
							"clk_ext3", "audio_pll2_out", };

static const char * const imx8mp_usb_phy_ref_sels[] = {"osc_24m", "sys_pll1_100m", "sys_pll1_40m",
						       "sys_pll2_100m", "sys_pll2_200m", "clk_ext2",
						       "clk_ext3", "audio_pll2_out", };

static const char * const imx8mp_gic_sels[] = {"osc_24m", "sys_pll2_200m", "sys_pll1_40m",
					       "sys_pll2_100m", "sys_pll1_800m",
					       "sys_pll2_500m", "clk_ext4", "audio_pll2_out" };

static const char * const imx8mp_pwm1_sels[] = {"osc_24m", "sys_pll2_100m", "sys_pll1_160m",
						"sys_pll1_40m", "sys_pll3_out", "clk_ext1",
						"sys_pll1_80m", "video_pll1_out", };

static const char * const imx8mp_pwm2_sels[] = {"osc_24m", "sys_pll2_100m", "sys_pll1_160m",
						"sys_pll1_40m", "sys_pll3_out", "clk_ext1",
						"sys_pll1_80m", "video_pll1_out", };

static const char * const imx8mp_pwm3_sels[] = {"osc_24m", "sys_pll2_100m", "sys_pll1_160m",
						"sys_pll1_40m", "sys_pll3_out", "clk_ext2",
						"sys_pll1_80m", "video_pll1_out", };

static const char * const imx8mp_pwm4_sels[] = {"osc_24m", "sys_pll2_100m", "sys_pll1_160m",
						"sys_pll1_40m", "sys_pll3_out", "clk_ext2",
						"sys_pll1_80m", "video_pll1_out", };

static const char * const imx8mp_ecspi1_sels[] = {"osc_24m", "sys_pll2_200m", "sys_pll1_40m",
						  "sys_pll1_160m", "sys_pll1_800m", "sys_pll3_out",
						  "sys_pll2_250m", "audio_pll2_out", };

static const char * const imx8mp_ecspi2_sels[] = {"osc_24m", "sys_pll2_200m", "sys_pll1_40m",
						  "sys_pll1_160m", "sys_pll1_800m", "sys_pll3_out",
						  "sys_pll2_250m", "audio_pll2_out", };

static const char * const imx8mp_ecspi3_sels[] = {"osc_24m", "sys_pll2_200m", "sys_pll1_40m",
						  "sys_pll1_160m", "sys_pll1_800m", "sys_pll3_out",
						  "sys_pll2_250m", "audio_pll2_out", };

static const char * const imx8mp_wdog_sels[] = {"osc_24m", "sys_pll1_133m", "sys_pll1_160m",
						"vpu_pll_out", "sys_pll2_125m", "sys_pll3_out",
						"sys_pll1_80m", "sys_pll2_166m" };

static const char * const imx8mp_qspi_sels[] = {"osc_24m", "sys_pll1_400m", "sys_pll2_333m",
						"sys_pll2_500m", "audio_pll2_out", "sys_pll1_266m",
						"sys_pll3_out", "sys_pll1_100m", };

static const char * const imx8mp_usdhc3_sels[] = {"osc_24m", "sys_pll1_400m", "sys_pll1_800m",
						  "sys_pll2_500m", "sys_pll3_out", "sys_pll1_266m",
						  "audio_pll2_out", "sys_pll1_100m", };

#if CONFIG_IS_ENABLED(VIDEO)
static const char * const imx8mp_media_disp_pix_sels[] = {"osc_24m", "video_pll1_out", "audio_pll2_out",
							  "audio_pll1_out", "sys_pll1_800m",
							  "sys_pll2_1000m", "sys_pll3_out", "clk_ext4", };

static const char * const imx8mp_media_ldb_sels[] = {"osc_24m", "sys_pll2_333m", "sys_pll2_100m",
						     "sys_pll1_800m", "sys_pll2_1000m",
						     "clk_ext2", "audio_pll2_out",
						     "video_pll1_out", };
#endif

static const char * const imx8mp_enet_ref_sels[] = {"osc_24m", "sys_pll2_125m", "sys_pll2_50m",
						    "sys_pll2_100m", "sys_pll1_160m", "audio_pll1_out",
						    "video_pll1_out", "clk_ext4", };

static const char * const imx8mp_enet_timer_sels[] = {"osc_24m", "sys_pll2_100m", "audio_pll1_out",
						      "clk_ext1", "clk_ext2", "clk_ext3",
						      "clk_ext4", "video_pll1_out", };

static const char * const imx8mp_enet_phy_ref_sels[] = {"osc_24m", "sys_pll2_50m", "sys_pll2_125m",
							"sys_pll2_200m", "sys_pll2_500m", "audio_pll1_out",
							"video_pll1_out", "audio_pll2_out", };

static const char * const imx8mp_dram_core_sels[] = {"dram_pll_out", "dram_alt_root", };

static int imx8mp_clk_probe(struct udevice *dev)
{
	struct clk osc_24m_clk, osc_32k_clk;
	void __iomem *base;
	int ret;

	base = (void *)ANATOP_BASE_ADDR;

	clk_dm(IMX8MP_CLK_DUMMY, clk_register_fixed_rate(NULL, "dummy", 0));

#if CONFIG_IS_ENABLED(VIDEO)
	clk_dm(IMX8MP_VIDEO_PLL1_REF_SEL, imx_clk_mux(dev, "video_pll1_ref_sel", base + 0x28, 0, 2, pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
#endif
	clk_dm(IMX8MP_DRAM_PLL_REF_SEL, imx_clk_mux(dev, "dram_pll_ref_sel", base + 0x50, 0, 2, pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MP_ARM_PLL_REF_SEL, imx_clk_mux(dev, "arm_pll_ref_sel", base + 0x84, 0, 2, pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MP_SYS_PLL1_REF_SEL, imx_clk_mux(dev, "sys_pll1_ref_sel", base + 0x94, 0, 2, pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MP_SYS_PLL2_REF_SEL, imx_clk_mux(dev, "sys_pll2_ref_sel", base + 0x104, 0, 2, pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));
	clk_dm(IMX8MP_SYS_PLL3_REF_SEL, imx_clk_mux(dev, "sys_pll3_ref_sel", base + 0x114, 0, 2, pll_ref_sels, ARRAY_SIZE(pll_ref_sels)));

#if CONFIG_IS_ENABLED(VIDEO)
	clk_dm(IMX8MP_VIDEO_PLL1, imx_clk_pll14xx("video_pll1", "video_pll1_ref_sel", base + 0x28,
						  &imx_1443x_pll));
#endif
	clk_dm(IMX8MP_DRAM_PLL, imx_clk_pll14xx("dram_pll", "dram_pll_ref_sel", base + 0x50,
						&imx_1443x_dram_pll));
	clk_dm(IMX8MP_ARM_PLL, imx_clk_pll14xx("arm_pll", "arm_pll_ref_sel", base + 0x84,
					       &imx_1416x_pll));
	clk_dm(IMX8MP_SYS_PLL1, imx_clk_pll14xx("sys_pll1", "sys_pll1_ref_sel", base + 0x94,
						&imx_1416x_pll));
	clk_dm(IMX8MP_SYS_PLL2, imx_clk_pll14xx("sys_pll2", "sys_pll2_ref_sel", base + 0x104,
						&imx_1416x_pll));
	clk_dm(IMX8MP_SYS_PLL3, imx_clk_pll14xx("sys_pll3", "sys_pll3_ref_sel", base + 0x114,
						&imx_1416x_pll));

#if CONFIG_IS_ENABLED(VIDEO)
	clk_dm(IMX8MP_VIDEO_PLL1_BYPASS, imx_clk_mux_flags(dev, "video_pll1_bypass", base + 0x28, 16, 1, video_pll1_bypass_sels, ARRAY_SIZE(video_pll1_bypass_sels), CLK_SET_RATE_PARENT));
#endif
	clk_dm(IMX8MP_DRAM_PLL_BYPASS, imx_clk_mux_flags(dev, "dram_pll_bypass", base + 0x50, 4, 1, dram_pll_bypass_sels, ARRAY_SIZE(dram_pll_bypass_sels), CLK_SET_RATE_PARENT));
	clk_dm(IMX8MP_ARM_PLL_BYPASS, imx_clk_mux_flags(dev, "arm_pll_bypass", base + 0x84, 4, 1, arm_pll_bypass_sels, ARRAY_SIZE(arm_pll_bypass_sels), CLK_SET_RATE_PARENT));
	clk_dm(IMX8MP_SYS_PLL1_BYPASS, imx_clk_mux_flags(dev, "sys_pll1_bypass", base + 0x94, 4, 1, sys_pll1_bypass_sels, ARRAY_SIZE(sys_pll1_bypass_sels), CLK_SET_RATE_PARENT));
	clk_dm(IMX8MP_SYS_PLL2_BYPASS, imx_clk_mux_flags(dev, "sys_pll2_bypass", base + 0x104, 4, 1, sys_pll2_bypass_sels, ARRAY_SIZE(sys_pll2_bypass_sels), CLK_SET_RATE_PARENT));
	clk_dm(IMX8MP_SYS_PLL3_BYPASS, imx_clk_mux_flags(dev, "sys_pll3_bypass", base + 0x114, 4, 1, sys_pll3_bypass_sels, ARRAY_SIZE(sys_pll3_bypass_sels), CLK_SET_RATE_PARENT));

#if CONFIG_IS_ENABLED(VIDEO)
	clk_dm(IMX8MP_VIDEO_PLL1_OUT, imx_clk_gate(dev, "video_pll1_out", "video_pll1_bypass", base + 0x28, 13));
#endif
	clk_dm(IMX8MP_DRAM_PLL_OUT, imx_clk_gate(dev, "dram_pll_out", "dram_pll_bypass", base + 0x50, 13));
	clk_dm(IMX8MP_ARM_PLL_OUT, imx_clk_gate(dev, "arm_pll_out", "arm_pll_bypass", base + 0x84, 11));
	clk_dm(IMX8MP_SYS_PLL1_OUT, imx_clk_gate(dev, "sys_pll1_out", "sys_pll1_bypass", base + 0x94, 11));
	clk_dm(IMX8MP_SYS_PLL2_OUT, imx_clk_gate(dev, "sys_pll2_out", "sys_pll2_bypass", base + 0x104, 11));
	clk_dm(IMX8MP_SYS_PLL3_OUT, imx_clk_gate(dev, "sys_pll3_out", "sys_pll3_bypass", base + 0x114, 11));

	clk_dm(IMX8MP_SYS_PLL1_40M, imx_clk_fixed_factor(dev, "sys_pll1_40m", "sys_pll1_out", 1, 20));
	clk_dm(IMX8MP_SYS_PLL1_80M, imx_clk_fixed_factor(dev, "sys_pll1_80m", "sys_pll1_out", 1, 10));
	clk_dm(IMX8MP_SYS_PLL1_100M, imx_clk_fixed_factor(dev, "sys_pll1_100m", "sys_pll1_out", 1, 8));
	clk_dm(IMX8MP_SYS_PLL1_133M, imx_clk_fixed_factor(dev, "sys_pll1_133m", "sys_pll1_out", 1, 6));
	clk_dm(IMX8MP_SYS_PLL1_160M, imx_clk_fixed_factor(dev, "sys_pll1_160m", "sys_pll1_out", 1, 5));
	clk_dm(IMX8MP_SYS_PLL1_200M, imx_clk_fixed_factor(dev, "sys_pll1_200m", "sys_pll1_out", 1, 4));
	clk_dm(IMX8MP_SYS_PLL1_266M, imx_clk_fixed_factor(dev, "sys_pll1_266m", "sys_pll1_out", 1, 3));
	clk_dm(IMX8MP_SYS_PLL1_400M, imx_clk_fixed_factor(dev, "sys_pll1_400m", "sys_pll1_out", 1, 2));
	clk_dm(IMX8MP_SYS_PLL1_800M, imx_clk_fixed_factor(dev, "sys_pll1_800m", "sys_pll1_out", 1, 1));

	clk_dm(IMX8MP_SYS_PLL2_50M, imx_clk_fixed_factor(dev, "sys_pll2_50m", "sys_pll2_out", 1, 20));
	clk_dm(IMX8MP_SYS_PLL2_100M, imx_clk_fixed_factor(dev, "sys_pll2_100m", "sys_pll2_out", 1, 10));
	clk_dm(IMX8MP_SYS_PLL2_125M, imx_clk_fixed_factor(dev, "sys_pll2_125m", "sys_pll2_out", 1, 8));
	clk_dm(IMX8MP_SYS_PLL2_166M, imx_clk_fixed_factor(dev, "sys_pll2_166m", "sys_pll2_out", 1, 6));
	clk_dm(IMX8MP_SYS_PLL2_200M, imx_clk_fixed_factor(dev, "sys_pll2_200m", "sys_pll2_out", 1, 5));
	clk_dm(IMX8MP_SYS_PLL2_250M, imx_clk_fixed_factor(dev, "sys_pll2_250m", "sys_pll2_out", 1, 4));
	clk_dm(IMX8MP_SYS_PLL2_333M, imx_clk_fixed_factor(dev, "sys_pll2_333m", "sys_pll2_out", 1, 3));
	clk_dm(IMX8MP_SYS_PLL2_500M, imx_clk_fixed_factor(dev, "sys_pll2_500m", "sys_pll2_out", 1, 2));
	clk_dm(IMX8MP_SYS_PLL2_1000M, imx_clk_fixed_factor(dev, "sys_pll2_1000m", "sys_pll2_out", 1, 1));

	ret = clk_get_by_name(dev, "osc_24m", &osc_24m_clk);
	if (ret)
		return ret;
	clk_dm(IMX8MP_CLK_24M, dev_get_clk_ptr(osc_24m_clk.dev));

	ret = clk_get_by_name(dev, "osc_32k", &osc_32k_clk);
	if (ret)
		return ret;
	clk_dm(IMX8MP_CLK_32K, dev_get_clk_ptr(osc_32k_clk.dev));

	base = dev_read_addr_ptr(dev);
	if (!base)
		return -EINVAL;

	clk_dm(IMX8MP_CLK_A53_SRC, imx_clk_mux2(dev, "arm_a53_src", base + 0x8000, 24, 3, imx8mp_a53_sels, ARRAY_SIZE(imx8mp_a53_sels)));
	clk_dm(IMX8MP_CLK_A53_CG, imx_clk_gate3(dev, "arm_a53_cg", "arm_a53_src", base + 0x8000, 28));
	clk_dm(IMX8MP_CLK_A53_DIV, imx_clk_divider2(dev, "arm_a53_div", "arm_a53_cg", base + 0x8000, 0, 3));

	clk_dm(IMX8MP_CLK_HSIO_AXI, imx8m_clk_composite(dev, "hsio_axi", imx8mp_hsio_axi_sels, base + 0x8380));
#if CONFIG_IS_ENABLED(VIDEO)
	clk_dm(IMX8MP_CLK_MEDIA_ISP, imx8m_clk_composite(dev, "media_isp", imx8mp_media_isp_sels, base + 0x8400));
#endif
	clk_dm(IMX8MP_CLK_MAIN_AXI, imx8m_clk_composite_critical(dev, "main_axi", imx8mp_main_axi_sels, base + 0x8800));
	clk_dm(IMX8MP_CLK_ENET_AXI, imx8m_clk_composite_critical(dev, "enet_axi", imx8mp_enet_axi_sels, base + 0x8880));
	clk_dm(IMX8MP_CLK_NAND_USDHC_BUS, imx8m_clk_composite_critical(dev, "nand_usdhc_bus", imx8mp_nand_usdhc_sels, base + 0x8900));
#if CONFIG_IS_ENABLED(VIDEO)
	clk_dm(IMX8MP_CLK_MEDIA_AXI, imx8m_clk_composite(dev, "media_axi", imx8mp_media_axi_sels, base + 0x8a00));
	clk_dm(IMX8MP_CLK_MEDIA_APB, imx8m_clk_composite(dev, "media_apb", imx8mp_media_apb_sels, base + 0x8a80));
#endif
	clk_dm(IMX8MP_CLK_NOC, imx8m_clk_composite_critical(dev, "noc", imx8mp_noc_sels, base + 0x8d00));
	clk_dm(IMX8MP_CLK_NOC_IO, imx8m_clk_composite_critical(dev, "noc_io", imx8mp_noc_io_sels, base + 0x8d80));

	clk_dm(IMX8MP_CLK_AHB, imx8m_clk_composite_critical(dev, "ahb_root", imx8mp_ahb_sels, base + 0x9000));
#if CONFIG_IS_ENABLED(VIDEO)
	clk_dm(IMX8MP_CLK_MEDIA_DISP2_PIX, imx8m_clk_composite(dev, "media_disp2_pix", imx8mp_media_disp_pix_sels, base + 0x9300));
#endif

	clk_dm(IMX8MP_CLK_IPG_ROOT, imx_clk_divider2(dev, "ipg_root", "ahb_root", base + 0x9080, 0, 1));

	clk_dm(IMX8MP_CLK_DRAM_ALT, imx8m_clk_composite(dev, "dram_alt", imx8mp_dram_alt_sels, base + 0xa000));
	clk_dm(IMX8MP_CLK_DRAM_APB, imx8m_clk_composite_critical(dev, "dram_apb", imx8mp_dram_apb_sels, base + 0xa080));
	clk_dm(IMX8MP_CLK_PCIE_AUX, imx8m_clk_composite(dev, "pcie_aux", imx8mp_pcie_aux_sels, base + 0xa400));
	clk_dm(IMX8MP_CLK_I2C5, imx8m_clk_composite(dev, "i2c5", imx8mp_i2c5_sels, base + 0xa480));
	clk_dm(IMX8MP_CLK_I2C6, imx8m_clk_composite(dev, "i2c6", imx8mp_i2c6_sels, base + 0xa500));
	clk_dm(IMX8MP_CLK_ENET_QOS, imx8m_clk_composite(dev, "enet_qos", imx8mp_enet_qos_sels, base + 0xa880));
	clk_dm(IMX8MP_CLK_ENET_QOS_TIMER, imx8m_clk_composite(dev, "enet_qos_timer", imx8mp_enet_qos_timer_sels, base + 0xa900));
	clk_dm(IMX8MP_CLK_ENET_REF, imx8m_clk_composite(dev, "enet_ref", imx8mp_enet_ref_sels, base + 0xa980));
	clk_dm(IMX8MP_CLK_ENET_TIMER, imx8m_clk_composite(dev, "enet_timer", imx8mp_enet_timer_sels, base + 0xaa00));
	clk_dm(IMX8MP_CLK_ENET_PHY_REF, imx8m_clk_composite(dev, "enet_phy_ref", imx8mp_enet_phy_ref_sels, base + 0xaa80));
	clk_dm(IMX8MP_CLK_QSPI, imx8m_clk_composite(dev, "qspi", imx8mp_qspi_sels, base + 0xab80));
	clk_dm(IMX8MP_CLK_USDHC1, imx8m_clk_composite(dev, "usdhc1", imx8mp_usdhc1_sels, base + 0xac00));
	clk_dm(IMX8MP_CLK_USDHC2, imx8m_clk_composite(dev, "usdhc2", imx8mp_usdhc2_sels, base + 0xac80));
	clk_dm(IMX8MP_CLK_I2C1, imx8m_clk_composite(dev, "i2c1", imx8mp_i2c1_sels, base + 0xad00));
	clk_dm(IMX8MP_CLK_I2C2, imx8m_clk_composite(dev, "i2c2", imx8mp_i2c2_sels, base + 0xad80));
	clk_dm(IMX8MP_CLK_I2C3, imx8m_clk_composite(dev, "i2c3", imx8mp_i2c3_sels, base + 0xae00));
	clk_dm(IMX8MP_CLK_I2C4, imx8m_clk_composite(dev, "i2c4", imx8mp_i2c4_sels, base + 0xae80));

	clk_dm(IMX8MP_CLK_UART1, imx8m_clk_composite(dev, "uart1", imx8mp_uart1_sels, base + 0xaf00));
	clk_dm(IMX8MP_CLK_UART2, imx8m_clk_composite(dev, "uart2", imx8mp_uart2_sels, base + 0xaf80));
	clk_dm(IMX8MP_CLK_UART3, imx8m_clk_composite(dev, "uart3", imx8mp_uart3_sels, base + 0xb000));
	clk_dm(IMX8MP_CLK_UART4, imx8m_clk_composite(dev, "uart4", imx8mp_uart4_sels, base + 0xb080));
	clk_dm(IMX8MP_CLK_USB_CORE_REF, imx8m_clk_composite(dev, "usb_core_ref", imx8mp_usb_core_ref_sels, base + 0xb100));
	clk_dm(IMX8MP_CLK_USB_PHY_REF, imx8m_clk_composite(dev, "usb_phy_ref", imx8mp_usb_phy_ref_sels, base + 0xb180));
	clk_dm(IMX8MP_CLK_GIC, imx8m_clk_composite_critical(dev, "gic", imx8mp_gic_sels, base + 0xb200));
	clk_dm(IMX8MP_CLK_ECSPI1, imx8m_clk_composite(dev, "ecspi1", imx8mp_ecspi1_sels, base + 0xb280));
	clk_dm(IMX8MP_CLK_ECSPI2, imx8m_clk_composite(dev, "ecspi2", imx8mp_ecspi2_sels, base + 0xb300));
	clk_dm(IMX8MP_CLK_PWM1, imx8m_clk_composite_critical(dev, "pwm1", imx8mp_pwm1_sels, base + 0xb380));
	clk_dm(IMX8MP_CLK_PWM2, imx8m_clk_composite_critical(dev, "pwm2", imx8mp_pwm2_sels, base + 0xb400));
	clk_dm(IMX8MP_CLK_PWM3, imx8m_clk_composite_critical(dev, "pwm3", imx8mp_pwm3_sels, base + 0xb480));
	clk_dm(IMX8MP_CLK_PWM4, imx8m_clk_composite_critical(dev, "pwm4", imx8mp_pwm4_sels, base + 0xb500));
	clk_dm(IMX8MP_CLK_ECSPI3, imx8m_clk_composite(dev, "ecspi3", imx8mp_ecspi3_sels, base + 0xc180));

	clk_dm(IMX8MP_CLK_WDOG, imx8m_clk_composite(dev, "wdog", imx8mp_wdog_sels, base + 0xb900));
	clk_dm(IMX8MP_CLK_USDHC3, imx8m_clk_composite(dev, "usdhc3", imx8mp_usdhc3_sels, base + 0xbc80));
#if CONFIG_IS_ENABLED(VIDEO)
	clk_dm(IMX8MP_CLK_MEDIA_DISP1_PIX, imx8m_clk_composite(dev, "media_disp1_pix", imx8mp_media_disp_pix_sels, base + 0xbe00));
	clk_dm(IMX8MP_CLK_MEDIA_LDB, imx8m_clk_composite(dev, "media_ldb", imx8mp_media_ldb_sels, base + 0xbf00));
#endif

	clk_dm(IMX8MP_CLK_DRAM_ALT_ROOT, imx_clk_fixed_factor(dev, "dram_alt_root", "dram_alt", 1, 4));
	clk_dm(IMX8MP_CLK_DRAM_CORE, imx_clk_mux2_flags(dev, "dram_core_clk", base + 0x9800, 24, 1, imx8mp_dram_core_sels, ARRAY_SIZE(imx8mp_dram_core_sels), CLK_IS_CRITICAL));

	clk_dm(IMX8MP_CLK_DRAM1_ROOT, imx_clk_gate4_flags(dev, "dram1_root_clk", "dram_core_clk", base + 0x4050, 0, CLK_IS_CRITICAL));
	clk_dm(IMX8MP_CLK_ECSPI1_ROOT, imx_clk_gate4(dev, "ecspi1_root_clk", "ecspi1", base + 0x4070, 0));
	clk_dm(IMX8MP_CLK_ECSPI2_ROOT, imx_clk_gate4(dev, "ecspi2_root_clk", "ecspi2", base + 0x4080, 0));
	clk_dm(IMX8MP_CLK_ECSPI3_ROOT, imx_clk_gate4(dev, "ecspi3_root_clk", "ecspi3", base + 0x4090, 0));
	clk_dm(IMX8MP_CLK_ENET1_ROOT, imx_clk_gate4(dev, "enet1_root_clk", "enet_axi", base + 0x40a0, 0));
	clk_dm(IMX8MP_CLK_GPIO1_ROOT, imx_clk_gate4(dev, "gpio1_root_clk", "ipg_root", base + 0x40b0, 0));
	clk_dm(IMX8MP_CLK_GPIO2_ROOT, imx_clk_gate4(dev, "gpio2_root_clk", "ipg_root", base + 0x40c0, 0));
	clk_dm(IMX8MP_CLK_GPIO3_ROOT, imx_clk_gate4(dev, "gpio3_root_clk", "ipg_root", base + 0x40d0, 0));
	clk_dm(IMX8MP_CLK_GPIO4_ROOT, imx_clk_gate4(dev, "gpio4_root_clk", "ipg_root", base + 0x40e0, 0));
	clk_dm(IMX8MP_CLK_GPIO5_ROOT, imx_clk_gate4(dev, "gpio5_root_clk", "ipg_root", base + 0x40f0, 0));
	clk_dm(IMX8MP_CLK_I2C1_ROOT, imx_clk_gate4(dev, "i2c1_root_clk", "i2c1", base + 0x4170, 0));
	clk_dm(IMX8MP_CLK_I2C2_ROOT, imx_clk_gate4(dev, "i2c2_root_clk", "i2c2", base + 0x4180, 0));
	clk_dm(IMX8MP_CLK_I2C3_ROOT, imx_clk_gate4(dev, "i2c3_root_clk", "i2c3", base + 0x4190, 0));
	clk_dm(IMX8MP_CLK_I2C4_ROOT, imx_clk_gate4(dev, "i2c4_root_clk", "i2c4", base + 0x41a0, 0));
	clk_dm(IMX8MP_CLK_PCIE_ROOT, imx_clk_gate4(dev, "pcie_root_clk", "pcie_aux", base + 0x4250, 0));
	clk_dm(IMX8MP_CLK_PWM1_ROOT, imx_clk_gate4(dev, "pwm1_root_clk", "pwm1", base + 0x4280, 0));
	clk_dm(IMX8MP_CLK_PWM2_ROOT, imx_clk_gate4(dev, "pwm2_root_clk", "pwm2", base + 0x4290, 0));
	clk_dm(IMX8MP_CLK_PWM3_ROOT, imx_clk_gate4(dev, "pwm3_root_clk", "pwm3", base + 0x42a0, 0));
	clk_dm(IMX8MP_CLK_PWM4_ROOT, imx_clk_gate4(dev, "pwm4_root_clk", "pwm4", base + 0x42b0, 0));
	clk_dm(IMX8MP_CLK_QOS_ROOT, imx_clk_gate4(dev, "qos_root_clk", "ipg_root", base + 0x42c0, 0));
	clk_dm(IMX8MP_CLK_QOS_ENET_ROOT, imx_clk_gate4(dev, "qos_enet_root_clk", "ipg_root", base + 0x42e0, 0));
	clk_dm(IMX8MP_CLK_QSPI_ROOT, imx_clk_gate4(dev, "qspi_root_clk", "qspi", base + 0x42f0, 0));
	clk_dm(IMX8MP_CLK_I2C5_ROOT, imx_clk_gate2(dev, "i2c5_root_clk", "i2c5", base + 0x4330, 0));
	clk_dm(IMX8MP_CLK_I2C6_ROOT, imx_clk_gate2(dev, "i2c6_root_clk", "i2c6", base + 0x4340, 0));
	clk_dm(IMX8MP_CLK_SIM_ENET_ROOT, imx_clk_gate4(dev, "sim_enet_root_clk", "enet_axi", base + 0x4400, 0));
	clk_dm(IMX8MP_CLK_ENET_QOS_ROOT, imx_clk_gate4(dev, "enet_qos_root_clk", "sim_enet_root_clk", base + 0x43b0, 0));
	clk_dm(IMX8MP_CLK_UART1_ROOT, imx_clk_gate4(dev, "uart1_root_clk", "uart1", base + 0x4490, 0));
	clk_dm(IMX8MP_CLK_UART2_ROOT, imx_clk_gate4(dev, "uart2_root_clk", "uart2", base + 0x44a0, 0));
	clk_dm(IMX8MP_CLK_UART3_ROOT, imx_clk_gate4(dev, "uart3_root_clk", "uart3", base + 0x44b0, 0));
	clk_dm(IMX8MP_CLK_UART4_ROOT, imx_clk_gate4(dev, "uart4_root_clk", "uart4", base + 0x44c0, 0));
	clk_dm(IMX8MP_CLK_USB_ROOT, imx_clk_gate2(dev, "usb_root_clk", "hsio_axi", base + 0x44d0, 0));
	clk_dm(IMX8MP_CLK_USB_SUSP, imx_clk_gate2(dev, "usb_suspend_clk", "osc_24m", base + 0x44d0, 0));
	clk_dm(IMX8MP_CLK_USB_PHY_ROOT, imx_clk_gate4(dev, "usb_phy_root_clk", "usb_phy_ref", base + 0x44f0, 0));
	clk_dm(IMX8MP_CLK_USDHC1_ROOT, imx_clk_gate4(dev, "usdhc1_root_clk", "usdhc1", base + 0x4510, 0));
	clk_dm(IMX8MP_CLK_USDHC2_ROOT, imx_clk_gate4(dev, "usdhc2_root_clk", "usdhc2", base + 0x4520, 0));
	clk_dm(IMX8MP_CLK_WDOG1_ROOT, imx_clk_gate4(dev, "wdog1_root_clk", "wdog", base + 0x4530, 0));
	clk_dm(IMX8MP_CLK_WDOG2_ROOT, imx_clk_gate4(dev, "wdog2_root_clk", "wdog", base + 0x4540, 0));
	clk_dm(IMX8MP_CLK_WDOG3_ROOT, imx_clk_gate4(dev, "wdog3_root_clk", "wdog", base + 0x4550, 0));
	clk_dm(IMX8MP_CLK_HSIO_ROOT, imx_clk_gate4(dev, "hsio_root_clk", "ipg_root", base + 0x45c0, 0));
#if CONFIG_IS_ENABLED(VIDEO)
	clk_dm(IMX8MP_CLK_MEDIA_APB_ROOT, imx_clk_gate2_shared2(dev, "media_apb_root_clk", "media_apb", base + 0x45d0, 0, &share_count_media));
	clk_dm(IMX8MP_CLK_MEDIA_AXI_ROOT, imx_clk_gate2_shared2(dev, "media_axi_root_clk", "media_axi", base + 0x45d0, 0, &share_count_media));
	clk_dm(IMX8MP_CLK_MEDIA_DISP1_PIX_ROOT, imx_clk_gate2_shared2(dev, "media_disp1_pix_root_clk", "media_disp1_pix", base + 0x45d0, 0, &share_count_media));
	clk_dm(IMX8MP_CLK_MEDIA_DISP2_PIX_ROOT, imx_clk_gate2_shared2(dev, "media_disp2_pix_root_clk", "media_disp2_pix", base + 0x45d0, 0, &share_count_media));
	clk_dm(IMX8MP_CLK_MEDIA_LDB_ROOT, imx_clk_gate2_shared2(dev, "media_ldb_root_clk", "media_ldb", base + 0x45d0, 0, &share_count_media));
	clk_dm(IMX8MP_CLK_MEDIA_ISP_ROOT, imx_clk_gate2_shared2(dev, "media_isp_root_clk", "media_isp", base + 0x45d0, 0, &share_count_media));
#endif

	clk_dm(IMX8MP_CLK_USDHC3_ROOT, imx_clk_gate4(dev, "usdhc3_root_clk", "usdhc3", base + 0x45e0, 0));

	clk_dm(IMX8MP_CLK_ARM,
	       imx_clk_mux2_flags(dev, "arm_core", base + 0x9880, 24, 1,
				  imx8mp_arm_core_sels,
				  ARRAY_SIZE(imx8mp_arm_core_sels),
				  CLK_IS_CRITICAL));

	return 0;
}

static const struct udevice_id imx8mp_clk_ids[] = {
	{ .compatible = "fsl,imx8mp-ccm" },
	{ },
};

U_BOOT_DRIVER(imx8mp_clk) = {
	.name = "clk_imx8mp",
	.id = UCLASS_CLK,
	.of_match = imx8mp_clk_ids,
	.ops = &ccf_clk_ops,
	.probe = imx8mp_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
