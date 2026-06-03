// SPDX-License-Identifier: GPL-2.0+

#define LOG_CATEGORY UCLASS_SYSRESET

#include <command.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <sysreset.h>
#include <linux/delay.h>
#include <linux/err.h>

#ifdef CONFIG_SYSRESET_CMD_RESET
int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	enum sysreset_t reset_type = SYSRESET_DEFAULT;

	if (argc > 2)
		return CMD_RET_USAGE;

	if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'w') {
		reset_type = SYSRESET_WARM;
	}

	printf("resetting ...\n");
	mdelay(100);

	sysreset_walk_halt(reset_type);

	return 0;
}
#endif

U_BOOT_CMD(
	reset, 2, 0,	do_reset,
	"Perform RESET of the CPU",
	"- cold boot without level specifier\n"
	"reset -w - warm reset if implemented"
);
