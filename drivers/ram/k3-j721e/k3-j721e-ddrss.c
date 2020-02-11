// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' J721E DDRSS driver
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <hang.h>
#include <ram.h>
#include <asm/io.h>
#include <power-domain.h>
#include <wait_bit.h>
#include <dm/device_compat.h>

#include "lpddr4_obj_if.h"
#include "lpddr4_if.h"
#include "lpddr4_structs_if.h"
#include "lpddr4_ctl_regs.h"

#define SRAM_MAX 512

#define CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS	0x80
#define CTRLMMR_DDR4_FSP_CLKCHNG_ACK_OFFS	0xc0

struct j721e_ddrss_desc {
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
};

static LPDDR4_OBJ *driverdt;
static lpddr4_config config;
static lpddr4_privatedata pd;

static struct j721e_ddrss_desc *ddrss;

#define TH_MACRO_EXP(fld, str) (fld##str)

#define TH_FLD_MASK(fld)  TH_MACRO_EXP(fld, _MASK)
#define TH_FLD_SHIFT(fld) TH_MACRO_EXP(fld, _SHIFT)
#define TH_FLD_WIDTH(fld) TH_MACRO_EXP(fld, _WIDTH)
#define TH_FLD_WOCLR(fld) TH_MACRO_EXP(fld, _WOCLR)
#define TH_FLD_WOSET(fld) TH_MACRO_EXP(fld, _WOSET)

#define str(s) #s
#define xstr(s) str(s)

#define  CTL_SHIFT 11
#define  PHY_SHIFT 11
#define  PI_SHIFT 10

#define TH_OFFSET_FROM_REG(REG, SHIFT, offset) do {\
	char *i, *pstr= xstr(REG); offset = 0;\
	for (i = &pstr[SHIFT]; *i != '\0'; ++i) {\
		offset = offset * 10 + (*i - '0'); }\
	} while (0)

static void j721e_lpddr4_ack_freq_upd_req(void)
{
	unsigned int req_type, counter;

	debug("--->>> LPDDR4 Initialization is in progress ... <<<---\n");

	for (counter = 0; counter < ddrss->ddr_fhs_cnt; counter++) {
		if (wait_for_bit_le32(ddrss->ddrss_ctrl_mmr +
				      CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS, 0x80,
				      true, 10000, false)) {
			printf("Timeout during frequency handshake\n");
			hang();
		}

		req_type = readl(ddrss->ddrss_ctrl_mmr +
				 CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS) & 0x03;

		debug("%s: received freq change req: req type = %d, req no. = %d \n",
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

static void j721e_lpddr4_info_handler(const lpddr4_privatedata * pd,
				      lpddr4_infotype infotype)
{
	if (infotype == LPDDR4_DRV_SOC_PLL_UPDATE) {
		j721e_lpddr4_ack_freq_upd_req();
	}
}

static int j721e_ddrss_power_on(struct j721e_ddrss_desc *ddrss)
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

	return 0;
}

static int j721e_ddrss_ofdata_to_priv(struct udevice *dev)
{
	struct j721e_ddrss_desc *ddrss = dev_get_priv(dev);
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

	/* Put DDR pll in bypass mode */
	ret = clk_set_rate(&ddrss->ddr_clk, clk_get_rate(&ddrss->osc_clk));
	if (ret)
		dev_err(dev, "ddr clk bypass failed\n");

	return ret;
}

void j721e_lpddr4_probe(void)
{
	uint32_t status = 0U;
	uint16_t configsize = 0U;

	status = driverdt->probe(&config, &configsize);

	if ((status != 0) || (configsize != sizeof(lpddr4_privatedata))
	    || (configsize > SRAM_MAX)) {
		printf("LPDDR4_Probe: FAIL\n");
		hang();
	} else {
		debug("LPDDR4_Probe: PASS\n");
	}
}

void j721e_lpddr4_init(void)
{
	uint32_t status = 0U;

	if ((sizeof(pd) != sizeof(lpddr4_privatedata))
	    || (sizeof(pd) > SRAM_MAX)) {
		printf("LPDDR4_Init: FAIL\n");
		hang();
	}

	config.ctlbase = (struct lpddr4_ctlregs_s *)ddrss->ddrss_ss_cfg;
	config.infohandler = (lpddr4_infocallback) j721e_lpddr4_info_handler;

	status = driverdt->init(&pd, &config);

	if ((status > 0U) ||
	    (pd.ctlbase != (struct lpddr4_ctlregs_s *)config.ctlbase) ||
	    (pd.ctlinterrupthandler != config.ctlinterrupthandler) ||
	    (pd.phyindepinterrupthandler != config.phyindepinterrupthandler)) {
		printf("LPDDR4_Init: FAIL\n");
		hang();
	} else {
		debug("LPDDR4_Init: PASS\n");
	}
}

void populate_data_array_from_dt(lpddr4_reginitdata * reginit_data)
{
	int ret, i;

	ret = dev_read_u32_array(ddrss->dev, "ti,ctl-data",
				 (u32 *) reginit_data->denalictlreg,
				 LPDDR4_CTL_REG_COUNT);
	if (ret)
		printf("Error reading ctrl data\n");

	for (i = 0; i < LPDDR4_CTL_REG_COUNT; i++)
		reginit_data->updatectlreg[i] = true;

	ret = dev_read_u32_array(ddrss->dev, "ti,pi-data",
				 (u32 *) reginit_data->denaliphyindepreg,
				 LPDDR4_PHY_INDEP_REG_COUNT);
	if (ret)
		printf("Error reading PI data\n");

	for (i = 0; i < LPDDR4_PHY_INDEP_REG_COUNT; i++)
		reginit_data->updatephyindepreg[i] = true;

	ret = dev_read_u32_array(ddrss->dev, "ti,phy-data",
				 (u32 *) reginit_data->denaliphyreg,
				 LPDDR4_PHY_REG_COUNT);
	if (ret)
		printf("Error reading PHY data\n");

	for (i = 0; i < LPDDR4_PHY_REG_COUNT; i++)
		reginit_data->updatephyreg[i] = true;
}

void j721e_lpddr4_hardware_reg_init(void)
{
	uint32_t status = 0U;
	lpddr4_reginitdata reginitdata;

	populate_data_array_from_dt(&reginitdata);

	status = driverdt->writectlconfig(&pd, &reginitdata);
	if (!status) {
		status = driverdt->writephyindepconfig(&pd, &reginitdata);
	}
	if (!status) {
		status = driverdt->writephyconfig(&pd, &reginitdata);
	}
	if (status) {
		printf(" ERROR: LPDDR4_HardwareRegInit failed!!\n");
		hang();
	}

	return;
}

void j721e_lpddr4_start(void)
{
	uint32_t status = 0U;
	uint32_t regval = 0U;
	uint32_t offset = 0U;

	TH_OFFSET_FROM_REG(LPDDR4__START__REG, CTL_SHIFT, offset);

	status = driverdt->readreg(&pd, LPDDR4_CTL_REGS, offset, &regval);
	if ((status > 0U) || ((regval & TH_FLD_MASK(LPDDR4__START__FLD)) != 0U)) {
		printf("LPDDR4_StartTest: FAIL\n");
		hang();
	}

	status = driverdt->start(&pd);
	if (status > 0U) {
		printf("LPDDR4_StartTest: FAIL\n");
		hang();
	}

	status = driverdt->readreg(&pd, LPDDR4_CTL_REGS, offset, &regval);
	if ((status > 0U) || ((regval & TH_FLD_MASK(LPDDR4__START__FLD)) != 1U)) {
		printf("LPDDR4_Start: FAIL\n");
		hang();
	} else {
		debug("LPDDR4_Start: PASS\n");
	}
}

static int j721e_ddrss_probe(struct udevice *dev)
{
	int ret;
	ddrss = dev_get_priv(dev);

	debug("%s(dev=%p)\n", __func__, dev);

	ret = j721e_ddrss_ofdata_to_priv(dev);
	if (ret)
		return ret;

	ddrss->dev = dev;
	ret = j721e_ddrss_power_on(ddrss);
	if (ret)
		return ret;

	driverdt = lpddr4_getinstance();
	j721e_lpddr4_probe();
	j721e_lpddr4_init();
	j721e_lpddr4_hardware_reg_init();
	j721e_lpddr4_start();

	return ret;
}

static int j721e_ddrss_get_info(struct udevice *dev, struct ram_info *info)
{
	return 0;
}

static struct ram_ops j721e_ddrss_ops = {
	.get_info = j721e_ddrss_get_info,
};

static const struct udevice_id j721e_ddrss_ids[] = {
	{.compatible = "ti,j721e-ddrss"},
	{}
};

U_BOOT_DRIVER(j721e_ddrss) = {
	.name = "j721e_ddrss",
	.id = UCLASS_RAM,
	.of_match = j721e_ddrss_ids,
	.ops = &j721e_ddrss_ops,
	.probe = j721e_ddrss_probe,
	.priv_auto_alloc_size = sizeof(struct j721e_ddrss_desc),
};
