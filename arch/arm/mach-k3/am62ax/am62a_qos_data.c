// SPDX-License-Identifier: GPL-2.0+
/*
 * am62a Quality of Service (QoS) Configuration Data
 * Auto generated from K3 Resource Partitioning tool
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */
#include <common.h>
#include <asm/arch/hardware.h>
#include "common.h"

struct k3_qos_data am62a_qos_data[] = {
	/* modules_qosConfig0 - 1 endpoints, 4 channels */
	{
		.reg = K3_DSS_UL_MAIN_0_VBUSM_DMA + 0x100 + 0x4 * 0,
		.val = ORDERID_8,
	},
	{
		.reg = K3_DSS_UL_MAIN_0_VBUSM_DMA + 0x100 + 0x4 * 1,
		.val = ORDERID_8,
	},
	{
		.reg = K3_DSS_UL_MAIN_0_VBUSM_DMA + 0x100 + 0x4 * 2,
		.val = ORDERID_8,
	},
	{
		.reg = K3_DSS_UL_MAIN_0_VBUSM_DMA + 0x100 + 0x4 * 3,
		.val = ORDERID_8,
	},

	/* Following registers set 1:1 mapping for orderID MAP1/MAP2
	 * remap registers. orderID x is remapped to orderID x again
	 * This is to ensure orderID from MAP register is unchanged
	 */

	/* K3_DSS_UL_MAIN_0_VBUSM_DMA - 1 groups */
	{
		.reg = K3_DSS_UL_MAIN_0_VBUSM_DMA + 0,
		.val = 0x76543210,
	},
	{
		.reg = K3_DSS_UL_MAIN_0_VBUSM_DMA + 4,
		.val = 0xfedcba98,
	},
};

uint32_t am62a_qos_count = sizeof(am62a_qos_data) / sizeof(am62a_qos_data[0]);
