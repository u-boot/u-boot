/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Quality of Service (QoS) Configuration Header File
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */
#ifndef _K3_QOS_H_
#define _K3_QOS_H_

#include <linux/kernel.h>

/* K3_QOS_REG: Registers to configure the channel for a given endpoint */

#define K3_QOS_REG(base_reg, i)		(base_reg + 0x100 + (i) * 4)

#define K3_QOS_VAL(qos, orderid, asel, epriority, virtid, atype) \
	(qos		<< 0  | \
	 orderid	<< 4  | \
	 asel		<< 8  | \
	 epriority	<< 12 | \
	 virtid		<< 16 | \
	 atype		<< 28)

/*
 * K3_QOS_GROUP_REG: Registers to set 1:1 mapping for orderID MAP1/MAP2
 * remap registers.
 */
#define K3_QOS_GROUP_REG(base_reg, i)	(base_reg + (i) * 4)

#define K3_QOS_GROUP_DEFAULT_VAL_LOW	0x76543210
#define K3_QOS_GROUP_DEFAULT_VAL_HIGH	0xfedcba98
struct k3_qos_data {
	u32 reg;
	u32 val;
};

#if (IS_ENABLED(CONFIG_K3_QOS))
extern struct k3_qos_data qos_data[];
extern u32 qos_count;
#endif

#endif /* _K3_QOS_H_ */
