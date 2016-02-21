/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

/* Allow for arch specific config before we boot */
static int __arch_auxiliary_core_up(u32 core_id, u32 boot_private_data)
{
	/* please define platform specific arch_auxiliary_core_up() */
	return CMD_RET_FAILURE;
}

int arch_auxiliary_core_up(u32 core_id, u32 boot_private_data)
	__attribute__((weak, alias("__arch_auxiliary_core_up")));

/* Allow for arch specific config before we boot */
static int __arch_auxiliary_core_check_up(u32 core_id)
{
	/* please define platform specific arch_auxiliary_core_check_up() */
	return 0;
}

int arch_auxiliary_core_check_up(u32 core_id)
	__attribute__((weak, alias("__arch_auxiliary_core_check_up")));

/*
 * To i.MX6SX and i.MX7D, the image supported by bootaux needs
 * the reset vector at the head for the image, with SP and PC
 * as the first two words.
 *
 * Per the cortex-M reference manual, the reset vector of M4 needs
 * to exist at 0x0 (TCMUL). The PC and SP are the first two addresses
 * of that vector.  So to boot M4, the A core must build the M4's reset
 * vector with getting the PC and SP from image and filling them to
 * TCMUL. When M4 is kicked, it will load the PC and SP by itself.
 * The TCMUL is mapped to (M4_BOOTROM_BASE_ADDR) at A core side for
 * accessing the M4 TCMUL.
 */
int do_bootaux(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr;
	int ret, up;

	if (argc < 2)
		return CMD_RET_USAGE;

	up = arch_auxiliary_core_check_up(0);
	if (up) {
		printf("## Auxiliary core is already up\n");
		return CMD_RET_SUCCESS;
	}

	addr = simple_strtoul(argv[1], NULL, 16);

	printf("## Starting auxiliary core at 0x%08lX ...\n", addr);

	ret = arch_auxiliary_core_up(0, addr);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	bootaux, CONFIG_SYS_MAXARGS, 1,	do_bootaux,
	"Start auxiliary core",
	""
);
