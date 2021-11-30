// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 DDRSS driver
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <ram.h>
#include <hang.h>
#include <log.h>
#include <asm/io.h>
#include <power-domain.h>
#include <wait_bit.h>
#include <power/regulator.h>

#include "lpddr4_obj_if.h"
#include "lpddr4_if.h"
#include "lpddr4_structs_if.h"
#include "lpddr4_ctl_regs.h"

#define SRAM_MAX 512

#define CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS	0x80
#define CTRLMMR_DDR4_FSP_CLKCHNG_ACK_OFFS	0xc0

#define DDRSS_V2A_R1_MAT_REG			0x0020
#define DDRSS_ECC_CTRL_REG			0x0120

struct k3_ddrss_desc {
	struct udevice *dev;
	void __iomem *ddrss_ss_cfg;
	void __iomem *ddrss_ctrl_mmr;
	struct power_domain ddrcfg_pwrdmn;
	struct power_domain ddrdata_pwrdmn;
	struct clk ddr_clk;
	struct clk osc_clk;
	u32 ddr_freq1;
	u32 ddr_freq2;
	u32 ddr_fhs_cnt;
	struct udevice *vtt_supply;
};

static lpddr4_obj *driverdt;
static lpddr4_config config;
static lpddr4_privatedata pd;

static struct k3_ddrss_desc *ddrss;

struct reginitdata {
	u32 ctl_regs[LPDDR4_INTR_CTL_REG_COUNT];
	u16 ctl_regs_offs[LPDDR4_INTR_CTL_REG_COUNT];
	u32 pi_regs[LPDDR4_INTR_PHY_INDEP_REG_COUNT];
	u16 pi_regs_offs[LPDDR4_INTR_PHY_INDEP_REG_COUNT];
	u32 phy_regs[LPDDR4_INTR_PHY_REG_COUNT];
	u16 phy_regs_offs[LPDDR4_INTR_PHY_REG_COUNT];
};

#define TH_MACRO_EXP(fld, str) (fld##str)

#define TH_FLD_MASK(fld)  TH_MACRO_EXP(fld, _MASK)
#define TH_FLD_SHIFT(fld) TH_MACRO_EXP(fld, _SHIFT)
#define TH_FLD_WIDTH(fld) TH_MACRO_EXP(fld, _WIDTH)
#define TH_FLD_WOCLR(fld) TH_MACRO_EXP(fld, _WOCLR)
#define TH_FLD_WOSET(fld) TH_MACRO_EXP(fld, _WOSET)

#define str(s) #s
#define xstr(s) str(s)

#define CTL_SHIFT 11
#define PHY_SHIFT 11
#define PI_SHIFT 10

#define DENALI_CTL_0_DRAM_CLASS_DDR4		0xA
#define DENALI_CTL_0_DRAM_CLASS_LPDDR4		0xB

#define TH_OFFSET_FROM_REG(REG, SHIFT, offset) do {\
	char *i, *pstr = xstr(REG); offset = 0;\
	for (i = &pstr[SHIFT]; *i != '\0'; ++i) {\
		offset = offset * 10 + (*i - '0'); } \
	} while (0)

static u32 k3_lpddr4_read_ddr_type(void)
{
	u32 status = 0U;
	u32 offset = 0U;
	u32 regval = 0U;
	u32 dram_class = 0U;

	TH_OFFSET_FROM_REG(LPDDR4__DRAM_CLASS__REG, CTL_SHIFT, offset);
	status = driverdt->readreg(&pd, LPDDR4_CTL_REGS, offset, &regval);
	if (status > 0U) {
		printf("%s: Failed to read DRAM_CLASS\n", __func__);
		hang();
	}

	dram_class = ((regval & TH_FLD_MASK(LPDDR4__DRAM_CLASS__FLD)) >>
		TH_FLD_SHIFT(LPDDR4__DRAM_CLASS__FLD));
	return dram_class;
}

static void k3_lpddr4_freq_update(void)
{
	unsigned int req_type, counter;

	for (counter = 0; counter < ddrss->ddr_fhs_cnt; counter++) {
		if (wait_for_bit_le32(ddrss->ddrss_ctrl_mmr +
				      CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS, 0x80,
				      true, 10000, false)) {
			printf("Timeout during frequency handshake\n");
			hang();
		}

		req_type = readl(ddrss->ddrss_ctrl_mmr +
				 CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS) & 0x03;

		debug("%s: received freq change req: req type = %d, req no. = %d\n",
		      __func__, req_type, counter);

		if (req_type == 1)
			clk_set_rate(&ddrss->ddr_clk, ddrss->ddr_freq1);
		else if (req_type == 2)
			clk_set_rate(&ddrss->ddr_clk, ddrss->ddr_freq2);
		else if (req_type == 0)
			/* Put DDR pll in bypass mode */
			clk_set_rate(&ddrss->ddr_clk,
				     clk_get_rate(&ddrss->osc_clk));
		else
			printf("%s: Invalid freq request type\n", __func__);

		writel(0x1, ddrss->ddrss_ctrl_mmr +
		       CTRLMMR_DDR4_FSP_CLKCHNG_ACK_OFFS);
		if (wait_for_bit_le32(ddrss->ddrss_ctrl_mmr +
				      CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS, 0x80,
				      false, 10, false)) {
			printf("Timeout during frequency handshake\n");
			hang();
		}
		writel(0x0, ddrss->ddrss_ctrl_mmr +
		       CTRLMMR_DDR4_FSP_CLKCHNG_ACK_OFFS);
	}
}

static void k3_lpddr4_ack_freq_upd_req(void)
{
	u32 dram_class;

	debug("--->>> LPDDR4 Initialization is in progress ... <<<---\n");

	dram_class = k3_lpddr4_read_ddr_type();

	switch (dram_class) {
	case DENALI_CTL_0_DRAM_CLASS_DDR4:
		break;
	case DENALI_CTL_0_DRAM_CLASS_LPDDR4:
		k3_lpddr4_freq_update();
		break;
	default:
		printf("Unrecognized dram_class cannot update frequency!\n");
	}
}

static int k3_ddrss_init_freq(struct k3_ddrss_desc *ddrss)
{
	u32 dram_class;
	int ret;

	dram_class = k3_lpddr4_read_ddr_type();

	switch (dram_class) {
	case DENALI_CTL_0_DRAM_CLASS_DDR4:
		/* Set to ddr_freq1 from DT for DDR4 */
		ret = clk_set_rate(&ddrss->ddr_clk, ddrss->ddr_freq1);
		break;
	case DENALI_CTL_0_DRAM_CLASS_LPDDR4:
		/* Set to bypass frequency for LPDDR4*/
		ret = clk_set_rate(&ddrss->ddr_clk, clk_get_rate(&ddrss->osc_clk));
		break;
	default:
		ret = -EINVAL;
		printf("Unrecognized dram_class cannot init frequency!\n");
	}

	if (ret < 0)
		dev_err(ddrss->dev, "ddr clk init failed: %d\n", ret);
	else
		ret = 0;

	return ret;
}

static void k3_lpddr4_info_handler(const lpddr4_privatedata *pd,
				   lpddr4_infotype infotype)
{
	if (infotype == LPDDR4_DRV_SOC_PLL_UPDATE)
		k3_lpddr4_ack_freq_upd_req();
}

static int k3_ddrss_power_on(struct k3_ddrss_desc *ddrss)
{
	int ret;

	debug("%s(ddrss=%p)\n", __func__, ddrss);

	ret = power_domain_on(&ddrss->ddrcfg_pwrdmn);
	if (ret) {
		dev_err(ddrss->dev, "power_domain_on() failed: %d\n", ret);
		return ret;
	}

	ret = power_domain_on(&ddrss->ddrdata_pwrdmn);
	if (ret) {
		dev_err(ddrss->dev, "power_domain_on() failed: %d\n", ret);
		return ret;
	}

	ret = device_get_supply_regulator(ddrss->dev, "vtt-supply",
					  &ddrss->vtt_supply);
	if (ret) {
		dev_dbg(ddrss->dev, "vtt-supply not found.\n");
	} else {
		ret = regulator_set_value(ddrss->vtt_supply, 3300000);
		if (ret)
			return ret;
		dev_dbg(ddrss->dev, "VTT regulator enabled, volt = %d\n",
			regulator_get_value(ddrss->vtt_supply));
	}

	return 0;
}

static int k3_ddrss_ofdata_to_priv(struct udevice *dev)
{
	struct k3_ddrss_desc *ddrss = dev_get_priv(dev);
	phys_addr_t reg;
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	reg = dev_read_addr_name(dev, "cfg");
	if (reg == FDT_ADDR_T_NONE) {
		dev_err(dev, "No reg property for DDRSS wrapper logic\n");
		return -EINVAL;
	}
	ddrss->ddrss_ss_cfg = (void *)reg;

	reg = dev_read_addr_name(dev, "ctrl_mmr_lp4");
	if (reg == FDT_ADDR_T_NONE) {
		dev_err(dev, "No reg property for CTRL MMR\n");
		return -EINVAL;
	}
	ddrss->ddrss_ctrl_mmr = (void *)reg;

	ret = power_domain_get_by_index(dev, &ddrss->ddrcfg_pwrdmn, 0);
	if (ret) {
		dev_err(dev, "power_domain_get() failed: %d\n", ret);
		return ret;
	}

	ret = power_domain_get_by_index(dev, &ddrss->ddrdata_pwrdmn, 1);
	if (ret) {
		dev_err(dev, "power_domain_get() failed: %d\n", ret);
		return ret;
	}

	ret = clk_get_by_index(dev, 0, &ddrss->ddr_clk);
	if (ret)
		dev_err(dev, "clk get failed%d\n", ret);

	ret = clk_get_by_index(dev, 1, &ddrss->osc_clk);
	if (ret)
		dev_err(dev, "clk get failed for osc clk %d\n", ret);

	ret = dev_read_u32(dev, "ti,ddr-freq1", &ddrss->ddr_freq1);
	if (ret)
		dev_err(dev, "ddr freq1 not populated %d\n", ret);

	ret = dev_read_u32(dev, "ti,ddr-freq2", &ddrss->ddr_freq2);
	if (ret)
		dev_err(dev, "ddr freq2 not populated %d\n", ret);

	ret = dev_read_u32(dev, "ti,ddr-fhs-cnt", &ddrss->ddr_fhs_cnt);
	if (ret)
		dev_err(dev, "ddr fhs cnt not populated %d\n", ret);

	return ret;
}

void k3_lpddr4_probe(void)
{
	u32 status = 0U;
	u16 configsize = 0U;

	status = driverdt->probe(&config, &configsize);

	if ((status != 0) || (configsize != sizeof(lpddr4_privatedata))
	    || (configsize > SRAM_MAX)) {
		printf("%s: FAIL\n", __func__);
		hang();
	} else {
		debug("%s: PASS\n", __func__);
	}
}

void k3_lpddr4_init(void)
{
	u32 status = 0U;

	if ((sizeof(pd) != sizeof(lpddr4_privatedata))
	    || (sizeof(pd) > SRAM_MAX)) {
		printf("%s: FAIL\n", __func__);
		hang();
	}

	config.ctlbase = (struct lpddr4_ctlregs_s *)ddrss->ddrss_ss_cfg;
	config.infohandler = (lpddr4_infocallback) k3_lpddr4_info_handler;

	status = driverdt->init(&pd, &config);

	if ((status > 0U) ||
	    (pd.ctlbase != (struct lpddr4_ctlregs_s *)config.ctlbase) ||
	    (pd.ctlinterrupthandler != config.ctlinterrupthandler) ||
	    (pd.phyindepinterrupthandler != config.phyindepinterrupthandler)) {
		printf("%s: FAIL\n", __func__);
		hang();
	} else {
		debug("%s: PASS\n", __func__);
	}
}

void populate_data_array_from_dt(struct reginitdata *reginit_data)
{
	int ret, i;

	ret = dev_read_u32_array(ddrss->dev, "ti,ctl-data",
				 (u32 *)reginit_data->ctl_regs,
				 LPDDR4_INTR_CTL_REG_COUNT);
	if (ret)
		printf("Error reading ctrl data %d\n", ret);

	for (i = 0; i < LPDDR4_INTR_CTL_REG_COUNT; i++)
		reginit_data->ctl_regs_offs[i] = i;

	ret = dev_read_u32_array(ddrss->dev, "ti,pi-data",
				 (u32 *)reginit_data->pi_regs,
				 LPDDR4_INTR_PHY_INDEP_REG_COUNT);
	if (ret)
		printf("Error reading PI data\n");

	for (i = 0; i < LPDDR4_INTR_PHY_INDEP_REG_COUNT; i++)
		reginit_data->pi_regs_offs[i] = i;

	ret = dev_read_u32_array(ddrss->dev, "ti,phy-data",
				 (u32 *)reginit_data->phy_regs,
				 LPDDR4_INTR_PHY_REG_COUNT);
	if (ret)
		printf("Error reading PHY data %d\n", ret);

	for (i = 0; i < LPDDR4_INTR_PHY_REG_COUNT; i++)
		reginit_data->phy_regs_offs[i] = i;
}

void k3_lpddr4_hardware_reg_init(void)
{
	u32 status = 0U;
	struct reginitdata reginitdata;

	populate_data_array_from_dt(&reginitdata);

	status = driverdt->writectlconfig(&pd, reginitdata.ctl_regs,
					  reginitdata.ctl_regs_offs,
					  LPDDR4_INTR_CTL_REG_COUNT);
	if (!status)
		status = driverdt->writephyindepconfig(&pd, reginitdata.pi_regs,
						       reginitdata.pi_regs_offs,
						       LPDDR4_INTR_PHY_INDEP_REG_COUNT);
	if (!status)
		status = driverdt->writephyconfig(&pd, reginitdata.phy_regs,
						  reginitdata.phy_regs_offs,
						  LPDDR4_INTR_PHY_REG_COUNT);
	if (status) {
		printf("%s: FAIL\n", __func__);
		hang();
	}
}

void k3_lpddr4_start(void)
{
	u32 status = 0U;
	u32 regval = 0U;
	u32 offset = 0U;

	TH_OFFSET_FROM_REG(LPDDR4__START__REG, CTL_SHIFT, offset);

	status = driverdt->readreg(&pd, LPDDR4_CTL_REGS, offset, &regval);
	if ((status > 0U) || ((regval & TH_FLD_MASK(LPDDR4__START__FLD)) != 0U)) {
		printf("%s: Pre start FAIL\n", __func__);
		hang();
	}

	status = driverdt->start(&pd);
	if (status > 0U) {
		printf("%s: FAIL\n", __func__);
		hang();
	}

	status = driverdt->readreg(&pd, LPDDR4_CTL_REGS, offset, &regval);
	if ((status > 0U) || ((regval & TH_FLD_MASK(LPDDR4__START__FLD)) != 1U)) {
		printf("%s: Post start FAIL\n", __func__);
		hang();
	} else {
		debug("%s: Post start PASS\n", __func__);
	}
}

static int k3_ddrss_probe(struct udevice *dev)
{
	int ret;

	ddrss = dev_get_priv(dev);

	debug("%s(dev=%p)\n", __func__, dev);

	ret = k3_ddrss_ofdata_to_priv(dev);
	if (ret)
		return ret;

	ddrss->dev = dev;
	ret = k3_ddrss_power_on(ddrss);
	if (ret)
		return ret;

#ifdef CONFIG_K3_AM64_DDRSS

	writel(0x000001EF, ddrss->ddrss_ss_cfg + DDRSS_V2A_R1_MAT_REG);
	writel(0x0, ddrss->ddrss_ss_cfg + DDRSS_ECC_CTRL_REG);
#endif

	driverdt = lpddr4_getinstance();
	k3_lpddr4_probe();
	k3_lpddr4_init();
	k3_lpddr4_hardware_reg_init();

	ret = k3_ddrss_init_freq(ddrss);
	if (ret)
		return ret;

	k3_lpddr4_start();

	return ret;
}

static int k3_ddrss_get_info(struct udevice *dev, struct ram_info *info)
{
	return 0;
}

static struct ram_ops k3_ddrss_ops = {
	.get_info = k3_ddrss_get_info,
};

static const struct udevice_id k3_ddrss_ids[] = {
	{.compatible = "ti,am64-ddrss"},
	{.compatible = "ti,j721e-ddrss"},
	{}
};

U_BOOT_DRIVER(k3_ddrss) = {
	.name			= "k3_ddrss",
	.id			= UCLASS_RAM,
	.of_match		= k3_ddrss_ids,
	.ops			= &k3_ddrss_ops,
	.probe			= k3_ddrss_probe,
	.priv_auto		= sizeof(struct k3_ddrss_desc),
};
