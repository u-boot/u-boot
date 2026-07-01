// SPDX-License-Identifier: GPL-2.0+

#define LOG_CATEGORY	UCLASS_HOST

#include <bootdev.h>
#include <dm.h>
#include <log.h>
#include <sandbox_host.h>

static int sandbox_bootdev_bind(struct udevice *dev)
{
	struct bootdev_uc_plat *ucp = dev_get_uclass_plat(dev);

	ucp->prio = BOOTDEVP_4_SCAN_FAST; /* at least priority 4 */

	return 0;
}

static int sandbox_bootdev_hunt(struct bootdev_hunter *info, bool show)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	uclass_id_foreach_dev(UCLASS_HOST, dev, uc) {
		struct host_sb_plat *plat = dev_get_plat(dev);

		log_debug("hunting %s\n", plat->label);

		if (plat->flags & BLK_HOST_BROKEN) {
			ret = -ETIME;
			log_err("cannot hunt sandbox device '%s': %d\n",
			       plat->label, ret);
			return ret;
		}
	}

	return 0;
}

struct bootdev_ops sandbox_bootdev_ops = {
};

static const struct udevice_id sandbox_bootdev_ids[] = {
	{ .compatible = "u-boot,bootdev-sandbox" },
	{ }
};

U_BOOT_DRIVER(sandbox_bootdev) = {
	.name		= "sandbox_bootdev",
	.id		= UCLASS_BOOTDEV,
	.ops		= &sandbox_bootdev_ops,
	.bind		= sandbox_bootdev_bind,
	.of_match	= sandbox_bootdev_ids,
};

BOOTDEV_HUNTER(sandbox_bootdev_hunter) = {
	.prio		= BOOTDEVP_4_SCAN_FAST,
	.uclass		= UCLASS_HOST,
	.hunt		= sandbox_bootdev_hunt,
	.drv		= DM_DRIVER_REF(sandbox_bootdev),
};
