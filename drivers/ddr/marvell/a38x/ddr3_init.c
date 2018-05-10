// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "ddr3_init.h"
#include "mv_ddr_common.h"

/*
 * Translates topology map definitions to real memory size in bits
  * (per values in ddr3_training_ip_def.h)
 */
u32 mem_size[] = {
	ADDR_SIZE_512MB,
	ADDR_SIZE_1GB,
	ADDR_SIZE_2GB,
	ADDR_SIZE_4GB,
	ADDR_SIZE_8GB
};

static char *ddr_type = "DDR3";

/*
 * generic_init_controller controls D-unit configuration:
 * '1' - dynamic D-unit configuration,
 */
u8 generic_init_controller = 1;

static int mv_ddr_training_params_set(u8 dev_num);

/*
 * Name:     ddr3_init - Main DDR3 Init function
 * Desc:     This routine initialize the DDR3 MC and runs HW training.
 * Args:     None.
 * Notes:
 * Returns:  None.
 */
int ddr3_init(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 octets_per_if_num;
	int status;
	int is_manual_cal_done;

	/* Print mv_ddr version */
	mv_ddr_ver_print();

	mv_ddr_pre_training_fixup();

	/* SoC/Board special initializations */
	mv_ddr_pre_training_soc_config(ddr_type);

	/* Set log level for training library */
	mv_ddr_user_log_level_set(DEBUG_BLOCK_ALL);

	mv_ddr_early_init();

	if (mv_ddr_topology_map_update() == NULL) {
		printf("mv_ddr: failed to update topology\n");
		return MV_FAIL;
	}

	if (mv_ddr_early_init2() != MV_OK)
		return MV_FAIL;

	/* Set training algorithm's parameters */
	status = mv_ddr_training_params_set(0);
	if (MV_OK != status)
		return status;


	mv_ddr_mc_config();

	is_manual_cal_done = mv_ddr_manual_cal_do();

	mv_ddr_mc_init();

	if (!is_manual_cal_done) {
	}


	status = ddr3_silicon_post_init();
	if (MV_OK != status) {
		printf("DDR3 Post Init - FAILED 0x%x\n", status);
		return status;
	}

	/* PHY initialization (Training) */
	status = hws_ddr3_tip_run_alg(0, ALGO_TYPE_DYNAMIC);
	if (MV_OK != status) {
		printf("%s Training Sequence - FAILED\n", ddr_type);
		return status;
	}

#if defined(CONFIG_PHY_STATIC_PRINT)
	mv_ddr_phy_static_print();
#endif

	/* Post MC/PHY initializations */
	mv_ddr_post_training_soc_config(ddr_type);

	mv_ddr_post_training_fixup();

	octets_per_if_num = ddr3_tip_dev_attr_get(0, MV_ATTR_OCTET_PER_INTERFACE);
	if (ddr3_if_ecc_enabled()) {
		if (MV_DDR_IS_64BIT_DRAM_MODE(tm->bus_act_mask) ||
		    MV_DDR_IS_32BIT_IN_64BIT_DRAM_MODE(tm->bus_act_mask, octets_per_if_num))
			mv_ddr_mem_scrubbing();
		else
			ddr3_new_tip_ecc_scrub();
	}

	printf("mv_ddr: completed successfully\n");

	return MV_OK;
}

uint64_t mv_ddr_get_memory_size_per_cs_in_bits(void)
{
	uint64_t memory_size_per_cs;

	u32 bus_cnt, num_of_active_bus = 0;
	u32 num_of_sub_phys_per_ddr_unit = 0;

	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	u32 octets_per_if_num = ddr3_tip_dev_attr_get(DEV_NUM_0, MV_ATTR_OCTET_PER_INTERFACE);

	/* count the number of active bus */
	for (bus_cnt = 0; bus_cnt < octets_per_if_num - 1/* ignore ecc octet */; bus_cnt++) {
		VALIDATE_BUS_ACTIVE(tm->bus_act_mask, bus_cnt);
		num_of_active_bus++;
	}

	/* calculate number of sub-phys per ddr unit */
	if (tm->interface_params[0].bus_width/* supports only single interface */ == MV_DDR_DEV_WIDTH_16BIT)
		num_of_sub_phys_per_ddr_unit = TWO_SUB_PHYS;
	if (tm->interface_params[0].bus_width/* supports only single interface */ == MV_DDR_DEV_WIDTH_8BIT)
		num_of_sub_phys_per_ddr_unit = SINGLE_SUB_PHY;

	/* calculate dram size per cs */
	memory_size_per_cs = (uint64_t)mem_size[tm->interface_params[0].memory_size] * (uint64_t)num_of_active_bus
		/ (uint64_t)num_of_sub_phys_per_ddr_unit * (uint64_t)MV_DDR_NUM_BITS_IN_BYTE;

	return memory_size_per_cs;
}

uint64_t mv_ddr_get_total_memory_size_in_bits(void)
{
	uint64_t total_memory_size = 0;
	uint64_t memory_size_per_cs = 0;

	/* get the number of cs */
	u32 max_cs = ddr3_tip_max_cs_get(DEV_NUM_0);

	memory_size_per_cs = mv_ddr_get_memory_size_per_cs_in_bits();
	total_memory_size = (uint64_t)max_cs * memory_size_per_cs;

	return total_memory_size;
}

int ddr3_if_ecc_enabled(void)
{
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();

	if (DDR3_IS_ECC_PUP4_MODE(tm->bus_act_mask) ||
	    DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask) ||
	    DDR3_IS_ECC_PUP8_MODE(tm->bus_act_mask))
		return 1;
	else
		return 0;
}

/*
 * Name:	mv_ddr_training_params_set
 * Desc:
 * Args:
 * Notes:	sets internal training params
 * Returns:
 */
static int mv_ddr_training_params_set(u8 dev_num)
{
	struct tune_train_params params;
	int status;
	struct mv_ddr_topology_map *tm = mv_ddr_topology_map_get();
	u32 if_id;
	u32 cs_num;

	CHECK_STATUS(ddr3_tip_get_first_active_if
		     (dev_num, tm->if_act_mask,
		      &if_id));

	CHECK_STATUS(calc_cs_num(dev_num, if_id, &cs_num));

	/* NOTE: do not remove any field initilization */
	params.ck_delay = TUNE_TRAINING_PARAMS_CK_DELAY;
	params.phy_reg3_val = TUNE_TRAINING_PARAMS_PHYREG3VAL;
	params.g_zpri_data = TUNE_TRAINING_PARAMS_PRI_DATA;
	params.g_znri_data = TUNE_TRAINING_PARAMS_NRI_DATA;
	params.g_zpri_ctrl = TUNE_TRAINING_PARAMS_PRI_CTRL;
	params.g_znri_ctrl = TUNE_TRAINING_PARAMS_NRI_CTRL;
	params.g_znodt_data = TUNE_TRAINING_PARAMS_N_ODT_DATA;
	params.g_zpodt_ctrl = TUNE_TRAINING_PARAMS_P_ODT_CTRL;
	params.g_znodt_ctrl = TUNE_TRAINING_PARAMS_N_ODT_CTRL;

	params.g_zpodt_data = TUNE_TRAINING_PARAMS_P_ODT_DATA;
	params.g_dic = TUNE_TRAINING_PARAMS_DIC;
	params.g_rtt_nom = TUNE_TRAINING_PARAMS_RTT_NOM;
	if (cs_num == 1) {
		params.g_rtt_wr = TUNE_TRAINING_PARAMS_RTT_WR_1CS;
		params.g_odt_config = TUNE_TRAINING_PARAMS_ODT_CONFIG_1CS;
	} else {
		params.g_rtt_wr = TUNE_TRAINING_PARAMS_RTT_WR_2CS;
		params.g_odt_config = TUNE_TRAINING_PARAMS_ODT_CONFIG_2CS;
	}

	status = ddr3_tip_tune_training_params(dev_num, &params);
	if (MV_OK != status) {
		printf("%s Training Sequence - FAILED\n", ddr_type);
		return status;
	}

	return MV_OK;
}
