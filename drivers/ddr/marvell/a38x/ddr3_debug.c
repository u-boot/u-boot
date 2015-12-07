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

u8 is_reg_dump = 0;
u8 debug_pbs = DEBUG_LEVEL_ERROR;

/*
 * API to change flags outside of the lib
 */
#ifndef SILENT_LIB
/* Debug flags for other Training modules */
u8 debug_training_static = DEBUG_LEVEL_ERROR;
u8 debug_training = DEBUG_LEVEL_ERROR;
u8 debug_leveling = DEBUG_LEVEL_ERROR;
u8 debug_centralization = DEBUG_LEVEL_ERROR;
u8 debug_training_ip = DEBUG_LEVEL_ERROR;
u8 debug_training_bist = DEBUG_LEVEL_ERROR;
u8 debug_training_hw_alg = DEBUG_LEVEL_ERROR;
u8 debug_training_access = DEBUG_LEVEL_ERROR;
u8 debug_training_a38x = DEBUG_LEVEL_ERROR;

void ddr3_hws_set_log_level(enum ddr_lib_debug_block block, u8 level)
{
	switch (block) {
	case DEBUG_BLOCK_STATIC:
		debug_training_static = level;
		break;
	case DEBUG_BLOCK_TRAINING_MAIN:
		debug_training = level;
		break;
	case DEBUG_BLOCK_LEVELING:
		debug_leveling = level;
		break;
	case DEBUG_BLOCK_CENTRALIZATION:
		debug_centralization = level;
		break;
	case DEBUG_BLOCK_PBS:
		debug_pbs = level;
		break;
	case DEBUG_BLOCK_ALG:
		debug_training_hw_alg = level;
		break;
	case DEBUG_BLOCK_DEVICE:
		debug_training_a38x = level;
		break;
	case DEBUG_BLOCK_ACCESS:
		debug_training_access = level;
		break;
	case DEBUG_STAGES_REG_DUMP:
		if (level == DEBUG_LEVEL_TRACE)
			is_reg_dump = 1;
		else
			is_reg_dump = 0;
		break;
	case DEBUG_BLOCK_ALL:
	default:
		debug_training_static = level;
		debug_training = level;
		debug_leveling = level;
		debug_centralization = level;
		debug_pbs = level;
		debug_training_hw_alg = level;
		debug_training_access = level;
		debug_training_a38x = level;
	}
}
#else
void ddr3_hws_set_log_level(enum ddr_lib_debug_block block, u8 level)
{
	return;
}
#endif

struct hws_tip_config_func_db config_func_info[HWS_MAX_DEVICE_NUM];
u8 is_default_centralization = 0;
u8 is_tune_result = 0;
u8 is_validate_window_per_if = 0;
u8 is_validate_window_per_pup = 0;
u8 sweep_cnt = 1;
u32 is_bist_reset_bit = 1;
static struct hws_xsb_info xsb_info[HWS_MAX_DEVICE_NUM];

/*
 * Dump Dunit & Phy registers
 */
int ddr3_tip_reg_dump(u32 dev_num)
{
	u32 if_id, reg_addr, data_value, bus_id;
	u32 read_data[MAX_INTERFACE_NUM];
	struct hws_topology_map *tm = ddr3_get_topology_map();

	printf("-- dunit registers --\n");
	for (reg_addr = 0x1400; reg_addr < 0x19f0; reg_addr += 4) {
		printf("0x%x ", reg_addr);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			CHECK_STATUS(ddr3_tip_if_read
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, reg_addr, read_data,
				      MASK_ALL_BITS));
			printf("0x%x ", read_data[if_id]);
		}
		printf("\n");
	}

	printf("-- Phy registers --\n");
	for (reg_addr = 0; reg_addr <= 0xff; reg_addr++) {
		printf("0x%x ", reg_addr);
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			for (bus_id = 0;
			     bus_id < tm->num_of_bus_per_interface;
			     bus_id++) {
				VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, bus_id,
					      DDR_PHY_DATA, reg_addr,
					      &data_value));
				printf("0x%x ", data_value);
			}
			for (bus_id = 0;
			     bus_id < tm->num_of_bus_per_interface;
			     bus_id++) {
				VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, bus_id,
					      DDR_PHY_CONTROL, reg_addr,
					      &data_value));
				printf("0x%x ", data_value);
			}
		}
		printf("\n");
	}

	return MV_OK;
}

/*
 * Register access func registration
 */
int ddr3_tip_init_config_func(u32 dev_num,
			      struct hws_tip_config_func_db *config_func)
{
	if (config_func == NULL)
		return MV_BAD_PARAM;

	memcpy(&config_func_info[dev_num], config_func,
	       sizeof(struct hws_tip_config_func_db));

	return MV_OK;
}

/*
 * Read training result table
 */
int hws_ddr3_tip_read_training_result(
	u32 dev_num, enum hws_result result[MAX_STAGE_LIMIT][MAX_INTERFACE_NUM])
{
	dev_num = dev_num;

	if (result == NULL)
		return MV_BAD_PARAM;
	memcpy(result, training_result, sizeof(result));

	return MV_OK;
}

/*
 * Get training result info pointer
 */
enum hws_result *ddr3_tip_get_result_ptr(u32 stage)
{
	return training_result[stage];
}

/*
 * Device info read
 */
int ddr3_tip_get_device_info(u32 dev_num, struct ddr3_device_info *info_ptr)
{
	if (config_func_info[dev_num].tip_get_device_info_func != NULL) {
		return config_func_info[dev_num].
			tip_get_device_info_func((u8) dev_num, info_ptr);
	}

	return MV_FAIL;
}

#ifndef EXCLUDE_SWITCH_DEBUG
/*
 * Convert freq to character string
 */
static char *convert_freq(enum hws_ddr_freq freq)
{
	switch (freq) {
	case DDR_FREQ_LOW_FREQ:
		return "DDR_FREQ_LOW_FREQ";
	case DDR_FREQ_400:
		return "400";

	case DDR_FREQ_533:
		return "533";
	case DDR_FREQ_667:
		return "667";

	case DDR_FREQ_800:
		return "800";

	case DDR_FREQ_933:
		return "933";

	case DDR_FREQ_1066:
		return "1066";
	case DDR_FREQ_311:
		return "311";

	case DDR_FREQ_333:
		return "333";

	case DDR_FREQ_467:
		return "467";

	case DDR_FREQ_850:
		return "850";

	case DDR_FREQ_900:
		return "900";

	case DDR_FREQ_360:
		return "DDR_FREQ_360";

	case DDR_FREQ_1000:
		return "DDR_FREQ_1000";
	default:
		return "Unknown Frequency";
	}
}

/*
 * Convert device ID to character string
 */
static char *convert_dev_id(u32 dev_id)
{
	switch (dev_id) {
	case 0x6800:
		return "A38xx";
	case 0x6900:
		return "A39XX";
	case 0xf400:
		return "AC3";
	case 0xfc00:
		return "BC2";

	default:
		return "Unknown Device";
	}
}

/*
 * Convert device ID to character string
 */
static char *convert_mem_size(u32 dev_id)
{
	switch (dev_id) {
	case 0:
		return "512 MB";
	case 1:
		return "1 GB";
	case 2:
		return "2 GB";
	case 3:
		return "4 GB";
	case 4:
		return "8 GB";

	default:
		return "wrong mem size";
	}
}

int print_device_info(u8 dev_num)
{
	struct ddr3_device_info info_ptr;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	CHECK_STATUS(ddr3_tip_get_device_info(dev_num, &info_ptr));
	printf("=== DDR setup START===\n");
	printf("\tDevice ID: %s\n", convert_dev_id(info_ptr.device_id));
	printf("\tDDR3  CK delay: %d\n", info_ptr.ck_delay);
	print_topology(tm);
	printf("=== DDR setup END===\n");

	return MV_OK;
}

void hws_ddr3_tip_sweep_test(int enable)
{
	if (enable) {
		is_validate_window_per_if = 1;
		is_validate_window_per_pup = 1;
		debug_training = DEBUG_LEVEL_TRACE;
	} else {
		is_validate_window_per_if = 0;
		is_validate_window_per_pup = 0;
	}
}
#endif

char *ddr3_tip_convert_tune_result(enum hws_result tune_result)
{
	switch (tune_result) {
	case TEST_FAILED:
		return "FAILED";
	case TEST_SUCCESS:
		return "PASS";
	case NO_TEST_DONE:
		return "NOT COMPLETED";
	default:
		return "Un-KNOWN";
	}
}

/*
 * Print log info
 */
int ddr3_tip_print_log(u32 dev_num, u32 mem_addr)
{
	u32 if_id = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	mem_addr = mem_addr;

#ifndef EXCLUDE_SWITCH_DEBUG
	if ((is_validate_window_per_if != 0) ||
	    (is_validate_window_per_pup != 0)) {
		u32 is_pup_log = 0;
		enum hws_ddr_freq freq;

		freq = tm->interface_params[first_active_if].memory_freq;

		is_pup_log = (is_validate_window_per_pup != 0) ? 1 : 0;
		printf("===VALIDATE WINDOW LOG START===\n");
		printf("DDR Frequency: %s   ======\n", convert_freq(freq));
		/* print sweep windows */
		ddr3_tip_run_sweep_test(dev_num, sweep_cnt, 1, is_pup_log);
		ddr3_tip_run_sweep_test(dev_num, sweep_cnt, 0, is_pup_log);
		ddr3_tip_print_all_pbs_result(dev_num);
		ddr3_tip_print_wl_supp_result(dev_num);
		printf("===VALIDATE WINDOW LOG END ===\n");
		CHECK_STATUS(ddr3_tip_restore_dunit_regs(dev_num));
		ddr3_tip_reg_dump(dev_num);
	}
#endif

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);

		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
				  ("IF %d Status:\n", if_id));

		if (mask_tune_func & INIT_CONTROLLER_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tInit Controller: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[INIT_CONTROLLER]
					    [if_id])));
		}
		if (mask_tune_func & SET_LOW_FREQ_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tLow freq Config: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[SET_LOW_FREQ]
					    [if_id])));
		}
		if (mask_tune_func & LOAD_PATTERN_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tLoad Pattern: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[LOAD_PATTERN]
					    [if_id])));
		}
		if (mask_tune_func & SET_MEDIUM_FREQ_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tMedium freq Config: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[SET_MEDIUM_FREQ]
					    [if_id])));
		}
		if (mask_tune_func & WRITE_LEVELING_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tWL: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[WRITE_LEVELING]
					    [if_id])));
		}
		if (mask_tune_func & LOAD_PATTERN_2_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tLoad Pattern: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[LOAD_PATTERN_2]
					    [if_id])));
		}
		if (mask_tune_func & READ_LEVELING_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tRL: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[READ_LEVELING]
					    [if_id])));
		}
		if (mask_tune_func & WRITE_LEVELING_SUPP_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tWL Supp: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[WRITE_LEVELING_SUPP]
					    [if_id])));
		}
		if (mask_tune_func & PBS_RX_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tPBS RX: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[PBS_RX]
					    [if_id])));
		}
		if (mask_tune_func & PBS_TX_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tPBS TX: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[PBS_TX]
					    [if_id])));
		}
		if (mask_tune_func & SET_TARGET_FREQ_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tTarget freq Config: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[SET_TARGET_FREQ]
					    [if_id])));
		}
		if (mask_tune_func & WRITE_LEVELING_TF_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tWL TF: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[WRITE_LEVELING_TF]
					    [if_id])));
		}
		if (mask_tune_func & READ_LEVELING_TF_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tRL TF: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[READ_LEVELING_TF]
					    [if_id])));
		}
		if (mask_tune_func & WRITE_LEVELING_SUPP_TF_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tWL TF Supp: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result
					    [WRITE_LEVELING_SUPP_TF]
					    [if_id])));
		}
		if (mask_tune_func & CENTRALIZATION_RX_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tCentr RX: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[CENTRALIZATION_RX]
					    [if_id])));
		}
		if (mask_tune_func & VREF_CALIBRATION_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tVREF_CALIBRATION: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[VREF_CALIBRATION]
					    [if_id])));
		}
		if (mask_tune_func & CENTRALIZATION_TX_MASK_BIT) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
					  ("\tCentr TX: %s\n",
					   ddr3_tip_convert_tune_result
					   (training_result[CENTRALIZATION_TX]
					    [if_id])));
		}
	}

	return MV_OK;
}

/*
 * Print stability log info
 */
int ddr3_tip_print_stability_log(u32 dev_num)
{
	u8 if_id = 0, csindex = 0, bus_id = 0, idx = 0;
	u32 reg_data;
	u32 read_data[MAX_INTERFACE_NUM];
	u32 max_cs = hws_ddr3_tip_max_cs_get();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/* Title print */
	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		printf("Title: I/F# , Tj, Calibration_n0, Calibration_p0, Calibration_n1, Calibration_p1, Calibration_n2, Calibration_p2,");
		for (csindex = 0; csindex < max_cs; csindex++) {
			printf("CS%d , ", csindex);
			printf("\n");
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
			printf("VWTx, VWRx, WL_tot, WL_ADLL, WL_PH, RL_Tot, RL_ADLL, RL_PH, RL_Smp, Cen_tx, Cen_rx, Vref, DQVref,");
			printf("\t\t");
			for (idx = 0; idx < 11; idx++)
				printf("PBSTx-Pad%d,", idx);
			printf("\t\t");
			for (idx = 0; idx < 11; idx++)
				printf("PBSRx-Pad%d,", idx);
		}
	}
	printf("\n");

	/* Data print */
	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);

		printf("Data: %d,%d,", if_id,
		       (config_func_info[dev_num].tip_get_temperature != NULL)
		       ? (config_func_info[dev_num].
			  tip_get_temperature(dev_num)) : (0));

		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x14c8,
			      read_data, MASK_ALL_BITS));
		printf("%d,%d,", ((read_data[if_id] & 0x3f0) >> 4),
		       ((read_data[if_id] & 0xfc00) >> 10));
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x17c8,
			      read_data, MASK_ALL_BITS));
		printf("%d,%d,", ((read_data[if_id] & 0x3f0) >> 4),
		       ((read_data[if_id] & 0xfc00) >> 10));
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1dc8,
			      read_data, MASK_ALL_BITS));
		printf("%d,%d,", ((read_data[if_id] & 0x3f0000) >> 16),
		       ((read_data[if_id] & 0xfc00000) >> 22));

		for (csindex = 0; csindex < max_cs; csindex++) {
			printf("CS%d , ", csindex);
			for (bus_id = 0; bus_id < MAX_BUS_NUM; bus_id++) {
				printf("\n");
				VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST,
						  bus_id, DDR_PHY_DATA,
						  RESULT_DB_PHY_REG_ADDR +
						  csindex, &reg_data);
				printf("%d,%d,", (reg_data & 0x1f),
				       ((reg_data & 0x3e0) >> 5));
				/* WL */
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST,
						  bus_id, DDR_PHY_DATA,
						  WL_PHY_REG +
						  csindex * 4, &reg_data);
				printf("%d,%d,%d,",
				       (reg_data & 0x1f) +
				       ((reg_data & 0x1c0) >> 6) * 32,
				       (reg_data & 0x1f),
				       (reg_data & 0x1c0) >> 6);
				/* RL */
				CHECK_STATUS(ddr3_tip_if_read
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id,
					      READ_DATA_SAMPLE_DELAY,
					      read_data, MASK_ALL_BITS));
				read_data[if_id] =
					(read_data[if_id] &
					 (0xf << (4 * csindex))) >>
					(4 * csindex);
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_id,
						  DDR_PHY_DATA,
						  RL_PHY_REG + csindex * 4,
						  &reg_data);
				printf("%d,%d,%d,%d,",
				       (reg_data & 0x1f) +
				       ((reg_data & 0x1c0) >> 6) * 32 +
				       read_data[if_id] * 64,
				       (reg_data & 0x1f),
				       ((reg_data & 0x1c0) >> 6),
				       read_data[if_id]);
				/* Centralization */
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_id,
						  DDR_PHY_DATA,
						  WRITE_CENTRALIZATION_PHY_REG
						  + csindex * 4, &reg_data);
				printf("%d,", (reg_data & 0x3f));
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_id,
						  DDR_PHY_DATA,
						  READ_CENTRALIZATION_PHY_REG
						  + csindex * 4, &reg_data);
				printf("%d,", (reg_data & 0x1f));
				/* Vref */
				ddr3_tip_bus_read(dev_num, if_id,
						  ACCESS_TYPE_UNICAST, bus_id,
						  DDR_PHY_DATA,
						  PAD_CONFIG_PHY_REG,
						  &reg_data);
				printf("%d,", (reg_data & 0x7));
				/* DQVref */
				/* Need to add the Read Function from device */
				printf("%d,", 0);
				printf("\t\t");
				for (idx = 0; idx < 11; idx++) {
					ddr3_tip_bus_read(dev_num, if_id,
							  ACCESS_TYPE_UNICAST,
							  bus_id, DDR_PHY_DATA,
							  0xd0 +
							  12 * csindex +
							  idx, &reg_data);
					printf("%d,", (reg_data & 0x3f));
				}
				printf("\t\t");
				for (idx = 0; idx < 11; idx++) {
					ddr3_tip_bus_read(dev_num, if_id,
							  ACCESS_TYPE_UNICAST,
							  bus_id, DDR_PHY_DATA,
							  0x10 +
							  16 * csindex +
							  idx, &reg_data);
					printf("%d,", (reg_data & 0x3f));
				}
				printf("\t\t");
				for (idx = 0; idx < 11; idx++) {
					ddr3_tip_bus_read(dev_num, if_id,
							  ACCESS_TYPE_UNICAST,
							  bus_id, DDR_PHY_DATA,
							  0x50 +
							  16 * csindex +
							  idx, &reg_data);
					printf("%d,", (reg_data & 0x3f));
				}
			}
		}
	}
	printf("\n");

	return MV_OK;
}

/*
 * Register XSB information
 */
int ddr3_tip_register_xsb_info(u32 dev_num, struct hws_xsb_info *xsb_info_table)
{
	memcpy(&xsb_info[dev_num], xsb_info_table, sizeof(struct hws_xsb_info));
	return MV_OK;
}

/*
 * Read ADLL Value
 */
int read_adll_value(u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
		    int reg_addr, u32 mask)
{
	u32 data_value;
	u32 if_id = 0, bus_id = 0;
	u32 dev_num = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/*
	 * multi CS support - reg_addr is calucalated in calling function
	 * with CS offset
	 */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_id = 0; bus_id < tm->num_of_bus_per_interface;
		     bus_id++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
			CHECK_STATUS(ddr3_tip_bus_read(dev_num, if_id,
						       ACCESS_TYPE_UNICAST,
						       bus_id,
						       DDR_PHY_DATA, reg_addr,
						       &data_value));
			pup_values[if_id *
				   tm->num_of_bus_per_interface + bus_id] =
				data_value & mask;
		}
	}

	return 0;
}

/*
 * Write ADLL Value
 */
int write_adll_value(u32 pup_values[MAX_INTERFACE_NUM * MAX_BUS_NUM],
		     int reg_addr)
{
	u32 if_id = 0, bus_id = 0;
	u32 dev_num = 0, data;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/*
	 * multi CS support - reg_addr is calucalated in calling function
	 * with CS offset
	 */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_id = 0; bus_id < tm->num_of_bus_per_interface;
		     bus_id++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
			data = pup_values[if_id *
					  tm->num_of_bus_per_interface +
					  bus_id];
			CHECK_STATUS(ddr3_tip_bus_write(dev_num,
							ACCESS_TYPE_UNICAST,
							if_id,
							ACCESS_TYPE_UNICAST,
							bus_id, DDR_PHY_DATA,
							reg_addr, data));
		}
	}

	return 0;
}

#ifndef EXCLUDE_SWITCH_DEBUG
u32 rl_version = 1;		/* 0 - old RL machine */
struct hws_tip_config_func_db config_func_info[HWS_MAX_DEVICE_NUM];
u32 start_xsb_offset = 0;
u8 is_rl_old = 0;
u8 is_freq_old = 0;
u8 is_dfs_disabled = 0;
u32 default_centrlization_value = 0x12;
u32 vref = 0x4;
u32 activate_select_before_run_alg = 1, activate_deselect_after_run_alg = 1,
	rl_test = 0, reset_read_fifo = 0;
int debug_acc = 0;
u32 ctrl_sweepres[ADLL_LENGTH][MAX_INTERFACE_NUM][MAX_BUS_NUM];
u32 ctrl_adll[MAX_CS_NUM * MAX_INTERFACE_NUM * MAX_BUS_NUM];
u8 cs_mask_reg[] = {
	0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

u32 xsb_test_table[][8] = {
	{0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555,
	 0x66666666, 0x77777777},
	{0x88888888, 0x99999999, 0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc, 0xdddddddd,
	 0xeeeeeeee, 0xffffffff},
	{0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xffffffff,
	 0x00000000, 0xffffffff},
	{0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xffffffff,
	 0x00000000, 0xffffffff},
	{0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xffffffff,
	 0x00000000, 0xffffffff},
	{0x00000000, 0xffffffff, 0x00000000, 0xffffffff, 0x00000000, 0xffffffff,
	 0x00000000, 0xffffffff},
	{0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000,
	 0xffffffff, 0xffffffff},
	{0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000,
	 0x00000000, 0x00000000},
	{0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff,
	 0xffffffff, 0xffffffff}
};

static int ddr3_tip_access_atr(u32 dev_num, u32 flag_id, u32 value, u32 **ptr);

int ddr3_tip_print_adll(void)
{
	u32 bus_cnt = 0, if_id, data_p1, data_p2, ui_data3, dev_num = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_cnt = 0; bus_cnt < GET_TOPOLOGY_NUM_OF_BUSES();
		     bus_cnt++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_cnt);
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id,
				      ACCESS_TYPE_UNICAST, bus_cnt,
				      DDR_PHY_DATA, 0x1, &data_p1));
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id, ACCESS_TYPE_UNICAST,
				      bus_cnt, DDR_PHY_DATA, 0x2, &data_p2));
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id, ACCESS_TYPE_UNICAST,
				      bus_cnt, DDR_PHY_DATA, 0x3, &ui_data3));
			DEBUG_TRAINING_IP(DEBUG_LEVEL_TRACE,
					  (" IF %d bus_cnt %d  phy_reg_1_data 0x%x phy_reg_2_data 0x%x phy_reg_3_data 0x%x\n",
					   if_id, bus_cnt, data_p1, data_p2,
					   ui_data3));
			}
	}

	return MV_OK;
}

/*
 * Set attribute value
 */
int ddr3_tip_set_atr(u32 dev_num, u32 flag_id, u32 value)
{
	int ret;
	u32 *ptr_flag = NULL;

	ret = ddr3_tip_access_atr(dev_num, flag_id, value, &ptr_flag);
	if (ptr_flag != NULL) {
		printf("ddr3_tip_set_atr Flag ID 0x%x value is set to 0x%x (was 0x%x)\n",
		       flag_id, value, *ptr_flag);
		*ptr_flag = value;
	} else {
		printf("ddr3_tip_set_atr Flag ID 0x%x value is set to 0x%x\n",
		       flag_id, value);
	}

	return ret;
}

/*
 * Access attribute
 */
static int ddr3_tip_access_atr(u32 dev_num, u32 flag_id, u32 value, u32 **ptr)
{
	u32 tmp_val = 0, if_id = 0, pup_id = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	dev_num = dev_num;
	*ptr = NULL;

	switch (flag_id) {
	case 0:
		*ptr = (u32 *)&(tm->if_act_mask);
		break;

	case 0x1:
		*ptr = (u32 *)&mask_tune_func;
		break;

	case 0x2:
		*ptr = (u32 *)&low_freq;
		break;

	case 0x3:
		*ptr = (u32 *)&medium_freq;
		break;

	case 0x4:
		*ptr = (u32 *)&generic_init_controller;
		break;

	case 0x5:
		*ptr = (u32 *)&rl_version;
		break;

	case 0x8:
		*ptr = (u32 *)&start_xsb_offset;
		break;

	case 0x20:
		*ptr = (u32 *)&is_rl_old;
		break;

	case 0x21:
		*ptr = (u32 *)&is_freq_old;
		break;

	case 0x23:
		*ptr = (u32 *)&is_dfs_disabled;
		break;

	case 0x24:
		*ptr = (u32 *)&is_pll_before_init;
		break;

	case 0x25:
		*ptr = (u32 *)&is_adll_calib_before_init;
		break;
#ifdef STATIC_ALGO_SUPPORT
	case 0x26:
		*ptr = (u32 *)&(silicon_delay[0]);
		break;

	case 0x27:
		*ptr = (u32 *)&wl_debug_delay;
		break;
#endif
	case 0x28:
		*ptr = (u32 *)&is_tune_result;
		break;

	case 0x29:
		*ptr = (u32 *)&is_validate_window_per_if;
		break;

	case 0x2a:
		*ptr = (u32 *)&is_validate_window_per_pup;
		break;

	case 0x30:
		*ptr = (u32 *)&sweep_cnt;
		break;

	case 0x31:
		*ptr = (u32 *)&is_bist_reset_bit;
		break;

	case 0x32:
		*ptr = (u32 *)&is_dfs_in_init;
		break;

	case 0x33:
		*ptr = (u32 *)&p_finger;
		break;

	case 0x34:
		*ptr = (u32 *)&n_finger;
		break;

	case 0x35:
		*ptr = (u32 *)&init_freq;
		break;

	case 0x36:
		*ptr = (u32 *)&(freq_val[DDR_FREQ_LOW_FREQ]);
		break;

	case 0x37:
		*ptr = (u32 *)&start_pattern;
		break;

	case 0x38:
		*ptr = (u32 *)&end_pattern;
		break;

	case 0x39:
		*ptr = (u32 *)&phy_reg0_val;
		break;

	case 0x4a:
		*ptr = (u32 *)&phy_reg1_val;
		break;

	case 0x4b:
		*ptr = (u32 *)&phy_reg2_val;
		break;

	case 0x4c:
		*ptr = (u32 *)&phy_reg3_val;
		break;

	case 0x4e:
		*ptr = (u32 *)&sweep_pattern;
		break;

	case 0x50:
		*ptr = (u32 *)&is_rzq6;
		break;

	case 0x51:
		*ptr = (u32 *)&znri_data_phy_val;
		break;

	case 0x52:
		*ptr = (u32 *)&zpri_data_phy_val;
		break;

	case 0x53:
		*ptr = (u32 *)&finger_test;
		break;

	case 0x54:
		*ptr = (u32 *)&n_finger_start;
		break;

	case 0x55:
		*ptr = (u32 *)&n_finger_end;
		break;

	case 0x56:
		*ptr = (u32 *)&p_finger_start;
		break;

	case 0x57:
		*ptr = (u32 *)&p_finger_end;
		break;

	case 0x58:
		*ptr = (u32 *)&p_finger_step;
		break;

	case 0x59:
		*ptr = (u32 *)&n_finger_step;
		break;

	case 0x5a:
		*ptr = (u32 *)&znri_ctrl_phy_val;
		break;

	case 0x5b:
		*ptr = (u32 *)&zpri_ctrl_phy_val;
		break;

	case 0x5c:
		*ptr = (u32 *)&is_reg_dump;
		break;

	case 0x5d:
		*ptr = (u32 *)&vref;
		break;

	case 0x5e:
		*ptr = (u32 *)&mode2_t;
		break;

	case 0x5f:
		*ptr = (u32 *)&xsb_validate_type;
		break;

	case 0x60:
		*ptr = (u32 *)&xsb_validation_base_address;
		break;

	case 0x67:
		*ptr = (u32 *)&activate_select_before_run_alg;
		break;

	case 0x68:
		*ptr = (u32 *)&activate_deselect_after_run_alg;
		break;

	case 0x69:
		*ptr = (u32 *)&odt_additional;
		break;

	case 0x70:
		*ptr = (u32 *)&debug_mode;
		break;

	case 0x71:
		*ptr = (u32 *)&pbs_pattern;
		break;

	case 0x72:
		*ptr = (u32 *)&delay_enable;
		break;

	case 0x73:
		*ptr = (u32 *)&ck_delay;
		break;

	case 0x74:
		*ptr = (u32 *)&ck_delay_16;
		break;

	case 0x75:
		*ptr = (u32 *)&ca_delay;
		break;

	case 0x100:
		*ptr = (u32 *)&debug_dunit;
		break;

	case 0x101:
		debug_acc = (int)value;
		break;

	case 0x102:
		debug_training = (u8)value;
		break;

	case 0x103:
		debug_training_bist = (u8)value;
		break;

	case 0x104:
		debug_centralization = (u8)value;
		break;

	case 0x105:
		debug_training_ip = (u8)value;
		break;

	case 0x106:
		debug_leveling = (u8)value;
		break;

	case 0x107:
		debug_pbs = (u8)value;
		break;

	case 0x108:
		debug_training_static = (u8)value;
		break;

	case 0x109:
		debug_training_access = (u8)value;
		break;

	case 0x112:
		*ptr = &start_pattern;
		break;

	case 0x113:
		*ptr = &end_pattern;
		break;

	default:
		if ((flag_id >= 0x200) && (flag_id < 0x210)) {
			if_id = flag_id - 0x200;
			*ptr = (u32 *)&(tm->interface_params
					[if_id].memory_freq);
		} else if ((flag_id >= 0x210) && (flag_id < 0x220)) {
			if_id = flag_id - 0x210;
			*ptr = (u32 *)&(tm->interface_params
					[if_id].speed_bin_index);
		} else if ((flag_id >= 0x220) && (flag_id < 0x230)) {
			if_id = flag_id - 0x220;
			*ptr = (u32 *)&(tm->interface_params
					[if_id].bus_width);
		} else if ((flag_id >= 0x230) && (flag_id < 0x240)) {
			if_id = flag_id - 0x230;
			*ptr = (u32 *)&(tm->interface_params
					[if_id].memory_size);
		} else if ((flag_id >= 0x240) && (flag_id < 0x250)) {
			if_id = flag_id - 0x240;
			*ptr = (u32 *)&(tm->interface_params
					[if_id].cas_l);
		} else if ((flag_id >= 0x250) && (flag_id < 0x260)) {
			if_id = flag_id - 0x250;
			*ptr = (u32 *)&(tm->interface_params
					[if_id].cas_wl);
		} else if ((flag_id >= 0x270) && (flag_id < 0x2cf)) {
			if_id = (flag_id - 0x270) / MAX_BUS_NUM;
			pup_id = (flag_id - 0x270) % MAX_BUS_NUM;
			*ptr = (u32 *)&(tm->interface_params[if_id].
					as_bus_params[pup_id].is_ck_swap);
		} else if ((flag_id >= 0x2d0) && (flag_id < 0x32f)) {
			if_id = (flag_id - 0x2d0) / MAX_BUS_NUM;
			pup_id = (flag_id - 0x2d0) % MAX_BUS_NUM;
			*ptr = (u32 *)&(tm->interface_params[if_id].
					as_bus_params[pup_id].is_dqs_swap);
		} else if ((flag_id >= 0x330) && (flag_id < 0x38f)) {
			if_id = (flag_id - 0x330) / MAX_BUS_NUM;
			pup_id = (flag_id - 0x330) % MAX_BUS_NUM;
			*ptr = (u32 *)&(tm->interface_params[if_id].
					as_bus_params[pup_id].cs_bitmask);
		} else if ((flag_id >= 0x390) && (flag_id < 0x3ef)) {
			if_id = (flag_id - 0x390) / MAX_BUS_NUM;
			pup_id = (flag_id - 0x390) % MAX_BUS_NUM;
			*ptr = (u32 *)&(tm->interface_params
					[if_id].as_bus_params
					[pup_id].mirror_enable_bitmask);
		} else if ((flag_id >= 0x500) && (flag_id <= 0x50f)) {
			tmp_val = flag_id - 0x320;
			*ptr = (u32 *)&(clamp_tbl[tmp_val]);
		} else {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("flag_id out of boundary %d\n",
					   flag_id));
			return MV_BAD_PARAM;
		}
	}

	return MV_OK;
}

#ifndef EXCLUDE_SWITCH_DEBUG
/*
 * Print ADLL
 */
int print_adll(u32 dev_num, u32 adll[MAX_INTERFACE_NUM * MAX_BUS_NUM])
{
	u32 i, j;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	dev_num = dev_num;

	for (j = 0; j < tm->num_of_bus_per_interface; j++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, j);
		for (i = 0; i < MAX_INTERFACE_NUM; i++) {
			printf("%d ,",
			       adll[i * tm->num_of_bus_per_interface + j]);
		}
	}
	printf("\n");

	return MV_OK;
}
#endif

/* byte_index - only byte 0, 1, 2, or 3, oxff - test all bytes */
static u32 ddr3_tip_compare(u32 if_id, u32 *p_src, u32 *p_dst,
			    u32 byte_index)
{
	u32 burst_cnt = 0, addr_offset, i_id;
	int b_is_fail = 0;

	addr_offset =
		(byte_index ==
		 0xff) ? (u32) 0xffffffff : (u32) (0xff << (byte_index * 8));
	for (burst_cnt = 0; burst_cnt < EXT_ACCESS_BURST_LENGTH; burst_cnt++) {
		if ((p_src[burst_cnt] & addr_offset) !=
		    (p_dst[burst_cnt] & addr_offset))
			b_is_fail = 1;
	}

	if (b_is_fail == 1) {
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("IF %d exp: ", if_id));
		for (i_id = 0; i_id <= MAX_INTERFACE_NUM - 1; i_id++) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("0x%8x ", p_src[i_id]));
		}
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
				  ("\n_i_f %d rcv: ", if_id));
		for (i_id = 0; i_id <= MAX_INTERFACE_NUM - 1; i_id++) {
			DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR,
					  ("(0x%8x ", p_dst[i_id]));
		}
		DEBUG_TRAINING_IP(DEBUG_LEVEL_ERROR, ("\n "));
	}

	return b_is_fail;
}

/* test_type = 0-tx , 1-rx */
int ddr3_tip_sweep_test(u32 dev_num, u32 test_type,
			u32 mem_addr, u32 is_modify_adll,
			u32 start_if, u32 end_if, u32 startpup, u32 endpup)
{
	u32 bus_cnt = 0, adll_val = 0, if_id, ui_prev_adll, ui_mask_bit,
		end_adll, start_adll;
	u32 reg_addr = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	mem_addr = mem_addr;

	if (test_type == 0) {
		reg_addr = 1;
		ui_mask_bit = 0x3f;
		start_adll = 0;
		end_adll = ui_mask_bit;
	} else {
		reg_addr = 3;
		ui_mask_bit = 0x1f;
		start_adll = 0;
		end_adll = ui_mask_bit;
	}

	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
			  ("==============================\n"));
	DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO,
			  ("Test type %d (0-tx, 1-rx)\n", test_type));

	for (if_id = start_if; if_id <= end_if; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_cnt = startpup; bus_cnt < endpup; bus_cnt++) {
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id, ACCESS_TYPE_UNICAST,
				      bus_cnt, DDR_PHY_DATA, reg_addr,
				      &ui_prev_adll));

			for (adll_val = start_adll; adll_val <= end_adll;
			     adll_val++) {
				if (is_modify_adll == 1) {
					CHECK_STATUS(ddr3_tip_bus_read_modify_write
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id, bus_cnt,
						      DDR_PHY_DATA, reg_addr,
						      adll_val, ui_mask_bit));
				}
			}
			if (is_modify_adll == 1) {
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      bus_cnt, DDR_PHY_DATA, reg_addr,
					      ui_prev_adll));
			}
			DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("\n"));
		}
		DEBUG_TRAINING_IP(DEBUG_LEVEL_INFO, ("\n"));
	}

	return MV_OK;
}

#ifndef EXCLUDE_SWITCH_DEBUG
/*
 * Sweep validation
 */
int ddr3_tip_run_sweep_test(int dev_num, u32 repeat_num, u32 direction,
			    u32 mode)
{
	u32 pup = 0, start_pup = 0, end_pup = 0;
	u32 adll = 0;
	u32 res[MAX_INTERFACE_NUM] = { 0 };
	int if_id = 0;
	u32 adll_value = 0;
	int reg = (direction == 0) ? WRITE_CENTRALIZATION_PHY_REG :
		READ_CENTRALIZATION_PHY_REG;
	enum hws_access_type pup_access;
	u32 cs;
	u32 max_cs = hws_ddr3_tip_max_cs_get();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	repeat_num = repeat_num;

	if (mode == 1) {
		/* per pup */
		start_pup = 0;
		end_pup = tm->num_of_bus_per_interface - 1;
		pup_access = ACCESS_TYPE_UNICAST;
	} else {
		start_pup = 0;
		end_pup = 0;
		pup_access = ACCESS_TYPE_MULTICAST;
	}

	for (cs = 0; cs < max_cs; cs++) {
		for (adll = 0; adll < ADLL_LENGTH; adll++) {
			for (if_id = 0;
			     if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_ACTIVE
					(tm->if_act_mask,
					 if_id);
				for (pup = start_pup; pup <= end_pup; pup++) {
					ctrl_sweepres[adll][if_id][pup] =
						0;
				}
			}
		}

		for (adll = 0; adll < (MAX_INTERFACE_NUM * MAX_BUS_NUM); adll++)
			ctrl_adll[adll] = 0;
		/* Save DQS value(after algorithm run) */
		read_adll_value(ctrl_adll,
				(reg + (cs * CS_REGISTER_ADDR_OFFSET)),
				MASK_ALL_BITS);

		/*
		 * Sweep ADLL  from 0:31 on all I/F on all Pup and perform
		 * BIST on each stage.
		 */
		for (pup = start_pup; pup <= end_pup; pup++) {
			for (adll = 0; adll < ADLL_LENGTH; adll++) {
				adll_value =
					(direction == 0) ? (adll * 2) : adll;
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_MULTICAST, 0,
					      pup_access, pup, DDR_PHY_DATA,
					      reg + CS_REG_VALUE(cs),
					      adll_value));
				hws_ddr3_run_bist(dev_num, sweep_pattern, res,
						  cs);
				/* ddr3_tip_reset_fifo_ptr(dev_num); */
				for (if_id = 0;
				     if_id <= MAX_INTERFACE_NUM - 1;
				     if_id++) {
					VALIDATE_ACTIVE
						(tm->if_act_mask,
						 if_id);
					ctrl_sweepres[adll][if_id][pup]
						= res[if_id];
					if (mode == 1) {
						CHECK_STATUS
							(ddr3_tip_bus_write
							 (dev_num,
							  ACCESS_TYPE_UNICAST,
							  if_id,
							  ACCESS_TYPE_UNICAST,
							  pup,
							  DDR_PHY_DATA,
							  reg + CS_REG_VALUE(cs),
							  ctrl_adll[if_id *
								    cs *
								    tm->num_of_bus_per_interface
								    + pup]));
					}
				}
			}
		}
		printf("Final, CS %d,%s, Sweep, Result, Adll,", cs,
		       ((direction == 0) ? "TX" : "RX"));
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			if (mode == 1) {
				for (pup = start_pup; pup <= end_pup; pup++) {
					VALIDATE_ACTIVE(tm->bus_act_mask, pup);
					printf("I/F%d-PHY%d , ", if_id, pup);
				}
			} else {
				printf("I/F%d , ", if_id);
			}
		}
		printf("\n");

		for (adll = 0; adll < ADLL_LENGTH; adll++) {
			adll_value = (direction == 0) ? (adll * 2) : adll;
			printf("Final,%s, Sweep, Result, %d ,",
			       ((direction == 0) ? "TX" : "RX"), adll_value);

			for (if_id = 0;
			     if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_ACTIVE(tm->if_act_mask, if_id);
				for (pup = start_pup; pup <= end_pup; pup++) {
					printf("%d , ",
					       ctrl_sweepres[adll][if_id]
					       [pup]);
				}
			}
			printf("\n");
		}

		/*
		 * Write back to the phy the Rx DQS value, we store in
		 * the beginning.
		 */
		write_adll_value(ctrl_adll,
				 (reg + cs * CS_REGISTER_ADDR_OFFSET));
		/* print adll results */
		read_adll_value(ctrl_adll, (reg + cs * CS_REGISTER_ADDR_OFFSET),
				MASK_ALL_BITS);
		printf("%s, DQS, ADLL,,,", (direction == 0) ? "Tx" : "Rx");
		print_adll(dev_num, ctrl_adll);
	}
	ddr3_tip_reset_fifo_ptr(dev_num);

	return 0;
}

void print_topology(struct hws_topology_map *topology_db)
{
	u32 ui, uj;

	printf("\tinterface_mask: 0x%x\n", topology_db->if_act_mask);
	printf("\tNum Bus:  %d\n", topology_db->num_of_bus_per_interface);
	printf("\tbus_act_mask: 0x%x\n", topology_db->bus_act_mask);

	for (ui = 0; ui < MAX_INTERFACE_NUM; ui++) {
		VALIDATE_ACTIVE(topology_db->if_act_mask, ui);
		printf("\n\tInterface ID: %d\n", ui);
		printf("\t\tDDR Frequency: %s\n",
		       convert_freq(topology_db->
				    interface_params[ui].memory_freq));
		printf("\t\tSpeed_bin: %d\n",
		       topology_db->interface_params[ui].speed_bin_index);
		printf("\t\tBus_width: %d\n",
		       (4 << topology_db->interface_params[ui].bus_width));
		printf("\t\tMem_size: %s\n",
		       convert_mem_size(topology_db->
					interface_params[ui].memory_size));
		printf("\t\tCAS-WL: %d\n",
		       topology_db->interface_params[ui].cas_wl);
		printf("\t\tCAS-L: %d\n",
		       topology_db->interface_params[ui].cas_l);
		printf("\t\tTemperature: %d\n",
		       topology_db->interface_params[ui].interface_temp);
		printf("\n");
		for (uj = 0; uj < 4; uj++) {
			printf("\t\tBus %d parameters- CS Mask: 0x%x\t", uj,
			       topology_db->interface_params[ui].
			       as_bus_params[uj].cs_bitmask);
			printf("Mirror: 0x%x\t",
			       topology_db->interface_params[ui].
			       as_bus_params[uj].mirror_enable_bitmask);
			printf("DQS Swap is %s \t",
			       (topology_db->
				interface_params[ui].as_bus_params[uj].
				is_dqs_swap == 1) ? "enabled" : "disabled");
			printf("Ck Swap:%s\t",
			       (topology_db->
				interface_params[ui].as_bus_params[uj].
				is_ck_swap == 1) ? "enabled" : "disabled");
			printf("\n");
		}
	}
}
#endif

/*
 * Execute XSB Test transaction (rd/wr/both)
 */
int run_xsb_test(u32 dev_num, u32 mem_addr, u32 write_type,
		 u32 read_type, u32 burst_length)
{
	u32 seq = 0, if_id = 0, addr, cnt;
	int ret = MV_OK, ret_tmp;
	u32 data_read[MAX_INTERFACE_NUM];
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		addr = mem_addr;
		for (cnt = 0; cnt <= burst_length; cnt++) {
			seq = (seq + 1) % 8;
			if (write_type != 0) {
				CHECK_STATUS(ddr3_tip_ext_write
					     (dev_num, if_id, addr, 1,
					      xsb_test_table[seq]));
			}
			if (read_type != 0) {
				CHECK_STATUS(ddr3_tip_ext_read
					     (dev_num, if_id, addr, 1,
					      data_read));
			}
			if ((read_type != 0) && (write_type != 0)) {
				ret_tmp =
					ddr3_tip_compare(if_id,
							 xsb_test_table[seq],
							 data_read,
							 0xff);
				addr += (EXT_ACCESS_BURST_LENGTH * 4);
				ret = (ret != MV_OK) ? ret : ret_tmp;
			}
		}
	}

	return ret;
}

#else /*EXCLUDE_SWITCH_DEBUG */

u32 rl_version = 1;		/* 0 - old RL machine */
u32 vref = 0x4;
u32 start_xsb_offset = 0;
u8 cs_mask_reg[] = {
	0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int run_xsb_test(u32 dev_num, u32 mem_addr, u32 write_type,
		 u32 read_type, u32 burst_length)
{
	return MV_OK;
}

#endif
