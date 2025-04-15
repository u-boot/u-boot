// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#include <cpu_func.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <linux/errno.h>
#include <linux/intel-smc.h>
#include <linux/string.h>

int invoke_smc(u32 func_id, u64 *args, int arg_len, u64 *ret_arg, int ret_len)
{
	struct pt_regs regs;

	memset(&regs, 0, sizeof(regs));
	regs.regs[0] = func_id;

	if (args)
		memcpy(&regs.regs[1], args, arg_len * sizeof(*args));

	smc_call(&regs);

	if (ret_arg)
		memcpy(ret_arg, &regs.regs[1], ret_len * sizeof(*ret_arg));

	return regs.regs[0];
}

int smc_send_mailbox(u32 cmd, u32 len, u32 *arg, u8 urgent, u32 *resp_buf_len,
		     u32 *resp_buf)
{
	int ret;
	u64 args[6];
	u64 resp[3];

	args[0] = cmd;
	args[1] = (u64)arg;
	args[2] = len;
	args[3] = urgent;
	args[4] = (u64)resp_buf;

	if (arg && len > 0)
		flush_dcache_range((uintptr_t)arg, (uintptr_t)arg + len);

	if (resp_buf && resp_buf_len && *resp_buf_len > 0) {
		args[5] = *resp_buf_len;
		flush_dcache_range((uintptr_t)resp_buf, (uintptr_t)resp_buf + *resp_buf_len);
	} else {
		args[5] = 0;
	}

	ret = invoke_smc(INTEL_SIP_SMC_MBOX_SEND_CMD, args, ARRAY_SIZE(args),
			 resp, ARRAY_SIZE(resp));

	if (ret == INTEL_SIP_SMC_STATUS_OK && resp_buf && resp_buf_len) {
		if (!resp[0])
			*resp_buf_len = resp[1];
	}

	return (int)resp[0];
}

int smc_get_usercode(u32 *usercode)
{
	int ret;
	u64 resp;

	if (!usercode)
		return -EINVAL;

	ret = invoke_smc(INTEL_SIP_SMC_GET_USERCODE, NULL, 0,
			 &resp, 1);

	if (ret == INTEL_SIP_SMC_STATUS_OK)
		*usercode = (u32)resp;

	return ret;
}
