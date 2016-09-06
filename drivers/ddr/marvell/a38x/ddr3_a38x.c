/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <i2c.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "ddr3_init.h"

#define A38X_NUMBER_OF_INTERFACES	5

#define SAR_DEV_ID_OFFS			27
#define SAR_DEV_ID_MASK			0x7

/* Termal Sensor Registers */
#define TSEN_STATE_REG			0xe4070
#define TSEN_STATE_OFFSET		31
#define TSEN_STATE_MASK			(0x1 << TSEN_STATE_OFFSET)
#define TSEN_CONF_REG			0xe4074
#define TSEN_CONF_RST_OFFSET		8
#define TSEN_CONF_RST_MASK		(0x1 << TSEN_CONF_RST_OFFSET)
#define TSEN_STATUS_REG			0xe4078
#define TSEN_STATUS_READOUT_VALID_OFFSET	10
#define TSEN_STATUS_READOUT_VALID_MASK	(0x1 <<				\
					 TSEN_STATUS_READOUT_VALID_OFFSET)
#define TSEN_STATUS_TEMP_OUT_OFFSET	0
#define TSEN_STATUS_TEMP_OUT_MASK	(0x3ff << TSEN_STATUS_TEMP_OUT_OFFSET)

static struct dfx_access interface_map[] = {
	/* Pipe	Client */
	{ 0, 17 },
	{ 1, 7 },
	{ 1, 11 },
	{ 0, 3 },
	{ 1, 25 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 }
};

/* This array hold the board round trip delay (DQ and CK) per <interface,bus> */
struct trip_delay_element a38x_board_round_trip_delay_array[] = {
	/* 1st board */
	/* Interface bus DQS-delay CK-delay */
	{ 3952, 5060 },
	{ 3192, 4493 },
	{ 4785, 6677 },
	{ 3413, 7267 },
	{ 4282, 6086 },	/* ECC PUP */
	{ 3952, 5134 },
	{ 3192, 4567 },
	{ 4785, 6751 },
	{ 3413, 7341 },
	{ 4282, 6160 },	/* ECC PUP */

	/* 2nd board */
	/* Interface bus DQS-delay CK-delay */
	{ 3952, 5060 },
	{ 3192, 4493 },
	{ 4785, 6677 },
	{ 3413, 7267 },
	{ 4282, 6086 },	/* ECC PUP */
	{ 3952, 5134 },
	{ 3192, 4567 },
	{ 4785, 6751 },
	{ 3413, 7341 },
	{ 4282, 6160 }	/* ECC PUP */
};

#ifdef STATIC_ALGO_SUPPORT
/* package trace */
static struct trip_delay_element a38x_package_round_trip_delay_array[] = {
	/* IF BUS DQ_DELAY CK_DELAY */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 }
};

static int a38x_silicon_delay_offset[] = {
	/* board 0 */
	0,
	/* board 1 */
	0,
	/* board 2 */
	0
};
#endif

static u8 a38x_bw_per_freq[DDR_FREQ_LIMIT] = {
	0x3,			/* DDR_FREQ_100 */
	0x4,			/* DDR_FREQ_400 */
	0x4,			/* DDR_FREQ_533 */
	0x5,			/* DDR_FREQ_667 */
	0x5,			/* DDR_FREQ_800 */
	0x5,			/* DDR_FREQ_933 */
	0x5,			/* DDR_FREQ_1066 */
	0x3,			/* DDR_FREQ_311 */
	0x3,			/* DDR_FREQ_333 */
	0x4,			/* DDR_FREQ_467 */
	0x5,			/* DDR_FREQ_850 */
	0x5,			/* DDR_FREQ_600 */
	0x3,			/* DDR_FREQ_300 */
	0x5,			/* DDR_FREQ_900 */
	0x3,			/* DDR_FREQ_360 */
	0x5			/* DDR_FREQ_1000 */
};

static u8 a38x_rate_per_freq[DDR_FREQ_LIMIT] = {
	 /*TBD*/ 0x1,		/* DDR_FREQ_100 */
	0x2,			/* DDR_FREQ_400 */
	0x2,			/* DDR_FREQ_533 */
	0x2,			/* DDR_FREQ_667 */
	0x2,			/* DDR_FREQ_800 */
	0x3,			/* DDR_FREQ_933 */
	0x3,			/* DDR_FREQ_1066 */
	0x1,			/* DDR_FREQ_311 */
	0x1,			/* DDR_FREQ_333 */
	0x2,			/* DDR_FREQ_467 */
	0x2,			/* DDR_FREQ_850 */
	0x2,			/* DDR_FREQ_600 */
	0x1,			/* DDR_FREQ_300 */
	0x2,			/* DDR_FREQ_900 */
	0x1,			/* DDR_FREQ_360 */
	0x2			/* DDR_FREQ_1000 */
};

static u16 a38x_vco_freq_per_sar[] = {
	666,			/* 0 */
	1332,
	800,
	1600,
	1066,
	2132,
	1200,
	2400,
	1332,
	1332,
	1500,
	1500,
	1600,			/* 12 */
	1600,
	1700,
	1700,
	1866,
	1866,
	1800,			/* 18 */
	2000,
	2000,
	4000,
	2132,
	2132,
	2300,
	2300,
	2400,
	2400,
	2500,
	2500,
	800
};

u32 pipe_multicast_mask;

u32 dq_bit_map_2_phy_pin[] = {
	1, 0, 2, 6, 9, 8, 3, 7,	/* 0 */
	8, 9, 1, 7, 2, 6, 3, 0,	/* 1 */
	3, 9, 7, 8, 1, 0, 2, 6,	/* 2 */
	1, 0, 6, 2, 8, 3, 7, 9,	/* 3 */
	0, 1, 2, 9, 7, 8, 3, 6,	/* 4 */
};

static int ddr3_tip_a38x_set_divider(u8 dev_num, u32 if_id,
				     enum hws_ddr_freq freq);

/*
 * Read temperature TJ value
 */
u32 ddr3_ctrl_get_junc_temp(u8 dev_num)
{
	int reg = 0;

	/* Initiates TSEN hardware reset once */
	if ((reg_read(TSEN_CONF_REG) & TSEN_CONF_RST_MASK) == 0)
		reg_bit_set(TSEN_CONF_REG, TSEN_CONF_RST_MASK);
	mdelay(10);

	/* Check if the readout field is valid */
	if ((reg_read(TSEN_STATUS_REG) & TSEN_STATUS_READOUT_VALID_MASK) == 0) {
		printf("%s: TSEN not ready\n", __func__);
		return 0;
	}

	reg = reg_read(TSEN_STATUS_REG);
	reg = (reg & TSEN_STATUS_TEMP_OUT_MASK) >> TSEN_STATUS_TEMP_OUT_OFFSET;

	return ((((10000 * reg) / 21445) * 1000) - 272674) / 1000;
}

/*
 * Name:     ddr3_tip_a38x_get_freq_config.
 * Desc:
 * Args:
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_tip_a38x_get_freq_config(u8 dev_num, enum hws_ddr_freq freq,
				  struct hws_tip_freq_config_info
				  *freq_config_info)
{
	if (a38x_bw_per_freq[freq] == 0xff)
		return MV_NOT_SUPPORTED;

	if (freq_config_info == NULL)
		return MV_BAD_PARAM;

	freq_config_info->bw_per_freq = a38x_bw_per_freq[freq];
	freq_config_info->rate_per_freq = a38x_rate_per_freq[freq];
	freq_config_info->is_supported = 1;

	return MV_OK;
}

/*
 * Name:     ddr3_tip_a38x_pipe_enable.
 * Desc:
 * Args:
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_tip_a38x_pipe_enable(u8 dev_num, enum hws_access_type interface_access,
			      u32 if_id, int enable)
{
	u32 data_value, pipe_enable_mask = 0;

	if (enable == 0) {
		pipe_enable_mask = 0;
	} else {
		if (interface_access == ACCESS_TYPE_MULTICAST)
			pipe_enable_mask = pipe_multicast_mask;
		else
			pipe_enable_mask = (1 << interface_map[if_id].pipe);
	}

	CHECK_STATUS(ddr3_tip_reg_read
		     (dev_num, PIPE_ENABLE_ADDR, &data_value, MASK_ALL_BITS));
	data_value = (data_value & (~0xff)) | pipe_enable_mask;
	CHECK_STATUS(ddr3_tip_reg_write(dev_num, PIPE_ENABLE_ADDR, data_value));

	return MV_OK;
}

/*
 * Name:     ddr3_tip_a38x_if_write.
 * Desc:
 * Args:
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_tip_a38x_if_write(u8 dev_num, enum hws_access_type interface_access,
			   u32 if_id, u32 reg_addr, u32 data_value,
			   u32 mask)
{
	u32 ui_data_read;

	if (mask != MASK_ALL_BITS) {
		CHECK_STATUS(ddr3_tip_a38x_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, reg_addr,
			      &ui_data_read, MASK_ALL_BITS));
		data_value = (ui_data_read & (~mask)) | (data_value & mask);
	}

	reg_write(reg_addr, data_value);

	return MV_OK;
}

/*
 * Name:     ddr3_tip_a38x_if_read.
 * Desc:
 * Args:
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_tip_a38x_if_read(u8 dev_num, enum hws_access_type interface_access,
			  u32 if_id, u32 reg_addr, u32 *data, u32 mask)
{
	*data = reg_read(reg_addr) & mask;

	return MV_OK;
}

/*
 * Name:     ddr3_tip_a38x_select_ddr_controller.
 * Desc:     Enable/Disable access to Marvell's server.
 * Args:     dev_num     - device number
 *           enable        - whether to enable or disable the server
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_tip_a38x_select_ddr_controller(u8 dev_num, int enable)
{
	u32 reg;

	reg = reg_read(CS_ENABLE_REG);

	if (enable)
		reg |= (1 << 6);
	else
		reg &= ~(1 << 6);

	reg_write(CS_ENABLE_REG, reg);

	return MV_OK;
}

/*
 * Name:     ddr3_tip_init_a38x_silicon.
 * Desc:     init Training SW DB.
 * Args:
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
static int ddr3_tip_init_a38x_silicon(u32 dev_num, u32 board_id)
{
	struct hws_tip_config_func_db config_func;
	enum hws_ddr_freq ddr_freq;
	int status;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/* new read leveling version */
	config_func.tip_dunit_read_func = ddr3_tip_a38x_if_read;
	config_func.tip_dunit_write_func = ddr3_tip_a38x_if_write;
	config_func.tip_dunit_mux_select_func =
		ddr3_tip_a38x_select_ddr_controller;
	config_func.tip_get_freq_config_info_func =
		ddr3_tip_a38x_get_freq_config;
	config_func.tip_set_freq_divider_func = ddr3_tip_a38x_set_divider;
	config_func.tip_get_device_info_func = ddr3_tip_a38x_get_device_info;
	config_func.tip_get_temperature = ddr3_ctrl_get_junc_temp;

	ddr3_tip_init_config_func(dev_num, &config_func);

	ddr3_tip_register_dq_table(dev_num, dq_bit_map_2_phy_pin);

#ifdef STATIC_ALGO_SUPPORT
	{
		struct hws_tip_static_config_info static_config;
		u32 board_offset =
		    board_id * A38X_NUMBER_OF_INTERFACES *
		    tm->num_of_bus_per_interface;

		static_config.silicon_delay =
			a38x_silicon_delay_offset[board_id];
		static_config.package_trace_arr =
			a38x_package_round_trip_delay_array;
		static_config.board_trace_arr =
			&a38x_board_round_trip_delay_array[board_offset];
		ddr3_tip_init_static_config_db(dev_num, &static_config);
	}
#endif
	status = ddr3_tip_a38x_get_init_freq(dev_num, &ddr_freq);
	if (MV_OK != status) {
		DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR,
				      ("DDR3 silicon get target frequency - FAILED 0x%x\n",
				       status));
		return status;
	}

	rl_version = 1;
	mask_tune_func = (SET_LOW_FREQ_MASK_BIT |
			  LOAD_PATTERN_MASK_BIT |
			  SET_MEDIUM_FREQ_MASK_BIT | WRITE_LEVELING_MASK_BIT |
			  /* LOAD_PATTERN_2_MASK_BIT | */
			  WRITE_LEVELING_SUPP_MASK_BIT |
			  READ_LEVELING_MASK_BIT |
			  PBS_RX_MASK_BIT |
			  PBS_TX_MASK_BIT |
			  SET_TARGET_FREQ_MASK_BIT |
			  WRITE_LEVELING_TF_MASK_BIT |
			  WRITE_LEVELING_SUPP_TF_MASK_BIT |
			  READ_LEVELING_TF_MASK_BIT |
			  CENTRALIZATION_RX_MASK_BIT |
			  CENTRALIZATION_TX_MASK_BIT);
	rl_mid_freq_wa = 1;

	if ((ddr_freq == DDR_FREQ_333) || (ddr_freq == DDR_FREQ_400)) {
		mask_tune_func = (WRITE_LEVELING_MASK_BIT |
				  LOAD_PATTERN_2_MASK_BIT |
				  WRITE_LEVELING_SUPP_MASK_BIT |
				  READ_LEVELING_MASK_BIT |
				  PBS_RX_MASK_BIT |
				  PBS_TX_MASK_BIT |
				  CENTRALIZATION_RX_MASK_BIT |
				  CENTRALIZATION_TX_MASK_BIT);
		rl_mid_freq_wa = 0; /* WA not needed if 333/400 is TF */
	}

	/* Supplementary not supported for ECC modes */
	if (1 == ddr3_if_ecc_enabled()) {
		mask_tune_func &= ~WRITE_LEVELING_SUPP_TF_MASK_BIT;
		mask_tune_func &= ~WRITE_LEVELING_SUPP_MASK_BIT;
		mask_tune_func &= ~PBS_TX_MASK_BIT;
		mask_tune_func &= ~PBS_RX_MASK_BIT;
	}

	if (ck_delay == -1)
		ck_delay = 160;
	if (ck_delay_16 == -1)
		ck_delay_16 = 160;
	ca_delay = 0;
	delay_enable = 1;

	calibration_update_control = 1;

	init_freq = tm->interface_params[first_active_if].memory_freq;

	ddr3_tip_a38x_get_medium_freq(dev_num, &medium_freq);

	return MV_OK;
}

int ddr3_a38x_update_topology_map(u32 dev_num, struct hws_topology_map *tm)
{
	u32 if_id = 0;
	enum hws_ddr_freq freq;

	ddr3_tip_a38x_get_init_freq(dev_num, &freq);
	tm->interface_params[if_id].memory_freq = freq;

	/*
	 * re-calc topology parameters according to topology updates
	 * (if needed)
	 */
	CHECK_STATUS(hws_ddr3_tip_load_topology_map(dev_num, tm));

	return MV_OK;
}

int ddr3_tip_init_a38x(u32 dev_num, u32 board_id)
{
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (NULL == tm)
		return MV_FAIL;

	ddr3_a38x_update_topology_map(dev_num, tm);
	ddr3_tip_init_a38x_silicon(dev_num, board_id);

	return MV_OK;
}

int ddr3_tip_a38x_get_init_freq(int dev_num, enum hws_ddr_freq *freq)
{
	u32 reg;

	/* Read sample at reset setting */
	reg = (reg_read(REG_DEVICE_SAR1_ADDR) >>
	       RST2_CPU_DDR_CLOCK_SELECT_IN_OFFSET) &
		RST2_CPU_DDR_CLOCK_SELECT_IN_MASK;
	switch (reg) {
	case 0x0:
	case 0x1:
		*freq = DDR_FREQ_333;
		break;
	case 0x2:
	case 0x3:
		*freq = DDR_FREQ_400;
		break;
	case 0x4:
	case 0xd:
		*freq = DDR_FREQ_533;
		break;
	case 0x6:
		*freq = DDR_FREQ_600;
		break;
	case 0x8:
	case 0x11:
	case 0x14:
		*freq = DDR_FREQ_667;
		break;
	case 0xc:
	case 0x15:
	case 0x1b:
		*freq = DDR_FREQ_800;
		break;
	case 0x10:
		*freq = DDR_FREQ_933;
		break;
	case 0x12:
		*freq = DDR_FREQ_900;
		break;
	case 0x13:
		*freq = DDR_FREQ_900;
		break;
	default:
		*freq = 0;
		return MV_NOT_SUPPORTED;
	}

	return MV_OK;
}

int ddr3_tip_a38x_get_medium_freq(int dev_num, enum hws_ddr_freq *freq)
{
	u32 reg;

	/* Read sample at reset setting */
	reg = (reg_read(REG_DEVICE_SAR1_ADDR) >>
	       RST2_CPU_DDR_CLOCK_SELECT_IN_OFFSET) &
		RST2_CPU_DDR_CLOCK_SELECT_IN_MASK;
	switch (reg) {
	case 0x0:
	case 0x1:
		/* Medium is same as TF to run PBS in this freq */
		*freq = DDR_FREQ_333;
		break;
	case 0x2:
	case 0x3:
		/* Medium is same as TF to run PBS in this freq */
		*freq = DDR_FREQ_400;
		break;
	case 0x4:
	case 0xd:
		*freq = DDR_FREQ_533;
		break;
	case 0x8:
	case 0x11:
	case 0x14:
		*freq = DDR_FREQ_333;
		break;
	case 0xc:
	case 0x15:
	case 0x1b:
		*freq = DDR_FREQ_400;
		break;
	case 0x6:
		*freq = DDR_FREQ_300;
		break;
	case 0x12:
		*freq = DDR_FREQ_360;
		break;
	case 0x13:
		*freq = DDR_FREQ_400;
		break;
	default:
		*freq = 0;
		return MV_NOT_SUPPORTED;
	}

	return MV_OK;
}

u32 ddr3_tip_get_init_freq(void)
{
	enum hws_ddr_freq freq;

	ddr3_tip_a38x_get_init_freq(0, &freq);

	return freq;
}

static int ddr3_tip_a38x_set_divider(u8 dev_num, u32 if_id,
				     enum hws_ddr_freq frequency)
{
	u32 divider = 0;
	u32 sar_val;

	if (if_id != 0) {
		DEBUG_TRAINING_ACCESS(DEBUG_LEVEL_ERROR,
				      ("A38x does not support interface 0x%x\n",
				       if_id));
		return MV_BAD_PARAM;
	}

	/* get VCO freq index */
	sar_val = (reg_read(REG_DEVICE_SAR1_ADDR) >>
		   RST2_CPU_DDR_CLOCK_SELECT_IN_OFFSET) &
		RST2_CPU_DDR_CLOCK_SELECT_IN_MASK;
	divider = a38x_vco_freq_per_sar[sar_val] / freq_val[frequency];

	/* Set Sync mode */
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x20220, 0x0,
		      0x1000));
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0xe42f4, 0x0,
		      0x200));

	/* cpupll_clkdiv_reset_mask */
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0xe4264, 0x1f,
		      0xff));

	/* cpupll_clkdiv_reload_smooth */
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0xe4260,
		      (0x2 << 8), (0xff << 8)));

	/* cpupll_clkdiv_relax_en */
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0xe4260,
		      (0x2 << 24), (0xff << 24)));

	/* write the divider */
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0xe4268,
		      (divider << 8), (0x3f << 8)));

	/* set cpupll_clkdiv_reload_ratio */
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0xe4264,
		      (1 << 8), (1 << 8)));

	/* undet cpupll_clkdiv_reload_ratio */
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0xe4264, 0,
		      (1 << 8)));

	/* clear cpupll_clkdiv_reload_force */
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0xe4260, 0,
		      (0xff << 8)));

	/* clear cpupll_clkdiv_relax_en */
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0xe4260, 0,
		      (0xff << 24)));

	/* clear cpupll_clkdiv_reset_mask */
	CHECK_STATUS(ddr3_tip_a38x_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0xe4264, 0,
		      0xff));

	/* Dunit training clock + 1:1 mode */
	if ((frequency == DDR_FREQ_LOW_FREQ) || (freq_val[frequency] <= 400)) {
		CHECK_STATUS(ddr3_tip_a38x_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x18488,
			      (1 << 16), (1 << 16)));
		CHECK_STATUS(ddr3_tip_a38x_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1524,
			      (0 << 15), (1 << 15)));
	} else {
		CHECK_STATUS(ddr3_tip_a38x_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x18488,
			      0, (1 << 16)));
		CHECK_STATUS(ddr3_tip_a38x_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1524,
			      (1 << 15), (1 << 15)));
	}

	return MV_OK;
}

/*
 * external read from memory
 */
int ddr3_tip_ext_read(u32 dev_num, u32 if_id, u32 reg_addr,
		      u32 num_of_bursts, u32 *data)
{
	u32 burst_num;

	for (burst_num = 0; burst_num < num_of_bursts * 8; burst_num++)
		data[burst_num] = readl(reg_addr + 4 * burst_num);

	return MV_OK;
}

/*
 * external write to memory
 */
int ddr3_tip_ext_write(u32 dev_num, u32 if_id, u32 reg_addr,
		       u32 num_of_bursts, u32 *data) {
	u32 burst_num;

	for (burst_num = 0; burst_num < num_of_bursts * 8; burst_num++)
		writel(data[burst_num], reg_addr + 4 * burst_num);

	return MV_OK;
}

int ddr3_silicon_pre_init(void)
{
	return ddr3_silicon_init();
}

int ddr3_post_run_alg(void)
{
	return MV_OK;
}

int ddr3_silicon_post_init(void)
{
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/* Set half bus width */
	if (DDR3_IS_16BIT_DRAM_MODE(tm->bus_act_mask)) {
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      REG_SDRAM_CONFIG_ADDR, 0x0, 0x8000));
	}

	return MV_OK;
}

int ddr3_tip_a38x_get_device_info(u8 dev_num, struct ddr3_device_info *info_ptr)
{
	info_ptr->device_id = 0x6800;
	info_ptr->ck_delay = ck_delay;

	return MV_OK;
}
