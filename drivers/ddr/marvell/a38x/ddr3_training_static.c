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

/* Design Guidelines parameters */
u32 g_zpri_data = 123;		/* controller data - P drive strength */
u32 g_znri_data = 123;		/* controller data - N drive strength */
u32 g_zpri_ctrl = 74;		/* controller C/A - P drive strength */
u32 g_znri_ctrl = 74;		/* controller C/A - N drive strength */
u32 g_zpodt_data = 45;		/* controller data - P ODT */
u32 g_znodt_data = 45;		/* controller data - N ODT */
u32 g_zpodt_ctrl = 45;		/* controller data - P ODT */
u32 g_znodt_ctrl = 45;		/* controller data - N ODT */
u32 g_odt_config = 0x120012;
u32 g_rtt_nom = 0x44;
u32 g_dic = 0x2;

#ifdef STATIC_ALGO_SUPPORT

#define PARAM_NOT_CARE		0
#define MAX_STATIC_SEQ		48

u32 silicon_delay[HWS_MAX_DEVICE_NUM];
struct hws_tip_static_config_info static_config[HWS_MAX_DEVICE_NUM];
static reg_data *static_init_controller_config[HWS_MAX_DEVICE_NUM];

/* debug delay in write leveling */
int wl_debug_delay = 0;
/* pup register #3 for functional board */
int function_reg_value = 8;
u32 silicon;

u32 read_ready_delay_phase_offset[] = { 4, 4, 4, 4, 6, 6, 6, 6 };

static struct cs_element chip_select_map[] = {
	/* CS Value (single only)  Num_CS */
	{0, 0},
	{0, 1},
	{1, 1},
	{0, 2},
	{2, 1},
	{0, 2},
	{0, 2},
	{0, 3},
	{3, 1},
	{0, 2},
	{0, 2},
	{0, 3},
	{0, 2},
	{0, 3},
	{0, 3},
	{0, 4}
};

/*
 * Register static init controller DB
 */
int ddr3_tip_init_specific_reg_config(u32 dev_num, reg_data *reg_config_arr)
{
	static_init_controller_config[dev_num] = reg_config_arr;
	return MV_OK;
}

/*
 * Register static info DB
 */
int ddr3_tip_init_static_config_db(
	u32 dev_num, struct hws_tip_static_config_info *static_config_info)
{
	static_config[dev_num].board_trace_arr =
		static_config_info->board_trace_arr;
	static_config[dev_num].package_trace_arr =
		static_config_info->package_trace_arr;
	silicon_delay[dev_num] = static_config_info->silicon_delay;

	return MV_OK;
}

/*
 * Static round trip flow - Calculates the total round trip delay.
 */
int ddr3_tip_static_round_trip_arr_build(u32 dev_num,
					 struct trip_delay_element *table_ptr,
					 int is_wl, u32 *round_trip_delay_arr)
{
	u32 bus_index, global_bus;
	u32 if_id;
	u32 bus_per_interface;
	int sign;
	u32 temp;
	u32 board_trace;
	struct trip_delay_element *pkg_delay_ptr;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/*
	 * In WL we calc the diff between Clock to DQs in RL we sum the round
	 * trip of Clock and DQs
	 */
	sign = (is_wl) ? -1 : 1;

	bus_per_interface = GET_TOPOLOGY_NUM_OF_BUSES();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_index = 0; bus_index < bus_per_interface;
		     bus_index++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_index);
			global_bus = (if_id * bus_per_interface) + bus_index;

			/* calculate total trip delay (package and board) */
			board_trace = (table_ptr[global_bus].dqs_delay * sign) +
				table_ptr[global_bus].ck_delay;
			temp = (board_trace * 163) / 1000;

			/* Convert the length to delay in psec units */
			pkg_delay_ptr =
				static_config[dev_num].package_trace_arr;
			round_trip_delay_arr[global_bus] = temp +
				(int)(pkg_delay_ptr[global_bus].dqs_delay *
				      sign) +
				(int)pkg_delay_ptr[global_bus].ck_delay +
				(int)((is_wl == 1) ? wl_debug_delay :
				      (int)silicon_delay[dev_num]);
			DEBUG_TRAINING_STATIC_IP(
				DEBUG_LEVEL_TRACE,
				("Round Trip Build round_trip_delay_arr[0x%x]: 0x%x    temp 0x%x\n",
				 global_bus, round_trip_delay_arr[global_bus],
				 temp));
		}
	}

	return MV_OK;
}

/*
 * Write leveling for static flow - calculating the round trip delay of the
 * DQS signal.
 */
int ddr3_tip_write_leveling_static_config(u32 dev_num, u32 if_id,
					  enum hws_ddr_freq frequency,
					  u32 *round_trip_delay_arr)
{
	u32 bus_index;		/* index to the bus loop */
	u32 bus_start_index;
	u32 bus_per_interface;
	u32 phase = 0;
	u32 adll = 0, adll_cen, adll_inv, adll_final;
	u32 adll_period = MEGA / freq_val[frequency] / 64;

	DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
				 ("ddr3_tip_write_leveling_static_config\n"));
	DEBUG_TRAINING_STATIC_IP(
		DEBUG_LEVEL_TRACE,
		("dev_num 0x%x IF 0x%x freq %d (adll_period 0x%x)\n",
		 dev_num, if_id, frequency, adll_period));

	bus_per_interface = GET_TOPOLOGY_NUM_OF_BUSES();
	bus_start_index = if_id * bus_per_interface;
	for (bus_index = bus_start_index;
	     bus_index < (bus_start_index + bus_per_interface); bus_index++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, bus_index);
		phase = round_trip_delay_arr[bus_index] / (32 * adll_period);
		adll = (round_trip_delay_arr[bus_index] -
			(phase * 32 * adll_period)) / adll_period;
		adll = (adll > 31) ? 31 : adll;
		adll_cen = 16 + adll;
		adll_inv = adll_cen / 32;
		adll_final = adll_cen - (adll_inv * 32);
		adll_final = (adll_final > 31) ? 31 : adll_final;

		DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
					 ("\t%d - phase 0x%x adll 0x%x\n",
					  bus_index, phase, adll));
		/*
		 * Writing to all 4 phy of Interface number,
		 * bit 0 \96 4 \96 ADLL, bit 6-8 phase
		 */
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      (bus_index % 4), DDR_PHY_DATA,
			      PHY_WRITE_DELAY(cs),
			      ((phase << 6) + (adll & 0x1f)), 0x1df));
		CHECK_STATUS(ddr3_tip_bus_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      ACCESS_TYPE_UNICAST, (bus_index % 4),
			      DDR_PHY_DATA, WRITE_CENTRALIZATION_PHY_REG,
			      ((adll_inv & 0x1) << 5) + adll_final));
	}

	return MV_OK;
}

/*
 * Read leveling for static flow
 */
int ddr3_tip_read_leveling_static_config(u32 dev_num,
					 u32 if_id,
					 enum hws_ddr_freq frequency,
					 u32 *total_round_trip_delay_arr)
{
	u32 cs, data0, data1, data3 = 0;
	u32 bus_index;		/* index to the bus loop */
	u32 bus_start_index;
	u32 phase0, phase1, max_phase;
	u32 adll0, adll1;
	u32 cl_value;
	u32 min_delay;
	u32 sdr_period = MEGA / freq_val[frequency];
	u32 ddr_period = MEGA / freq_val[frequency] / 2;
	u32 adll_period = MEGA / freq_val[frequency] / 64;
	enum hws_speed_bin speed_bin_index;
	u32 rd_sample_dly[MAX_CS_NUM] = { 0 };
	u32 rd_ready_del[MAX_CS_NUM] = { 0 };
	u32 bus_per_interface = GET_TOPOLOGY_NUM_OF_BUSES();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
				 ("ddr3_tip_read_leveling_static_config\n"));
	DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
				 ("dev_num 0x%x ifc 0x%x freq %d\n", dev_num,
				  if_id, frequency));
	DEBUG_TRAINING_STATIC_IP(
		DEBUG_LEVEL_TRACE,
		("Sdr_period 0x%x Ddr_period 0x%x adll_period 0x%x\n",
		 sdr_period, ddr_period, adll_period));

	if (tm->interface_params[first_active_if].memory_freq ==
	    frequency) {
		cl_value = tm->interface_params[first_active_if].cas_l;
		DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
					 ("cl_value 0x%x\n", cl_value));
	} else {
		speed_bin_index = tm->interface_params[if_id].speed_bin_index;
		cl_value = cas_latency_table[speed_bin_index].cl_val[frequency];
		DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
					 ("cl_value 0x%x speed_bin_index %d\n",
					  cl_value, speed_bin_index));
	}

	bus_start_index = if_id * bus_per_interface;

	for (bus_index = bus_start_index;
	     bus_index < (bus_start_index + bus_per_interface);
	     bus_index += 2) {
		VALIDATE_ACTIVE(tm->bus_act_mask, bus_index);
		cs = chip_select_map[
			tm->interface_params[if_id].as_bus_params[
				(bus_index % 4)].cs_bitmask].cs_num;

		/* read sample delay calculation */
		min_delay = (total_round_trip_delay_arr[bus_index] <
			     total_round_trip_delay_arr[bus_index + 1]) ?
			total_round_trip_delay_arr[bus_index] :
			total_round_trip_delay_arr[bus_index + 1];
		/* round down */
		rd_sample_dly[cs] = 2 * (min_delay / (sdr_period * 2));
		DEBUG_TRAINING_STATIC_IP(
			DEBUG_LEVEL_TRACE,
			("\t%d - min_delay 0x%x cs 0x%x rd_sample_dly[cs] 0x%x\n",
			 bus_index, min_delay, cs, rd_sample_dly[cs]));

		/* phase calculation */
		phase0 = (total_round_trip_delay_arr[bus_index] -
			  (sdr_period * rd_sample_dly[cs])) / (ddr_period);
		phase1 = (total_round_trip_delay_arr[bus_index + 1] -
			  (sdr_period * rd_sample_dly[cs])) / (ddr_period);
		max_phase = (phase0 > phase1) ? phase0 : phase1;
		DEBUG_TRAINING_STATIC_IP(
			DEBUG_LEVEL_TRACE,
			("\tphase0 0x%x phase1 0x%x max_phase 0x%x\n",
			 phase0, phase1, max_phase));

		/* ADLL calculation */
		adll0 = (u32)((total_round_trip_delay_arr[bus_index] -
			       (sdr_period * rd_sample_dly[cs]) -
			       (ddr_period * phase0)) / adll_period);
		adll0 = (adll0 > 31) ? 31 : adll0;
		adll1 = (u32)((total_round_trip_delay_arr[bus_index + 1] -
			       (sdr_period * rd_sample_dly[cs]) -
			       (ddr_period * phase1)) / adll_period);
		adll1 = (adll1 > 31) ? 31 : adll1;

		/* The Read delay close the Read FIFO */
		rd_ready_del[cs] = rd_sample_dly[cs] +
			read_ready_delay_phase_offset[max_phase];
		DEBUG_TRAINING_STATIC_IP(
			DEBUG_LEVEL_TRACE,
			("\tadll0 0x%x adll1 0x%x rd_ready_del[cs] 0x%x\n",
			 adll0, adll1, rd_ready_del[cs]));

		/*
		 * Write to the phy of Interface (bit 0 \96 4 \96 ADLL,
		 * bit 6-8 phase)
		 */
		data0 = ((phase0 << 6) + (adll0 & 0x1f));
		data1 = ((phase1 << 6) + (adll1 & 0x1f));

		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      (bus_index % 4), DDR_PHY_DATA, PHY_READ_DELAY(cs),
			      data0, 0x1df));
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      ((bus_index + 1) % 4), DDR_PHY_DATA,
			      PHY_READ_DELAY(cs), data1, 0x1df));
	}

	for (bus_index = 0; bus_index < bus_per_interface; bus_index++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, bus_index);
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      bus_index, DDR_PHY_DATA, 0x3, data3, 0x1f));
	}
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id,
		      READ_DATA_SAMPLE_DELAY,
		      (rd_sample_dly[0] + cl_value) + (rd_sample_dly[1] << 8),
		      MASK_ALL_BITS));

	/* Read_ready_del0 bit 0-4 , CS bits 8-12 */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id,
		      READ_DATA_READY_DELAY,
		      rd_ready_del[0] + (rd_ready_del[1] << 8) + cl_value,
		      MASK_ALL_BITS));

	return MV_OK;
}

/*
 * DDR3 Static flow
 */
int ddr3_tip_run_static_alg(u32 dev_num, enum hws_ddr_freq freq)
{
	u32 if_id = 0;
	struct trip_delay_element *table_ptr;
	u32 wl_total_round_trip_delay_arr[MAX_TOTAL_BUS_NUM];
	u32 rl_total_round_trip_delay_arr[MAX_TOTAL_BUS_NUM];
	struct init_cntr_param init_cntr_prm;
	int ret;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
				 ("ddr3_tip_run_static_alg"));

	init_cntr_prm.do_mrs_phy = 1;
	init_cntr_prm.is_ctrl64_bit = 0;
	init_cntr_prm.init_phy = 1;
	ret = hws_ddr3_tip_init_controller(dev_num, &init_cntr_prm);
	if (ret != MV_OK) {
		DEBUG_TRAINING_STATIC_IP(
			DEBUG_LEVEL_ERROR,
			("hws_ddr3_tip_init_controller failure\n"));
	}

	/* calculate the round trip delay for Write Leveling */
	table_ptr = static_config[dev_num].board_trace_arr;
	CHECK_STATUS(ddr3_tip_static_round_trip_arr_build
		     (dev_num, table_ptr, 1,
		      wl_total_round_trip_delay_arr));
	/* calculate the round trip delay  for Read Leveling */
	CHECK_STATUS(ddr3_tip_static_round_trip_arr_build
		     (dev_num, table_ptr, 0,
		      rl_total_round_trip_delay_arr));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		/* check if the interface is enabled */
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		/*
		 * Static frequency is defined according to init-frequency
		 * (not target)
		 */
		DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
					 ("Static IF %d freq %d\n",
					  if_id, freq));
		CHECK_STATUS(ddr3_tip_write_leveling_static_config
			     (dev_num, if_id, freq,
			      wl_total_round_trip_delay_arr));
		CHECK_STATUS(ddr3_tip_read_leveling_static_config
			     (dev_num, if_id, freq,
			      rl_total_round_trip_delay_arr));
	}

	return MV_OK;
}

/*
 * Init controller for static flow
 */
int ddr3_tip_static_init_controller(u32 dev_num)
{
	u32 index_cnt = 0;

	DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
				 ("ddr3_tip_static_init_controller\n"));
	while (static_init_controller_config[dev_num][index_cnt].reg_addr !=
	       0) {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      static_init_controller_config[dev_num][index_cnt].
			      reg_addr,
			      static_init_controller_config[dev_num][index_cnt].
			      reg_data,
			      static_init_controller_config[dev_num][index_cnt].
			      reg_mask));

		DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
					 ("Init_controller index_cnt %d\n",
					  index_cnt));
		index_cnt++;
	}

	return MV_OK;
}

int ddr3_tip_static_phy_init_controller(u32 dev_num)
{
	DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
				 ("Phy Init Controller 2\n"));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA, 0xa4,
		      0x3dfe));

	DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
				 ("Phy Init Controller 3\n"));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA, 0xa6,
		      0xcb2));

	DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
				 ("Phy Init Controller 4\n"));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA, 0xa9,
		      0));

	DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
				 ("Static Receiver Calibration\n"));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA, 0xd0,
		      0x1f));

	DEBUG_TRAINING_STATIC_IP(DEBUG_LEVEL_TRACE,
				 ("Static V-REF Calibration\n"));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA, 0xa8,
		      0x434));

	return MV_OK;
}
#endif

/*
 * Configure phy (called by static init controller) for static flow
 */
int ddr3_tip_configure_phy(u32 dev_num)
{
	u32 if_id, phy_id;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		      PAD_ZRI_CALIB_PHY_REG,
		      ((0x7f & g_zpri_data) << 7 | (0x7f & g_znri_data))));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_CONTROL,
		      PAD_ZRI_CALIB_PHY_REG,
		      ((0x7f & g_zpri_ctrl) << 7 | (0x7f & g_znri_ctrl))));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		      PAD_ODT_CALIB_PHY_REG,
		      ((0x3f & g_zpodt_data) << 6 | (0x3f & g_znodt_data))));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_CONTROL,
		      PAD_ODT_CALIB_PHY_REG,
		      ((0x3f & g_zpodt_ctrl) << 6 | (0x3f & g_znodt_ctrl))));

	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		      PAD_PRE_DISABLE_PHY_REG, 0));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA,
		      CMOS_CONFIG_PHY_REG, 0));
	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_CONTROL,
		      CMOS_CONFIG_PHY_REG, 0));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		/* check if the interface is enabled */
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);

		for (phy_id = 0;
		     phy_id < tm->num_of_bus_per_interface;
		     phy_id++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, phy_id);
			/* Vref & clamp */
			CHECK_STATUS(ddr3_tip_bus_read_modify_write
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, phy_id, DDR_PHY_DATA,
				      PAD_CONFIG_PHY_REG,
				      ((clamp_tbl[if_id] << 4) | vref),
				      ((0x7 << 4) | 0x7)));
			/* clamp not relevant for control */
			CHECK_STATUS(ddr3_tip_bus_read_modify_write
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, phy_id, DDR_PHY_CONTROL,
				      PAD_CONFIG_PHY_REG, 0x4, 0x7));
		}
	}

	CHECK_STATUS(ddr3_tip_bus_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, DDR_PHY_DATA, 0x90,
		      0x6002));

	return MV_OK;
}
