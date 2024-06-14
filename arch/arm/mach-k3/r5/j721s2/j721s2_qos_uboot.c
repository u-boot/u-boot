// SPDX-License-Identifier: GPL-2.0+
/*
 * j721s2 Quality of Service (QoS) Configuration Data
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <asm/arch/k3-qos.h>
#include "j721s2_qos.h"

struct k3_qos_data qos_data[] = {
	/* DSS_PIPE_VID1 - 2 endpoints, 2 channels */
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA, 0),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA, 1),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC, 0),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC, 1),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},

	/* DSS_PIPE_VIDL1 - 2 endpoints, 2 channels */
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA, 2),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA, 3),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC, 2),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC, 3),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},

	/* DSS_PIPE_VID2 - 2 endpoints, 2 channels */
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA, 4),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA, 5),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC, 4),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC, 5),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},

	/* DSS_PIPE_VIDL2 - 2 endpoints, 2 channels */
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA, 6),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA, 7),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC, 6),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},
	{
		.reg = K3_QOS_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC, 7),
		.val = K3_QOS_VAL(0, 15, 0, 0, 0, 0),
	},

	/* Following registers set 1:1 mapping for orderID MAP1/MAP2
	 * remap registers. orderID x is remapped to orderID x again
	 * This is to ensure orderID from MAP register is unchanged
	 */

	/* K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA - 1 groups */
	{
		.reg = K3_QOS_GROUP_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA, 0),
		.val = K3_QOS_GROUP_DEFAULT_VAL_LOW,
	},
	{
		.reg = K3_QOS_GROUP_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_DMA, 1),
		.val = K3_QOS_GROUP_DEFAULT_VAL_HIGH,
	},

	/* K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC - 1 groups */
	{
		.reg = K3_QOS_GROUP_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC, 0),
		.val = K3_QOS_GROUP_DEFAULT_VAL_LOW,
	},
	{
		.reg = K3_QOS_GROUP_REG(K3_DSS_MAIN_0_DSS_INST0_VBUSM_FBDC, 1),
		.val = K3_QOS_GROUP_DEFAULT_VAL_HIGH,
	},
};

u32 qos_count = ARRAY_SIZE(qos_data);
