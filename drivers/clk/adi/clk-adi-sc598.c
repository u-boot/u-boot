// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Author: Greg Malysa <greg.malysa@timesys.com>
 *
 * Ported from Linux: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 */

#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-bindings/clock/adi-sc5xx-clock.h>
#include <linux/compiler_types.h>
#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/printk.h>
#include <linux/types.h>

#include "clk.h"

static const char * const cgu1_in_sels[] = {"sys_clkin0", "sys_clkin1"};
static const char * const cgu0_s1sels[] = {"cgu0_s1seldiv", "cgu0_s1selexdiv"};
static const char * const cgu1_s0sels[] = {"cgu1_s0seldiv", "cgu1_s0selexdiv"};
static const char * const cgu1_s1sels[] = {"cgu1_s1seldiv", "cgu1_s1selexdiv"};
static const char * const sharc0_sels[] = {"cclk0_0", "dummy", "dummy", "dummy"};
static const char * const sharc1_sels[] = {"cclk0_0", "dummy", "dummy", "dummy"};
static const char * const arm_sels[] = {"dummy", "dummy", "cclk2_0", "cclk2_1"};
static const char * const cdu_ddr_sels[] = {"dclk_0", "dclk_1", "dummy", "dummy"};
static const char * const can_sels[] = {"dummy", "oclk_1", "dummy", "dummy"};
static const char * const spdif_sels[] = {"sclk1_0", "dummy", "dummy", "dummy"};
static const char * const spi_sels[] = {"sclk0_0", "oclk_0", "dummy", "dummy"};
static const char * const gige_sels[] = {"sclk0_0", "sclk0_1", "dummy", "dummy"};
static const char * const lp_sels[] = {"oclk_0", "sclk0_0", "cclk0_1", "dummy"};
static const char * const lp_ddr_sels[] = {"oclk_0", "dclk_0", "sysclk_1", "dummy"};
static const char * const ospi_refclk_sels[] = {"sysclk_0", "sclk0_0", "sclk1_1",
	"dummy"};
static const char * const trace_sels[] = {"sclk0_0", "dummy", "dummy", "dummy"};
static const char * const emmc_sels[] = {"oclk_0", "sclk0_1", "dclk_0_half",
	"dclk_1_half"};
static const char * const emmc_timer_sels[] = {"dummy", "sclk1_1_half", "dummy",
	"dummy"};
static const char * const ddr_sels[] = {"cdu_ddr", "3pll_ddiv"};

static int sc598_clock_probe(struct udevice *dev)
{
	void __iomem *cgu0;
	void __iomem *cgu1;
	void __iomem *cdu;
	void __iomem *pll3;
	int ret;
	struct resource res;

	struct clk *clks[ADSP_SC598_CLK_END];
	struct clk dummy, clkin0, clkin1;

	ret = dev_read_resource_byname(dev, "cgu0", &res);
	if (ret)
		return ret;
	cgu0 = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "cgu1", &res);
	if (ret)
		return ret;
	cgu1 = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "cdu", &res);
	if (ret)
		return ret;
	cdu = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "pll3", &res);
	if (ret)
		return ret;
	pll3 = devm_ioremap(dev, res.start, resource_size(&res));

	// We only access this one register for pll3
	pll3 = pll3 + PLL3_OFFSET;

	// Input clock configuration
	clk_get_by_name(dev, "dummy", &dummy);
	clk_get_by_name(dev, "sys_clkin0", &clkin0);
	clk_get_by_name(dev, "sys_clkin1", &clkin1);

	clks[ADSP_SC598_CLK_DUMMY] = &dummy;
	clks[ADSP_SC598_CLK_SYS_CLKIN0] = &clkin0;
	clks[ADSP_SC598_CLK_SYS_CLKIN1] = &clkin1;

	clks[ADSP_SC598_CLK_CGU1_IN] = clk_register_mux(NULL, "cgu1_in_sel", cgu1_in_sels,
							2, CLK_SET_RATE_PARENT,
							cdu + CDU_CLKINSEL, 0, 1, 0);

	// 3rd pll reuses cgu1 clk in selection, feeds directly into 3pll df
	// changing the cgu1 in sel mux will affect 3pll so reuse the same clocks

	// CGU configuration and internal clocks
	clks[ADSP_SC598_CLK_CGU0_PLL_IN] = clk_register_divider(NULL, "cgu0_df",
								"sys_clkin0",
								CLK_SET_RATE_PARENT,
								cgu0 + CGU_CTL, 0, 1, 0);
	clks[ADSP_SC598_CLK_CGU1_PLL_IN] = clk_register_divider(NULL, "cgu1_df",
								"cgu1_in_sel",
								CLK_SET_RATE_PARENT,
								cgu1 + CGU_CTL, 0, 1, 0);
	clks[ADSP_SC598_CLK_3PLL_PLL_IN] = clk_register_divider(NULL, "3pll_df",
								"cgu1_in_sel",
								CLK_SET_RATE_PARENT,
								pll3, 3, 1, 0);

	// VCO output inside PLL
	clks[ADSP_SC598_CLK_CGU0_VCO_OUT] = sc5xx_cgu_pll("cgu0_vco", "cgu0_df",
							  cgu0 + CGU_CTL, CGU_MSEL_SHIFT,
							  CGU_MSEL_WIDTH, 0, true);
	clks[ADSP_SC598_CLK_CGU1_VCO_OUT] = sc5xx_cgu_pll("cgu1_vco", "cgu1_df",
							  cgu1 + CGU_CTL, CGU_MSEL_SHIFT,
							  CGU_MSEL_WIDTH, 0, true);
	clks[ADSP_SC598_CLK_3PLL_VCO_OUT] = sc5xx_cgu_pll("3pll_vco", "3pll_df",
							  pll3, PLL3_MSEL_SHIFT,
							  PLL3_MSEL_WIDTH, 1, true);

	// Final PLL output
	clks[ADSP_SC598_CLK_CGU0_PLLCLK] = clk_register_fixed_factor(NULL, "cgu0_pllclk",
								     "cgu0_vco",
								     CLK_SET_RATE_PARENT,
								     1, 2);
	clks[ADSP_SC598_CLK_CGU1_PLLCLK] = clk_register_fixed_factor(NULL, "cgu1_pllclk",
								     "cgu1_vco",
								     CLK_SET_RATE_PARENT,
								     1, 2);
	clks[ADSP_SC598_CLK_3PLL_PLLCLK] = clk_register_fixed_factor(NULL, "3pll_pllclk",
								     "3pll_vco",
								     CLK_SET_RATE_PARENT,
								     1, 2);

	// Dividers from pll output
	clks[ADSP_SC598_CLK_CGU0_CDIV] = cgu_divider("cgu0_cdiv", "cgu0_pllclk",
						     cgu0 + CGU_DIV, 0, 5, 0);
	clks[ADSP_SC598_CLK_CGU0_SYSCLK] = cgu_divider("sysclk_0", "cgu0_pllclk",
						       cgu0 + CGU_DIV, 8, 5, 0);
	clks[ADSP_SC598_CLK_CGU0_DDIV] = cgu_divider("cgu0_ddiv", "cgu0_pllclk",
						     cgu0 + CGU_DIV, 16, 5, 0);
	clks[ADSP_SC598_CLK_CGU0_ODIV] = cgu_divider("cgu0_odiv", "cgu0_pllclk",
						     cgu0 + CGU_DIV, 22, 7, 0);
	clks[ADSP_SC598_CLK_CGU0_S0SELDIV] = cgu_divider("cgu0_s0seldiv", "sysclk_0",
							 cgu0 + CGU_DIV, 5, 3, 0);
	clks[ADSP_SC598_CLK_CGU0_S1SELDIV] = cgu_divider("cgu0_s1seldiv", "sysclk_0",
							 cgu0 + CGU_DIV, 13, 3, 0);
	clks[ADSP_SC598_CLK_CGU0_S1SELEXDIV] = cgu_divider("cgu0_s1selexdiv",
							   "cgu0_pllclk",
							   cgu0 + CGU_DIVEX, 16, 8, 0);
	clks[ADSP_SC598_CLK_CGU0_S1SEL] = clk_register_mux(NULL, "cgu0_sclk1sel",
							   cgu0_s1sels, 2,
							   CLK_SET_RATE_PARENT,
							   cgu0 + CGU_CTL, 17, 1, 0);
	clks[ADSP_SC598_CLK_CGU0_CCLK2] = clk_register_fixed_factor(NULL, "cclk2_0",
								    "cgu0_vco",
								    CLK_SET_RATE_PARENT,
								    1, 3);

	clks[ADSP_SC598_CLK_CGU1_CDIV] = cgu_divider("cgu1_cdiv", "cgu1_pllclk",
						     cgu1 + CGU_DIV, 0, 5, 0);
	clks[ADSP_SC598_CLK_CGU1_SYSCLK] = cgu_divider("sysclk_1", "cgu1_pllclk",
						       cgu1 + CGU_DIV, 8, 5, 0);
	clks[ADSP_SC598_CLK_CGU1_DDIV] = cgu_divider("cgu1_ddiv", "cgu1_pllclk",
						     cgu1 + CGU_DIV, 16, 5, 0);
	clks[ADSP_SC598_CLK_CGU1_ODIV] = cgu_divider("cgu1_odiv", "cgu1_pllclk",
						     cgu1 + CGU_DIV, 22, 7, 0);
	clks[ADSP_SC598_CLK_CGU1_S0SELDIV] = cgu_divider("cgu1_s0seldiv", "sysclk_1",
							 cgu1 + CGU_DIV, 5, 3, 0);
	clks[ADSP_SC598_CLK_CGU1_S1SELDIV] = cgu_divider("cgu1_s1seldiv", "sysclk_1",
							 cgu1 + CGU_DIV, 13, 3, 0);
	clks[ADSP_SC598_CLK_CGU1_S0SELEXDIV] = cgu_divider("cgu1_s0selexdiv",
							   "cgu1_pllclk",
							   cgu1 + CGU_DIVEX, 0, 8, 0);
	clks[ADSP_SC598_CLK_CGU1_S1SELEXDIV] = cgu_divider("cgu1_s1selexdiv",
							   "cgu1_pllclk",
							   cgu1 + CGU_DIVEX, 16, 8, 0);
	clks[ADSP_SC598_CLK_CGU1_S0SEL] = clk_register_mux(NULL, "cgu1_sclk0sel",
							   cgu1_s0sels, 2,
							   CLK_SET_RATE_PARENT,
							   cgu1 + CGU_CTL, 16, 1, 0);
	clks[ADSP_SC598_CLK_CGU1_S1SEL] = clk_register_mux(NULL, "cgu1_sclk1sel",
							   cgu1_s1sels, 2,
							   CLK_SET_RATE_PARENT,
							   cgu1 + CGU_CTL, 17, 1, 0);
	clks[ADSP_SC598_CLK_CGU1_CCLK2] = clk_register_fixed_factor(NULL, "cclk2_1",
								    "cgu1_vco",
								    CLK_SET_RATE_PARENT,
								    1, 3);

	clks[ADSP_SC598_CLK_3PLL_DDIV] = clk_register_divider(NULL, "3pll_ddiv",
							      "3pll_pllclk",
							      CLK_SET_RATE_PARENT, pll3,
							      12, 5, 0);

	// Gates to enable CGU outputs
	clks[ADSP_SC598_CLK_CGU0_CCLK0] = cgu_gate("cclk0_0", "cgu0_cdiv",
						   cgu0 + CGU_CCBF_DIS, 0);
	clks[ADSP_SC598_CLK_CGU0_OCLK] = cgu_gate("oclk_0", "cgu0_odiv",
						  cgu0 + CGU_SCBF_DIS, 3);
	clks[ADSP_SC598_CLK_CGU0_DCLK] = cgu_gate("dclk_0", "cgu0_ddiv",
						  cgu0 + CGU_SCBF_DIS, 2);
	clks[ADSP_SC598_CLK_CGU0_SCLK1] = cgu_gate("sclk1_0", "cgu0_sclk1sel",
						   cgu0 + CGU_SCBF_DIS, 1);
	clks[ADSP_SC598_CLK_CGU0_SCLK0] = cgu_gate("sclk0_0", "cgu0_s0seldiv",
						   cgu0 + CGU_SCBF_DIS, 0);

	clks[ADSP_SC598_CLK_CGU1_CCLK0] = cgu_gate("cclk0_1", "cgu1_cdiv",
						   cgu1 + CGU_CCBF_DIS, 0);
	clks[ADSP_SC598_CLK_CGU1_OCLK] = cgu_gate("oclk_1", "cgu1_odiv",
						  cgu1 + CGU_SCBF_DIS, 3);
	clks[ADSP_SC598_CLK_CGU1_DCLK] = cgu_gate("dclk_1", "cgu1_ddiv",
						  cgu1 + CGU_SCBF_DIS, 2);
	clks[ADSP_SC598_CLK_CGU1_SCLK1] = cgu_gate("sclk1_1", "cgu1_sclk1sel",
						   cgu1 + CGU_SCBF_DIS, 1);
	clks[ADSP_SC598_CLK_CGU1_SCLK0] = cgu_gate("sclk0_1", "cgu1_sclk0sel",
						   cgu1 + CGU_SCBF_DIS, 0);

	// Extra half rate clocks generated in the CDU
	clks[ADSP_SC598_CLK_DCLK0_HALF] = clk_register_fixed_factor(NULL, "dclk_0_half",
								    "dclk_0",
								    CLK_SET_RATE_PARENT,
								    1, 2);
	clks[ADSP_SC598_CLK_DCLK1_HALF] = clk_register_fixed_factor(NULL, "dclk_1_half",
								    "dclk_1",
								    CLK_SET_RATE_PARENT,
								    1, 2);
	clks[ADSP_SC598_CLK_CGU1_SCLK1_HALF] = clk_register_fixed_factor(NULL,
									 "sclk1_1_half",
									 "sclk1_1",
									 CLK_SET_RATE_PARENT,
									 1, 2);

	// CDU output muxes
	clks[ADSP_SC598_CLK_SHARC0_SEL] = cdu_mux("sharc0_sel", cdu + CDU_CFG0,
						  sharc0_sels);
	clks[ADSP_SC598_CLK_SHARC1_SEL] = cdu_mux("sharc1_sel", cdu + CDU_CFG1,
						  sharc1_sels);
	clks[ADSP_SC598_CLK_ARM_SEL] = cdu_mux("arm_sel", cdu + CDU_CFG2, arm_sels);
	clks[ADSP_SC598_CLK_CDU_DDR_SEL] = cdu_mux("cdu_ddr_sel", cdu + CDU_CFG3,
						   cdu_ddr_sels);
	clks[ADSP_SC598_CLK_CAN_SEL] = cdu_mux("can_sel", cdu + CDU_CFG4, can_sels);
	clks[ADSP_SC598_CLK_SPDIF_SEL] = cdu_mux("spdif_sel", cdu + CDU_CFG5, spdif_sels);
	clks[ADSP_SC598_CLK_SPI_SEL] = cdu_mux("spi_sel", cdu + CDU_CFG6, spi_sels);
	clks[ADSP_SC598_CLK_GIGE_SEL] = cdu_mux("gige_sel", cdu + CDU_CFG7, gige_sels);
	clks[ADSP_SC598_CLK_LP_SEL] = cdu_mux("lp_sel", cdu + CDU_CFG8, lp_sels);
	clks[ADSP_SC598_CLK_LP_DDR_SEL] = cdu_mux("lp_ddr_sel", cdu + CDU_CFG9,
						  lp_ddr_sels);
	clks[ADSP_SC598_CLK_OSPI_REFCLK_SEL] = cdu_mux("ospi_refclk_sel", cdu + CDU_CFG10,
						       ospi_refclk_sels);
	clks[ADSP_SC598_CLK_TRACE_SEL] = cdu_mux("trace_sel", cdu + CDU_CFG12,
						 trace_sels);
	clks[ADSP_SC598_CLK_EMMC_SEL] = cdu_mux("emmc_sel", cdu + CDU_CFG13, emmc_sels);
	clks[ADSP_SC598_CLK_EMMC_TIMER_QMC_SEL] = cdu_mux("emmc_timer_qmc_sel",
							  cdu + CDU_CFG14,
							  emmc_timer_sels);

	// CDU output enable gates
	clks[ADSP_SC598_CLK_SHARC0] = cdu_gate("sharc0", "sharc0_sel", cdu + CDU_CFG0,
					       CLK_IS_CRITICAL);
	clks[ADSP_SC598_CLK_SHARC1] = cdu_gate("sharc1", "sharc1_sel", cdu + CDU_CFG1,
					       CLK_IS_CRITICAL);
	clks[ADSP_SC598_CLK_ARM] = cdu_gate("arm", "arm_sel", cdu + CDU_CFG2,
					    CLK_IS_CRITICAL);
	clks[ADSP_SC598_CLK_CDU_DDR] = cdu_gate("cdu_ddr", "cdu_ddr_sel", cdu + CDU_CFG3,
						0);
	clks[ADSP_SC598_CLK_CAN] = cdu_gate("can", "can_sel", cdu + CDU_CFG4, 0);
	clks[ADSP_SC598_CLK_SPDIF] = cdu_gate("spdif", "spdif_sel", cdu + CDU_CFG5, 0);
	clks[ADSP_SC598_CLK_SPI] = cdu_gate("spi", "spi_sel", cdu + CDU_CFG6, 0);
	clks[ADSP_SC598_CLK_GIGE] = cdu_gate("gige", "gige_sel", cdu + CDU_CFG7, 0);
	clks[ADSP_SC598_CLK_LP] = cdu_gate("lp", "lp_sel", cdu + CDU_CFG8, 0);
	clks[ADSP_SC598_CLK_LP_DDR] = cdu_gate("lp_ddr", "lp_ddr_sel", cdu + CDU_CFG9, 0);
	clks[ADSP_SC598_CLK_OSPI_REFCLK] = cdu_gate("ospi_refclk", "ospi_refclk_sel",
						    cdu + CDU_CFG10, 0);
	clks[ADSP_SC598_CLK_TRACE] = cdu_gate("trace", "trace_sel", cdu + CDU_CFG12, 0);
	clks[ADSP_SC598_CLK_EMMC] = cdu_gate("emmc", "emmc_sel", cdu + CDU_CFG13, 0);
	clks[ADSP_SC598_CLK_EMMC_TIMER_QMC] = cdu_gate("emmc_timer_qmc",
						       "emmc_timer_qmc_sel",
						       cdu + CDU_CFG14, 0);

	// Dedicated DDR output mux
	clks[ADSP_SC598_CLK_DDR] = clk_register_mux(NULL, "ddr", ddr_sels, 2,
						    CLK_SET_RATE_PARENT | CLK_IS_CRITICAL,
						    pll3, 11, 1, 0);

	ret = cdu_check_clocks(clks, ARRAY_SIZE(clks));
	if (ret)
		pr_err("CDU error detected\n");

	return ret;
}

static const struct udevice_id adi_sc598_clk_ids[] = {
	{ .compatible = "adi,sc598-clocks" },
	{ },
};

U_BOOT_DRIVER(adi_sc598_clk) = {
	.name = "clk_adi_sc598",
	.id = UCLASS_CLK,
	.of_match = adi_sc598_clk_ids,
	.ops		= &adi_clk_ops,
	.probe = sc598_clock_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
