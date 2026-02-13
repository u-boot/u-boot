// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <dt-bindings/clock/imx6qdl-clock.h>

#include "clk.h"

#define SET_CLK_RATE(id, rate)           \
	do {                             \
		struct clk *clk;         \
		clk_get_by_id(id, &clk); \
		clk_set_rate(clk, rate); \
	} while (0)

#define SET_CLK_PARENT(child_id, parent_id)            \
	do {                                           \
		struct clk *clk, *clk_parent;          \
		clk_get_by_id(parent_id, &clk_parent); \
		clk_get_by_id(child_id, &clk);         \
		clk_set_parent(clk, clk_parent);       \
	} while (0)

static int imx6q_clk_request(struct clk *clk)
{
	if (clk->id < IMX6QDL_CLK_DUMMY || clk->id >= IMX6QDL_CLK_END) {
		printf("%s: Invalid clk ID #%lu\n", __func__, clk->id);
		return -EINVAL;
	}

	return 0;
}

static struct clk_ops imx6q_clk_ops = {
	.request = imx6q_clk_request,
	.set_rate = ccf_clk_set_rate,
	.get_rate = ccf_clk_get_rate,
	.enable = ccf_clk_enable,
	.disable = ccf_clk_disable,
};

static const char *const usdhc_sels[] = {
	"pll2_pfd2_396m",
	"pll2_pfd0_352m",
};
static const char *const periph_sels[] = {
	"periph_pre",
	"periph_clk2",
};
static const char *periph2_sels[] = {
	"periph2_pre",
	"periph2_clk2",
};
static const char *const periph_pre_sels[] = {
	"pll2_bus",
	"pll2_pfd2_396m",
	"pll2_pfd0_352m",
	"pll2_198m",
};
static const char *const uart_sels[] = {
	"pll3_80m",
	"osc",
};
static const char *const ecspi_sels[] = {
	"pll3_60m",
	"osc",
};
static const char *const ipu_sels[] = {
	"mmdc_ch0_axi",
	"pll2_pfd2_396m",
	"pll3_120m",
	"pll3_pfd1_540m",
};
static const char *const ldb_di_sels[] = {
	"pll5_video_div", "pll2_pfd0_352m", "pll2_pfd2_396m",
	"mmdc_ch1_axi",	  "pll3_usb_otg",
};
static const char *const ipu_di_pre_sels[] = {
	"mmdc_ch0_axi",	  "pll3_usb_otg",   "pll5_video_div",
	"pll2_pfd0_352m", "pll2_pfd2_396m", "pll3_pfd1_540m",
};
static const char *const ipu1_di0_sels[] = {
	"ipu1_di0_pre", "dummy", "dummy", "ldb_di0", "ldb_di1",
};
static const char *const ipu1_di1_sels[] = {
	"ipu1_di1_pre", "dummy", "dummy", "ldb_di0", "ldb_di1",
};
static const char *const ipu2_di0_sels[] = {
	"ipu2_di0_pre", "dummy", "dummy", "ldb_di0", "ldb_di1",
};
static const char *const ipu2_di1_sels[] = {
	"ipu2_di1_pre", "dummy", "dummy", "ldb_di0", "ldb_di1",
};
static const char *ipu1_di0_sels_2[] = {
	"ipu1_di0_pre", "dummy", "dummy", "ldb_di0_podf", "ldb_di1_podf",
};
static const char *ipu1_di1_sels_2[] = {
	"ipu1_di1_pre", "dummy", "dummy", "ldb_di0_podf", "ldb_di1_podf",
};
static const char *ipu2_di0_sels_2[] = {
	"ipu2_di0_pre", "dummy", "dummy", "ldb_di0_podf", "ldb_di1_podf",
};
static const char *ipu2_di1_sels_2[] = {
	"ipu2_di1_pre", "dummy", "dummy", "ldb_di0_podf", "ldb_di1_podf",
};

static unsigned int share_count_mipi_core_cfg;

static int imx6q_clk_probe(struct udevice *dev)
{
	void *base;

	/* Anatop clocks */
	base = (void *)ANATOP_BASE_ADDR;

	clk_dm(IMX6QDL_CLK_PLL2,
	       imx_clk_pllv3(dev, IMX_PLLV3_GENERIC, "pll2_bus", "osc",
			     base + 0x30, 0x1));
	clk_dm(IMX6QDL_CLK_PLL3_USB_OTG,
	       imx_clk_pllv3(dev, IMX_PLLV3_USB, "pll3_usb_otg", "osc",
			     base + 0x10, 0x3));
	clk_dm(IMX6QDL_CLK_PLL3_60M,
	       imx_clk_fixed_factor(dev, "pll3_60m", "pll3_usb_otg", 1, 8));
	clk_dm(IMX6QDL_CLK_PLL3_80M,
	       imx_clk_fixed_factor(dev, "pll3_80m", "pll3_usb_otg", 1, 6));
	clk_dm(IMX6QDL_CLK_PLL3_120M,
	       imx_clk_fixed_factor(dev, "pll3_120m", "pll3_usb_otg", 1, 4));
	clk_dm(IMX6QDL_CLK_PLL5, imx_clk_pllv3(dev, IMX_PLLV3_AV, "pll5", "osc",
					       base + 0xa0, 0x7f));
	clk_dm(IMX6QDL_CLK_PLL5_VIDEO,
	       imx_clk_gate(dev, "pll5_video", "pll5", base + 0xa0, 13));
	clk_dm(IMX6QDL_CLK_PLL6, imx_clk_pllv3(dev, IMX_PLLV3_ENET, "pll6",
					       "osc", base + 0xe0, 0x3));
	clk_dm(IMX6QDL_CLK_PLL6_ENET,
	       imx_clk_gate(dev, "pll6_enet", "pll6", base + 0xe0, 13));

	clk_dm(IMX6QDL_CLK_PLL2_PFD0_352M,
	       imx_clk_pfd("pll2_pfd0_352m", "pll2_bus", base + 0x100, 0));
	clk_dm(IMX6QDL_CLK_PLL2_PFD2_396M,
	       imx_clk_pfd("pll2_pfd2_396m", "pll2_bus", base + 0x100, 2));
	clk_dm(IMX6QDL_CLK_PLL3_PFD1_540M,
	       imx_clk_pfd("pll3_pfd1_540m", "pll3_usb_otg", base + 0xf0, 1));

	clk_dm(IMX6QDL_CLK_PLL2_198M,
	       imx_clk_fixed_factor(dev, "pll2_198m", "pll2_pfd2_396m", 1, 2));
	clk_dm(IMX6QDL_CLK_PLL5_POST_DIV,
	       imx_clk_fixed_factor(dev, "pll5_post_div", "pll5_video", 1, 1));
	clk_dm(IMX6QDL_CLK_PLL5_VIDEO_DIV,
	       imx_clk_fixed_factor(dev, "pll5_video_div", "pll5_post_div", 1,
				    1));
	clk_dm(IMX6QDL_CLK_VIDEO_27M,
	       imx_clk_fixed_factor(dev, "video_27m", "pll3_pfd1_540m", 1,
				    20));

	/* CCM clocks */
	base = dev_read_addr_ptr(dev);
	if (!base)
		return -EINVAL;

	clk_dm(IMX6QDL_CLK_USDHC1_SEL,
	       imx_clk_mux(dev, "usdhc1_sel", base + 0x1c, 16, 1, usdhc_sels,
			   ARRAY_SIZE(usdhc_sels)));
	clk_dm(IMX6QDL_CLK_USDHC2_SEL,
	       imx_clk_mux(dev, "usdhc2_sel", base + 0x1c, 17, 1, usdhc_sels,
			   ARRAY_SIZE(usdhc_sels)));
	clk_dm(IMX6QDL_CLK_USDHC3_SEL,
	       imx_clk_mux(dev, "usdhc3_sel", base + 0x1c, 18, 1, usdhc_sels,
			   ARRAY_SIZE(usdhc_sels)));
	clk_dm(IMX6QDL_CLK_USDHC4_SEL,
	       imx_clk_mux(dev, "usdhc4_sel", base + 0x1c, 19, 1, usdhc_sels,
			   ARRAY_SIZE(usdhc_sels)));

	if (of_machine_is_compatible("fsl,imx6qp")) {
		clk_dm(IMX6QDL_CLK_UART_SEL,
		       imx_clk_mux(dev, "uart_sel", base + 0x24, 6, 1,
				   uart_sels, ARRAY_SIZE(uart_sels)));
		clk_dm(IMX6QDL_CLK_ECSPI_SEL,
		       imx_clk_mux(dev, "ecspi_sel", base + 0x38, 18, 1,
				   ecspi_sels, ARRAY_SIZE(ecspi_sels)));
	}

	clk_dm(IMX6QDL_CLK_PERIPH_PRE,
	       imx_clk_mux(dev, "periph_pre", base + 0x18, 18, 2,
			   periph_pre_sels, ARRAY_SIZE(periph_pre_sels)));
	clk_dm(IMX6QDL_CLK_PERIPH2_PRE,
	       imx_clk_mux(dev, "periph2_pre", base + 0x18, 21, 2,
			   periph_pre_sels, ARRAY_SIZE(periph_pre_sels)));
	clk_dm(IMX6QDL_CLK_PERIPH,
	       imx_clk_busy_mux(dev, "periph", base + 0x14, 25, 1, base + 0x48,
				5, periph_sels, ARRAY_SIZE(periph_sels)));
	clk_dm(IMX6QDL_CLK_PERIPH2,
	       imx_clk_busy_mux(dev, "periph2", base + 0x14, 26, 1, base + 0x48,
				3, periph2_sels, ARRAY_SIZE(periph2_sels)));

	clk_dm(IMX6QDL_CLK_USDHC1_PODF,
	       imx_clk_divider(dev, "usdhc1_podf", "usdhc1_sel", base + 0x24,
			       11, 3));
	clk_dm(IMX6QDL_CLK_USDHC2_PODF,
	       imx_clk_divider(dev, "usdhc2_podf", "usdhc2_sel", base + 0x24,
			       16, 3));
	clk_dm(IMX6QDL_CLK_USDHC3_PODF,
	       imx_clk_divider(dev, "usdhc3_podf", "usdhc3_sel", base + 0x24,
			       19, 3));
	clk_dm(IMX6QDL_CLK_USDHC4_PODF,
	       imx_clk_divider(dev, "usdhc4_podf", "usdhc4_sel", base + 0x24,
			       22, 3));

	if (of_machine_is_compatible("fsl,imx6qp")) {
		clk_dm(IMX6QDL_CLK_UART_SERIAL_PODF,
		       imx_clk_divider(dev, "uart_serial_podf", "uart_sel",
				       base + 0x24, 0, 6));
		clk_dm(IMX6QDL_CLK_ECSPI_ROOT,
		       imx_clk_divider(dev, "ecspi_root", "ecspi_sel",
				       base + 0x38, 19, 6));
	} else {
		clk_dm(IMX6QDL_CLK_UART_SERIAL_PODF,
		       imx_clk_divider(dev, "uart_serial_podf", "pll3_80m",
				       base + 0x24, 0, 6));
		clk_dm(IMX6QDL_CLK_ECSPI_ROOT,
		       imx_clk_divider(dev, "ecspi_root", "pll3_60m",
				       base + 0x38, 19, 6));
	}

	clk_dm(IMX6QDL_CLK_AHB,
	       imx_clk_busy_divider(dev, "ahb", "periph", base + 0x14, 10, 3,
				    base + 0x48, 1));
	clk_dm(IMX6QDL_CLK_IPG,
	       imx_clk_divider(dev, "ipg", "ahb", base + 0x14, 8, 2));
	clk_dm(IMX6QDL_CLK_IPG_PER,
	       imx_clk_divider(dev, "ipg_per", "ipg", base + 0x1c, 0, 6));
	clk_dm(IMX6QDL_CLK_UART_IPG,
	       imx_clk_gate2(dev, "uart_ipg", "ipg", base + 0x7c, 24));

	if (of_machine_is_compatible("fsl,imx6qp")) {
		clk_dm(IMX6QDL_CLK_MMDC_CH1_AXI_CG,
		       imx_clk_gate2(dev, "mmdc_ch1_axi_cg", "periph2",
				     base + 0x4, 18));
		clk_dm(IMX6QDL_CLK_MMDC_CH1_AXI_PODF,
		       imx_clk_busy_divider(dev, "mmdc_ch1_axi_podf",
					    "mmdc_ch1_axi_cg", base + 0x14, 3,
					    3, base + 0x48, 2));
	} else {
		clk_dm(IMX6QDL_CLK_MMDC_CH1_AXI_PODF,
		       imx_clk_busy_divider(dev, "mmdc_ch1_axi_podf", "periph2",
					    base + 0x14, 3, 3, base + 0x48, 2));
	}

	clk_dm(IMX6QDL_CLK_MMDC_CH0_AXI_PODF,
	       imx_clk_busy_divider(dev, "mmdc_ch0_axi_podf", "periph",
				    base + 0x14, 19, 3, base + 0x48, 4));

	clk_dm(IMX6QDL_CLK_MMDC_CH0_AXI,
	       imx_clk_gate2_flags(dev, "mmdc_ch0_axi", "mmdc_ch0_axi_podf",
				   base + 0x74, 20, CLK_IS_CRITICAL));
	clk_dm(IMX6QDL_CLK_MMDC_CH1_AXI,
	       imx_clk_gate2(dev, "mmdc_ch1_axi", "mmdc_ch1_axi_podf",
			     base + 0x74, 22));

	clk_dm(IMX6QDL_CLK_IPU1_SEL,
	       imx_clk_mux(dev, "ipu1_sel", base + 0x3c, 9, 2, ipu_sels,
			   ARRAY_SIZE(ipu_sels)));
	clk_dm(IMX6QDL_CLK_IPU2_SEL,
	       imx_clk_mux(dev, "ipu2_sel", base + 0x3c, 14, 2, ipu_sels,
			   ARRAY_SIZE(ipu_sels)));

	if (of_machine_is_compatible("fsl,imx6qp")) {
		clk_dm(IMX6QDL_CLK_LDB_DI0_SEL,
		       imx_clk_mux(dev, "ldb_di0_sel", base + 0x2c, 9, 3,
				   ldb_di_sels, ARRAY_SIZE(ldb_di_sels)));
		clk_dm(IMX6QDL_CLK_LDB_DI1_SEL,
		       imx_clk_mux(dev, "ldb_di1_sel", base + 0x2c, 12, 3,
				   ldb_di_sels, ARRAY_SIZE(ldb_di_sels)));
	} else {
		/*
                 * Need to set these as read-only due to a hardware bug.
                 * Keeping default mux values. Fixed on the i.MX6 QuadPlus
                 */
		clk_dm(IMX6QDL_CLK_LDB_DI0_SEL,
		       imx_clk_mux_flags(dev, "ldb_di0_sel", base + 0x2c, 9, 3,
					 ldb_di_sels, ARRAY_SIZE(ldb_di_sels),
					 CLK_SET_RATE_PARENT |
						 CLK_MUX_READ_ONLY));
		clk_dm(IMX6QDL_CLK_LDB_DI1_SEL,
		       imx_clk_mux_flags(dev, "ldb_di1_sel", base + 0x2c, 12, 3,
					 ldb_di_sels, ARRAY_SIZE(ldb_di_sels),
					 CLK_SET_RATE_PARENT |
						 CLK_MUX_READ_ONLY));
	}

	clk_dm(IMX6QDL_CLK_IPU1_DI0_PRE_SEL,
	       imx_clk_mux_flags(dev, "ipu1_di0_pre_sel", base + 0x34, 6, 3,
				 ipu_di_pre_sels, ARRAY_SIZE(ipu_di_pre_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMX6QDL_CLK_IPU1_DI1_PRE_SEL,
	       imx_clk_mux_flags(dev, "ipu1_di1_pre_sel", base + 0x34, 15, 3,
				 ipu_di_pre_sels, ARRAY_SIZE(ipu_di_pre_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMX6QDL_CLK_IPU2_DI0_PRE_SEL,
	       imx_clk_mux_flags(dev, "ipu2_di0_pre_sel", base + 0x38, 6, 3,
				 ipu_di_pre_sels, ARRAY_SIZE(ipu_di_pre_sels),
				 CLK_SET_RATE_PARENT));
	clk_dm(IMX6QDL_CLK_IPU2_DI1_PRE_SEL,
	       imx_clk_mux_flags(dev, "ipu2_di1_pre_sel", base + 0x38, 15, 3,
				 ipu_di_pre_sels, ARRAY_SIZE(ipu_di_pre_sels),
				 CLK_SET_RATE_PARENT));

	if (of_machine_is_compatible("fsl,imx6qp")) {
		clk_dm(IMX6QDL_CLK_LDB_DI0,
		       imx_clk_gate2(dev, "ldb_di0", "ldb_di0_sel", base + 0x74,
				     12));
		clk_dm(IMX6QDL_CLK_LDB_DI1,
		       imx_clk_gate2(dev, "ldb_di1", "ldb_di1_sel", base + 0x74,
				     14));
		clk_dm(IMX6QDL_CLK_LDB_DI0_DIV_3_5,
		       imx_clk_fixed_factor(dev, "ldb_di0_div_3_5", "ldb_di0",
					    2, 7));
		clk_dm(IMX6QDL_CLK_LDB_DI1_DIV_3_5,
		       imx_clk_fixed_factor(dev, "ldb_di1_div_3_5", "ldb_di1",
					    2, 7));
		clk_dm(IMX6QDL_CLK_LDB_DI0_PODF,
		       imx_clk_divider(dev, "ldb_di0_podf", "ldb_di0_div_3_5",
				       base + 0x20, 10, 1));
		clk_dm(IMX6QDL_CLK_LDB_DI1_PODF,
		       imx_clk_divider(dev, "ldb_di1_podf", "ldb_di1_div_3_5",
				       base + 0x20, 11, 1));
	} else {
		clk_dm(IMX6QDL_CLK_LDB_DI0_DIV_3_5,
		       imx_clk_fixed_factor(dev, "ldb_di0_div_3_5",
					    "ldb_di0_sel", 2, 7));
		clk_dm(IMX6QDL_CLK_LDB_DI1_DIV_3_5,
		       imx_clk_fixed_factor(dev, "ldb_di1_div_3_5",
					    "ldb_di1_sel", 2, 7));
		clk_dm(IMX6QDL_CLK_LDB_DI0_PODF,
		       imx_clk_divider(dev, "ldb_di0_podf", "ldb_di0_div_3_5",
				       base + 0x20, 10, 1));
		clk_dm(IMX6QDL_CLK_LDB_DI1_PODF,
		       imx_clk_divider(dev, "ldb_di1_podf", "ldb_di1_div_3_5",
				       base + 0x20, 11, 1));
		clk_dm(IMX6QDL_CLK_LDB_DI0,
		       imx_clk_gate2(dev, "ldb_di0", "ldb_di0_podf",
				     base + 0x74, 12));
		clk_dm(IMX6QDL_CLK_LDB_DI1,
		       imx_clk_gate2(dev, "ldb_di1", "ldb_di1_podf",
				     base + 0x74, 14));
	}

	clk_dm(IMX6QDL_CLK_IPU1_PODF,
	       imx_clk_divider(dev, "ipu1_podf", "ipu1_sel", base + 0x3c, 11,
			       3));
	clk_dm(IMX6QDL_CLK_IPU2_PODF,
	       imx_clk_divider(dev, "ipu2_podf", "ipu2_sel", base + 0x3c, 16,
			       3));
	clk_dm(IMX6QDL_CLK_IPU1_DI0_PRE,
	       imx_clk_divider(dev, "ipu1_di0_pre", "ipu1_di0_pre_sel",
			       base + 0x34, 3, 3));
	clk_dm(IMX6QDL_CLK_IPU1_DI1_PRE,
	       imx_clk_divider(dev, "ipu1_di1_pre", "ipu1_di1_pre_sel",
			       base + 0x34, 12, 3));
	clk_dm(IMX6QDL_CLK_IPU2_DI0_PRE,
	       imx_clk_divider(dev, "ipu2_di0_pre", "ipu2_di0_pre_sel",
			       base + 0x38, 3, 3));
	clk_dm(IMX6QDL_CLK_IPU2_DI1_PRE,
	       imx_clk_divider(dev, "ipu2_di1_pre", "ipu2_di1_pre_sel",
			       base + 0x38, 12, 3));

	if (of_machine_is_compatible("fsl,imx6qp")) {
		clk_dm(IMX6QDL_CLK_IPU1_DI0_SEL,
		       imx_clk_mux_flags(dev, "ipu1_di0_sel", base + 0x34, 0, 3,
					 ipu1_di0_sels_2,
					 ARRAY_SIZE(ipu1_di0_sels_2),
					 CLK_SET_RATE_PARENT));
		clk_dm(IMX6QDL_CLK_IPU1_DI1_SEL,
		       imx_clk_mux_flags(dev, "ipu1_di1_sel", base + 0x34, 9, 3,
					 ipu1_di1_sels_2,
					 ARRAY_SIZE(ipu1_di1_sels_2),
					 CLK_SET_RATE_PARENT));
		clk_dm(IMX6QDL_CLK_IPU2_DI0_SEL,
		       imx_clk_mux_flags(dev, "ipu2_di0_sel", base + 0x38, 0, 3,
					 ipu2_di0_sels_2,
					 ARRAY_SIZE(ipu2_di0_sels_2),
					 CLK_SET_RATE_PARENT));
		clk_dm(IMX6QDL_CLK_IPU2_DI1_SEL,
		       imx_clk_mux_flags(dev, "ipu2_di1_sel", base + 0x38, 9, 3,
					 ipu2_di1_sels_2,
					 ARRAY_SIZE(ipu2_di1_sels_2),
					 CLK_SET_RATE_PARENT));
	} else {
		clk_dm(IMX6QDL_CLK_IPU1_DI0_SEL,
		       imx_clk_mux_flags(dev, "ipu1_di0_sel", base + 0x34, 0, 3,
					 ipu1_di0_sels,
					 ARRAY_SIZE(ipu1_di0_sels),
					 CLK_SET_RATE_PARENT));
		clk_dm(IMX6QDL_CLK_IPU1_DI1_SEL,
		       imx_clk_mux_flags(dev, "ipu1_di1_sel", base + 0x34, 9, 3,
					 ipu1_di1_sels,
					 ARRAY_SIZE(ipu1_di1_sels),
					 CLK_SET_RATE_PARENT));
		clk_dm(IMX6QDL_CLK_IPU2_DI0_SEL,
		       imx_clk_mux_flags(dev, "ipu2_di0_sel", base + 0x38, 0, 3,
					 ipu2_di0_sels,
					 ARRAY_SIZE(ipu2_di0_sels),
					 CLK_SET_RATE_PARENT));
		clk_dm(IMX6QDL_CLK_IPU2_DI1_SEL,
		       imx_clk_mux_flags(dev, "ipu2_di1_sel", base + 0x38, 9, 3,
					 ipu2_di1_sels,
					 ARRAY_SIZE(ipu2_di1_sels),
					 CLK_SET_RATE_PARENT));
	}

	clk_dm(IMX6QDL_CLK_ECSPI1,
	       imx_clk_gate2(dev, "ecspi1", "ecspi_root", base + 0x6c, 0));
	clk_dm(IMX6QDL_CLK_ECSPI2,
	       imx_clk_gate2(dev, "ecspi2", "ecspi_root", base + 0x6c, 2));
	clk_dm(IMX6QDL_CLK_ECSPI3,
	       imx_clk_gate2(dev, "ecspi3", "ecspi_root", base + 0x6c, 4));
	clk_dm(IMX6QDL_CLK_ECSPI4,
	       imx_clk_gate2(dev, "ecspi4", "ecspi_root", base + 0x6c, 6));
	clk_dm(IMX6QDL_CLK_UART_SERIAL,
	       imx_clk_gate2(dev, "uart_serial", "uart_serial_podf",
			     base + 0x7c, 26));
	clk_dm(IMX6QDL_CLK_USBOH3,
	       imx_clk_gate2(dev, "usboh3", "ipg", base + 0x80, 0));
	clk_dm(IMX6QDL_CLK_USDHC1,
	       imx_clk_gate2(dev, "usdhc1", "usdhc1_podf", base + 0x80, 2));
	clk_dm(IMX6QDL_CLK_USDHC2,
	       imx_clk_gate2(dev, "usdhc2", "usdhc2_podf", base + 0x80, 4));
	clk_dm(IMX6QDL_CLK_USDHC3,
	       imx_clk_gate2(dev, "usdhc3", "usdhc3_podf", base + 0x80, 6));
	clk_dm(IMX6QDL_CLK_USDHC4,
	       imx_clk_gate2(dev, "usdhc4", "usdhc4_podf", base + 0x80, 8));
	clk_dm(IMX6QDL_CLK_I2C1,
	       imx_clk_gate2(dev, "i2c1", "ipg_per", base + 0x70, 6));
	clk_dm(IMX6QDL_CLK_I2C2,
	       imx_clk_gate2(dev, "i2c2", "ipg_per", base + 0x70, 8));
	clk_dm(IMX6QDL_CLK_I2C3,
	       imx_clk_gate2(dev, "i2c3", "ipg_per", base + 0x70, 10));
	clk_dm(IMX6QDL_CLK_PWM1,
	       imx_clk_gate2(dev, "pwm1", "ipg_per", base + 0x78, 16));
	clk_dm(IMX6QDL_CLK_PWM2,
	       imx_clk_gate2(dev, "pwm2", "ipg_per", base + 0x78, 18));
	clk_dm(IMX6QDL_CLK_PWM3,
	       imx_clk_gate2(dev, "pwm3", "ipg_per", base + 0x78, 20));
	clk_dm(IMX6QDL_CLK_PWM4,
	       imx_clk_gate2(dev, "pwm4", "ipg_per", base + 0x78, 22));
	clk_dm(IMX6QDL_CLK_ENET,
	       imx_clk_gate2(dev, "enet", "ipg", base + 0x6c, 10));
	clk_dm(IMX6QDL_CLK_ENET_REF,
	       imx_clk_fixed_factor(dev, "enet_ref", "pll6_enet", 1, 1));
	clk_dm(IMX6QDL_CLK_MIPI_CORE_CFG,
	       imx_clk_gate2_shared(dev, "mipi_core_cfg", "video_27m",
				    base + 0x74, 16,
				    &share_count_mipi_core_cfg));
	clk_dm(IMX6QDL_CLK_HDMI_IAHB,
	       imx_clk_gate2(dev, "hdmi_iahb", "ahb", base + 0x70, 0));
	clk_dm(IMX6QDL_CLK_HDMI_ISFR,
	       imx_clk_gate2(dev, "hdmi_isfr", "mipi_core_cfg", base + 0x70,
			     4));
	clk_dm(IMX6QDL_CLK_IPU1,
	       imx_clk_gate2(dev, "ipu1", "ipu1_podf", base + 0x74, 0));
	clk_dm(IMX6QDL_CLK_IPU2,
	       imx_clk_gate2(dev, "ipu2", "ipu2_podf", base + 0x74, 6));
	clk_dm(IMX6QDL_CLK_IPU1_DI0,
	       imx_clk_gate2(dev, "ipu1_di0", "ipu1_di0_sel", base + 0x74, 2));
	clk_dm(IMX6QDL_CLK_IPU1_DI1,
	       imx_clk_gate2(dev, "ipu1_di1", "ipu1_di1_sel", base + 0x74, 4));
	clk_dm(IMX6QDL_CLK_IPU2_DI0,
	       imx_clk_gate2(dev, "ipu2_di0", "ipu2_di0_sel", base + 0x74, 8));
	clk_dm(IMX6QDL_CLK_IPU2_DI1,
	       imx_clk_gate2(dev, "ipu2_di1", "ipu2_di1_sel", base + 0x74, 10));

	if (of_machine_is_compatible("fsl,imx6dl")) {
		SET_CLK_RATE(IMX6QDL_CLK_PLL3_PFD1_540M, 540000000UL);
		SET_CLK_PARENT(IMX6QDL_CLK_IPU1_SEL,
			       IMX6QDL_CLK_PLL3_PFD1_540M);
	}

	return 0;
}

static const struct udevice_id imx6q_clk_ids[] = {
	{ .compatible = "fsl,imx6q-ccm" },
	{},
};

U_BOOT_DRIVER(imx6q_clk) = {
	.name = "clk_imx6q",
	.id = UCLASS_CLK,
	.of_match = imx6q_clk_ids,
	.ops = &imx6q_clk_ops,
	.probe = imx6q_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
