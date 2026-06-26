// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023-2026 NXP
 *
 */

#include <command.h>
#include <cpu_func.h>
#include <init.h>
#include <log.h>
#include <asm/io.h>
#include <errno.h>
#include <linux/bitops.h>
#include <asm/arch-imx/cpu.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/arch/sys_proto.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <display_options.h>

static int do_v2x_status(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;
	u32 resp = 0;
	struct v2x_get_state state;

	if (is_imx91() || is_imx93()) {
		printf("No V2X supported\n");
		return CMD_RET_FAILURE;
	}

	ret = ele_v2x_get_state(&state, &resp);
	if (ret) {
		printf("get v2x state failed, resp 0x%x, ret %d\n", resp, ret);
		return CMD_RET_FAILURE;
	}

	printf("V2X state: 0x%x\n", state.v2x_state);
	printf("V2X power state: 0x%x\n", state.v2x_power_state);
	printf("V2X err code: 0x%x\n", state.v2x_err_code);

	return CMD_RET_SUCCESS;
}

static int do_ele_info(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;
	u32 res = 0, length;
	struct ele_get_info_data *info;

	/* ELE can't access full DDR */
	info = (struct ele_get_info_data *)(CONFIG_TEXT_BASE + SZ_2M -
		sizeof(struct ele_get_info_data));
	flush_dcache_range((ulong)info, (ulong)info + sizeof(struct ele_get_info_data));

	ret = ele_get_info(info, &res);
	if (ret) {
		printf("Get ELE info failed, resp 0x%x, ret %d\n", res, ret);
		return CMD_RET_FAILURE;
	}

	invalidate_dcache_range((ulong)info, (ulong)info + sizeof(struct ele_get_info_data));

	printf("SOC: 0x%x\n", info->soc);
	printf("LC: 0x%x\n", info->lc);

	printf("\nUID:\n");
	print_buffer(0, &info->uid, 4, 4, 0);

	printf("\nSHA256 ROM PATCH:\n");
	print_buffer(0, &info->sha256_rom_patch, 4, 8, 0);

	printf("\nSHA FW:\n");
	print_buffer(0, &info->sha_fw, 4, 8, 0);

	printf("\nOEM SRKH:\n");
	print_buffer(0, &info->oem_srkh, 4, 16, 0);

	printf("\nSTATE: 0x%x\n", info->state);

	length = (info->hdr >> 16) & 0xffff;
	if (length == sizeof(struct ele_get_info_data)) {
		printf("\nOEM PQC SRKH:\n");
		print_buffer(0, &info->oem_pqc_srkh, 4, 16, 0);
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(v2x_status, CONFIG_SYS_MAXARGS, 1, do_v2x_status,
	   "display v2x status",
	   ""
);

U_BOOT_CMD(ele_info, CONFIG_SYS_MAXARGS, 1, do_ele_info,
	   "display ELE information",
	   ""
);
