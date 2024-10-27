// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 9elements GmbH
 */

#include <cpu.h>
#include <init.h>
#include <log.h>
#include <linux/arm-smccc.h>

#define SMC_SIP_FUNCTION_ID(n)  (0xC2000000 | (n))

#define SIP_SVC_VERSION        SMC_SIP_FUNCTION_ID(1)
#define SIP_SVC_GET_GIC        SMC_SIP_FUNCTION_ID(100)
#define SIP_SVC_GET_GIC_ITS    SMC_SIP_FUNCTION_ID(101)
#define SIP_SVC_GET_CPU_COUNT  SMC_SIP_FUNCTION_ID(200)
#define SIP_SVC_GET_CPU_NODE   SMC_SIP_FUNCTION_ID(201)
#define SIP_SVC_GET_MEMORY_NODE_COUNT SMC_SIP_FUNCTION_ID(300)
#define SIP_SVC_GET_MEMORY_NODE SMC_SIP_FUNCTION_ID(301)

int smc_get_mpidr(unsigned long id, u64 *mpidr)
{
	struct arm_smccc_res res;

	res.a0 = ~0;
	arm_smccc_smc(SIP_SVC_GET_CPU_NODE, id, 0, 0, 0, 0, 0, 0, &res);

	if (!res.a0)
		*mpidr = res.a2;

	return res.a0;
}

int smc_get_gic_dist_base(u64 *base)
{
	struct arm_smccc_res res;

	res.a0 = ~0;
	arm_smccc_smc(SIP_SVC_GET_GIC, 0, 0, 0, 0, 0, 0, 0, &res);

	if (!res.a0)
		*base = res.a1;

	return res.a0;
}

int smc_get_gic_redist_base(u64 *base)
{
	struct arm_smccc_res res;

	res.a0 = ~0;
	arm_smccc_smc(SIP_SVC_GET_GIC, 0, 0, 0, 0, 0, 0, 0, &res);

	if (!res.a0)
		*base = res.a2;

	return res.a0;
}

int smc_get_gic_its_base(u64 *base)
{
	struct arm_smccc_res res;

	res.a0 = ~0;
	arm_smccc_smc(SIP_SVC_GET_GIC_ITS, 0, 0, 0, 0, 0, 0, 0, &res);

	if (!res.a0)
		*base = res.a1;

	return res.a0;
}
