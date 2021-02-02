// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <errno.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/lpddr4_define.h>
#include <asm/arch/sys_proto.h>

static unsigned int g_cdd_rr_max[4];
static unsigned int g_cdd_rw_max[4];
static unsigned int g_cdd_wr_max[4];
static unsigned int g_cdd_ww_max[4];

static inline void poll_pmu_message_ready(void)
{
	unsigned int reg;

	do {
		reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0004);
	} while (reg & 0x1);
}

static inline void ack_pmu_message_receive(void)
{
	unsigned int reg;

	reg32_write(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0031, 0x0);

	do {
		reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0004);
	} while (!(reg & 0x1));

	reg32_write(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0031, 0x1);
}

static inline unsigned int get_mail(void)
{
	unsigned int reg;

	poll_pmu_message_ready();

	reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0032);

	ack_pmu_message_receive();

	return reg;
}

static inline unsigned int get_stream_message(void)
{
	unsigned int reg, reg2;

	poll_pmu_message_ready();

	reg = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0032);

	reg2 = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + 4 * 0xd0034);

	reg2 = (reg2 << 16) | reg;

	ack_pmu_message_receive();

	return reg2;
}

static inline void decode_major_message(unsigned int mail)
{
	debug("[PMU Major message = 0x%08x]\n", mail);
}

static inline void decode_streaming_message(void)
{
	unsigned int string_index, arg __maybe_unused;
	int i = 0;

	string_index = get_stream_message();
	debug("PMU String index = 0x%08x\n", string_index);
	while (i < (string_index & 0xffff)) {
		arg = get_stream_message();
		debug("arg[%d] = 0x%08x\n", i, arg);
		i++;
	}

	debug("\n");
}

int wait_ddrphy_training_complete(void)
{
	unsigned int mail;

	while (1) {
		mail = get_mail();
		decode_major_message(mail);
		if (mail == 0x08) {
			decode_streaming_message();
		} else if (mail == 0x07) {
			debug("Training PASS\n");
			return 0;
		} else if (mail == 0xff) {
			debug("Training FAILED\n");
			return -1;
		}
	}
}

void ddrphy_init_set_dfi_clk(unsigned int drate)
{
	switch (drate) {
	case 4000:
		dram_pll_init(MHZ(1000));
		dram_disable_bypass();
		break;
	case 3200:
		dram_pll_init(MHZ(800));
		dram_disable_bypass();
		break;
	case 3000:
		dram_pll_init(MHZ(750));
		dram_disable_bypass();
		break;
	case 2400:
		dram_pll_init(MHZ(600));
		dram_disable_bypass();
		break;
	case 1600:
		dram_pll_init(MHZ(400));
		dram_disable_bypass();
		break;
	case 1066:
		dram_pll_init(MHZ(266));
		dram_disable_bypass();
		break;
	case 667:
		dram_pll_init(MHZ(167));
		dram_disable_bypass();
		break;
	case 400:
		dram_enable_bypass(MHZ(400));
		break;
	case 100:
		dram_enable_bypass(MHZ(100));
		break;
	default:
		return;
	}
}

void ddrphy_init_read_msg_block(enum fw_type type)
{
}

void lpddr4_mr_write(unsigned int mr_rank, unsigned int mr_addr,
		     unsigned int mr_data)
{
	unsigned int tmp;
	/*
	 * 1. Poll MRSTAT.mr_wr_busy until it is 0.
	 * This checks that there is no outstanding MR transaction.
	 * No writes should be performed to MRCTRL0 and MRCTRL1 if
	 * MRSTAT.mr_wr_busy = 1.
	 */
	do {
		tmp = reg32_read(DDRC_MRSTAT(0));
	} while (tmp & 0x1);
	/*
	 * 2. Write the MRCTRL0.mr_type, MRCTRL0.mr_addr, MRCTRL0.mr_rank and
	 * (for MRWs) MRCTRL1.mr_data to define the MR transaction.
	 */
	reg32_write(DDRC_MRCTRL0(0), (mr_rank << 4));
	reg32_write(DDRC_MRCTRL1(0), (mr_addr << 8) | mr_data);
	reg32setbit(DDRC_MRCTRL0(0), 31);
}

unsigned int lpddr4_mr_read(unsigned int mr_rank, unsigned int mr_addr)
{
	unsigned int tmp;

	reg32_write(DRC_PERF_MON_MRR0_DAT(0), 0x1);
	do {
		tmp = reg32_read(DDRC_MRSTAT(0));
	} while (tmp & 0x1);

	reg32_write(DDRC_MRCTRL0(0), (mr_rank << 4) | 0x1);
	reg32_write(DDRC_MRCTRL1(0), (mr_addr << 8));
	reg32setbit(DDRC_MRCTRL0(0), 31);
	do {
		tmp = reg32_read(DRC_PERF_MON_MRR0_DAT(0));
	} while ((tmp & 0x8) == 0);
	tmp = reg32_read(DRC_PERF_MON_MRR1_DAT(0));
	tmp = tmp & 0xff;
	reg32_write(DRC_PERF_MON_MRR0_DAT(0), 0x4);

	return tmp;
}

unsigned int look_for_max(unsigned int data[],
			  unsigned int addr_start, unsigned int addr_end)
{
	unsigned int i, imax = 0;

	for (i = addr_start; i <= addr_end; i++) {
		if (((data[i] >> 7) == 0) && (data[i] > imax))
			imax = data[i];
	}

	return imax;
}

void get_trained_CDD(u32 fsp)
{
	unsigned int i, ddr_type, tmp;
	unsigned int cdd_cha[12], cdd_chb[12];
	unsigned int cdd_cha_rr_max, cdd_cha_rw_max, cdd_cha_wr_max, cdd_cha_ww_max;
	unsigned int cdd_chb_rr_max, cdd_chb_rw_max, cdd_chb_wr_max, cdd_chb_ww_max;

	ddr_type = reg32_read(DDRC_MSTR(0)) & 0x3f;
	if (ddr_type == 0x20) {
		for (i = 0; i < 6; i++) {
			tmp = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + (0x54013 + i) * 4);
			cdd_cha[i * 2] = tmp & 0xff;
			cdd_cha[i * 2 + 1] = (tmp >> 8) & 0xff;
		}

		for (i = 0; i < 7; i++) {
			tmp = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + (0x5402c + i) * 4);
			if (i == 0) {
				cdd_cha[0] = (tmp >> 8) & 0xff;
			} else if (i == 6) {
				cdd_cha[11] = tmp & 0xff;
			} else {
				cdd_chb[i * 2 - 1] = tmp & 0xff;
				cdd_chb[i * 2] = (tmp >> 8) & 0xff;
			}
		}

		cdd_cha_rr_max = look_for_max(cdd_cha, 0, 1);
		cdd_cha_rw_max = look_for_max(cdd_cha, 2, 5);
		cdd_cha_wr_max = look_for_max(cdd_cha, 6, 9);
		cdd_cha_ww_max = look_for_max(cdd_cha, 10, 11);
		cdd_chb_rr_max = look_for_max(cdd_chb, 0, 1);
		cdd_chb_rw_max = look_for_max(cdd_chb, 2, 5);
		cdd_chb_wr_max = look_for_max(cdd_chb, 6, 9);
		cdd_chb_ww_max = look_for_max(cdd_chb, 10, 11);
		g_cdd_rr_max[fsp] =  cdd_cha_rr_max > cdd_chb_rr_max ? cdd_cha_rr_max : cdd_chb_rr_max;
		g_cdd_rw_max[fsp] =  cdd_cha_rw_max > cdd_chb_rw_max ? cdd_cha_rw_max : cdd_chb_rw_max;
		g_cdd_wr_max[fsp] =  cdd_cha_wr_max > cdd_chb_wr_max ? cdd_cha_wr_max : cdd_chb_wr_max;
		g_cdd_ww_max[fsp] =  cdd_cha_ww_max > cdd_chb_ww_max ? cdd_cha_ww_max : cdd_chb_ww_max;
	} else {
		unsigned int ddr4_cdd[64];

		for (i = 0; i < 29; i++) {
			tmp = reg32_read(IP2APB_DDRPHY_IPS_BASE_ADDR(0) + (0x54012 + i) * 4);
			ddr4_cdd[i * 2] = tmp & 0xff;
			ddr4_cdd[i * 2 + 1] = (tmp >> 8) & 0xff;
		}

		g_cdd_rr_max[fsp] = look_for_max(ddr4_cdd, 1, 12);
		g_cdd_ww_max[fsp] = look_for_max(ddr4_cdd, 13, 24);
		g_cdd_rw_max[fsp] = look_for_max(ddr4_cdd, 25, 40);
		g_cdd_wr_max[fsp] = look_for_max(ddr4_cdd, 41, 56);
	}
}

void update_umctl2_rank_space_setting(unsigned int pstat_num)
{
	unsigned int i, ddr_type;
	unsigned int addr_slot, rdata, tmp, tmp_t;
	unsigned int ddrc_w2r, ddrc_r2w, ddrc_wr_gap, ddrc_rd_gap;

	ddr_type = reg32_read(DDRC_MSTR(0)) & 0x3f;
	for (i = 0; i < pstat_num; i++) {
		addr_slot = i ? (i + 1) * 0x1000 : 0;
		if (ddr_type == 0x20) {
			/* update r2w:[13:8], w2r:[5:0] */
			rdata = reg32_read(DDRC_DRAMTMG2(0) + addr_slot);
			ddrc_w2r = rdata & 0x3f;
			if (is_imx8mp())
				tmp = ddrc_w2r + (g_cdd_wr_max[i] >> 1);
			else
				tmp = ddrc_w2r + (g_cdd_wr_max[i] >> 1) + 1;
			ddrc_w2r = (tmp > 0x3f) ? 0x3f : tmp;

			ddrc_r2w = (rdata >> 8) & 0x3f;
			if (is_imx8mp())
				tmp = ddrc_r2w + (g_cdd_rw_max[i] >> 1);
			else
				tmp = ddrc_r2w + (g_cdd_rw_max[i] >> 1) + 1;
			ddrc_r2w = (tmp > 0x3f) ? 0x3f : tmp;

			tmp_t = (rdata & 0xffffc0c0) | (ddrc_r2w << 8) | ddrc_w2r;
			reg32_write((DDRC_DRAMTMG2(0) + addr_slot), tmp_t);
		} else {
			/* update w2r:[5:0] */
			rdata = reg32_read(DDRC_DRAMTMG9(0) + addr_slot);
			ddrc_w2r = rdata & 0x3f;
			if (is_imx8mp())
				tmp = ddrc_w2r + (g_cdd_wr_max[i] >> 1);
			else
				tmp = ddrc_w2r + (g_cdd_wr_max[i] >> 1) + 1;
			ddrc_w2r = (tmp > 0x3f) ? 0x3f : tmp;
			tmp_t = (rdata & 0xffffffc0) | ddrc_w2r;
			reg32_write((DDRC_DRAMTMG9(0) + addr_slot), tmp_t);

			/* update r2w:[13:8] */
			rdata = reg32_read(DDRC_DRAMTMG2(0) + addr_slot);
			ddrc_r2w = (rdata >> 8) & 0x3f;
			if (is_imx8mp())
				tmp = ddrc_r2w + (g_cdd_rw_max[i] >> 1);
			else
				tmp = ddrc_r2w + (g_cdd_rw_max[i] >> 1) + 1;
			ddrc_r2w = (tmp > 0x3f) ? 0x3f : tmp;

			tmp_t = (rdata & 0xffffc0ff) | (ddrc_r2w << 8);
			reg32_write((DDRC_DRAMTMG2(0) + addr_slot), tmp_t);
		}

		if (!is_imx8mq()) {
			/* update rankctl: wr_gap:11:8; rd:gap:7:4; quasi-dymic, doc wrong(static) */
			rdata = reg32_read(DDRC_RANKCTL(0) + addr_slot);
			ddrc_wr_gap = (rdata >> 8) & 0xf;
			if (is_imx8mp())
				tmp = ddrc_wr_gap + (g_cdd_ww_max[i] >> 1);
			else
				tmp = ddrc_wr_gap + (g_cdd_ww_max[i] >> 1) + 1;
			ddrc_wr_gap = (tmp > 0xf) ? 0xf : tmp;

			ddrc_rd_gap = (rdata >> 4) & 0xf;
			if (is_imx8mp())
				tmp = ddrc_rd_gap + (g_cdd_rr_max[i] >> 1);
			else
				tmp = ddrc_rd_gap + (g_cdd_rr_max[i] >> 1) + 1;
			ddrc_rd_gap = (tmp > 0xf) ? 0xf : tmp;

			tmp_t = (rdata & 0xfffff00f) | (ddrc_wr_gap << 8) | (ddrc_rd_gap << 4);
			reg32_write((DDRC_RANKCTL(0) + addr_slot), tmp_t);
		}
	}

	if (is_imx8mq()) {
		/* update rankctl: wr_gap:11:8; rd:gap:7:4; quasi-dymic, doc wrong(static) */
		rdata = reg32_read(DDRC_RANKCTL(0));
		ddrc_wr_gap = (rdata >> 8) & 0xf;
		tmp = ddrc_wr_gap + (g_cdd_ww_max[0] >> 1) + 1;
		ddrc_wr_gap = (tmp > 0xf) ? 0xf : tmp;

		ddrc_rd_gap = (rdata >> 4) & 0xf;
		tmp = ddrc_rd_gap + (g_cdd_rr_max[0] >> 1) + 1;
		ddrc_rd_gap = (tmp > 0xf) ? 0xf : tmp;

		tmp_t = (rdata & 0xfffff00f) | (ddrc_wr_gap << 8) | (ddrc_rd_gap << 4);
		reg32_write(DDRC_RANKCTL(0), tmp_t);
	}
}
