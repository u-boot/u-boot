// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018, Google Inc.
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <command.h>
#include <dm.h>
#include <spl.h>
#include <asm/cpu.h>
#include <asm/global_data.h>
#include <asm/state.h>

static int do_sb_handoff(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
#if CONFIG_IS_ENABLED(HANDOFF)
	if (gd->spl_handoff)
		printf("SPL handoff magic %lx\n", gd->spl_handoff->arch.magic);
	else
		printf("SPL handoff info not received\n");

	return 0;
#else
	printf("Command not supported\n");

	return CMD_RET_USAGE;
#endif
}

static int do_sb_map(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	sandbox_map_list();

	return 0;
}

static int do_sb_state(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct sandbox_state *state;

	state = state_get_current();
	state_show(state);

	return 0;
}

U_BOOT_LONGHELP(sb,
	"handoff     - Show handoff data received from SPL\n"
	"sb map         - Show mapped memory\n"
	"sb state       - Show sandbox state");

U_BOOT_CMD_WITH_SUBCMDS(sb, "Sandbox status commands", sb_help_text,
	U_BOOT_SUBCMD_MKENT(handoff, 1, 1, do_sb_handoff),
	U_BOOT_SUBCMD_MKENT(map, 1, 1, do_sb_map),
	U_BOOT_SUBCMD_MKENT(state, 1, 1, do_sb_state));
