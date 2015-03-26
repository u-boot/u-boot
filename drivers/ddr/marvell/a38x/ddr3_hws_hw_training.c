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

#define REG_READ_DATA_SAMPLE_DELAYS_ADDR	0x1538
#define REG_READ_DATA_SAMPLE_DELAYS_MASK	0x1f
#define REG_READ_DATA_SAMPLE_DELAYS_OFFS	8

#define REG_READ_DATA_READY_DELAYS_ADDR		0x153c
#define REG_READ_DATA_READY_DELAYS_MASK		0x1f
#define REG_READ_DATA_READY_DELAYS_OFFS		8

int ddr3_if_ecc_enabled(void)
{
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (DDR3_IS_ECC_PUP4_MODE(tm->bus_act_mask) ||
	    DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask))
		return 1;
	else
		return 0;
}

int ddr3_pre_algo_config(void)
{
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/* Set Bus3 ECC training mode */
	if (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask)) {
		/* Set Bus3 ECC MUX */
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      REG_SDRAM_PINS_MUX, 0x100, 0x100));
	}

	/* Set regular ECC training mode (bus4 and bus 3) */
	if ((DDR3_IS_ECC_PUP4_MODE(tm->bus_act_mask)) ||
	    (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask))) {
		/* Enable ECC Write MUX */
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x100, 0x100));
		/* General ECC enable */
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      REG_SDRAM_CONFIG_ADDR, 0x40000, 0x40000));
		/* Disable Read Data ECC MUX */
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x0, 0x2));
	}

	return MV_OK;
}

int ddr3_post_algo_config(void)
{
	struct hws_topology_map *tm = ddr3_get_topology_map();
	int status;

	status = ddr3_post_run_alg();
	if (MV_OK != status) {
		printf("DDR3 Post Run Alg - FAILED 0x%x\n", status);
		return status;
	}

	/* Un_set ECC training mode */
	if ((DDR3_IS_ECC_PUP4_MODE(tm->bus_act_mask)) ||
	    (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask))) {
		/* Disable ECC Write MUX */
		CHECK_STATUS(ddr3_tip_if_write
			     (0, ACCESS_TYPE_UNICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x0, 0x100));
		/* General ECC and Bus3 ECC MUX remains enabled */
	}

	return MV_OK;
}

int ddr3_hws_hw_training(void)
{
	enum hws_algo_type algo_mode = ALGO_TYPE_DYNAMIC;
	int status;
	struct init_cntr_param init_param;

	status = ddr3_silicon_pre_init();
	if (MV_OK != status) {
		printf("DDR3 Pre silicon Config - FAILED 0x%x\n", status);
		return status;
	}

	init_param.do_mrs_phy = 1;
#if defined(CONFIG_ARMADA_38X)  || defined(CONFIG_ARMADA_39X)
	init_param.is_ctrl64_bit = 0;
#else
	init_param.is_ctrl64_bit = 1;
#endif
#if defined(CONFIG_ALLEYCAT3) || defined(CONFIG_ARMADA_38X) || \
	defined(CONFIG_ARMADA_39X)
	init_param.init_phy = 1;
#else
	init_param.init_phy = 0;
#endif
	init_param.msys_init = 1;
	status = hws_ddr3_tip_init_controller(0, &init_param);
	if (MV_OK != status) {
		printf("DDR3 init controller - FAILED 0x%x\n", status);
		return status;
	}

	status = ddr3_silicon_post_init();
	if (MV_OK != status) {
		printf("DDR3 Post Init - FAILED 0x%x\n", status);
		return status;
	}

	status = ddr3_pre_algo_config();
	if (MV_OK != status) {
		printf("DDR3 Pre Algo Config - FAILED 0x%x\n", status);
		return status;
	}

	/* run algorithm in order to configure the PHY */
	status = hws_ddr3_tip_run_alg(0, algo_mode);
	if (MV_OK != status) {
		printf("DDR3 run algorithm - FAILED 0x%x\n", status);
		return status;
	}

	status = ddr3_post_algo_config();
	if (MV_OK != status) {
		printf("DDR3 Post Algo Config - FAILED 0x%x\n", status);
		return status;
	}

	return MV_OK;
}
