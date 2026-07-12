// SPDX-License-Identifier: GPL-2.0+

#define LOG_CATEGORY UCLASS_SYSRESET

#include <command.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <sysreset.h>
#include <linux/delay.h>
#include <linux/err.h>

#ifdef CONFIG_SYSRESET_CMD_POWEROFF
int do_poweroff(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	puts("poweroff ...\n");
	mdelay(100);

	ret = sysreset_walk(SYSRESET_POWER_OFF);

	if (ret == -EINPROGRESS)
		mdelay(1000);

	/*NOTREACHED when power off*/
	return CMD_RET_FAILURE;
}
#endif

#ifdef CONFIG_CMD_POWEROFF
U_BOOT_CMD(
	poweroff, 1, 0,	do_poweroff,
	"Perform POWEROFF of the device",
	""
);
#endif
