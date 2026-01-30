// SPDX-License-Identifier: GPL-2.0+
#include <command.h>

int do_bootd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
return run_command("run bootcmd", 0);
}
