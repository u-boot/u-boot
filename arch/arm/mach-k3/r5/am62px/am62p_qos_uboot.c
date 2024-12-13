// SPDX-License-Identifier: GPL-2.0+
/*
 * am62p Quality of Service (QoS) Configuration Data
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <asm/arch/k3-qos.h>
#include "am62p_qos.h"

struct k3_qos_data qos_data[] = {
	/* modules_qosConfig0 - 1 endpoints, 4 channels */
	{
		.reg = K3_QOS_REG(K3_DSS_UL_MAIN_0_VBUSM_DMA, 0),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_UL_MAIN_0_VBUSM_DMA, 1),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_UL_MAIN_0_VBUSM_DMA, 2),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_UL_MAIN_0_VBUSM_DMA, 3),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},

	/* modules_qosConfig1 - 1 endpoints, 4 channels */
	{
		.reg = K3_QOS_REG(K3_DSS_UL_MAIN_1_VBUSM_DMA, 0),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_UL_MAIN_1_VBUSM_DMA, 1),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_UL_MAIN_1_VBUSM_DMA, 2),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_UL_MAIN_1_VBUSM_DMA, 3),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},

	/* Following registers set 1:1 mapping for orderID MAP1/MAP2
	 * remap registers. orderID x is remapped to orderID x again
	 * This is to ensure orderID from MAP register is unchanged
	 */

	/* K3_DSS_UL_MAIN_0_VBUSM_DMA - 0 groups */

	/* K3_DSS_UL_MAIN_1_VBUSM_DMA - 0 groups */
};

u32 qos_count = ARRAY_SIZE(qos_data);
