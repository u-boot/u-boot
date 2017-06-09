/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "ddr3_init.h"

#define GET_MAX_VALUE(x, y)			\
	((x) > (y)) ? (x) : (y)
#define CEIL_DIVIDE(x, y)					\
	((x - (x / y) * y) == 0) ? ((x / y) - 1) : (x / y)

#define TIME_2_CLOCK_CYCLES	CEIL_DIVIDE

#define GET_CS_FROM_MASK(mask)	(cs_mask2_num[mask])
#define CS_CBE_VALUE(cs_num)	(cs_cbe_reg[cs_num])

u32 window_mem_addr = 0;
u32 phy_reg0_val = 0;
u32 phy_reg1_val = 8;
u32 phy_reg2_val = 0;
u32 phy_reg3_val = 0xa;
enum hws_ddr_freq init_freq = DDR_FREQ_667;
enum hws_ddr_freq low_freq = DDR_FREQ_LOW_FREQ;
enum hws_ddr_freq medium_freq;
u32 debug_dunit = 0;
u32 odt_additional = 1;
u32 *dq_map_table = NULL;
u32 odt_config = 1;

#if defined(CONFIG_ARMADA_38X) || defined(CONFIG_ALLEYCAT3) ||	\
	defined(CONFIG_ARMADA_39X)
u32 is_pll_before_init = 0, is_adll_calib_before_init = 0, is_dfs_in_init = 0;
u32 dfs_low_freq = 130;
#else
u32 is_pll_before_init = 0, is_adll_calib_before_init = 1, is_dfs_in_init = 0;
u32 dfs_low_freq = 100;
#endif
u32 g_rtt_nom_c_s0, g_rtt_nom_c_s1;
u8 calibration_update_control;	/* 2 external only, 1 is internal only */

enum hws_result training_result[MAX_STAGE_LIMIT][MAX_INTERFACE_NUM];
enum auto_tune_stage training_stage = INIT_CONTROLLER;
u32 finger_test = 0, p_finger_start = 11, p_finger_end = 64,
	n_finger_start = 11, n_finger_end = 64,
	p_finger_step = 3, n_finger_step = 3;
u32 clamp_tbl[] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

/* Initiate to 0xff, this variable is define by user in debug mode */
u32 mode2_t = 0xff;
u32 xsb_validate_type = 0;
u32 xsb_validation_base_address = 0xf000;
u32 first_active_if = 0;
u32 dfs_low_phy1 = 0x1f;
u32 multicast_id = 0;
int use_broadcast = 0;
struct hws_tip_freq_config_info *freq_info_table = NULL;
u8 is_cbe_required = 0;
u32 debug_mode = 0;
u32 delay_enable = 0;
int rl_mid_freq_wa = 0;

u32 effective_cs = 0;

u32 mask_tune_func = (SET_MEDIUM_FREQ_MASK_BIT |
		      WRITE_LEVELING_MASK_BIT |
		      LOAD_PATTERN_2_MASK_BIT |
		      READ_LEVELING_MASK_BIT |
		      SET_TARGET_FREQ_MASK_BIT | WRITE_LEVELING_TF_MASK_BIT |
		      READ_LEVELING_TF_MASK_BIT |
		      CENTRALIZATION_RX_MASK_BIT | CENTRALIZATION_TX_MASK_BIT);

void ddr3_print_version(void)
{
	printf(DDR3_TIP_VERSION_STRING);
}

static int ddr3_tip_ddr3_training_main_flow(u32 dev_num);
static int ddr3_tip_write_odt(u32 dev_num, enum hws_access_type access_type,
			      u32 if_id, u32 cl_value, u32 cwl_value);
static int ddr3_tip_ddr3_auto_tune(u32 dev_num);
static int is_bus_access_done(u32 dev_num, u32 if_id,
			      u32 dunit_reg_adrr, u32 bit);
#ifdef ODT_TEST_SUPPORT
static int odt_test(u32 dev_num, enum hws_algo_type algo_type);
#endif

int adll_calibration(u32 dev_num, enum hws_access_type access_type,
		     u32 if_id, enum hws_ddr_freq frequency);
static int ddr3_tip_set_timing(u32 dev_num, enum hws_access_type access_type,
			       u32 if_id, enum hws_ddr_freq frequency);

static struct page_element page_param[] = {
	/*
	 * 8bits	16 bits
	 * page-size(K)	page-size(K)	mask
	 */
	{ 1,		2,		2},
	/* 512M */
	{ 1,		2,		3},
	/* 1G */
	{ 1,		2,		0},
	/* 2G */
	{ 1,		2,		4},
	/* 4G */
	{ 2,		2,		5}
	/* 8G */
};

static u8 mem_size_config[MEM_SIZE_LAST] = {
	0x2,			/* 512Mbit  */
	0x3,			/* 1Gbit    */
	0x0,			/* 2Gbit    */
	0x4,			/* 4Gbit    */
	0x5			/* 8Gbit    */
};

static u8 cs_mask2_num[] = { 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3 };

static struct reg_data odpg_default_value[] = {
	{0x1034, 0x38000, MASK_ALL_BITS},
	{0x1038, 0x0, MASK_ALL_BITS},
	{0x10b0, 0x0, MASK_ALL_BITS},
	{0x10b8, 0x0, MASK_ALL_BITS},
	{0x10c0, 0x0, MASK_ALL_BITS},
	{0x10f0, 0x0, MASK_ALL_BITS},
	{0x10f4, 0x0, MASK_ALL_BITS},
	{0x10f8, 0xff, MASK_ALL_BITS},
	{0x10fc, 0xffff, MASK_ALL_BITS},
	{0x1130, 0x0, MASK_ALL_BITS},
	{0x1830, 0x2000000, MASK_ALL_BITS},
	{0x14d0, 0x0, MASK_ALL_BITS},
	{0x14d4, 0x0, MASK_ALL_BITS},
	{0x14d8, 0x0, MASK_ALL_BITS},
	{0x14dc, 0x0, MASK_ALL_BITS},
	{0x1454, 0x0, MASK_ALL_BITS},
	{0x1594, 0x0, MASK_ALL_BITS},
	{0x1598, 0x0, MASK_ALL_BITS},
	{0x159c, 0x0, MASK_ALL_BITS},
	{0x15a0, 0x0, MASK_ALL_BITS},
	{0x15a4, 0x0, MASK_ALL_BITS},
	{0x15a8, 0x0, MASK_ALL_BITS},
	{0x15ac, 0x0, MASK_ALL_BITS},
	{0x1604, 0x0, MASK_ALL_BITS},
	{0x1608, 0x0, MASK_ALL_BITS},
	{0x160c, 0x0, MASK_ALL_BITS},
	{0x1610, 0x0, MASK_ALL_BITS},
	{0x1614, 0x0, MASK_ALL_BITS},
	{0x1618, 0x0, MASK_ALL_BITS},
	{0x1624, 0x0, MASK_ALL_BITS},
	{0x1690, 0x0, MASK_ALL_BITS},
	{0x1694, 0x0, MASK_ALL_BITS},
	{0x1698, 0x0, MASK_ALL_BITS},
	{0x169c, 0x0, MASK_ALL_BITS},
	{0x14b8, 0x6f67, MASK_ALL_BITS},
	{0x1630, 0x0, MASK_ALL_BITS},
	{0x1634, 0x0, MASK_ALL_BITS},
	{0x1638, 0x0, MASK_ALL_BITS},
	{0x163c, 0x0, MASK_ALL_BITS},
	{0x16b0, 0x0, MASK_ALL_BITS},
	{0x16b4, 0x0, MASK_ALL_BITS},
	{0x16b8, 0x0, MASK_ALL_BITS},
	{0x16bc, 0x0, MASK_ALL_BITS},
	{0x16c0, 0x0, MASK_ALL_BITS},
	{0x16c4, 0x0, MASK_ALL_BITS},
	{0x16c8, 0x0, MASK_ALL_BITS},
	{0x16cc, 0x1, MASK_ALL_BITS},
	{0x16f0, 0x1, MASK_ALL_BITS},
	{0x16f4, 0x0, MASK_ALL_BITS},
	{0x16f8, 0x0, MASK_ALL_BITS},
	{0x16fc, 0x0, MASK_ALL_BITS}
};

static int ddr3_tip_bus_access(u32 dev_num, enum hws_access_type interface_access,
			       u32 if_id, enum hws_access_type phy_access,
			       u32 phy_id, enum hws_ddr_phy phy_type, u32 reg_addr,
			       u32 data_value, enum hws_operation oper_type);
static int ddr3_tip_pad_inv(u32 dev_num, u32 if_id);
static int ddr3_tip_rank_control(u32 dev_num, u32 if_id);

/*
 * Update global training parameters by data from user
 */
int ddr3_tip_tune_training_params(u32 dev_num,
				  struct tune_train_params *params)
{
	if (params->ck_delay != -1)
		ck_delay = params->ck_delay;
	if (params->ck_delay_16 != -1)
		ck_delay_16 = params->ck_delay_16;
	if (params->phy_reg3_val != -1)
		phy_reg3_val = params->phy_reg3_val;

	return MV_OK;
}

/*
 * Configure CS
 */
int ddr3_tip_configure_cs(u32 dev_num, u32 if_id, u32 cs_num, u32 enable)
{
	u32 data, addr_hi, data_high;
	u32 mem_index;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (enable == 1) {
		data = (tm->interface_params[if_id].bus_width ==
			BUS_WIDTH_8) ? 0 : 1;
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      SDRAM_ACCESS_CONTROL_REG, (data << (cs_num * 4)),
			      0x3 << (cs_num * 4)));
		mem_index = tm->interface_params[if_id].memory_size;

		addr_hi = mem_size_config[mem_index] & 0x3;
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      SDRAM_ACCESS_CONTROL_REG,
			      (addr_hi << (2 + cs_num * 4)),
			      0x3 << (2 + cs_num * 4)));

		data_high = (mem_size_config[mem_index] & 0x4) >> 2;
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      SDRAM_ACCESS_CONTROL_REG,
			      data_high << (20 + cs_num), 1 << (20 + cs_num)));

		/* Enable Address Select Mode */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      SDRAM_ACCESS_CONTROL_REG, 1 << (16 + cs_num),
			      1 << (16 + cs_num)));
	}
	switch (cs_num) {
	case 0:
	case 1:
	case 2:
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      DDR_CONTROL_LOW_REG, (enable << (cs_num + 11)),
			      1 << (cs_num + 11)));
		break;
	case 3:
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      DDR_CONTROL_LOW_REG, (enable << 15), 1 << 15));
		break;
	}

	return MV_OK;
}

/*
 * Calculate number of CS
 */
static int calc_cs_num(u32 dev_num, u32 if_id, u32 *cs_num)
{
	u32 cs;
	u32 bus_cnt;
	u32 cs_count;
	u32 cs_bitmask;
	u32 curr_cs_num = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (bus_cnt = 0; bus_cnt < GET_TOPOLOGY_NUM_OF_BUSES(); bus_cnt++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, bus_cnt);
		cs_count = 0;
		cs_bitmask = tm->interface_params[if_id].
			as_bus_params[bus_cnt].cs_bitmask;
		for (cs = 0; cs < MAX_CS_NUM; cs++) {
			if ((cs_bitmask >> cs) & 1)
				cs_count++;
		}

		if (curr_cs_num == 0) {
			curr_cs_num = cs_count;
		} else if (cs_count != curr_cs_num) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("CS number is different per bus (IF %d BUS %d cs_num %d curr_cs_num %d)\n",
					   if_id, bus_cnt, cs_count,
					   curr_cs_num));
			return MV_NOT_SUPPORTED;
		}
	}
	*cs_num = curr_cs_num;

	return MV_OK;
}

/*
 * Init Controller Flow
 */
int hws_ddr3_tip_init_controller(u32 dev_num, struct init_cntr_param *init_cntr_prm)
{
	u32 if_id;
	u32 cs_num;
	u32 t_refi = 0, t_hclk = 0, t_ckclk = 0, t_faw = 0, t_pd = 0,
		t_wr = 0, t2t = 0, txpdll = 0;
	u32 data_value = 0, bus_width = 0, page_size = 0, cs_cnt = 0,
		mem_mask = 0, bus_index = 0;
	enum hws_speed_bin speed_bin_index = SPEED_BIN_DDR_2133N;
	enum hws_mem_size memory_size = MEM_2G;
	enum hws_ddr_freq freq = init_freq;
	enum hws_timing timing;
	u32 cs_mask = 0;
	u32 cl_value = 0, cwl_val = 0;
	u32 refresh_interval_cnt = 0, bus_cnt = 0, adll_tap = 0;
	enum hws_access_type access_type = ACCESS_TYPE_UNICAST;
	u32 data_read[MAX_INTERFACE_NUM];
	struct hws_topology_map *tm = ddr3_get_topology_map();

	DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
			  ("Init_controller, do_mrs_phy=%d, is_ctrl64_bit=%d\n",
			   init_cntr_prm->do_mrs_phy,
			   init_cntr_prm->is_ctrl64_bit));

	if (init_cntr_prm->init_phy == 1) {
		CHECK_STATUS(ddr3_tip_configure_phy(dev_num));
	}

	if (generic_init_controller == 1) {
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
					  ("active IF %d\n", if_id));
			mem_mask = 0;
			for (bus_index = 0;
			     bus_index < GET_TOPOLOGY_NUM_OF_BUSES();
			     bus_index++) {
				VALIDATE_ACTIVE(tm->bus_act_mask, bus_index);
				mem_mask |=
					tm->interface_params[if_id].
					as_bus_params[bus_index].mirror_enable_bitmask;
			}

			if (mem_mask != 0) {
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, ACCESS_TYPE_MULTICAST,
					      if_id, CS_ENABLE_REG, 0,
					      0x8));
			}

			memory_size =
				tm->interface_params[if_id].
				memory_size;
			speed_bin_index =
				tm->interface_params[if_id].
				speed_bin_index;
			freq = init_freq;
			t_refi =
				(tm->interface_params[if_id].
				 interface_temp ==
				 HWS_TEMP_HIGH) ? TREFI_HIGH : TREFI_LOW;
			t_refi *= 1000;	/* psec */
			DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
					  ("memy_size %d speed_bin_ind %d freq %d t_refi %d\n",
					   memory_size, speed_bin_index, freq,
					   t_refi));
			/* HCLK & CK CLK in 2:1[ps] */
			/* t_ckclk is external clock */
			t_ckclk = (MEGA / freq_val[freq]);
			/* t_hclk is internal clock */
			t_hclk = 2 * t_ckclk;
			refresh_interval_cnt = t_refi / t_hclk;	/* no units */
			bus_width =
				(DDR3_IS_16BIT_DRAM_MODE(tm->bus_act_mask)
				 == 1) ? (16) : (32);

			if (init_cntr_prm->is_ctrl64_bit)
				bus_width = 64;

			data_value =
				(refresh_interval_cnt | 0x4000 |
				 ((bus_width ==
				   32) ? 0x8000 : 0) | 0x1000000) & ~(1 << 26);

			/* Interface Bus Width */
			/* SRMode */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      SDRAM_CONFIGURATION_REG, data_value,
				      0x100ffff));

			/* Interleave first command pre-charge enable (TBD) */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      SDRAM_OPEN_PAGE_CONTROL_REG, (1 << 10),
				      (1 << 10)));

			/* PHY configuration */
			/*
			 * Postamble Length = 1.5cc, Addresscntl to clk skew
			 * \BD, Preamble length normal, parralal ADLL enable
			 */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DRAM_PHY_CONFIGURATION, 0x28, 0x3e));
			if (init_cntr_prm->is_ctrl64_bit) {
				/* positive edge */
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      DRAM_PHY_CONFIGURATION, 0x0,
					      0xff80));
			}

			/* calibration block disable */
			/* Xbar Read buffer select (for Internal access) */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      CALIB_MACHINE_CTRL_REG, 0x1200c,
				      0x7dffe01c));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      CALIB_MACHINE_CTRL_REG,
				      calibration_update_control << 3, 0x3 << 3));

			/* Pad calibration control - enable */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      CALIB_MACHINE_CTRL_REG, 0x1, 0x1));

			cs_mask = 0;
			data_value = 0x7;
			/*
			 * Address ctrl \96 Part of the Generic code
			 * The next configuration is done:
			 * 1)  Memory Size
			 * 2) Bus_width
			 * 3) CS#
			 * 4) Page Number
			 * 5) t_faw
			 * Per Dunit get from the Map_topology the parameters:
			 * Bus_width
			 * t_faw is per Dunit not per CS
			 */
			page_size =
				(tm->interface_params[if_id].
				 bus_width ==
				 BUS_WIDTH_8) ? page_param[memory_size].
				page_size_8bit : page_param[memory_size].
				page_size_16bit;

			t_faw =
				(page_size == 1) ? speed_bin_table(speed_bin_index,
								   SPEED_BIN_TFAW1K)
				: speed_bin_table(speed_bin_index,
						  SPEED_BIN_TFAW2K);

			data_value = TIME_2_CLOCK_CYCLES(t_faw, t_ckclk);
			data_value = data_value << 24;
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      SDRAM_ACCESS_CONTROL_REG, data_value,
				      0x7f000000));

			data_value =
				(tm->interface_params[if_id].
				 bus_width == BUS_WIDTH_8) ? 0 : 1;

			/* create merge cs mask for all cs available in dunit */
			for (bus_cnt = 0;
			     bus_cnt < GET_TOPOLOGY_NUM_OF_BUSES();
			     bus_cnt++) {
				VALIDATE_ACTIVE(tm->bus_act_mask, bus_cnt);
				cs_mask |=
					tm->interface_params[if_id].
					as_bus_params[bus_cnt].cs_bitmask;
			}
			DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
					  ("Init_controller IF %d cs_mask %d\n",
					   if_id, cs_mask));
			/*
			 * Configure the next upon the Map Topology \96 If the
			 * Dunit is CS0 Configure CS0 if it is multi CS
			 * configure them both:  The Bust_width it\92s the
			 * Memory Bus width \96 x8 or x16
			 */
			for (cs_cnt = 0; cs_cnt < NUM_OF_CS; cs_cnt++) {
				ddr3_tip_configure_cs(dev_num, if_id, cs_cnt,
						      ((cs_mask & (1 << cs_cnt)) ? 1
						       : 0));
			}

			if (init_cntr_prm->do_mrs_phy) {
				/*
				 * MR0 \96 Part of the Generic code
				 * The next configuration is done:
				 * 1) Burst Length
				 * 2) CAS Latency
				 * get for each dunit what is it Speed_bin &
				 * Target Frequency. From those both parameters
				 * get the appropriate Cas_l from the CL table
				 */
				cl_value =
					tm->interface_params[if_id].
					cas_l;
				cwl_val =
					tm->interface_params[if_id].
					cas_wl;
				DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
						  ("cl_value 0x%x cwl_val 0x%x\n",
						   cl_value, cwl_val));

				data_value =
					((cl_mask_table[cl_value] & 0x1) << 2) |
					((cl_mask_table[cl_value] & 0xe) << 3);
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      MR0_REG, data_value,
					      (0x7 << 4) | (1 << 2)));
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      MR0_REG, twr_mask_table[t_wr + 1],
					      0xe00));

				/*
				 * MR1: Set RTT and DIC Design GL values
				 * configured by user
				 */
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, ACCESS_TYPE_MULTICAST,
					      PARAM_NOT_CARE, MR1_REG,
					      g_dic | g_rtt_nom, 0x266));

				/* MR2 - Part of the Generic code */
				/*
				 * The next configuration is done:
				 * 1)  SRT
				 * 2) CAS Write Latency
				 */
				data_value = (cwl_mask_table[cwl_val] << 3);
				data_value |=
					((tm->interface_params[if_id].
					  interface_temp ==
					  HWS_TEMP_HIGH) ? (1 << 7) : 0);
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      MR2_REG, data_value,
					      (0x7 << 3) | (0x1 << 7) | (0x3 <<
									 9)));
			}

			ddr3_tip_write_odt(dev_num, access_type, if_id,
					   cl_value, cwl_val);
			ddr3_tip_set_timing(dev_num, access_type, if_id, freq);

			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DUNIT_CONTROL_HIGH_REG, 0x177,
				      0x1000177));

			if (init_cntr_prm->is_ctrl64_bit) {
				/* disable 0.25 cc delay */
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type, if_id,
					      DUNIT_CONTROL_HIGH_REG, 0x0,
					      0x800));
			}

			/* reset bit 7 */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DUNIT_CONTROL_HIGH_REG,
				      (init_cntr_prm->msys_init << 7), (1 << 7)));

			timing = tm->interface_params[if_id].timing;

			if (mode2_t != 0xff) {
				t2t = mode2_t;
			} else if (timing != HWS_TIM_DEFAULT) {
				/* Board topology map is forcing timing */
				t2t = (timing == HWS_TIM_2T) ? 1 : 0;
			} else {
				/* calculate number of CS (per interface) */
				CHECK_STATUS(calc_cs_num
					     (dev_num, if_id, &cs_num));
				t2t = (cs_num == 1) ? 0 : 1;
			}

			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DDR_CONTROL_LOW_REG, t2t << 3,
				      0x3 << 3));
			/* move the block to ddr3_tip_set_timing - start */
			t_pd = GET_MAX_VALUE(t_ckclk * 3,
					     speed_bin_table(speed_bin_index,
							     SPEED_BIN_TPD));
			t_pd = TIME_2_CLOCK_CYCLES(t_pd, t_ckclk);
			txpdll = GET_MAX_VALUE(t_ckclk * 10, 24);
			txpdll = CEIL_DIVIDE((txpdll - 1), t_ckclk);
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DDR_TIMING_REG, txpdll << 4,
				      0x1f << 4));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DDR_TIMING_REG, 0x28 << 9, 0x3f << 9));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DDR_TIMING_REG, 0xa << 21, 0xff << 21));

			/* move the block to ddr3_tip_set_timing - end */
			/* AUTO_ZQC_TIMING */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      TIMING_REG, (AUTO_ZQC_TIMING | (2 << 20)),
				      0x3fffff));
			CHECK_STATUS(ddr3_tip_if_read
				     (dev_num, access_type, if_id,
				      DRAM_PHY_CONFIGURATION, data_read, 0x30));
			data_value =
				(data_read[if_id] == 0) ? (1 << 11) : 0;
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DUNIT_CONTROL_HIGH_REG, data_value,
				      (1 << 11)));

			/* Set Active control for ODT write transactions */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, ACCESS_TYPE_MULTICAST,
				      PARAM_NOT_CARE, 0x1494, g_odt_config,
				      MASK_ALL_BITS));
		}
	} else {
#ifdef STATIC_ALGO_SUPPORT
		CHECK_STATUS(ddr3_tip_static_init_controller(dev_num));
#if defined(CONFIG_ARMADA_38X) || defined(CONFIG_ARMADA_39X)
		CHECK_STATUS(ddr3_tip_static_phy_init_controller(dev_num));
#endif
#endif /* STATIC_ALGO_SUPPORT */
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(ddr3_tip_rank_control(dev_num, if_id));

		if (init_cntr_prm->do_mrs_phy) {
			CHECK_STATUS(ddr3_tip_pad_inv(dev_num, if_id));
		}

		/* Pad calibration control - disable */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      CALIB_MACHINE_CTRL_REG, 0x0, 0x1));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      CALIB_MACHINE_CTRL_REG,
			      calibration_update_control << 3, 0x3 << 3));
	}

	CHECK_STATUS(ddr3_tip_enable_init_sequence(dev_num));

	if (delay_enable != 0) {
		adll_tap = MEGA / (freq_val[freq] * 64);
		ddr3_tip_cmd_addr_init_delay(dev_num, adll_tap);
	}

	return MV_OK;
}

/*
 * Load Topology map
 */
int hws_ddr3_tip_load_topology_map(u32 dev_num, struct hws_topology_map *tm)
{
	enum hws_speed_bin speed_bin_index;
	enum hws_ddr_freq freq = DDR_FREQ_LIMIT;
	u32 if_id;

	freq_val[DDR_FREQ_LOW_FREQ] = dfs_low_freq;
	tm = ddr3_get_topology_map();
	CHECK_STATUS(ddr3_tip_get_first_active_if
		     ((u8)dev_num, tm->if_act_mask,
		      &first_active_if));
	DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
			  ("board IF_Mask=0x%x num_of_bus_per_interface=0x%x\n",
			   tm->if_act_mask,
			   tm->num_of_bus_per_interface));

	/*
	 * if CL, CWL values are missing in topology map, then fill them
	 * according to speedbin tables
	 */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		speed_bin_index =
			tm->interface_params[if_id].speed_bin_index;
		/* TBD memory frequency of interface 0 only is used ! */
		freq = tm->interface_params[first_active_if].memory_freq;

		DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
				  ("speed_bin_index =%d freq=%d cl=%d cwl=%d\n",
				   speed_bin_index, freq_val[freq],
				   tm->interface_params[if_id].
				   cas_l,
				   tm->interface_params[if_id].
				   cas_wl));

		if (tm->interface_params[if_id].cas_l == 0) {
			tm->interface_params[if_id].cas_l =
				cas_latency_table[speed_bin_index].cl_val[freq];
		}

		if (tm->interface_params[if_id].cas_wl == 0) {
			tm->interface_params[if_id].cas_wl =
				cas_write_latency_table[speed_bin_index].cl_val[freq];
		}
	}

	return MV_OK;
}

/*
 * RANK Control Flow
 */
static int ddr3_tip_rank_control(u32 dev_num, u32 if_id)
{
	u32 data_value = 0, bus_cnt;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (bus_cnt = 1; bus_cnt < GET_TOPOLOGY_NUM_OF_BUSES(); bus_cnt++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, bus_cnt);
		if ((tm->interface_params[if_id].
		     as_bus_params[0].cs_bitmask !=
		     tm->interface_params[if_id].
		     as_bus_params[bus_cnt].cs_bitmask) ||
		    (tm->interface_params[if_id].
		     as_bus_params[0].mirror_enable_bitmask !=
		     tm->interface_params[if_id].
		     as_bus_params[bus_cnt].mirror_enable_bitmask))
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("WARNING:Wrong configuration for pup #%d CS mask and CS mirroring for all pups should be the same\n",
					   bus_cnt));
	}

	data_value |= tm->interface_params[if_id].
		as_bus_params[0].cs_bitmask;
	data_value |= tm->interface_params[if_id].
		as_bus_params[0].mirror_enable_bitmask << 4;

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, RANK_CTRL_REG,
		      data_value, 0xff));

	return MV_OK;
}

/*
 * PAD Inverse Flow
 */
static int ddr3_tip_pad_inv(u32 dev_num, u32 if_id)
{
	u32 bus_cnt, data_value, ck_swap_pup_ctrl;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (bus_cnt = 0; bus_cnt < GET_TOPOLOGY_NUM_OF_BUSES(); bus_cnt++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, bus_cnt);
		if (tm->interface_params[if_id].
		    as_bus_params[bus_cnt].is_dqs_swap == 1) {
			/* dqs swap */
			ddr3_tip_bus_read_modify_write(dev_num, ACCESS_TYPE_UNICAST,
						       if_id, bus_cnt,
						       DDR_PHY_DATA,
						       PHY_CONTROL_PHY_REG, 0xc0,
						       0xc0);
		}

		if (tm->interface_params[if_id].
		    as_bus_params[bus_cnt].is_ck_swap == 1) {
			if (bus_cnt <= 1)
				data_value = 0x5 << 2;
			else
				data_value = 0xa << 2;

			/* mask equals data */
			/* ck swap pup is only control pup #0 ! */
			ck_swap_pup_ctrl = 0;
			ddr3_tip_bus_read_modify_write(dev_num, ACCESS_TYPE_UNICAST,
						       if_id, ck_swap_pup_ctrl,
						       DDR_PHY_CONTROL,
						       PHY_CONTROL_PHY_REG,
						       data_value, data_value);
		}
	}

	return MV_OK;
}

/*
 * Run Training Flow
 */
int hws_ddr3_tip_run_alg(u32 dev_num, enum hws_algo_type algo_type)
{
	int ret = MV_OK, ret_tune = MV_OK;

#ifdef ODT_TEST_SUPPORT
	if (finger_test == 1)
		return odt_test(dev_num, algo_type);
#endif

	if (algo_type == ALGO_TYPE_DYNAMIC) {
		ret = ddr3_tip_ddr3_auto_tune(dev_num);
	} else {
#ifdef STATIC_ALGO_SUPPORT
		{
			enum hws_ddr_freq freq;
			freq = init_freq;

			/* add to mask */
			if (is_adll_calib_before_init != 0) {
				printf("with adll calib before init\n");
				adll_calibration(dev_num, ACCESS_TYPE_MULTICAST,
						 0, freq);
			}
			/*
			 * Frequency per interface is not relevant,
			 * only interface 0
			 */
			ret = ddr3_tip_run_static_alg(dev_num,
						      freq);
		}
#endif
	}

	if (ret != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Run_alg: tuning failed %d\n", ret_tune));
	}

	return ret;
}

#ifdef ODT_TEST_SUPPORT
/*
 * ODT Test
 */
static int odt_test(u32 dev_num, enum hws_algo_type algo_type)
{
	int ret = MV_OK, ret_tune = MV_OK;
	int pfinger_val = 0, nfinger_val;

	for (pfinger_val = p_finger_start; pfinger_val <= p_finger_end;
	     pfinger_val += p_finger_step) {
		for (nfinger_val = n_finger_start; nfinger_val <= n_finger_end;
		     nfinger_val += n_finger_step) {
			if (finger_test != 0) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
						  ("pfinger_val %d nfinger_val %d\n",
						   pfinger_val, nfinger_val));
				p_finger = pfinger_val;
				n_finger = nfinger_val;
			}

			if (algo_type == ALGO_TYPE_DYNAMIC) {
				ret = ddr3_tip_ddr3_auto_tune(dev_num);
			} else {
				/*
				 * Frequency per interface is not relevant,
				 * only interface 0
				 */
				ret = ddr3_tip_run_static_alg(dev_num,
							      init_freq);
			}
		}
	}

	if (ret_tune != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Run_alg: tuning failed %d\n", ret_tune));
		ret = (ret == MV_OK) ? ret_tune : ret;
	}

	return ret;
}
#endif

/*
 * Select Controller
 */
int hws_ddr3_tip_select_ddr_controller(u32 dev_num, int enable)
{
	if (config_func_info[dev_num].tip_dunit_mux_select_func != NULL) {
		return config_func_info[dev_num].
			tip_dunit_mux_select_func((u8)dev_num, enable);
	}

	return MV_FAIL;
}

/*
 * Dunit Register Write
 */
int ddr3_tip_if_write(u32 dev_num, enum hws_access_type interface_access,
		      u32 if_id, u32 reg_addr, u32 data_value, u32 mask)
{
	if (config_func_info[dev_num].tip_dunit_write_func != NULL) {
		return config_func_info[dev_num].
			tip_dunit_write_func((u8)dev_num, interface_access,
					     if_id, reg_addr,
					     data_value, mask);
	}

	return MV_FAIL;
}

/*
 * Dunit Register Read
 */
int ddr3_tip_if_read(u32 dev_num, enum hws_access_type interface_access,
		     u32 if_id, u32 reg_addr, u32 *data, u32 mask)
{
	if (config_func_info[dev_num].tip_dunit_read_func != NULL) {
		return config_func_info[dev_num].
			tip_dunit_read_func((u8)dev_num, interface_access,
					    if_id, reg_addr,
					    data, mask);
	}

	return MV_FAIL;
}

/*
 * Dunit Register Polling
 */
int ddr3_tip_if_polling(u32 dev_num, enum hws_access_type access_type,
			u32 if_id, u32 exp_value, u32 mask, u32 offset,
			u32 poll_tries)
{
	u32 poll_cnt = 0, interface_num = 0, start_if, end_if;
	u32 read_data[MAX_INTERFACE_NUM];
	int ret;
	int is_fail = 0, is_if_fail;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (access_type == ACCESS_TYPE_MULTICAST) {
		start_if = 0;
		end_if = MAX_INTERFACE_NUM - 1;
	} else {
		start_if = if_id;
		end_if = if_id;
	}

	for (interface_num = start_if; interface_num <= end_if; interface_num++) {
		/* polling bit 3 for n times */
		VALIDATE_ACTIVE(tm->if_act_mask, interface_num);

		is_if_fail = 0;
		for (poll_cnt = 0; poll_cnt < poll_tries; poll_cnt++) {
			ret =
				ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST,
						 interface_num, offset, read_data,
						 mask);
			if (ret != MV_OK)
				return ret;

			if (read_data[interface_num] == exp_value)
				break;
		}

		if (poll_cnt >= poll_tries) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("max poll IF #%d\n", interface_num));
			is_fail = 1;
			is_if_fail = 1;
		}

		training_result[training_stage][interface_num] =
			(is_if_fail == 1) ? TEST_FAILED : TEST_SUCCESS;
	}

	return (is_fail == 0) ? MV_OK : MV_FAIL;
}

/*
 * Bus read access
 */
int ddr3_tip_bus_read(u32 dev_num, u32 if_id,
		      enum hws_access_type phy_access, u32 phy_id,
		      enum hws_ddr_phy phy_type, u32 reg_addr, u32 *data)
{
	u32 bus_index = 0;
	u32 data_read[MAX_INTERFACE_NUM];
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (phy_access == ACCESS_TYPE_MULTICAST) {
		for (bus_index = 0; bus_index < GET_TOPOLOGY_NUM_OF_BUSES();
		     bus_index++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_index);
			CHECK_STATUS(ddr3_tip_bus_access
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, ACCESS_TYPE_UNICAST,
				      bus_index, phy_type, reg_addr, 0,
				      OPERATION_READ));
			CHECK_STATUS(ddr3_tip_if_read
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      PHY_REG_FILE_ACCESS, data_read,
				      MASK_ALL_BITS));
			data[bus_index] = (data_read[if_id] & 0xffff);
		}
	} else {
		CHECK_STATUS(ddr3_tip_bus_access
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      phy_access, phy_id, phy_type, reg_addr, 0,
			      OPERATION_READ));
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      PHY_REG_FILE_ACCESS, data_read, MASK_ALL_BITS));

		/*
		 * only 16 lsb bit are valid in Phy (each register is different,
		 * some can actually be less than 16 bits)
		 */
		*data = (data_read[if_id] & 0xffff);
	}

	return MV_OK;
}

/*
 * Bus write access
 */
int ddr3_tip_bus_write(u32 dev_num, enum hws_access_type interface_access,
		       u32 if_id, enum hws_access_type phy_access,
		       u32 phy_id, enum hws_ddr_phy phy_type, u32 reg_addr,
		       u32 data_value)
{
	CHECK_STATUS(ddr3_tip_bus_access
		     (dev_num, interface_access, if_id, phy_access,
		      phy_id, phy_type, reg_addr, data_value, OPERATION_WRITE));

	return MV_OK;
}

/*
 * Bus access routine (relevant for both read & write)
 */
static int ddr3_tip_bus_access(u32 dev_num, enum hws_access_type interface_access,
			       u32 if_id, enum hws_access_type phy_access,
			       u32 phy_id, enum hws_ddr_phy phy_type, u32 reg_addr,
			       u32 data_value, enum hws_operation oper_type)
{
	u32 addr_low = 0x3f & reg_addr;
	u32 addr_hi = ((0xc0 & reg_addr) >> 6);
	u32 data_p1 =
		(oper_type << 30) + (addr_hi << 28) + (phy_access << 27) +
		(phy_type << 26) + (phy_id << 22) + (addr_low << 16) +
		(data_value & 0xffff);
	u32 data_p2 = data_p1 + (1 << 31);
	u32 start_if, end_if;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, interface_access, if_id, PHY_REG_FILE_ACCESS,
		      data_p1, MASK_ALL_BITS));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, interface_access, if_id, PHY_REG_FILE_ACCESS,
		      data_p2, MASK_ALL_BITS));

	if (interface_access == ACCESS_TYPE_UNICAST) {
		start_if = if_id;
		end_if = if_id;
	} else {
		start_if = 0;
		end_if = MAX_INTERFACE_NUM - 1;
	}

	/* polling for read/write execution done */
	for (if_id = start_if; if_id <= end_if; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(is_bus_access_done
			     (dev_num, if_id, PHY_REG_FILE_ACCESS, 31));
	}

	return MV_OK;
}

/*
 * Check bus access done
 */
static int is_bus_access_done(u32 dev_num, u32 if_id, u32 dunit_reg_adrr,
			      u32 bit)
{
	u32 rd_data = 1;
	u32 cnt = 0;
	u32 data_read[MAX_INTERFACE_NUM];

	CHECK_STATUS(ddr3_tip_if_read
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, dunit_reg_adrr,
		      data_read, MASK_ALL_BITS));
	rd_data = data_read[if_id];
	rd_data &= (1 << bit);

	while (rd_data != 0) {
		if (cnt++ >= MAX_POLLING_ITERATIONS)
			break;

		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      dunit_reg_adrr, data_read, MASK_ALL_BITS));
		rd_data = data_read[if_id];
		rd_data &= (1 << bit);
	}

	if (cnt < MAX_POLLING_ITERATIONS)
		return MV_OK;
	else
		return MV_FAIL;
}

/*
 * Phy read-modify-write
 */
int ddr3_tip_bus_read_modify_write(u32 dev_num, enum hws_access_type access_type,
				   u32 interface_id, u32 phy_id,
				   enum hws_ddr_phy phy_type, u32 reg_addr,
				   u32 data_value, u32 reg_mask)
{
	u32 data_val = 0, if_id, start_if, end_if;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (access_type == ACCESS_TYPE_MULTICAST) {
		start_if = 0;
		end_if = MAX_INTERFACE_NUM - 1;
	} else {
		start_if = interface_id;
		end_if = interface_id;
	}

	for (if_id = start_if; if_id <= end_if; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(ddr3_tip_bus_read
			     (dev_num, if_id, ACCESS_TYPE_UNICAST, phy_id,
			      phy_type, reg_addr, &data_val));
		data_value = (data_val & (~reg_mask)) | (data_value & reg_mask);
		CHECK_STATUS(ddr3_tip_bus_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      ACCESS_TYPE_UNICAST, phy_id, phy_type, reg_addr,
			      data_value));
	}

	return MV_OK;
}

/*
 * ADLL Calibration
 */
int adll_calibration(u32 dev_num, enum hws_access_type access_type,
		     u32 if_id, enum hws_ddr_freq frequency)
{
	struct hws_tip_freq_config_info freq_config_info;
	u32 bus_cnt = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/* Reset Diver_b assert -> de-assert */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, SDRAM_CONFIGURATION_REG,
		      0, 0x10000000));
	mdelay(10);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, SDRAM_CONFIGURATION_REG,
		      0x10000000, 0x10000000));

	if (config_func_info[dev_num].tip_get_freq_config_info_func != NULL) {
		CHECK_STATUS(config_func_info[dev_num].
			     tip_get_freq_config_info_func((u8)dev_num, frequency,
							   &freq_config_info));
	} else {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("tip_get_freq_config_info_func is NULL"));
		return MV_NOT_INITIALIZED;
	}

	for (bus_cnt = 0; bus_cnt < GET_TOPOLOGY_NUM_OF_BUSES(); bus_cnt++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, bus_cnt);
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, access_type, if_id, bus_cnt,
			      DDR_PHY_DATA, BW_PHY_REG,
			      freq_config_info.bw_per_freq << 8, 0x700));
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, access_type, if_id, bus_cnt,
			      DDR_PHY_DATA, RATE_PHY_REG,
			      freq_config_info.rate_per_freq, 0x7));
	}

	/* DUnit to Phy drive post edge, ADLL reset assert de-assert */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, DRAM_PHY_CONFIGURATION,
		      0, (0x80000000 | 0x40000000)));
	mdelay(100 / (freq_val[frequency] / freq_val[DDR_FREQ_LOW_FREQ]));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, DRAM_PHY_CONFIGURATION,
		      (0x80000000 | 0x40000000), (0x80000000 | 0x40000000)));

	/* polling for ADLL Done */
	if (ddr3_tip_if_polling(dev_num, access_type, if_id,
				0x3ff03ff, 0x3ff03ff, PHY_LOCK_STATUS_REG,
				MAX_POLLING_ITERATIONS) != MV_OK) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Freq_set: DDR3 poll failed(1)"));
	}

	/* pup data_pup reset assert-> deassert */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, SDRAM_CONFIGURATION_REG,
		      0, 0x60000000));
	mdelay(10);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id, SDRAM_CONFIGURATION_REG,
		      0x60000000, 0x60000000));

	return MV_OK;
}

int ddr3_tip_freq_set(u32 dev_num, enum hws_access_type access_type,
		      u32 if_id, enum hws_ddr_freq frequency)
{
	u32 cl_value = 0, cwl_value = 0, mem_mask = 0, val = 0,
		bus_cnt = 0, t_hclk = 0, t_wr = 0,
		refresh_interval_cnt = 0, cnt_id;
	u32 t_refi = 0, end_if, start_if;
	u32 bus_index = 0;
	int is_dll_off = 0;
	enum hws_speed_bin speed_bin_index = 0;
	struct hws_tip_freq_config_info freq_config_info;
	enum hws_result *flow_result = training_result[training_stage];
	u32 adll_tap = 0;
	u32 cs_mask[MAX_INTERFACE_NUM];
	struct hws_topology_map *tm = ddr3_get_topology_map();

	DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
			  ("dev %d access %d IF %d freq %d\n", dev_num,
			   access_type, if_id, frequency));

	if (frequency == DDR_FREQ_LOW_FREQ)
		is_dll_off = 1;
	if (access_type == ACCESS_TYPE_MULTICAST) {
		start_if = 0;
		end_if = MAX_INTERFACE_NUM - 1;
	} else {
		start_if = if_id;
		end_if = if_id;
	}

	/* calculate interface cs mask - Oferb 4/11 */
	/* speed bin can be different for each interface */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		/* cs enable is active low */
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		cs_mask[if_id] = CS_BIT_MASK;
		training_result[training_stage][if_id] = TEST_SUCCESS;
		ddr3_tip_calc_cs_mask(dev_num, if_id, effective_cs,
				      &cs_mask[if_id]);
	}

	/* speed bin can be different for each interface */
	/*
	 * moti b - need to remove the loop for multicas access functions
	 * and loop the unicast access functions
	 */
	for (if_id = start_if; if_id <= end_if; if_id++) {
		if (IS_ACTIVE(tm->if_act_mask, if_id) == 0)
			continue;

		flow_result[if_id] = TEST_SUCCESS;
		speed_bin_index =
			tm->interface_params[if_id].speed_bin_index;
		if (tm->interface_params[if_id].memory_freq ==
		    frequency) {
			cl_value =
				tm->interface_params[if_id].cas_l;
			cwl_value =
				tm->interface_params[if_id].cas_wl;
		} else {
			cl_value =
				cas_latency_table[speed_bin_index].cl_val[frequency];
			cwl_value =
				cas_write_latency_table[speed_bin_index].
				cl_val[frequency];
		}

		DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
				  ("Freq_set dev 0x%x access 0x%x if 0x%x freq 0x%x speed %d:\n\t",
				   dev_num, access_type, if_id,
				   frequency, speed_bin_index));

		for (cnt_id = 0; cnt_id < DDR_FREQ_LIMIT; cnt_id++) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
					  ("%d ",
					   cas_latency_table[speed_bin_index].
					   cl_val[cnt_id]));
		}

		DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE, ("\n"));
		mem_mask = 0;
		for (bus_index = 0; bus_index < GET_TOPOLOGY_NUM_OF_BUSES();
		     bus_index++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_index);
			mem_mask |=
				tm->interface_params[if_id].
				as_bus_params[bus_index].mirror_enable_bitmask;
		}

		if (mem_mask != 0) {
			/* motib redundant in KW28 */
			CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
						       if_id,
						       CS_ENABLE_REG, 0, 0x8));
		}

		/* dll state after exiting SR */
		if (is_dll_off == 1) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DFS_REG, 0x1, 0x1));
		} else {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      DFS_REG, 0, 0x1));
		}

		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      DUNIT_MMASK_REG, 0, 0x1));
		/* DFS  - block  transactions */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      DFS_REG, 0x2, 0x2));

		/* disable ODT in case of dll off */
		if (is_dll_off == 1) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      0x1874, 0, 0x244));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      0x1884, 0, 0x244));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      0x1894, 0, 0x244));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id,
				      0x18a4, 0, 0x244));
		}

		/* DFS  - Enter Self-Refresh */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG, 0x4,
			      0x4));
		/* polling on self refresh entry */
		if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST,
					if_id, 0x8, 0x8, DFS_REG,
					MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("Freq_set: DDR3 poll failed on SR entry\n"));
		}

		/* PLL configuration */
		if (config_func_info[dev_num].tip_set_freq_divider_func != NULL) {
			config_func_info[dev_num].
				tip_set_freq_divider_func(dev_num, if_id,
							  frequency);
		}

		/* PLL configuration End */

		/* adjust t_refi to new frequency */
		t_refi = (tm->interface_params[if_id].interface_temp ==
			  HWS_TEMP_HIGH) ? TREFI_LOW : TREFI_HIGH;
		t_refi *= 1000;	/*psec */

		/* HCLK in[ps] */
		t_hclk = MEGA / (freq_val[frequency] / 2);
		refresh_interval_cnt = t_refi / t_hclk;	/* no units */
		val = 0x4000 | refresh_interval_cnt;
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_CONFIGURATION_REG, val, 0x7fff));

		/* DFS  - CL/CWL/WR parameters after exiting SR */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG,
			      (cl_mask_table[cl_value] << 8), 0xf00));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG,
			      (cwl_mask_table[cwl_value] << 12), 0x7000));
		t_wr = speed_bin_table(speed_bin_index, SPEED_BIN_TWR);
		t_wr = (t_wr / 1000);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG,
			      (twr_mask_table[t_wr + 1] << 16), 0x70000));

		/* Restore original RTT values if returning from DLL OFF mode */
		if (is_dll_off == 1) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id, 0x1874,
				      g_dic | g_rtt_nom, 0x266));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id, 0x1884,
				      g_dic | g_rtt_nom, 0x266));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id, 0x1894,
				      g_dic | g_rtt_nom, 0x266));
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, if_id, 0x18a4,
				      g_dic | g_rtt_nom, 0x266));
		}

		/* Reset Diver_b assert -> de-assert */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_CONFIGURATION_REG, 0, 0x10000000));
		mdelay(10);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_CONFIGURATION_REG, 0x10000000, 0x10000000));

		/* Adll configuration function of process and Frequency */
		if (config_func_info[dev_num].tip_get_freq_config_info_func != NULL) {
			CHECK_STATUS(config_func_info[dev_num].
				     tip_get_freq_config_info_func(dev_num, frequency,
								   &freq_config_info));
		}
		/* TBD check milo5 using device ID ? */
		for (bus_cnt = 0; bus_cnt < GET_TOPOLOGY_NUM_OF_BUSES();
		     bus_cnt++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_cnt);
			CHECK_STATUS(ddr3_tip_bus_read_modify_write
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, bus_cnt, DDR_PHY_DATA,
				      0x92,
				      freq_config_info.
				      bw_per_freq << 8
				      /*freq_mask[dev_num][frequency] << 8 */
				      , 0x700));
			CHECK_STATUS(ddr3_tip_bus_read_modify_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      bus_cnt, DDR_PHY_DATA, 0x94,
				      freq_config_info.rate_per_freq, 0x7));
		}

		/* DUnit to Phy drive post edge, ADLL reset assert de-assert */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      DRAM_PHY_CONFIGURATION, 0,
			      (0x80000000 | 0x40000000)));
		mdelay(100 / (freq_val[frequency] / freq_val[DDR_FREQ_LOW_FREQ]));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      DRAM_PHY_CONFIGURATION, (0x80000000 | 0x40000000),
			      (0x80000000 | 0x40000000)));

		/* polling for ADLL Done */
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x3ff03ff,
		     0x3ff03ff, PHY_LOCK_STATUS_REG,
		     MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("Freq_set: DDR3 poll failed(1)\n"));
		}

		/* pup data_pup reset assert-> deassert */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_CONFIGURATION_REG, 0, 0x60000000));
		mdelay(10);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_CONFIGURATION_REG, 0x60000000, 0x60000000));

		/* Set proper timing params before existing Self-Refresh */
		ddr3_tip_set_timing(dev_num, access_type, if_id, frequency);
		if (delay_enable != 0) {
			adll_tap = MEGA / (freq_val[frequency] * 64);
			ddr3_tip_cmd_addr_init_delay(dev_num, adll_tap);
		}

		/* Exit SR */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG, 0,
			      0x4));
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0, 0x8, DFS_REG,
		     MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("Freq_set: DDR3 poll failed(2)"));
		}

		/* Refresh Command */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      SDRAM_OPERATION_REG, 0x2, 0xf1f));
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0, 0x1f,
		     SDRAM_OPERATION_REG, MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("Freq_set: DDR3 poll failed(3)"));
		}

		/* Release DFS Block */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DFS_REG, 0,
			      0x2));
		/* Controller to MBUS Retry - normal */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, DUNIT_MMASK_REG,
			      0x1, 0x1));

		/* MRO: Burst Length 8, CL , Auto_precharge 0x16cc */
		val =
			((cl_mask_table[cl_value] & 0x1) << 2) |
			((cl_mask_table[cl_value] & 0xe) << 3);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id, MR0_REG,
			      val, (0x7 << 4) | (1 << 2)));
		/* MR2:  CWL = 10 , Auto Self-Refresh - disable */
		val = (cwl_mask_table[cwl_value] << 3);
		/*
		 * nklein 24.10.13 - should not be here - leave value as set in
		 * the init configuration val |= (1 << 9);
		 * val |= ((tm->interface_params[if_id].
		 * interface_temp == HWS_TEMP_HIGH) ? (1 << 7) : 0);
		 */
		/* nklein 24.10.13 - see above comment */
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
					       if_id, MR2_REG,
					       val, (0x7 << 3)));

		/* ODT TIMING */
		val = ((cl_value - cwl_value + 1) << 4) |
			((cl_value - cwl_value + 6) << 8) |
			((cl_value - 1) << 12) | ((cl_value + 6) << 16);
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
					       if_id, ODT_TIMING_LOW,
					       val, 0xffff0));
		val = 0x71 | ((cwl_value - 1) << 8) | ((cwl_value + 5) << 12);
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
					       if_id, ODT_TIMING_HI_REG,
					       val, 0xffff));

		/* ODT Active */
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
					       if_id,
					       DUNIT_ODT_CONTROL_REG,
					       0xf, 0xf));

		/* re-write CL */
		val = ((cl_mask_table[cl_value] & 0x1) << 2) |
			((cl_mask_table[cl_value] & 0xe) << 3);
		CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
					       0, MR0_REG, val,
					       (0x7 << 4) | (1 << 2)));

		/* re-write CWL */
		val = (cwl_mask_table[cwl_value] << 3);
		CHECK_STATUS(ddr3_tip_write_mrs_cmd(dev_num, cs_mask, MRS2_CMD,
						    val, (0x7 << 3)));
		CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
					       0, MR2_REG, val, (0x7 << 3)));

		if (mem_mask != 0) {
			CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
						       if_id,
						       CS_ENABLE_REG,
						       1 << 3, 0x8));
		}
	}

	return MV_OK;
}

/*
 * Set ODT values
 */
static int ddr3_tip_write_odt(u32 dev_num, enum hws_access_type access_type,
			      u32 if_id, u32 cl_value, u32 cwl_value)
{
	/* ODT TIMING */
	u32 val = (cl_value - cwl_value + 6);

	val = ((cl_value - cwl_value + 1) << 4) | ((val & 0xf) << 8) |
		(((cl_value - 1) & 0xf) << 12) |
		(((cl_value + 6) & 0xf) << 16) | (((val & 0x10) >> 4) << 21);
	val |= (((cl_value - 1) >> 4) << 22) | (((cl_value + 6) >> 4) << 23);

	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       ODT_TIMING_LOW, val, 0xffff0));
	val = 0x71 | ((cwl_value - 1) << 8) | ((cwl_value + 5) << 12);
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       ODT_TIMING_HI_REG, val, 0xffff));
	if (odt_additional == 1) {
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type,
					       if_id,
					       SDRAM_ODT_CONTROL_HIGH_REG,
					       0xf, 0xf));
	}

	/* ODT Active */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       DUNIT_ODT_CONTROL_REG, 0xf, 0xf));

	return MV_OK;
}

/*
 * Set Timing values for training
 */
static int ddr3_tip_set_timing(u32 dev_num, enum hws_access_type access_type,
			       u32 if_id, enum hws_ddr_freq frequency)
{
	u32 t_ckclk = 0, t_ras = 0;
	u32 t_rcd = 0, t_rp = 0, t_wr = 0, t_wtr = 0, t_rrd = 0, t_rtp = 0,
		t_rfc = 0, t_mod = 0;
	u32 val = 0, page_size = 0;
	enum hws_speed_bin speed_bin_index;
	enum hws_mem_size memory_size = MEM_2G;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	speed_bin_index = tm->interface_params[if_id].speed_bin_index;
	memory_size = tm->interface_params[if_id].memory_size;
	page_size =
		(tm->interface_params[if_id].bus_width ==
		 BUS_WIDTH_8) ? page_param[memory_size].
		page_size_8bit : page_param[memory_size].page_size_16bit;
	t_ckclk = (MEGA / freq_val[frequency]);
	t_rrd =	(page_size == 1) ? speed_bin_table(speed_bin_index,
						   SPEED_BIN_TRRD1K) :
		speed_bin_table(speed_bin_index, SPEED_BIN_TRRD2K);
	t_rrd = GET_MAX_VALUE(t_ckclk * 4, t_rrd);
	t_rtp =	GET_MAX_VALUE(t_ckclk * 4, speed_bin_table(speed_bin_index,
							   SPEED_BIN_TRTP));
	t_wtr = GET_MAX_VALUE(t_ckclk * 4, speed_bin_table(speed_bin_index,
							   SPEED_BIN_TWTR));
	t_ras = TIME_2_CLOCK_CYCLES(speed_bin_table(speed_bin_index,
						    SPEED_BIN_TRAS),
				    t_ckclk);
	t_rcd = TIME_2_CLOCK_CYCLES(speed_bin_table(speed_bin_index,
						    SPEED_BIN_TRCD),
				    t_ckclk);
	t_rp = TIME_2_CLOCK_CYCLES(speed_bin_table(speed_bin_index,
						   SPEED_BIN_TRP),
				   t_ckclk);
	t_wr = TIME_2_CLOCK_CYCLES(speed_bin_table(speed_bin_index,
						   SPEED_BIN_TWR),
				   t_ckclk);
	t_wtr = TIME_2_CLOCK_CYCLES(t_wtr, t_ckclk);
	t_rrd = TIME_2_CLOCK_CYCLES(t_rrd, t_ckclk);
	t_rtp = TIME_2_CLOCK_CYCLES(t_rtp, t_ckclk);
	t_rfc = TIME_2_CLOCK_CYCLES(rfc_table[memory_size] * 1000, t_ckclk);
	t_mod = GET_MAX_VALUE(t_ckclk * 24, 15000);
	t_mod = TIME_2_CLOCK_CYCLES(t_mod, t_ckclk);

	/* SDRAM Timing Low */
	val = (t_ras & 0xf) | (t_rcd << 4) | (t_rp << 8) | (t_wr << 12) |
		(t_wtr << 16) | (((t_ras & 0x30) >> 4) << 20) | (t_rrd << 24) |
		(t_rtp << 28);
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_LOW_REG, val, 0xff3fffff));

	/* SDRAM Timing High */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG,
				       t_rfc & 0x7f, 0x7f));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG,
				       0x180, 0x180));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG,
				       0x600, 0x600));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG,
				       0x1800, 0xf800));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG,
				       ((t_rfc & 0x380) >> 7) << 16, 0x70000));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG, 0,
				       0x380000));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG,
				       (t_mod & 0xf) << 25, 0x1e00000));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG,
				       (t_mod >> 4) << 30, 0xc0000000));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG,
				       0x16000000, 0x1e000000));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
				       SDRAM_TIMING_HIGH_REG,
				       0x40000000, 0xc0000000));

	return MV_OK;
}

/*
 * Mode Read
 */
int hws_ddr3_tip_mode_read(u32 dev_num, struct mode_info *mode_info)
{
	u32 ret;

	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			       MR0_REG, mode_info->reg_mr0, MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;

	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			       MR1_REG, mode_info->reg_mr1, MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;

	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			       MR2_REG, mode_info->reg_mr2, MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;

	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			       MR3_REG, mode_info->reg_mr2, MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;

	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			       READ_DATA_SAMPLE_DELAY, mode_info->read_data_sample,
			       MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;

	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			       READ_DATA_READY_DELAY, mode_info->read_data_ready,
			       MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;

	return MV_OK;
}

/*
 * Get first active IF
 */
int ddr3_tip_get_first_active_if(u8 dev_num, u32 interface_mask,
				 u32 *interface_id)
{
	u32 if_id;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		if (interface_mask & (1 << if_id)) {
			*interface_id = if_id;
			break;
		}
	}

	return MV_OK;
}

/*
 * Write CS Result
 */
int ddr3_tip_write_cs_result(u32 dev_num, u32 offset)
{
	u32 if_id, bus_num, cs_bitmask, data_val, cs_num;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_num = 0; bus_num < tm->num_of_bus_per_interface;
		     bus_num++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_num);
			cs_bitmask =
				tm->interface_params[if_id].
				as_bus_params[bus_num].cs_bitmask;
			if (cs_bitmask != effective_cs) {
				cs_num = GET_CS_FROM_MASK(cs_bitmask);
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_num,
						  DDR_PHY_DATA,
						  offset +
						  CS_REG_VALUE(effective_cs),
						  &data_val);
				ddr3_tip_bus_write(dev_num,
						   ACCESS_TYPE_UNICAST,
						   if_id,
						   ACCESS_TYPE_UNICAST,
						   bus_num, DDR_PHY_DATA,
						   offset +
						   CS_REG_VALUE(cs_num),
						   data_val);
			}
		}
	}

	return MV_OK;
}

/*
 * Write MRS
 */
int ddr3_tip_write_mrs_cmd(u32 dev_num, u32 *cs_mask_arr, u32 cmd,
			   u32 data, u32 mask)
{
	u32 if_id, reg;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	reg = (cmd == MRS1_CMD) ? MR1_REG : MR2_REG;
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       PARAM_NOT_CARE, reg, data, mask));
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      SDRAM_OPERATION_REG,
			      (cs_mask_arr[if_id] << 8) | cmd, 0xf1f));
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		if (ddr3_tip_if_polling(dev_num, ACCESS_TYPE_UNICAST, if_id, 0,
					0x1f, SDRAM_OPERATION_REG,
					MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("write_mrs_cmd: Poll cmd fail"));
		}
	}

	return MV_OK;
}

/*
 * Reset XSB Read FIFO
 */
int ddr3_tip_reset_fifo_ptr(u32 dev_num)
{
	u32 if_id = 0;

	/* Configure PHY reset value to 0 in order to "clean" the FIFO */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, 0x15c8, 0, 0xff000000));
	/*
	 * Move PHY to RL mode (only in RL mode the PHY overrides FIFO values
	 * during FIFO reset)
	 */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, TRAINING_SW_2_REG,
				       0x1, 0x9));
	/* In order that above configuration will influence the PHY */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, 0x15b0,
				       0x80000000, 0x80000000));
	/* Reset read fifo assertion */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, 0x1400, 0, 0x40000000));
	/* Reset read fifo deassertion */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, 0x1400,
				       0x40000000, 0x40000000));
	/* Move PHY back to functional mode */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, TRAINING_SW_2_REG,
				       0x8, 0x9));
	/* Stop training machine */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       if_id, 0x15b4, 0x10000, 0x10000));

	return MV_OK;
}

/*
 * Reset Phy registers
 */
int ddr3_tip_ddr3_reset_phy_regs(u32 dev_num)
{
	u32 if_id, phy_id, cs;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (phy_id = 0; phy_id < tm->num_of_bus_per_interface;
		     phy_id++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, phy_id);
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, ACCESS_TYPE_UNICAST,
				      phy_id, DDR_PHY_DATA,
				      WL_PHY_REG +
				      CS_REG_VALUE(effective_cs),
				      phy_reg0_val));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      RL_PHY_REG + CS_REG_VALUE(effective_cs),
				      phy_reg2_val));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      READ_CENTRALIZATION_PHY_REG +
				      CS_REG_VALUE(effective_cs), phy_reg3_val));
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, phy_id, DDR_PHY_DATA,
				      WRITE_CENTRALIZATION_PHY_REG +
				      CS_REG_VALUE(effective_cs), phy_reg3_val));
		}
	}

	/* Set Receiver Calibration value */
	for (cs = 0; cs < MAX_CS_NUM; cs++) {
		/* PHY register 0xdb bits[5:0] - configure to 63 */
		CHECK_STATUS(ddr3_tip_bus_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      DDR_PHY_DATA, CSN_IOB_VREF_REG(cs), 63));
	}

	return MV_OK;
}

/*
 * Restore Dunit registers
 */
int ddr3_tip_restore_dunit_regs(u32 dev_num)
{
	u32 index_cnt;

	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       PARAM_NOT_CARE, CALIB_MACHINE_CTRL_REG,
				       0x1, 0x1));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       PARAM_NOT_CARE, CALIB_MACHINE_CTRL_REG,
				       calibration_update_control << 3,
				       0x3 << 3));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST,
				       PARAM_NOT_CARE,
				       ODPG_WRITE_READ_MODE_ENABLE_REG,
				       0xffff, MASK_ALL_BITS));

	for (index_cnt = 0; index_cnt < ARRAY_SIZE(odpg_default_value);
	     index_cnt++) {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      odpg_default_value[index_cnt].reg_addr,
			      odpg_default_value[index_cnt].reg_data,
			      odpg_default_value[index_cnt].reg_mask));
	}

	return MV_OK;
}

/*
 * Auto tune main flow
 */
static int ddr3_tip_ddr3_training_main_flow(u32 dev_num)
{
	enum hws_ddr_freq freq = init_freq;
	struct init_cntr_param init_cntr_prm;
	int ret = MV_OK;
	u32 if_id;
	u32 max_cs = hws_ddr3_tip_max_cs_get();
	struct hws_topology_map *tm = ddr3_get_topology_map();

#ifndef EXCLUDE_SWITCH_DEBUG
	if (debug_training == DEBUG_LEVEL_TRACE) {
		CHECK_STATUS(print_device_info((u8)dev_num));
	}
#endif

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		CHECK_STATUS(ddr3_tip_ddr3_reset_phy_regs(dev_num));
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	freq = init_freq;
	if (is_pll_before_init != 0) {
		for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			config_func_info[dev_num].tip_set_freq_divider_func(
				(u8)dev_num, if_id, freq);
		}
	}

	if (is_adll_calib_before_init != 0) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("with adll calib before init\n"));
		adll_calibration(dev_num, ACCESS_TYPE_MULTICAST, 0, freq);
	}

	if (is_reg_dump != 0) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("Dump before init controller\n"));
		ddr3_tip_reg_dump(dev_num);
	}

	if (mask_tune_func & INIT_CONTROLLER_MASK_BIT) {
		training_stage = INIT_CONTROLLER;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("INIT_CONTROLLER_MASK_BIT\n"));
		init_cntr_prm.do_mrs_phy = 1;
		init_cntr_prm.is_ctrl64_bit = 0;
		init_cntr_prm.init_phy = 1;
		init_cntr_prm.msys_init = 0;
		ret = hws_ddr3_tip_init_controller(dev_num, &init_cntr_prm);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("hws_ddr3_tip_init_controller failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

#ifdef STATIC_ALGO_SUPPORT
	if (mask_tune_func & STATIC_LEVELING_MASK_BIT) {
		training_stage = STATIC_LEVELING;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("STATIC_LEVELING_MASK_BIT\n"));
		ret = ddr3_tip_run_static_alg(dev_num, freq);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_run_static_alg failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}
#endif

	if (mask_tune_func & SET_LOW_FREQ_MASK_BIT) {
		training_stage = SET_LOW_FREQ;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("SET_LOW_FREQ_MASK_BIT %d\n",
				   freq_val[low_freq]));
		ret = ddr3_tip_freq_set(dev_num, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE, low_freq);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_freq_set failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & LOAD_PATTERN_MASK_BIT) {
			training_stage = LOAD_PATTERN;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("LOAD_PATTERN_MASK_BIT #%d\n",
					   effective_cs));
			ret = ddr3_tip_load_all_pattern_to_mem(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_load_all_pattern_to_mem failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	if (mask_tune_func & SET_MEDIUM_FREQ_MASK_BIT) {
		training_stage = SET_MEDIUM_FREQ;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("SET_MEDIUM_FREQ_MASK_BIT %d\n",
				   freq_val[medium_freq]));
		ret =
			ddr3_tip_freq_set(dev_num, ACCESS_TYPE_MULTICAST,
					  PARAM_NOT_CARE, medium_freq);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_freq_set failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & WRITE_LEVELING_MASK_BIT) {
		training_stage = WRITE_LEVELING;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("WRITE_LEVELING_MASK_BIT\n"));
		if ((rl_mid_freq_wa == 0) || (freq_val[medium_freq] == 533)) {
			ret = ddr3_tip_dynamic_write_leveling(dev_num);
		} else {
			/* Use old WL */
			ret = ddr3_tip_legacy_dynamic_write_leveling(dev_num);
		}

		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_dynamic_write_leveling failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & LOAD_PATTERN_2_MASK_BIT) {
			training_stage = LOAD_PATTERN_2;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("LOAD_PATTERN_2_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_load_all_pattern_to_mem(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_load_all_pattern_to_mem failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	if (mask_tune_func & READ_LEVELING_MASK_BIT) {
		training_stage = READ_LEVELING;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("READ_LEVELING_MASK_BIT\n"));
		if ((rl_mid_freq_wa == 0) || (freq_val[medium_freq] == 533)) {
			ret = ddr3_tip_dynamic_read_leveling(dev_num, medium_freq);
		} else {
			/* Use old RL */
			ret = ddr3_tip_legacy_dynamic_read_leveling(dev_num);
		}

		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_dynamic_read_leveling failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & WRITE_LEVELING_SUPP_MASK_BIT) {
		training_stage = WRITE_LEVELING_SUPP;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("WRITE_LEVELING_SUPP_MASK_BIT\n"));
		ret = ddr3_tip_dynamic_write_leveling_supp(dev_num);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_dynamic_write_leveling_supp failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & PBS_RX_MASK_BIT) {
			training_stage = PBS_RX;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("PBS_RX_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_pbs_rx(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_pbs_rx failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & PBS_TX_MASK_BIT) {
			training_stage = PBS_TX;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("PBS_TX_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_pbs_tx(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_pbs_tx failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	if (mask_tune_func & SET_TARGET_FREQ_MASK_BIT) {
		training_stage = SET_TARGET_FREQ;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("SET_TARGET_FREQ_MASK_BIT %d\n",
				   freq_val[tm->
					    interface_params[first_active_if].
					    memory_freq]));
		ret = ddr3_tip_freq_set(dev_num, ACCESS_TYPE_MULTICAST,
					PARAM_NOT_CARE,
					tm->interface_params[first_active_if].
					memory_freq);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_freq_set failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & WRITE_LEVELING_TF_MASK_BIT) {
		training_stage = WRITE_LEVELING_TF;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("WRITE_LEVELING_TF_MASK_BIT\n"));
		ret = ddr3_tip_dynamic_write_leveling(dev_num);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_dynamic_write_leveling TF failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & LOAD_PATTERN_HIGH_MASK_BIT) {
		training_stage = LOAD_PATTERN_HIGH;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("LOAD_PATTERN_HIGH\n"));
		ret = ddr3_tip_load_all_pattern_to_mem(dev_num);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_load_all_pattern_to_mem failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & READ_LEVELING_TF_MASK_BIT) {
		training_stage = READ_LEVELING_TF;
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("READ_LEVELING_TF_MASK_BIT\n"));
		ret = ddr3_tip_dynamic_read_leveling(dev_num, tm->
						     interface_params[first_active_if].
						     memory_freq);
		if (is_reg_dump != 0)
			ddr3_tip_reg_dump(dev_num);
		if (ret != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("ddr3_tip_dynamic_read_leveling TF failure\n"));
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}

	if (mask_tune_func & DM_PBS_TX_MASK_BIT) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("DM_PBS_TX_MASK_BIT\n"));
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & VREF_CALIBRATION_MASK_BIT) {
			training_stage = VREF_CALIBRATION;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("VREF\n"));
			ret = ddr3_tip_vref(dev_num);
			if (is_reg_dump != 0) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("VREF Dump\n"));
				ddr3_tip_reg_dump(dev_num);
			}
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_vref failure\n"));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & CENTRALIZATION_RX_MASK_BIT) {
			training_stage = CENTRALIZATION_RX;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("CENTRALIZATION_RX_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_centralization_rx(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_centralization_rx failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & WRITE_LEVELING_SUPP_TF_MASK_BIT) {
			training_stage = WRITE_LEVELING_SUPP_TF;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("WRITE_LEVELING_SUPP_TF_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_dynamic_write_leveling_supp(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_dynamic_write_leveling_supp TF failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		if (mask_tune_func & CENTRALIZATION_TX_MASK_BIT) {
			training_stage = CENTRALIZATION_TX;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("CENTRALIZATION_TX_MASK_BIT CS #%d\n",
					   effective_cs));
			ret = ddr3_tip_centralization_tx(dev_num);
			if (is_reg_dump != 0)
				ddr3_tip_reg_dump(dev_num);
			if (ret != MV_OK) {
				DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
						  ("ddr3_tip_centralization_tx failure CS #%d\n",
						   effective_cs));
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("restore registers to default\n"));
	/* restore register values */
	CHECK_STATUS(ddr3_tip_restore_dunit_regs(dev_num));

	if (is_reg_dump != 0)
		ddr3_tip_reg_dump(dev_num);

	return MV_OK;
}

/*
 * DDR3 Dynamic training flow
 */
static int ddr3_tip_ddr3_auto_tune(u32 dev_num)
{
	u32 if_id, stage, ret;
	int is_if_fail = 0, is_auto_tune_fail = 0;

	training_stage = INIT_CONTROLLER;

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		for (stage = 0; stage < MAX_STAGE_LIMIT; stage++)
			training_result[stage][if_id] = NO_TEST_DONE;
	}

	ret = ddr3_tip_ddr3_training_main_flow(dev_num);

	/* activate XSB test */
	if (xsb_validate_type != 0) {
		run_xsb_test(dev_num, xsb_validation_base_address, 1, 1,
			     0x1024);
	}

	if (is_reg_dump != 0)
		ddr3_tip_reg_dump(dev_num);

	/* print log */
	CHECK_STATUS(ddr3_tip_print_log(dev_num, window_mem_addr));

	if (ret != MV_OK) {
		CHECK_STATUS(ddr3_tip_print_stability_log(dev_num));
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		is_if_fail = 0;
		for (stage = 0; stage < MAX_STAGE_LIMIT; stage++) {
			if (training_result[stage][if_id] == TEST_FAILED)
				is_if_fail = 1;
		}
		if (is_if_fail == 1) {
			is_auto_tune_fail = 1;
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("Auto Tune failed for IF %d\n",
					   if_id));
		}
	}

	if ((ret == MV_FAIL) || (is_auto_tune_fail == 1))
		return MV_FAIL;
	else
		return MV_OK;
}

/*
 * Enable init sequence
 */
int ddr3_tip_enable_init_sequence(u32 dev_num)
{
	int is_fail = 0;
	u32 if_id = 0, mem_mask = 0, bus_index = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/* Enable init sequence */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, ACCESS_TYPE_MULTICAST, 0,
				       SDRAM_INIT_CONTROL_REG, 0x1, 0x1));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);

		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0, 0x1,
		     SDRAM_INIT_CONTROL_REG,
		     MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("polling failed IF %d\n",
					   if_id));
			is_fail = 1;
			continue;
		}

		mem_mask = 0;
		for (bus_index = 0; bus_index < GET_TOPOLOGY_NUM_OF_BUSES();
		     bus_index++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_index);
			mem_mask |=
				tm->interface_params[if_id].
				as_bus_params[bus_index].mirror_enable_bitmask;
		}

		if (mem_mask != 0) {
			/* Disable Multi CS */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, ACCESS_TYPE_MULTICAST,
				      if_id, CS_ENABLE_REG, 1 << 3,
				      1 << 3));
		}
	}

	return (is_fail == 0) ? MV_OK : MV_FAIL;
}

int ddr3_tip_register_dq_table(u32 dev_num, u32 *table)
{
	dq_map_table = table;

	return MV_OK;
}

/*
 * Check if pup search is locked
 */
int ddr3_tip_is_pup_lock(u32 *pup_buf, enum hws_training_result read_mode)
{
	u32 bit_start = 0, bit_end = 0, bit_id;

	if (read_mode == RESULT_PER_BIT) {
		bit_start = 0;
		bit_end = BUS_WIDTH_IN_BITS - 1;
	} else {
		bit_start = 0;
		bit_end = 0;
	}

	for (bit_id = bit_start; bit_id <= bit_end; bit_id++) {
		if (GET_LOCK_RESULT(pup_buf[bit_id]) == 0)
			return 0;
	}

	return 1;
}

/*
 * Get minimum buffer value
 */
u8 ddr3_tip_get_buf_min(u8 *buf_ptr)
{
	u8 min_val = 0xff;
	u8 cnt = 0;

	for (cnt = 0; cnt < BUS_WIDTH_IN_BITS; cnt++) {
		if (buf_ptr[cnt] < min_val)
			min_val = buf_ptr[cnt];
	}

	return min_val;
}

/*
 * Get maximum buffer value
 */
u8 ddr3_tip_get_buf_max(u8 *buf_ptr)
{
	u8 max_val = 0;
	u8 cnt = 0;

	for (cnt = 0; cnt < BUS_WIDTH_IN_BITS; cnt++) {
		if (buf_ptr[cnt] > max_val)
			max_val = buf_ptr[cnt];
	}

	return max_val;
}

/*
 * The following functions return memory parameters:
 * bus and device width, device size
 */

u32 hws_ddr3_get_bus_width(void)
{
	struct hws_topology_map *tm = ddr3_get_topology_map();

	return (DDR3_IS_16BIT_DRAM_MODE(tm->bus_act_mask) ==
		1) ? 16 : 32;
}

u32 hws_ddr3_get_device_width(u32 if_id)
{
	struct hws_topology_map *tm = ddr3_get_topology_map();

	return (tm->interface_params[if_id].bus_width ==
		BUS_WIDTH_8) ? 8 : 16;
}

u32 hws_ddr3_get_device_size(u32 if_id)
{
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (tm->interface_params[if_id].memory_size >=
	    MEM_SIZE_LAST) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Error: Wrong device size of Cs: %d",
				   tm->interface_params[if_id].memory_size));
		return 0;
	} else {
		return 1 << tm->interface_params[if_id].memory_size;
	}
}

int hws_ddr3_calc_mem_cs_size(u32 if_id, u32 cs, u32 *cs_size)
{
	u32 cs_mem_size, dev_size;

	dev_size = hws_ddr3_get_device_size(if_id);
	if (dev_size != 0) {
		cs_mem_size = ((hws_ddr3_get_bus_width() /
				hws_ddr3_get_device_width(if_id)) * dev_size);

		/* the calculated result in Gbytex16 to avoid float using */

		if (cs_mem_size == 2) {
			*cs_size = _128M;
		} else if (cs_mem_size == 4) {
			*cs_size = _256M;
		} else if (cs_mem_size == 8) {
			*cs_size = _512M;
		} else if (cs_mem_size == 16) {
			*cs_size = _1G;
		} else if (cs_mem_size == 32) {
			*cs_size = _2G;
		} else {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("Error: Wrong Memory size of Cs: %d", cs));
			return MV_FAIL;
		}
		return MV_OK;
	} else {
		return MV_FAIL;
	}
}

int hws_ddr3_cs_base_adr_calc(u32 if_id, u32 cs, u32 *cs_base_addr)
{
	u32 cs_mem_size = 0;
#ifdef DEVICE_MAX_DRAM_ADDRESS_SIZE
	u32 physical_mem_size;
	u32 max_mem_size = DEVICE_MAX_DRAM_ADDRESS_SIZE;
#endif

	if (hws_ddr3_calc_mem_cs_size(if_id, cs, &cs_mem_size) != MV_OK)
		return MV_FAIL;

#ifdef DEVICE_MAX_DRAM_ADDRESS_SIZE
	struct hws_topology_map *tm = ddr3_get_topology_map();
	/*
	 * if number of address pins doesn't allow to use max mem size that
	 * is defined in topology mem size is defined by
	 * DEVICE_MAX_DRAM_ADDRESS_SIZE
	 */
	physical_mem_size =
		mv_hwsmem_size[tm->interface_params[0].memory_size];

	if (hws_ddr3_get_device_width(cs) == 16) {
		/*
		 * 16bit mem device can be twice more - no need in less
		 * significant pin
		 */
		max_mem_size = DEVICE_MAX_DRAM_ADDRESS_SIZE * 2;
	}

	if (physical_mem_size > max_mem_size) {
		cs_mem_size = max_mem_size *
			(hws_ddr3_get_bus_width() /
			 hws_ddr3_get_device_width(if_id));
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("Updated Physical Mem size is from 0x%x to %x\n",
				   physical_mem_size,
				   DEVICE_MAX_DRAM_ADDRESS_SIZE));
	}
#endif

	/* calculate CS base addr */
	*cs_base_addr = ((cs_mem_size) * cs) & 0xffff0000;

	return MV_OK;
}
