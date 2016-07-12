/*
 * (C) Copyright 2015 Google, Inc
 * Copyright 2014 Rockchip Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0
 *
 * Adapted from coreboot.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <ram.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3288.h>
#include <asm/arch/ddr_rk3288.h>
#include <asm/arch/grf_rk3288.h>
#include <asm/arch/pmu_rk3288.h>
#include <asm/arch/sdram.h>
#include <linux/err.h>
#include <power/regulator.h>
#include <power/rk808_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

struct chan_info {
	struct rk3288_ddr_pctl *pctl;
	struct rk3288_ddr_publ *publ;
	struct rk3288_msch *msch;
};

struct dram_info {
	struct chan_info chan[2];
	struct ram_info info;
	struct clk ddr_clk;
	struct rk3288_cru *cru;
	struct rk3288_grf *grf;
	struct rk3288_sgrf *sgrf;
	struct rk3288_pmu *pmu;
};

#ifdef CONFIG_SPL_BUILD
static void copy_to_reg(u32 *dest, const u32 *src, u32 n)
{
	int i;

	for (i = 0; i < n / sizeof(u32); i++) {
		writel(*src, dest);
		src++;
		dest++;
	}
}

static void ddr_reset(struct rk3288_cru *cru, u32 ch, u32 ctl, u32 phy)
{
	u32 phy_ctl_srstn_shift = 4 + 5 * ch;
	u32 ctl_psrstn_shift = 3 + 5 * ch;
	u32 ctl_srstn_shift = 2 + 5 * ch;
	u32 phy_psrstn_shift = 1 + 5 * ch;
	u32 phy_srstn_shift = 5 * ch;

	rk_clrsetreg(&cru->cru_softrst_con[10],
		     1 << phy_ctl_srstn_shift | 1 << ctl_psrstn_shift |
		     1 << ctl_srstn_shift | 1 << phy_psrstn_shift |
		     1 << phy_srstn_shift,
		     phy << phy_ctl_srstn_shift | ctl << ctl_psrstn_shift |
		     ctl << ctl_srstn_shift | phy << phy_psrstn_shift |
		     phy << phy_srstn_shift);
}

static void ddr_phy_ctl_reset(struct rk3288_cru *cru, u32 ch, u32 n)
{
	u32 phy_ctl_srstn_shift = 4 + 5 * ch;

	rk_clrsetreg(&cru->cru_softrst_con[10],
		     1 << phy_ctl_srstn_shift, n << phy_ctl_srstn_shift);
}

static void phy_pctrl_reset(struct rk3288_cru *cru,
			    struct rk3288_ddr_publ *publ,
			    u32 channel)
{
	int i;

	ddr_reset(cru, channel, 1, 1);
	udelay(1);
	clrbits_le32(&publ->acdllcr, ACDLLCR_DLLSRST);
	for (i = 0; i < 4; i++)
		clrbits_le32(&publ->datx8[i].dxdllcr, DXDLLCR_DLLSRST);

	udelay(10);
	setbits_le32(&publ->acdllcr, ACDLLCR_DLLSRST);
	for (i = 0; i < 4; i++)
		setbits_le32(&publ->datx8[i].dxdllcr, DXDLLCR_DLLSRST);

	udelay(10);
	ddr_reset(cru, channel, 1, 0);
	udelay(10);
	ddr_reset(cru, channel, 0, 0);
	udelay(10);
}

static void phy_dll_bypass_set(struct rk3288_ddr_publ *publ,
	u32 freq)
{
	int i;
	if (freq <= 250000000) {
		if (freq <= 150000000)
			clrbits_le32(&publ->dllgcr, SBIAS_BYPASS);
		else
			setbits_le32(&publ->dllgcr, SBIAS_BYPASS);
		setbits_le32(&publ->acdllcr, ACDLLCR_DLLDIS);
		for (i = 0; i < 4; i++)
			setbits_le32(&publ->datx8[i].dxdllcr,
				     DXDLLCR_DLLDIS);

		setbits_le32(&publ->pir, PIR_DLLBYP);
	} else {
		clrbits_le32(&publ->dllgcr, SBIAS_BYPASS);
		clrbits_le32(&publ->acdllcr, ACDLLCR_DLLDIS);
		for (i = 0; i < 4; i++) {
			clrbits_le32(&publ->datx8[i].dxdllcr,
				     DXDLLCR_DLLDIS);
		}

		clrbits_le32(&publ->pir, PIR_DLLBYP);
	}
}

static void dfi_cfg(struct rk3288_ddr_pctl *pctl, u32 dramtype)
{
	writel(DFI_INIT_START, &pctl->dfistcfg0);
	writel(DFI_DRAM_CLK_SR_EN | DFI_DRAM_CLK_DPD_EN,
	       &pctl->dfistcfg1);
	writel(DFI_PARITY_INTR_EN | DFI_PARITY_EN, &pctl->dfistcfg2);
	writel(7 << TLP_RESP_TIME_SHIFT | LP_SR_EN | LP_PD_EN,
	       &pctl->dfilpcfg0);

	writel(2 << TCTRL_DELAY_TIME_SHIFT, &pctl->dfitctrldelay);
	writel(1 << TPHY_WRDATA_TIME_SHIFT, &pctl->dfitphywrdata);
	writel(0xf << TPHY_RDLAT_TIME_SHIFT, &pctl->dfitphyrdlat);
	writel(2 << TDRAM_CLK_DIS_TIME_SHIFT, &pctl->dfitdramclkdis);
	writel(2 << TDRAM_CLK_EN_TIME_SHIFT, &pctl->dfitdramclken);
	writel(1, &pctl->dfitphyupdtype0);

	/* cs0 and cs1 write odt enable */
	writel((RANK0_ODT_WRITE_SEL | RANK1_ODT_WRITE_SEL),
	       &pctl->dfiodtcfg);
	/* odt write length */
	writel(7 << ODT_LEN_BL8_W_SHIFT, &pctl->dfiodtcfg1);
	/* phyupd and ctrlupd disabled */
	writel(0, &pctl->dfiupdcfg);
}

static void ddr_set_enable(struct rk3288_grf *grf, uint channel, bool enable)
{
	uint val = 0;

	if (enable) {
		val = 1 << (channel ? DDR1_16BIT_EN_SHIFT :
				DDR0_16BIT_EN_SHIFT);
	}
	rk_clrsetreg(&grf->soc_con0,
		     1 << (channel ? DDR1_16BIT_EN_SHIFT : DDR0_16BIT_EN_SHIFT),
		     val);
}

static void ddr_set_ddr3_mode(struct rk3288_grf *grf, uint channel,
			      bool ddr3_mode)
{
	uint mask, val;

	mask = 1 << (channel ? MSCH1_MAINDDR3_SHIFT : MSCH0_MAINDDR3_SHIFT);
	val = ddr3_mode << (channel ? MSCH1_MAINDDR3_SHIFT :
					MSCH0_MAINDDR3_SHIFT);
	rk_clrsetreg(&grf->soc_con0, mask, val);
}

static void ddr_set_en_bst_odt(struct rk3288_grf *grf, uint channel,
			       bool enable, bool enable_bst, bool enable_odt)
{
	uint mask;
	bool disable_bst = !enable_bst;

	mask = channel ?
		(1 << LPDDR3_EN1_SHIFT | 1 << UPCTL1_BST_DIABLE_SHIFT |
			1 << UPCTL1_LPDDR3_ODT_EN_SHIFT) :
		(1 << LPDDR3_EN0_SHIFT | 1 << UPCTL0_BST_DIABLE_SHIFT |
			1 << UPCTL0_LPDDR3_ODT_EN_SHIFT);
	rk_clrsetreg(&grf->soc_con2, mask,
		     enable << (channel ? LPDDR3_EN1_SHIFT : LPDDR3_EN0_SHIFT) |
		     disable_bst << (channel ? UPCTL1_BST_DIABLE_SHIFT :
				UPCTL0_BST_DIABLE_SHIFT) |
		     enable_odt << (channel ? UPCTL1_LPDDR3_ODT_EN_SHIFT :
				UPCTL0_LPDDR3_ODT_EN_SHIFT));
}

static void pctl_cfg(u32 channel, struct rk3288_ddr_pctl *pctl,
		     const struct rk3288_sdram_params *sdram_params,
		     struct rk3288_grf *grf)
{
	unsigned int burstlen;

	burstlen = (sdram_params->base.noc_timing >> 18) & 0x7;
	copy_to_reg(&pctl->togcnt1u, &sdram_params->pctl_timing.togcnt1u,
		    sizeof(sdram_params->pctl_timing));
	switch (sdram_params->base.dramtype) {
	case LPDDR3:
		writel(sdram_params->pctl_timing.tcl - 1,
		       &pctl->dfitrddataen);
		writel(sdram_params->pctl_timing.tcwl,
		       &pctl->dfitphywrlat);
		burstlen >>= 1;
		writel(LPDDR2_S4 | 0 << MDDR_LPDDR2_CLK_STOP_IDLE_SHIFT |
		       LPDDR2_EN | burstlen << BURSTLENGTH_SHIFT |
		       (6 - 4) << TFAW_SHIFT | PD_EXIT_FAST |
		       1 << PD_TYPE_SHIFT | 0 << PD_IDLE_SHIFT,
		       &pctl->mcfg);
		ddr_set_ddr3_mode(grf, channel, false);
		ddr_set_enable(grf, channel, true);
		ddr_set_en_bst_odt(grf, channel, true, false,
				   sdram_params->base.odt);
		break;
	case DDR3:
		if (sdram_params->phy_timing.mr[1] & DDR3_DLL_DISABLE) {
			writel(sdram_params->pctl_timing.tcl - 3,
			       &pctl->dfitrddataen);
		} else {
			writel(sdram_params->pctl_timing.tcl - 2,
			       &pctl->dfitrddataen);
		}
		writel(sdram_params->pctl_timing.tcwl - 1,
		       &pctl->dfitphywrlat);
		writel(0 << MDDR_LPDDR2_CLK_STOP_IDLE_SHIFT | DDR3_EN |
		       DDR2_DDR3_BL_8 | (6 - 4) << TFAW_SHIFT | PD_EXIT_SLOW |
		       1 << PD_TYPE_SHIFT | 0 << PD_IDLE_SHIFT,
		       &pctl->mcfg);
		ddr_set_ddr3_mode(grf, channel, true);
		ddr_set_enable(grf, channel, true);

		ddr_set_en_bst_odt(grf, channel, false, true, false);
		break;
	}

	setbits_le32(&pctl->scfg, 1);
}

static void phy_cfg(const struct chan_info *chan, u32 channel,
		    const struct rk3288_sdram_params *sdram_params)
{
	struct rk3288_ddr_publ *publ = chan->publ;
	struct rk3288_msch *msch = chan->msch;
	uint ddr_freq_mhz = sdram_params->base.ddr_freq / 1000000;
	u32 dinit2, tmp;
	int i;

	dinit2 = DIV_ROUND_UP(ddr_freq_mhz * 200000, 1000);
	/* DDR PHY Timing */
	copy_to_reg(&publ->dtpr[0], &sdram_params->phy_timing.dtpr0,
		    sizeof(sdram_params->phy_timing));
	writel(sdram_params->base.noc_timing, &msch->ddrtiming);
	writel(0x3f, &msch->readlatency);
	writel(sdram_params->base.noc_activate, &msch->activate);
	writel(2 << BUSWRTORD_SHIFT | 2 << BUSRDTOWR_SHIFT |
	       1 << BUSRDTORD_SHIFT, &msch->devtodev);
	writel(DIV_ROUND_UP(ddr_freq_mhz * 5120, 1000) << PRT_DLLLOCK_SHIFT |
	       DIV_ROUND_UP(ddr_freq_mhz * 50, 1000) << PRT_DLLSRST_SHIFT |
	       8 << PRT_ITMSRST_SHIFT, &publ->ptr[0]);
	writel(DIV_ROUND_UP(ddr_freq_mhz * 500000, 1000) << PRT_DINIT0_SHIFT |
	       DIV_ROUND_UP(ddr_freq_mhz * 400, 1000) << PRT_DINIT1_SHIFT,
	       &publ->ptr[1]);
	writel(min(dinit2, 0x1ffffU) << PRT_DINIT2_SHIFT |
	       DIV_ROUND_UP(ddr_freq_mhz * 1000, 1000) << PRT_DINIT3_SHIFT,
	       &publ->ptr[2]);

	switch (sdram_params->base.dramtype) {
	case LPDDR3:
		clrsetbits_le32(&publ->pgcr, 0x1F,
				0 << PGCR_DFTLMT_SHIFT |
				0 << PGCR_DFTCMP_SHIFT |
				1 << PGCR_DQSCFG_SHIFT |
				0 << PGCR_ITMDMD_SHIFT);
		/* DDRMODE select LPDDR3 */
		clrsetbits_le32(&publ->dcr, DDRMD_MASK << DDRMD_SHIFT,
				DDRMD_LPDDR2_LPDDR3 << DDRMD_SHIFT);
		clrsetbits_le32(&publ->dxccr,
				DQSNRES_MASK << DQSNRES_SHIFT |
				DQSRES_MASK << DQSRES_SHIFT,
				4 << DQSRES_SHIFT | 0xc << DQSNRES_SHIFT);
		tmp = readl(&publ->dtpr[1]);
		tmp = ((tmp >> TDQSCKMAX_SHIFT) & TDQSCKMAX_MASK) -
			((tmp >> TDQSCK_SHIFT) & TDQSCK_MASK);
		clrsetbits_le32(&publ->dsgcr,
				DQSGE_MASK << DQSGE_SHIFT |
				DQSGX_MASK << DQSGX_SHIFT,
				tmp << DQSGE_SHIFT | tmp << DQSGX_SHIFT);
		break;
	case DDR3:
		clrbits_le32(&publ->pgcr, 0x1f);
		clrsetbits_le32(&publ->dcr, DDRMD_MASK << DDRMD_SHIFT,
				DDRMD_DDR3 << DDRMD_SHIFT);
		break;
	}
	if (sdram_params->base.odt) {
		/*dynamic RTT enable */
		for (i = 0; i < 4; i++)
			setbits_le32(&publ->datx8[i].dxgcr, DQSRTT | DQRTT);
	} else {
		/*dynamic RTT disable */
		for (i = 0; i < 4; i++)
			clrbits_le32(&publ->datx8[i].dxgcr, DQSRTT | DQRTT);
	}
}

static void phy_init(struct rk3288_ddr_publ *publ)
{
	setbits_le32(&publ->pir, PIR_INIT | PIR_DLLSRST
		| PIR_DLLLOCK | PIR_ZCAL | PIR_ITMSRST | PIR_CLRSR);
	udelay(1);
	while ((readl(&publ->pgsr) &
		(PGSR_IDONE | PGSR_DLDONE | PGSR_ZCDONE)) !=
		(PGSR_IDONE | PGSR_DLDONE | PGSR_ZCDONE))
		;
}

static void send_command(struct rk3288_ddr_pctl *pctl, u32 rank,
			 u32 cmd, u32 arg)
{
	writel((START_CMD | (rank << 20) | arg | cmd), &pctl->mcmd);
	udelay(1);
	while (readl(&pctl->mcmd) & START_CMD)
		;
}

static inline void send_command_op(struct rk3288_ddr_pctl *pctl,
				   u32 rank, u32 cmd, u32 ma, u32 op)
{
	send_command(pctl, rank, cmd, (ma & LPDDR2_MA_MASK) << LPDDR2_MA_SHIFT |
		     (op & LPDDR2_OP_MASK) << LPDDR2_OP_SHIFT);
}

static void memory_init(struct rk3288_ddr_publ *publ,
			u32 dramtype)
{
	setbits_le32(&publ->pir,
		     (PIR_INIT | PIR_DRAMINIT | PIR_LOCKBYP
		      | PIR_ZCALBYP | PIR_CLRSR | PIR_ICPC
		      | (dramtype == DDR3 ? PIR_DRAMRST : 0)));
	udelay(1);
	while ((readl(&publ->pgsr) & (PGSR_IDONE | PGSR_DLDONE))
		!= (PGSR_IDONE | PGSR_DLDONE))
		;
}

static void move_to_config_state(struct rk3288_ddr_publ *publ,
				 struct rk3288_ddr_pctl *pctl)
{
	unsigned int state;

	while (1) {
		state = readl(&pctl->stat) & PCTL_STAT_MSK;

		switch (state) {
		case LOW_POWER:
			writel(WAKEUP_STATE, &pctl->sctl);
			while ((readl(&pctl->stat) & PCTL_STAT_MSK)
				!= ACCESS)
				;
			/* wait DLL lock */
			while ((readl(&publ->pgsr) & PGSR_DLDONE)
				!= PGSR_DLDONE)
				;
			/* if at low power state,need wakeup first,
			 * and then enter the config
			 * so here no break.
			 */
		case ACCESS:
			/* no break */
		case INIT_MEM:
			writel(CFG_STATE, &pctl->sctl);
			while ((readl(&pctl->stat) & PCTL_STAT_MSK) != CONFIG)
				;
			break;
		case CONFIG:
			return;
		default:
			break;
		}
	}
}

static void set_bandwidth_ratio(const struct chan_info *chan, u32 channel,
				u32 n, struct rk3288_grf *grf)
{
	struct rk3288_ddr_pctl *pctl = chan->pctl;
	struct rk3288_ddr_publ *publ = chan->publ;
	struct rk3288_msch *msch = chan->msch;

	if (n == 1) {
		setbits_le32(&pctl->ppcfg, 1);
		rk_setreg(&grf->soc_con0, 1 << (8 + channel));
		setbits_le32(&msch->ddrtiming, 1 << 31);
		/* Data Byte disable*/
		clrbits_le32(&publ->datx8[2].dxgcr, 1);
		clrbits_le32(&publ->datx8[3].dxgcr, 1);
		/* disable DLL */
		setbits_le32(&publ->datx8[2].dxdllcr, DXDLLCR_DLLDIS);
		setbits_le32(&publ->datx8[3].dxdllcr, DXDLLCR_DLLDIS);
	} else {
		clrbits_le32(&pctl->ppcfg, 1);
		rk_clrreg(&grf->soc_con0, 1 << (8 + channel));
		clrbits_le32(&msch->ddrtiming, 1 << 31);
		/* Data Byte enable*/
		setbits_le32(&publ->datx8[2].dxgcr, 1);
		setbits_le32(&publ->datx8[3].dxgcr, 1);

		/* enable DLL */
		clrbits_le32(&publ->datx8[2].dxdllcr, DXDLLCR_DLLDIS);
		clrbits_le32(&publ->datx8[3].dxdllcr, DXDLLCR_DLLDIS);
		/* reset DLL */
		clrbits_le32(&publ->datx8[2].dxdllcr, DXDLLCR_DLLSRST);
		clrbits_le32(&publ->datx8[3].dxdllcr, DXDLLCR_DLLSRST);
		udelay(10);
		setbits_le32(&publ->datx8[2].dxdllcr, DXDLLCR_DLLSRST);
		setbits_le32(&publ->datx8[3].dxdllcr, DXDLLCR_DLLSRST);
	}
	setbits_le32(&pctl->dfistcfg0, 1 << 2);
}

static int data_training(const struct chan_info *chan, u32 channel,
			 const struct rk3288_sdram_params *sdram_params)
{
	unsigned int j;
	int ret = 0;
	u32 rank;
	int i;
	u32 step[2] = { PIR_QSTRN, PIR_RVTRN };
	struct rk3288_ddr_publ *publ = chan->publ;
	struct rk3288_ddr_pctl *pctl = chan->pctl;

	/* disable auto refresh */
	writel(0, &pctl->trefi);

	if (sdram_params->base.dramtype != LPDDR3)
		setbits_le32(&publ->pgcr, 1 << PGCR_DQSCFG_SHIFT);
	rank = sdram_params->ch[channel].rank | 1;
	for (j = 0; j < ARRAY_SIZE(step); j++) {
		/*
		 * trigger QSTRN and RVTRN
		 * clear DTDONE status
		 */
		setbits_le32(&publ->pir, PIR_CLRSR);

		/* trigger DTT */
		setbits_le32(&publ->pir,
			     PIR_INIT | step[j] | PIR_LOCKBYP | PIR_ZCALBYP |
			     PIR_CLRSR);
		udelay(1);
		/* wait echo byte DTDONE */
		while ((readl(&publ->datx8[0].dxgsr[0]) & rank)
			!= rank)
			;
		while ((readl(&publ->datx8[1].dxgsr[0]) & rank)
			!= rank)
			;
		if (!(readl(&pctl->ppcfg) & 1)) {
			while ((readl(&publ->datx8[2].dxgsr[0])
				& rank) != rank)
				;
			while ((readl(&publ->datx8[3].dxgsr[0])
				& rank) != rank)
				;
		}
		if (readl(&publ->pgsr) &
		    (PGSR_DTERR | PGSR_RVERR | PGSR_RVEIRR)) {
			ret = -1;
			break;
		}
	}
	/* send some auto refresh to complement the lost while DTT */
	for (i = 0; i < (rank > 1 ? 8 : 4); i++)
		send_command(pctl, rank, REF_CMD, 0);

	if (sdram_params->base.dramtype != LPDDR3)
		clrbits_le32(&publ->pgcr, 1 << PGCR_DQSCFG_SHIFT);

	/* resume auto refresh */
	writel(sdram_params->pctl_timing.trefi, &pctl->trefi);

	return ret;
}

static void move_to_access_state(const struct chan_info *chan)
{
	struct rk3288_ddr_publ *publ = chan->publ;
	struct rk3288_ddr_pctl *pctl = chan->pctl;
	unsigned int state;

	while (1) {
		state = readl(&pctl->stat) & PCTL_STAT_MSK;

		switch (state) {
		case LOW_POWER:
			if (((readl(&pctl->stat) >> LP_TRIG_SHIFT) &
					LP_TRIG_MASK) == 1)
				return;

			writel(WAKEUP_STATE, &pctl->sctl);
			while ((readl(&pctl->stat) & PCTL_STAT_MSK) != ACCESS)
				;
			/* wait DLL lock */
			while ((readl(&publ->pgsr) & PGSR_DLDONE)
				!= PGSR_DLDONE)
				;
			break;
		case INIT_MEM:
			writel(CFG_STATE, &pctl->sctl);
			while ((readl(&pctl->stat) & PCTL_STAT_MSK) != CONFIG)
				;
		case CONFIG:
			writel(GO_STATE, &pctl->sctl);
			while ((readl(&pctl->stat) & PCTL_STAT_MSK) == CONFIG)
				;
			break;
		case ACCESS:
			return;
		default:
			break;
		}
	}
}

static void dram_cfg_rbc(const struct chan_info *chan, u32 chnum,
			 const struct rk3288_sdram_params *sdram_params)
{
	struct rk3288_ddr_publ *publ = chan->publ;

	if (sdram_params->ch[chnum].bk == 3)
		clrsetbits_le32(&publ->dcr, PDQ_MASK << PDQ_SHIFT,
				1 << PDQ_SHIFT);
	else
		clrbits_le32(&publ->dcr, PDQ_MASK << PDQ_SHIFT);

	writel(sdram_params->base.ddrconfig, &chan->msch->ddrconf);
}

static void dram_all_config(const struct dram_info *dram,
			    const struct rk3288_sdram_params *sdram_params)
{
	unsigned int chan;
	u32 sys_reg = 0;

	sys_reg |= sdram_params->base.dramtype << SYS_REG_DDRTYPE_SHIFT;
	sys_reg |= (sdram_params->num_channels - 1) << SYS_REG_NUM_CH_SHIFT;
	for (chan = 0; chan < sdram_params->num_channels; chan++) {
		const struct rk3288_sdram_channel *info =
			&sdram_params->ch[chan];

		sys_reg |= info->row_3_4 << SYS_REG_ROW_3_4_SHIFT(chan);
		sys_reg |= chan << SYS_REG_CHINFO_SHIFT(chan);
		sys_reg |= (info->rank - 1) << SYS_REG_RANK_SHIFT(chan);
		sys_reg |= (info->col - 9) << SYS_REG_COL_SHIFT(chan);
		sys_reg |= info->bk == 3 ? 1 << SYS_REG_BK_SHIFT(chan) : 0;
		sys_reg |= (info->cs0_row - 13) << SYS_REG_CS0_ROW_SHIFT(chan);
		sys_reg |= (info->cs1_row - 13) << SYS_REG_CS1_ROW_SHIFT(chan);
		sys_reg |= info->bw << SYS_REG_BW_SHIFT(chan);
		sys_reg |= info->dbw << SYS_REG_DBW_SHIFT(chan);

		dram_cfg_rbc(&dram->chan[chan], chan, sdram_params);
	}
	writel(sys_reg, &dram->pmu->sys_reg[2]);
	rk_clrsetreg(&dram->sgrf->soc_con2, 0x1f, sdram_params->base.stride);
}

static int sdram_init(struct dram_info *dram,
		      const struct rk3288_sdram_params *sdram_params)
{
	int channel;
	int zqcr;
	int ret;

	debug("%s start\n", __func__);
	if ((sdram_params->base.dramtype == DDR3 &&
	     sdram_params->base.ddr_freq > 800000000) ||
	    (sdram_params->base.dramtype == LPDDR3 &&
	     sdram_params->base.ddr_freq > 533000000)) {
		debug("SDRAM frequency is too high!");
		return -E2BIG;
	}

	debug("ddr clk dpll\n");
	ret = clk_set_rate(&dram->ddr_clk, sdram_params->base.ddr_freq);
	debug("ret=%d\n", ret);
	if (ret) {
		debug("Could not set DDR clock\n");
		return ret;
	}

	for (channel = 0; channel < 2; channel++) {
		const struct chan_info *chan = &dram->chan[channel];
		struct rk3288_ddr_pctl *pctl = chan->pctl;
		struct rk3288_ddr_publ *publ = chan->publ;

		phy_pctrl_reset(dram->cru, publ, channel);
		phy_dll_bypass_set(publ, sdram_params->base.ddr_freq);

		if (channel >= sdram_params->num_channels)
			continue;

		dfi_cfg(pctl, sdram_params->base.dramtype);

		pctl_cfg(channel, pctl, sdram_params, dram->grf);

		phy_cfg(chan, channel, sdram_params);

		phy_init(publ);

		writel(POWER_UP_START, &pctl->powctl);
		while (!(readl(&pctl->powstat) & POWER_UP_DONE))
			;

		memory_init(publ, sdram_params->base.dramtype);
		move_to_config_state(publ, pctl);

		if (sdram_params->base.dramtype == LPDDR3) {
			send_command(pctl, 3, DESELECT_CMD, 0);
			udelay(1);
			send_command(pctl, 3, PREA_CMD, 0);
			udelay(1);
			send_command_op(pctl, 3, MRS_CMD, 63, 0xfc);
			udelay(1);
			send_command_op(pctl, 3, MRS_CMD, 1,
					sdram_params->phy_timing.mr[1]);
			udelay(1);
			send_command_op(pctl, 3, MRS_CMD, 2,
					sdram_params->phy_timing.mr[2]);
			udelay(1);
			send_command_op(pctl, 3, MRS_CMD, 3,
					sdram_params->phy_timing.mr[3]);
			udelay(1);
		}

		set_bandwidth_ratio(chan, channel,
				    sdram_params->ch[channel].bw, dram->grf);
		/*
		 * set cs
		 * CS0, n=1
		 * CS1, n=2
		 * CS0 & CS1, n = 3
		 */
		clrsetbits_le32(&publ->pgcr, 0xF << 18,
				(sdram_params->ch[channel].rank | 1) << 18);
		/* DS=40ohm,ODT=155ohm */
		zqcr = 1 << ZDEN_SHIFT | 2 << PU_ONDIE_SHIFT |
			2 << PD_ONDIE_SHIFT | 0x19 << PU_OUTPUT_SHIFT |
			0x19 << PD_OUTPUT_SHIFT;
		writel(zqcr, &publ->zq1cr[0]);
		writel(zqcr, &publ->zq0cr[0]);

		if (sdram_params->base.dramtype == LPDDR3) {
			/* LPDDR2/LPDDR3 need to wait DAI complete, max 10us */
			udelay(10);
			send_command_op(pctl,
					sdram_params->ch[channel].rank | 1,
					MRS_CMD, 11,
					sdram_params->base.odt ? 3 : 0);
			if (channel == 0) {
				writel(0, &pctl->mrrcfg0);
				send_command_op(pctl, 1, MRR_CMD, 8, 0);
				/* S8 */
				if ((readl(&pctl->mrrstat0) & 0x3) != 3) {
					debug("failed!");
					return -EREMOTEIO;
				}
			}
		}

		if (-1 == data_training(chan, channel, sdram_params)) {
			if (sdram_params->base.dramtype == LPDDR3) {
				ddr_phy_ctl_reset(dram->cru, channel, 1);
				udelay(10);
				ddr_phy_ctl_reset(dram->cru, channel, 0);
				udelay(10);
			}
			debug("failed!");
			return -EIO;
		}

		if (sdram_params->base.dramtype == LPDDR3) {
			u32 i;
			writel(0, &pctl->mrrcfg0);
			for (i = 0; i < 17; i++)
				send_command_op(pctl, 1, MRR_CMD, i, 0);
		}
		move_to_access_state(chan);
	}
	dram_all_config(dram, sdram_params);
	debug("%s done\n", __func__);

	return 0;
}
#endif

size_t sdram_size_mb(struct rk3288_pmu *pmu)
{
	u32 rank, col, bk, cs0_row, cs1_row, bw, row_3_4;
	size_t chipsize_mb = 0;
	size_t size_mb = 0;
	u32 ch;
	u32 sys_reg = readl(&pmu->sys_reg[2]);
	u32 chans;

	chans = 1 + ((sys_reg >> SYS_REG_NUM_CH_SHIFT) & SYS_REG_NUM_CH_MASK);

	for (ch = 0; ch < chans; ch++) {
		rank = 1 + (sys_reg >> SYS_REG_RANK_SHIFT(ch) &
			SYS_REG_RANK_MASK);
		col = 9 + (sys_reg >> SYS_REG_COL_SHIFT(ch) & SYS_REG_COL_MASK);
		bk = sys_reg & (1 << SYS_REG_BK_SHIFT(ch)) ? 3 : 0;
		cs0_row = 13 + (sys_reg >> SYS_REG_CS0_ROW_SHIFT(ch) &
				SYS_REG_CS0_ROW_MASK);
		cs1_row = 13 + (sys_reg >> SYS_REG_CS1_ROW_SHIFT(ch) &
				SYS_REG_CS1_ROW_MASK);
		bw = (sys_reg >> SYS_REG_BW_SHIFT(ch)) &
			SYS_REG_BW_MASK;
		row_3_4 = sys_reg >> SYS_REG_ROW_3_4_SHIFT(ch) &
			SYS_REG_ROW_3_4_MASK;

		chipsize_mb = (1 << (cs0_row + col + bk + bw - 20));

		if (rank > 1)
			chipsize_mb += chipsize_mb >>
				(cs0_row - cs1_row);
		if (row_3_4)
			chipsize_mb = chipsize_mb * 3 / 4;
		size_mb += chipsize_mb;
	}

	/*
	* we use the 0x00000000~0xfeffffff space since 0xff000000~0xffffffff
	* is SoC register space (i.e. reserved)
	*/
	size_mb = min(size_mb, 0xff000000 >> 20);

	return size_mb;
}

#ifdef CONFIG_SPL_BUILD
# ifdef CONFIG_ROCKCHIP_FAST_SPL
static int veyron_init(struct dram_info *priv)
{
	struct udevice *pmic;
	int ret;

	ret = uclass_first_device_err(UCLASS_PMIC, &pmic);
	if (ret)
		return ret;

	/* Slowly raise to max CPU voltage to prevent overshoot */
	ret = rk808_spl_configure_buck(pmic, 1, 1200000);
	if (ret)
		return ret;
	udelay(175);/* Must wait for voltage to stabilize, 2mV/us */
	ret = rk808_spl_configure_buck(pmic, 1, 1400000);
	if (ret)
		return ret;
	udelay(100);/* Must wait for voltage to stabilize, 2mV/us */

	rkclk_configure_cpu(priv->cru, priv->grf);

	return 0;
}
# endif

static int setup_sdram(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);
	struct rk3288_sdram_params params;
	const void *blob = gd->fdt_blob;
	int node = dev->of_offset;
	int i, ret;

	params.num_channels = fdtdec_get_int(blob, node,
					     "rockchip,num-channels", 1);
	for (i = 0; i < params.num_channels; i++) {
		ret = fdtdec_get_byte_array(blob, node,
					    "rockchip,sdram-channel",
					    (u8 *)&params.ch[i],
					    sizeof(params.ch[i]));
		if (ret) {
			debug("%s: Cannot read rockchip,sdram-channel\n",
			      __func__);
			return -EINVAL;
		}
	}
	ret = fdtdec_get_int_array(blob, node, "rockchip,pctl-timing",
				   (u32 *)&params.pctl_timing,
				   sizeof(params.pctl_timing) / sizeof(u32));
	if (ret) {
		debug("%s: Cannot read rockchip,pctl-timing\n", __func__);
		return -EINVAL;
	}
	ret = fdtdec_get_int_array(blob, node, "rockchip,phy-timing",
				   (u32 *)&params.phy_timing,
				   sizeof(params.phy_timing) / sizeof(u32));
	if (ret) {
		debug("%s: Cannot read rockchip,phy-timing\n", __func__);
		return -EINVAL;
	}
	ret = fdtdec_get_int_array(blob, node, "rockchip,sdram-params",
				   (u32 *)&params.base,
				   sizeof(params.base) / sizeof(u32));
	if (ret) {
		debug("%s: Cannot read rockchip,sdram-params\n", __func__);
		return -EINVAL;
	}

# ifdef CONFIG_ROCKCHIP_FAST_SPL
	if (!fdt_node_check_compatible(blob, 0, "google,veyron")) {
		ret = veyron_init(priv);
		if (ret)
			return ret;
	}
# endif

	return sdram_init(priv, &params);
}
#endif

static int rk3288_dmc_probe(struct udevice *dev)
{
	struct dram_info *priv = dev_get_priv(dev);
	struct regmap *map;
	int ret;
	struct udevice *dev_clk;

	map = syscon_get_regmap_by_driver_data(ROCKCHIP_SYSCON_NOC);
	if (IS_ERR(map))
		return PTR_ERR(map);
	priv->chan[0].msch = regmap_get_range(map, 0);
	priv->chan[1].msch = (struct rk3288_msch *)
			(regmap_get_range(map, 0) + 0x80);

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	priv->sgrf = syscon_get_first_range(ROCKCHIP_SYSCON_SGRF);
	priv->pmu = syscon_get_first_range(ROCKCHIP_SYSCON_PMU);

	ret = regmap_init_mem(dev, &map);
	if (ret)
		return ret;
	priv->chan[0].pctl = regmap_get_range(map, 0);
	priv->chan[0].publ = regmap_get_range(map, 1);
	priv->chan[1].pctl = regmap_get_range(map, 2);
	priv->chan[1].publ = regmap_get_range(map, 3);

	ret = uclass_get_device(UCLASS_CLK, 0, &dev_clk);
	if (ret)
		return ret;
	priv->ddr_clk.id = CLK_DDR;
	ret = clk_request(dev_clk, &priv->ddr_clk);
	if (ret)
		return ret;

	priv->cru = rockchip_get_cru();
	if (IS_ERR(priv->cru))
		return PTR_ERR(priv->cru);
#ifdef CONFIG_SPL_BUILD
	ret = setup_sdram(dev);
	if (ret)
		return ret;
#endif
	priv->info.base = 0;
	priv->info.size = sdram_size_mb(priv->pmu) << 20;

	return 0;
}

static int rk3288_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct dram_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops rk3288_dmc_ops = {
	.get_info = rk3288_dmc_get_info,
};

static const struct udevice_id rk3288_dmc_ids[] = {
	{ .compatible = "rockchip,rk3288-dmc" },
	{ }
};

U_BOOT_DRIVER(dmc_rk3288) = {
	.name = "rk3288_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3288_dmc_ids,
	.ops = &rk3288_dmc_ops,
	.probe = rk3288_dmc_probe,
	.priv_auto_alloc_size = sizeof(struct dram_info),
};
