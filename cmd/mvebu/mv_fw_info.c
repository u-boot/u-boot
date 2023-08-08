/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0
 * https://spdx.org/licenses
 */

#include <asm/io.h>
#include <command.h>
#include <common.h>
#include <config.h>
#include <mach/soc.h>

#define MSS_CP_CM3_SRAM_BASE		0x220000
#define RTOS_VERS_OFSSET		0x1000
#define	FW_INFO_OFFS			(MVEBU_REGS_BASE_CP(0, 0) + \
	MSS_CP_CM3_SRAM_BASE + RTOS_VERS_OFSSET)
#define	UNIT_FW_INFO(n_cp)		(FW_INFO_OFFS + (n_cp) * 0x40)

typedef struct{
	u8 unit_name[4];
	u32 mss_pm;
	u32 mci_link_mgmt;
	u32 fca;
	u8 rtos_version[32];
	u8 rtos_kernel_version[12];
} FW_INFO;

void mv_print_fw_info(const FW_INFO * const info)
{
	u32 first_flag = true;

	printf("Unit: %s\n FreeROTS release: %s\n Kernel: %s\n Flags:",
	       info->unit_name, info->rtos_version, info->rtos_kernel_version);

	if (info->mci_link_mgmt) {
		printf(" MCI link management");
		first_flag = false;
	}

	if (info->fca) {
		if (!first_flag)
			printf(",");
		printf(" PPv2 Flow Control");
		first_flag = false;
	}

	if (info->mss_pm) {
		if (!first_flag)
			printf(",");
		printf(" MSS Power Management");
	}
	printf("\n------------------------------------\n");
}

int mv_do_fw_info_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	FW_INFO info;
	u32 num_of_cps;
	u32 num_of_aps;
	u32 num_of_units;
	u32 i;

	soc_get_ap_cp_num(&num_of_aps, &num_of_cps);
	num_of_units = num_of_aps + num_of_cps;

	for (i = 0; i < num_of_units; ++i) {
		memcpy(&info, (void *)UNIT_FW_INFO(i), sizeof(FW_INFO));
		mv_print_fw_info(&info);
	}

	return 0;
}

U_BOOT_CMD(
	mv_fw_info, 1, 0, mv_do_fw_info_cmd,
	"Prints service CPU firmware information",
	"");
