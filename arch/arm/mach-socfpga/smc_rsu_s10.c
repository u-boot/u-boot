// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 Intel Corporation. All rights reserved
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 */

#include <errno.h>
#include <asm/io.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/smc_s10.h>
#include <asm/system.h>
#include <linux/intel-smc.h>

DECLARE_GLOBAL_DATA_PTR;

#define DCMF_STATUS_INVALID 0xFFFF

u32 smc_rsu_update_address __secure_data;
u32 smc_rsu_dcmf_version[4] __secure_data = {0, 0, 0, 0};
u16 smc_rsu_dcmf_status[4] __secure_data = {DCMF_STATUS_INVALID,
					    DCMF_STATUS_INVALID,
					    DCMF_STATUS_INVALID,
					    DCMF_STATUS_INVALID};
static u32 smc_rsu_max_retry __secure_data;

int smc_store_max_retry(u32 value)
{
	void *max_retry;

	/*
	 * Convert the address of smc_rsu_max_retry
	 * to pre-relocation address.
	 */
	max_retry = (char *)__secure_start - CONFIG_ARMV8_SECURE_BASE +
			(u64)secure_ram_addr(&smc_rsu_max_retry);

	memcpy(max_retry, &value, sizeof(u32));

	return 0;
}

static void __secure smc_socfpga_rsu_status_psci(unsigned long function_id)
{
	SMC_ALLOC_REG_MEM(r);
	u64 rsu_status[5];

	SMC_INIT_REG_MEM(r);

	if (mbox_rsu_status_psci((u32 *)rsu_status, 9)) {
		SMC_ASSIGN_REG_MEM(r, SMC_ARG0, INTEL_SIP_SMC_RSU_ERROR);
		SMC_RET_REG_MEM(r);
		return;
	}

	SMC_ASSIGN_REG_MEM(r, SMC_ARG0, rsu_status[0]);
	SMC_ASSIGN_REG_MEM(r, SMC_ARG1, rsu_status[1]);
	SMC_ASSIGN_REG_MEM(r, SMC_ARG2, rsu_status[2]);
	SMC_ASSIGN_REG_MEM(r, SMC_ARG3, rsu_status[3]);

	SMC_RET_REG_MEM(r);
}

static void __secure smc_socfpga_rsu_update_psci(unsigned long function_id,
					    unsigned long update_address)
{
	SMC_ALLOC_REG_MEM(r);

	SMC_INIT_REG_MEM(r);

	smc_rsu_update_address = update_address;

	SMC_ASSIGN_REG_MEM(r, SMC_ARG0, INTEL_SIP_SMC_STATUS_OK);

	SMC_RET_REG_MEM(r);
}

static void __secure smc_socfpga_rsu_notify_psci(unsigned long function_id,
					    unsigned long execution_stage)
{
	SMC_ALLOC_REG_MEM(r);
	SMC_INIT_REG_MEM(r);

	if (mbox_hps_stage_notify_psci(execution_stage))
		SMC_ASSIGN_REG_MEM(r, SMC_ARG0, INTEL_SIP_SMC_RSU_ERROR);
	else
		SMC_ASSIGN_REG_MEM(r, SMC_ARG0, INTEL_SIP_SMC_STATUS_OK);

	SMC_RET_REG_MEM(r);
}

static void __secure smc_socfpga_rsu_retry_counter_psci(
					    unsigned long function_id)
{
	SMC_ALLOC_REG_MEM(r);
	u32 rsu_status[9];

	SMC_INIT_REG_MEM(r);

	if (mbox_rsu_status_psci((u32 *)rsu_status, sizeof(rsu_status) / 4)) {
		SMC_ASSIGN_REG_MEM(r, SMC_ARG0, INTEL_SIP_SMC_RSU_ERROR);
		SMC_RET_REG_MEM(r);
		return;
	}

	SMC_ASSIGN_REG_MEM(r, SMC_ARG0, INTEL_SIP_SMC_STATUS_OK);
	SMC_ASSIGN_REG_MEM(r, SMC_ARG1, rsu_status[8]);

	SMC_RET_REG_MEM(r);
}

static void __secure smc_socfpga_rsu_dcmf_version_psci(unsigned long
						       function_id)
{
	SMC_ALLOC_REG_MEM(r);
	u64 resp0;
	u64 resp1;

	SMC_INIT_REG_MEM(r);

	resp0 = smc_rsu_dcmf_version[1];
	resp0 = (resp0 << 32) | (u64)smc_rsu_dcmf_version[0];

	resp1 = smc_rsu_dcmf_version[3];
	resp1 = (resp1 << 32) | (u64)smc_rsu_dcmf_version[2];

	SMC_ASSIGN_REG_MEM(r, SMC_ARG0, INTEL_SIP_SMC_STATUS_OK);
	SMC_ASSIGN_REG_MEM(r, SMC_ARG1, resp0);
	SMC_ASSIGN_REG_MEM(r, SMC_ARG2, resp1);

	SMC_RET_REG_MEM(r);
}

static void __secure smc_socfpga_rsu_max_retry_psci(unsigned long
						       function_id)
{
	SMC_ALLOC_REG_MEM(r);
	u64 resp0;

	SMC_INIT_REG_MEM(r);

	resp0 = smc_rsu_max_retry;

	SMC_ASSIGN_REG_MEM(r, SMC_ARG0, INTEL_SIP_SMC_STATUS_OK);
	SMC_ASSIGN_REG_MEM(r, SMC_ARG1, resp0);

	SMC_RET_REG_MEM(r);
}

static void __secure smc_socfpga_rsu_dcmf_status_psci(unsigned long function_id)
{
	SMC_ALLOC_REG_MEM(r);
	u64 resp0;

	SMC_INIT_REG_MEM(r);

	if (smc_rsu_dcmf_status[0] == DCMF_STATUS_INVALID ||
	    smc_rsu_dcmf_status[1] == DCMF_STATUS_INVALID ||
	    smc_rsu_dcmf_status[2] == DCMF_STATUS_INVALID ||
	    smc_rsu_dcmf_status[3] == DCMF_STATUS_INVALID) {
		SMC_ASSIGN_REG_MEM(r, SMC_ARG0, INTEL_SIP_SMC_RSU_ERROR);
		SMC_RET_REG_MEM(r);
		return;
	}

	resp0 = smc_rsu_dcmf_status[3];
	resp0 = (resp0 << 16) | (u64)smc_rsu_dcmf_status[2];
	resp0 = (resp0 << 16) | (u64)smc_rsu_dcmf_status[1];
	resp0 = (resp0 << 16) | (u64)smc_rsu_dcmf_status[0];

	SMC_ASSIGN_REG_MEM(r, SMC_ARG0, INTEL_SIP_SMC_STATUS_OK);
	SMC_ASSIGN_REG_MEM(r, SMC_ARG1, resp0);

	SMC_RET_REG_MEM(r);
}

DECLARE_SECURE_SVC(rsu_status_psci, INTEL_SIP_SMC_RSU_STATUS,
		   smc_socfpga_rsu_status_psci);
DECLARE_SECURE_SVC(rsu_update_psci, INTEL_SIP_SMC_RSU_UPDATE,
		   smc_socfpga_rsu_update_psci);
DECLARE_SECURE_SVC(rsu_notify_psci, INTEL_SIP_SMC_RSU_NOTIFY,
		   smc_socfpga_rsu_notify_psci);
DECLARE_SECURE_SVC(rsu_retry_counter_psci, INTEL_SIP_SMC_RSU_RETRY_COUNTER,
		   smc_socfpga_rsu_retry_counter_psci);
DECLARE_SECURE_SVC(rsu_dcmf_version_psci, INTEL_SIP_SMC_RSU_DCMF_VERSION,
		   smc_socfpga_rsu_dcmf_version_psci);
DECLARE_SECURE_SVC(rsu_max_retry_psci, INTEL_SIP_SMC_RSU_MAX_RETRY,
		   smc_socfpga_rsu_max_retry_psci);
DECLARE_SECURE_SVC(rsu_dcmf_status_psci, INTEL_SIP_SMC_RSU_DCMF_STATUS,
		   smc_socfpga_rsu_dcmf_status_psci);
