// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 *
 * DDR ECC CLI for U-Boot proper. Dispatches via ddr_ecc_ops so alternate
 * IO backends can share the same ddr_ecc command without CLI changes.
 */

#include <command.h>
#include <asm/arch/socfpga_ddr_ecc.h>
#include <linux/errno.h>
#include <linux/types.h>

/**
 * do_ddr_ecc_inject() - shared inject_sbe / inject_dbe implementation
 * @cmdtp: command table entry
 * @flag: command flags
 * @argc: argument count
 * @argv: unused (no arguments)
 * @dbe: true for DBE, false for SBE
 *
 * Return: CMD_RET_SUCCESS or CMD_RET_FAILURE.
 */
static int do_ddr_ecc_inject(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[], bool dbe)
{
	const struct ddr_ecc_ops *ops = ddr_ecc_get_ops();
	int ret;

	if (argc > 1)
		return CMD_RET_USAGE;

	if (!ops || !ops->inject) {
		printf("DDR ECC inject not supported on this platform\n");
		return CMD_RET_FAILURE;
	}

	if (dbe)
		printf("Injecting DBE (DDR access may hang and trigger watchdog recovery)\n");

	ret = ops->inject(dbe);
	if (ret)
		return CMD_RET_FAILURE;

	printf("DDR ECC %s inject OK\n", dbe ? "DBE" : "SBE");
	return CMD_RET_SUCCESS;
}

static int do_ddr_ecc_inject_sbe(struct cmd_tbl *cmdtp, int flag, int argc,
				 char *const argv[])
{
	return do_ddr_ecc_inject(cmdtp, flag, argc, argv, false);
}

static int do_ddr_ecc_inject_dbe(struct cmd_tbl *cmdtp, int flag, int argc,
				 char *const argv[])
{
	return do_ddr_ecc_inject(cmdtp, flag, argc, argv, true);
}

/**
 * do_ddr_ecc_status_io() - drain the IO ECC queue
 * @cmdtp: command table entry
 * @flag: command flags
 * @argc: argument count
 * @argv: unused (no arguments)
 *
 * Return: CMD_RET_SUCCESS if no DBE, CMD_RET_FAILURE on error or DBE present.
 */
static int do_ddr_ecc_status_io(struct cmd_tbl *cmdtp, int flag, int argc,
				char *const argv[])
{
	const struct ddr_ecc_ops *ops = ddr_ecc_get_ops();
	bool dbe = false;
	int ret;

	if (argc > 1)
		return CMD_RET_USAGE;

	if (!ops || !ops->status_io) {
		printf("DDR ECC status_io not supported on this platform\n");
		return CMD_RET_FAILURE;
	}

	ret = ops->status_io(&dbe);
	if (dbe) {
		printf("DDR ECC status_io: double-bit error(s) in IO ECC queue\n");
		return CMD_RET_FAILURE;
	}
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

/**
 * do_ddr_ecc_status_sdm() - drain the SDM ECC queue
 * @cmdtp: command table entry
 * @flag: command flags
 * @argc: argument count
 * @argv: unused (no arguments)
 *
 * Return: CMD_RET_SUCCESS if no DBE, CMD_RET_FAILURE on error or DBE present.
 */
static int do_ddr_ecc_status_sdm(struct cmd_tbl *cmdtp, int flag, int argc,
				 char *const argv[])
{
	const struct ddr_ecc_ops *ops = ddr_ecc_get_ops();
	bool dbe = false;
	int ret;

	if (argc > 1)
		return CMD_RET_USAGE;

	if (!ops || !ops->status_sdm) {
		printf("DDR ECC status_sdm not supported on this platform\n");
		return CMD_RET_FAILURE;
	}

	ret = ops->status_sdm(&dbe);
	if (dbe) {
		printf("DDR ECC status_sdm: double-bit error(s) in SDM ECC queue\n");
		return CMD_RET_FAILURE;
	}
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

/*
 * U_BOOT_SUBCMD_MKENT(name, maxargs, repeatable, cmd):
 * - maxargs includes the subcommand name; 1 => no user parameters
 * - repeatable: empty Enter re-runs last cmd (disable)
 */
#define DDR_ECC_SUBCMD_MAXARGS		1
#define DDR_ECC_SUBCMD_NO_REPEAT	0

U_BOOT_LONGHELP(ddr_ecc,
		"status_io - drain the IO ECC queue\n"
		"ddr_ecc status_sdm - drain the SDM ECC queue\n"
		"ddr_ecc inject_sbe - SBE injection\n"
		"ddr_ecc inject_dbe - DBE injection\n"
		);

U_BOOT_CMD_WITH_SUBCMDS(ddr_ecc,
			"DDR ECC inject and status",
			ddr_ecc_help_text,
	U_BOOT_SUBCMD_MKENT(inject_sbe, DDR_ECC_SUBCMD_MAXARGS,
			    DDR_ECC_SUBCMD_NO_REPEAT, do_ddr_ecc_inject_sbe),
	U_BOOT_SUBCMD_MKENT(inject_dbe, DDR_ECC_SUBCMD_MAXARGS,
			    DDR_ECC_SUBCMD_NO_REPEAT, do_ddr_ecc_inject_dbe),
	U_BOOT_SUBCMD_MKENT(status_io, DDR_ECC_SUBCMD_MAXARGS,
			    DDR_ECC_SUBCMD_NO_REPEAT, do_ddr_ecc_status_io),
	U_BOOT_SUBCMD_MKENT(status_sdm, DDR_ECC_SUBCMD_MAXARGS,
			    DDR_ECC_SUBCMD_NO_REPEAT, do_ddr_ecc_status_sdm),
);
