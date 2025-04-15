// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 JJ Hiblot <jjhiblot@ti.com>
 */

#include <command.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <dm/uclass-internal.h>

static int bind_by_class_seq(const char *uclass, int seq,
			     const char *drv_name)
{
	static enum uclass_id uclass_id;
	struct udevice *dev;
	struct udevice *parent;
	int ret;
	struct driver *drv;

	drv = lists_driver_lookup_name(drv_name);
	if (!drv) {
		printf("Cannot find driver '%s'\n", drv_name);
		return -ENOENT;
	}

	uclass_id = uclass_get_by_name(uclass);
	if (uclass_id == UCLASS_INVALID) {
		printf("%s is not a valid uclass\n", uclass);
		return -EINVAL;
	}

	ret = uclass_find_device_by_seq(uclass_id, seq, &parent);
	if (!parent || ret) {
		printf("Cannot find device %d of class %s\n", seq, uclass);
		return ret;
	}

	ret = device_bind_with_driver_data(parent, drv, drv->name, 0,
					   ofnode_null(), &dev);
	if (!dev || ret) {
		printf("Unable to bind. err:%d\n", ret);
		return ret;
	}

	return 0;
}

static int find_dev(const char *uclass, int seq, struct udevice **devp)
{
	static enum uclass_id uclass_id;
	int rc;

	uclass_id = uclass_get_by_name(uclass);
	if (uclass_id == UCLASS_INVALID) {
		printf("%s is not a valid uclass\n", uclass);
		return -EINVAL;
	}

	rc = uclass_find_device_by_seq(uclass_id, seq, devp);
	if (!*devp || rc) {
		printf("Cannot find device %d of class %s\n", seq, uclass);
		return rc;
	}

	return 0;
}

static int unbind_by_class_seq(const char *uclass, int seq)
{
	int ret;
	struct udevice *dev;

	ret = find_dev(uclass, seq, &dev);
	if (ret)
		return ret;

	ret = device_remove(dev, DM_REMOVE_NORMAL);
	if (ret) {
		printf("Unable to remove. err:%d\n", ret);
		return ret;
	}

	ret = device_unbind(dev);
	if (ret) {
		printf("Unable to unbind. err:%d\n", ret);
		return ret;
	}

	return 0;
}

static int unbind_child_by_class_seq(const char *uclass, int seq,
				     const char *drv_name)
{
	struct udevice *parent;
	int ret;
	struct driver *drv;

	drv = lists_driver_lookup_name(drv_name);
	if (!drv) {
		printf("Cannot find driver '%s'\n", drv_name);
		return -ENOENT;
	}

	ret = find_dev(uclass, seq, &parent);
	if (ret)
		return ret;

	ret = device_chld_remove(parent, drv, DM_REMOVE_NORMAL);
	if (ret)
		printf("Unable to remove all. err:%d\n", ret);

	ret = device_chld_unbind(parent, drv);
	if (ret)
		printf("Unable to unbind all. err:%d\n", ret);

	return ret;
}

static int bind_by_node_path(const char *path, const char *drv_name)
{
	struct udevice *dev;
	struct udevice *parent = NULL;
	int ret;
	ofnode ofnode;
	struct driver *drv;

	drv = lists_driver_lookup_name(drv_name);
	if (!drv) {
		printf("%s is not a valid driver name\n", drv_name);
		return -ENOENT;
	}

	ofnode = ofnode_path(path);
	if (!ofnode_valid(ofnode)) {
		printf("%s is not a valid node path\n", path);
		return -EINVAL;
	}

	while (ofnode_valid(ofnode)) {
		if (!device_find_global_by_ofnode(ofnode, &parent))
			break;
		ofnode = ofnode_get_parent(ofnode);
	}

	if (!parent) {
		printf("Cannot find a parent device for node path %s\n", path);
		return -ENODEV;
	}

	ofnode = ofnode_path(path);
	ret = lists_bind_fdt(parent, ofnode, &dev, drv, false);

	if (!dev || ret) {
		printf("Unable to bind. err:%d\n", ret);
		return ret;
	}

	return 0;
}

static int unbind_by_node_path(const char *path)
{
	struct udevice *dev;
	int ret;
	ofnode ofnode;

	ofnode = ofnode_path(path);
	if (!ofnode_valid(ofnode)) {
		printf("%s is not a valid node path\n", path);
		return -EINVAL;
	}

	ret = device_find_global_by_ofnode(ofnode, &dev);

	if (!dev || ret) {
		printf("Cannot find a device with path %s\n", path);
		return -ENODEV;
	}

	ret = device_remove(dev, DM_REMOVE_NORMAL);
	if (ret) {
		printf("Unable to remove. err:%d\n", ret);
		return ret;
	}

	ret = device_unbind(dev);
	if (ret) {
		printf("Unable to unbind. err:%d\n", ret);
		return ret;
	}

	return 0;
}

static int do_bind_unbind(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	int ret = 0;
	bool bind;
	bool by_node;

	if (argc < 2)
		return CMD_RET_USAGE;

	bind = (argv[0][0] == 'b');
	by_node = (argv[1][0] == '/');

	if (by_node && bind) {
		if (argc != 3)
			return CMD_RET_USAGE;
		ret = bind_by_node_path(argv[1], argv[2]);
	} else if (by_node && !bind) {
		if (argc != 2)
			return CMD_RET_USAGE;
		ret = unbind_by_node_path(argv[1]);
	} else if (!by_node && bind) {
		int seq = (argc > 2) ? dectoul(argv[2], NULL) : 0;

		if (argc != 4)
			return CMD_RET_USAGE;
		ret = bind_by_class_seq(argv[1], seq, argv[3]);
	} else if (!by_node && !bind) {
		int seq = (argc > 2) ? dectoul(argv[2], NULL) : 0;

		if (argc == 3)
			ret = unbind_by_class_seq(argv[1], seq);
		else if (argc == 4)
			ret = unbind_child_by_class_seq(argv[1], seq,
							argv[3]);
		else
			return CMD_RET_USAGE;
	}

	if (ret)
		return CMD_RET_FAILURE;
	else
		return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	bind,	4,	0,	do_bind_unbind,
	"Bind a device to a driver",
	"<node path> <driver>\n"
	"bind <class> <seq> <driver>\n"
	"Use 'dm tree' to list all devices registered in the driver model,\n"
	"their path, class, sequence and current driver.\n"
);

U_BOOT_CMD(
	unbind,	4,	0,	do_bind_unbind,
	"Unbind a device from a driver",
	"<node path>\n"
	"unbind <class> <seq>\n"
	"unbind <class> <seq> <driver>\n"
	"Use 'dm tree' to list all devices registered in the driver model,\n"
	"their path, class, sequence and current driver.\n"
);
