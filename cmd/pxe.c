// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Calxeda, Inc.
 * Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.
 */

#include <common.h>
#include <command.h>
#include <fs.h>
#include <net.h>

#include "pxe_utils.h"

#ifdef CONFIG_CMD_NET
const char *pxe_default_paths[] = {
#ifdef CONFIG_SYS_SOC
#ifdef CONFIG_SYS_BOARD
	"default-" CONFIG_SYS_ARCH "-" CONFIG_SYS_SOC "-" CONFIG_SYS_BOARD,
#endif
	"default-" CONFIG_SYS_ARCH "-" CONFIG_SYS_SOC,
#endif
	"default-" CONFIG_SYS_ARCH,
	"default",
	NULL
};

static int do_get_tftp(struct pxe_context *ctx, const char *file_path,
		       char *file_addr, ulong *sizep)
{
	char *tftp_argv[] = {"tftp", NULL, NULL, NULL};
	int ret;

	tftp_argv[1] = file_addr;
	tftp_argv[2] = (void *)file_path;

	if (do_tftpb(ctx->cmdtp, 0, 3, tftp_argv))
		return -ENOENT;
	ret = pxe_get_file_size(sizep);
	if (ret)
		return log_msg_ret("tftp", ret);
	ctx->pxe_file_size = *sizep;

	return 1;
}

/*
 * Looks for a pxe file with a name based on the pxeuuid environment variable.
 *
 * Returns 1 on success or < 0 on error.
 */
static int pxe_uuid_path(struct pxe_context *ctx, unsigned long pxefile_addr_r)
{
	char *uuid_str;

	uuid_str = from_env("pxeuuid");

	if (!uuid_str)
		return -ENOENT;

	return get_pxelinux_path(ctx, uuid_str, pxefile_addr_r);
}

/*
 * Looks for a pxe file with a name based on the 'ethaddr' environment
 * variable.
 *
 * Returns 1 on success or < 0 on error.
 */
static int pxe_mac_path(struct pxe_context *ctx, unsigned long pxefile_addr_r)
{
	char mac_str[21];
	int err;

	err = format_mac_pxe(mac_str, sizeof(mac_str));

	if (err < 0)
		return err;

	return get_pxelinux_path(ctx, mac_str, pxefile_addr_r);
}

/*
 * Looks for pxe files with names based on our IP address. See pxelinux
 * documentation for details on what these file names look like.  We match
 * that exactly.
 *
 * Returns 1 on success or < 0 on error.
 */
static int pxe_ipaddr_paths(struct pxe_context *ctx, unsigned long pxefile_addr_r)
{
	char ip_addr[9];
	int mask_pos, err;

	sprintf(ip_addr, "%08X", ntohl(net_ip.s_addr));

	for (mask_pos = 7; mask_pos >= 0;  mask_pos--) {
		err = get_pxelinux_path(ctx, ip_addr, pxefile_addr_r);

		if (err > 0)
			return err;

		ip_addr[mask_pos] = '\0';
	}

	return -ENOENT;
}

int pxe_get(ulong pxefile_addr_r, char **bootdirp, ulong *sizep)
{
	struct cmd_tbl cmdtp[] = {};	/* dummy */
	struct pxe_context ctx;
	int i;

	if (pxe_setup_ctx(&ctx, cmdtp, do_get_tftp, NULL, false,
			  env_get("bootfile")))
		return -ENOMEM;
	/*
	 * Keep trying paths until we successfully get a file we're looking
	 * for.
	 */
	if (pxe_uuid_path(&ctx, pxefile_addr_r) > 0 ||
	    pxe_mac_path(&ctx, pxefile_addr_r) > 0 ||
	    pxe_ipaddr_paths(&ctx, pxefile_addr_r) > 0)
		goto done;

	i = 0;
	while (pxe_default_paths[i]) {
		if (get_pxelinux_path(&ctx, pxe_default_paths[i],
				      pxefile_addr_r) > 0)
			goto done;
		i++;
	}

	pxe_destroy_ctx(&ctx);

	return -ENOENT;
done:
	*bootdirp = env_get("bootfile");

	/*
	 * The PXE file size is returned but not the name. It is probably not
	 * that useful.
	 */
	*sizep = ctx.pxe_file_size;
	pxe_destroy_ctx(&ctx);

	return 0;
}

/*
 * Entry point for the 'pxe get' command.
 * This Follows pxelinux's rules to download a config file from a tftp server.
 * The file is stored at the location given by the pxefile_addr_r environment
 * variable, which must be set.
 *
 * UUID comes from pxeuuid env variable, if defined
 * MAC addr comes from ethaddr env variable, if defined
 * IP
 *
 * see http://syslinux.zytor.com/wiki/index.php/PXELINUX
 *
 * Returns 0 on success or 1 on error.
 */
static int
do_pxe_get(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char *pxefile_addr_str;
	ulong pxefile_addr_r;
	char *fname;
	ulong size;
	int ret;

	if (argc != 1)
		return CMD_RET_USAGE;

	pxefile_addr_str = from_env("pxefile_addr_r");

	if (!pxefile_addr_str)
		return 1;

	ret = strict_strtoul(pxefile_addr_str, 16,
			     (unsigned long *)&pxefile_addr_r);
	if (ret < 0)
		return 1;

	ret = pxe_get(pxefile_addr_r, &fname, &size);
	switch (ret) {
	case 0:
		printf("Config file '%s' found\n", fname);
		break;
	case -ENOMEM:
		printf("Out of memory\n");
		return CMD_RET_FAILURE;
	default:
		printf("Config file not found\n");
		return CMD_RET_FAILURE;
	}

	return 0;
}

/*
 * Boots a system using a pxe file
 *
 * Returns 0 on success, 1 on error.
 */
static int
do_pxe_boot(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned long pxefile_addr_r;
	char *pxefile_addr_str;
	struct pxe_context ctx;
	int ret;

	if (argc == 1) {
		pxefile_addr_str = from_env("pxefile_addr_r");
		if (!pxefile_addr_str)
			return 1;

	} else if (argc == 2) {
		pxefile_addr_str = argv[1];
	} else {
		return CMD_RET_USAGE;
	}

	if (strict_strtoul(pxefile_addr_str, 16, &pxefile_addr_r) < 0) {
		printf("Invalid pxefile address: %s\n", pxefile_addr_str);
		return 1;
	}

	if (pxe_setup_ctx(&ctx, cmdtp, do_get_tftp, NULL, false,
			  env_get("bootfile"))) {
		printf("Out of memory\n");
		return CMD_RET_FAILURE;
	}
	ret = pxe_process(&ctx, pxefile_addr_r, false);
	pxe_destroy_ctx(&ctx);
	if (ret)
		return CMD_RET_FAILURE;

	copy_filename(net_boot_file_name, "", sizeof(net_boot_file_name));

	return 0;
}

static struct cmd_tbl cmd_pxe_sub[] = {
	U_BOOT_CMD_MKENT(get, 1, 1, do_pxe_get, "", ""),
	U_BOOT_CMD_MKENT(boot, 2, 1, do_pxe_boot, "", "")
};

static void __maybe_unused pxe_reloc(void)
{
	static int relocated_pxe;

	if (!relocated_pxe) {
		fixup_cmdtable(cmd_pxe_sub, ARRAY_SIZE(cmd_pxe_sub));
		relocated_pxe = 1;
	}
}

static int do_pxe(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *cp;

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	pxe_reloc();
#endif

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "pxe" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_pxe_sub, ARRAY_SIZE(cmd_pxe_sub));

	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(pxe, 3, 1, do_pxe,
	   "commands to get and boot from pxe files",
	   "get - try to retrieve a pxe file using tftp\n"
	   "pxe boot [pxefile_addr_r] - boot from the pxe file at pxefile_addr_r\n"
);
#endif
