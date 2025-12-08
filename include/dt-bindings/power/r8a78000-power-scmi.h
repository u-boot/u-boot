/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/*
 * Copyright (C) 2025 Renesas Electronics Corp.
 *
 * IDs match SCP 4.27
 */

#ifndef __DT_BINDINGS_R8A78000_SCMI_POWER_H__
#define __DT_BINDINGS_R8A78000_SCMI_POWER_H__

/*
 * These power domain indices match the Power Domain ID defined by SCP FW 4.27.
 */

#define X5H_POWER_DOMAIN_ID_UFS0	12
#define X5H_POWER_DOMAIN_ID_UFS1	13

#define X5H_POWER_DOMAIN_ID_RSW		15

#define X5H_POWER_DOMAIN_ID_MPP0	17
#define X5H_POWER_DOMAIN_ID_MPP1	18
#define X5H_POWER_DOMAIN_ID_MPP2	19
#define X5H_POWER_DOMAIN_ID_MPP3	20

#endif /* __DT_BINDINGS_R8A78000_SCMI_POWER_H__ */
