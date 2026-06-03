// SPDX-License-Identifier: GPL-2.0+

#define LOG_CATEGORY UCLASS_SYSRESET

#include <command.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <sysreset.h>
#include <linux/delay.h>
#include <linux/err.h>

#if IS_ENABLED(CONFIG_SYSRESET_CMD_RESET_ARGS)
static int sysreset_request_arg(struct udevice *dev, int argc, char * const argv[])
{
	struct sysreset_ops *ops = sysreset_get_ops(dev);

	if (!ops->request_arg)
		return -ENOSYS;

	return ops->request_arg(dev, argc, argv);
}

static int sysreset_walk_arg(int argc, char * const argv[])
{
	struct udevice *dev;
	int ret = -ENOSYS;

	while (ret != -EINPROGRESS && ret != -EPROTONOSUPPORT) {
		for (uclass_first_device(UCLASS_SYSRESET, &dev);
		     dev;
		     uclass_next_device(&dev)) {
			ret = sysreset_request_arg(dev, argc, argv);
			if (ret == -EINPROGRESS || ret == -EPROTONOSUPPORT)
				break;
		}
	}

	return ret;
}
#endif /* CONFIG_SYSRESET_CMD_RESET_ARGS */

#if IS_ENABLED(CONFIG_SYSRESET_CMD_RESET)
static enum sysreset_t sysreset_get_default_type(void)
{
	if (IS_ENABLED(CONFIG_SYSRESET_CMD_RESET_DEFAULT_WARM))
		return SYSRESET_WARM;

	if (IS_ENABLED(CONFIG_SYSRESET_CMD_RESET_DEFAULT_POWER))
		return SYSRESET_POWER;

	return SYSRESET_COLD;
}

int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	enum sysreset_t reset_type = sysreset_get_default_type();

	if (argc > 2)
		return CMD_RET_USAGE;

	if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'w') {
		reset_type = SYSRESET_WARM;
	}

	printf("resetting ...\n");
	mdelay(100);

#if IS_ENABLED(CONFIG_SYSRESET_CMD_RESET_ARGS)
	if (argc > 1 && sysreset_walk_arg(argc, argv) == -EINPROGRESS)
		return 0;
#endif

	sysreset_walk_halt(reset_type);

	return 0;
}
#endif /* CONFIG_SYSRESET_CMD_RESET */

U_BOOT_CMD(
	reset, 2, 0,	do_reset,
	"Perform RESET of the CPU",
	"- cold boot without level specifier\n"
#if IS_ENABLED(CONFIG_SYSRESET_CMD_RESET_ARGS)
// All options handled by sysreset drivers via their sysreset_ops.request_arg callback
#ifdef CONFIG_SYSRESET_QCOM_PSCI
	"reset -edl - Boot to Emergency DownLoad mode\n"
#endif
#endif
	"reset -w - warm reset if implemented"
);
