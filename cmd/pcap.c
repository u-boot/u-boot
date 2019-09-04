// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Ramon Fried <rfried.dev@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <net/pcap.h>

static int do_pcap_init(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	phys_addr_t addr;
	unsigned int size;

	if (argc != 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], NULL, 16);
	size = simple_strtoul(argv[2], NULL, 10);

	return pcap_init(addr, size) ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static int do_pcap_start(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	return pcap_start_stop(true) ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static int do_pcap_stop(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	return pcap_start_stop(false) ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static int do_pcap_status(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	return pcap_print_status() ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static int do_pcap_clear(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	return pcap_clear() ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static char pcap_help_text[] =
	"- network packet capture\n\n"
	"pcap\n"
	"pcap init\t\t\t<addr> <max_size>\n"
	"pcap start\t\t\tstart capture\n"
	"pcap stop\t\t\tstop capture\n"
	"pcap status\t\t\tprint status\n"
	"pcap clear\t\t\tclear capture buffer\n"
	"\n"
	"With:\n"
	"\t<addr>: user address to which pcap will be stored (hexedcimal)\n"
	"\t<max_size>: Maximum size of pcap file (decimal)\n"
	"\n";

U_BOOT_CMD_WITH_SUBCMDS(pcap, "pcap", pcap_help_text,
			U_BOOT_SUBCMD_MKENT(init, 3, 0, do_pcap_init),
			U_BOOT_SUBCMD_MKENT(start, 1, 0, do_pcap_start),
			U_BOOT_SUBCMD_MKENT(stop, 1, 0, do_pcap_stop),
			U_BOOT_SUBCMD_MKENT(status, 1, 0, do_pcap_status),
			U_BOOT_SUBCMD_MKENT(clear, 1, 0, do_pcap_clear),
);
