/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Quality of Service (QoS) Configuration Header File
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */
#ifndef _K3_QOS_H_
#define _K3_QOS_H_

#include <linux/kernel.h>

#define QOS_0	(0 << 0)
#define QOS_1	(1 << 0)
#define QOS_2	(2 << 0)
#define QOS_3	(3 << 0)
#define QOS_4	(4 << 0)
#define QOS_5	(5 << 0)
#define QOS_6	(6 << 0)
#define QOS_7	(7 << 0)

#define ORDERID_0	(0 << 4)
#define ORDERID_1	(1 << 4)
#define ORDERID_2	(2 << 4)
#define ORDERID_3	(3 << 4)
#define ORDERID_4	(4 << 4)
#define ORDERID_5	(5 << 4)
#define ORDERID_6	(6 << 4)
#define ORDERID_7	(7 << 4)
#define ORDERID_8	(8 << 4)
#define ORDERID_9	(9 << 4)
#define ORDERID_10	(10 << 4)
#define ORDERID_11	(11 << 4)
#define ORDERID_12	(12 << 4)
#define ORDERID_13	(13 << 4)
#define ORDERID_14	(14 << 4)
#define ORDERID_15	(15 << 4)

#define ASEL_0	(0 << 8)
#define ASEL_1	(1 << 8)
#define ASEL_2	(2 << 8)
#define ASEL_3	(3 << 8)
#define ASEL_4	(4 << 8)
#define ASEL_5	(5 << 8)
#define ASEL_6	(6 << 8)
#define ASEL_7	(7 << 8)
#define ASEL_8	(8 << 8)
#define ASEL_9	(9 << 8)
#define ASEL_10	(10 << 8)
#define ASEL_11	(11 << 8)
#define ASEL_12	(12 << 8)
#define ASEL_13	(13 << 8)
#define ASEL_14	(14 << 8)
#define ASEL_15	(15 << 8)

#define EPRIORITY_0	(0 << 12)
#define EPRIORITY_1	(1 << 12)
#define EPRIORITY_2	(2 << 12)
#define EPRIORITY_3	(3 << 12)
#define EPRIORITY_4	(4 << 12)
#define EPRIORITY_5	(5 << 12)
#define EPRIORITY_6	(6 << 12)
#define EPRIORITY_7	(7 << 12)

#define VIRTID_0	(0 << 16)
#define VIRTID_1	(1 << 16)
#define VIRTID_2	(2 << 16)
#define VIRTID_3	(3 << 16)
#define VIRTID_4	(4 << 16)
#define VIRTID_5	(5 << 16)
#define VIRTID_6	(6 << 16)
#define VIRTID_7	(7 << 16)
#define VIRTID_8	(8 << 16)
#define VIRTID_9	(9 << 16)
#define VIRTID_10	(10 << 16)
#define VIRTID_11	(11 << 16)
#define VIRTID_12	(12 << 16)
#define VIRTID_13	(13 << 16)
#define VIRTID_14	(14 << 16)
#define VIRTID_15	(15 << 16)

#define ATYPE_0	(0 << 28)
#define ATYPE_1	(1 << 28)
#define ATYPE_2	(2 << 28)
#define ATYPE_3	(3 << 28)

struct k3_qos_data {
	u32 reg;
	u32 val;
};

#if (IS_ENABLED(CONFIG_K3_QOS))
extern struct k3_qos_data qos_data[];
extern u32 qos_count;
#endif

#endif /* _K3_QOS_H_ */
