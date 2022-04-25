// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <common.h>
#include <blk.h>
#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <dm.h>
#include <env_internal.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

int bootmeth_check(struct udevice *dev, struct bootflow_iter *iter)
{
	const struct bootmeth_ops *ops = bootmeth_get_ops(dev);

	if (!ops->check)
		return 0;

	return ops->check(dev, iter);
}

int bootmeth_read_bootflow(struct udevice *dev, struct bootflow *bflow)
{
	const struct bootmeth_ops *ops = bootmeth_get_ops(dev);

	if (!ops->read_bootflow)
		return -ENOSYS;

	return ops->read_bootflow(dev, bflow);
}

int bootmeth_boot(struct udevice *dev, struct bootflow *bflow)
{
	const struct bootmeth_ops *ops = bootmeth_get_ops(dev);

	if (!ops->boot)
		return -ENOSYS;

	return ops->boot(dev, bflow);
}

int bootmeth_read_file(struct udevice *dev, struct bootflow *bflow,
		       const char *file_path, ulong addr, ulong *sizep)
{
	const struct bootmeth_ops *ops = bootmeth_get_ops(dev);

	if (!ops->read_file)
		return -ENOSYS;

	return ops->read_file(dev, bflow, file_path, addr, sizep);
}

/**
 * bootmeth_setup_iter_order() - Set up the ordering of bootmeths to scan
 *
 * This sets up the ordering information in @iter, based on the selected
 * ordering of the bootmethds in bootstd_priv->bootmeth_order. If there is no
 * ordering there, then all bootmethods are added
 *
 * @iter: Iterator to update with the order
 * Return: 0 if OK, -ENOENT if no bootdevs, -ENOMEM if out of memory, other -ve
 *	on other error
 */
int bootmeth_setup_iter_order(struct bootflow_iter *iter)
{
	struct bootstd_priv *std;
	struct udevice **order;
	int count;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return ret;

	/* Create an array large enough */
	count = std->bootmeth_count ? std->bootmeth_count :
		uclass_id_count(UCLASS_BOOTMETH);
	if (!count)
		return log_msg_ret("count", -ENOENT);

	order = calloc(count, sizeof(struct udevice *));
	if (!order)
		return log_msg_ret("order", -ENOMEM);

	/* If we have an ordering, copy it */
	if (IS_ENABLED(CONFIG_BOOTSTD_FULL) && std->bootmeth_count) {
		memcpy(order, std->bootmeth_order,
		       count * sizeof(struct bootmeth *));
	} else {
		struct udevice *dev;
		int i, upto;

		/*
		 * Get a list of bootmethods, in seq order (i.e. using aliases).
		 * There may be gaps so try to count up high enough to find them
		 * all.
		 */
		for (i = 0, upto = 0; upto < count && i < 20 + count * 2; i++) {
			ret = uclass_get_device_by_seq(UCLASS_BOOTMETH, i,
						       &dev);
			if (!ret)
				order[upto++] = dev;
		}
		count = upto;
	}

	iter->method_order = order;
	iter->num_methods = count;
	iter->cur_method = 0;

	return 0;
}

int bootmeth_set_order(const char *order_str)
{
	struct bootstd_priv *std;
	struct udevice **order;
	int count, ret, i, len;
	const char *s, *p;

	ret = bootstd_get_priv(&std);
	if (ret)
		return ret;

	if (!order_str) {
		free(std->bootmeth_order);
		std->bootmeth_order = NULL;
		std->bootmeth_count = 0;
		return 0;
	}

	/* Create an array large enough */
	count = uclass_id_count(UCLASS_BOOTMETH);
	if (!count)
		return log_msg_ret("count", -ENOENT);

	order = calloc(count + 1, sizeof(struct udevice *));
	if (!order)
		return log_msg_ret("order", -ENOMEM);

	for (i = 0, s = order_str; *s && i < count; s = p + (*p == ' '), i++) {
		struct udevice *dev;

		p = strchrnul(s, ' ');
		len = p - s;
		ret = uclass_find_device_by_namelen(UCLASS_BOOTMETH, s, len,
						    &dev);
		if (ret) {
			printf("Unknown bootmeth '%.*s'\n", len, s);
			free(order);
			return ret;
		}
		order[i] = dev;
	}
	order[i] = NULL;
	free(std->bootmeth_order);
	std->bootmeth_order = order;
	std->bootmeth_count = i;

	return 0;
}

/**
 * setup_fs() - Set up read to read a file
 *
 * We must redo the setup before each filesystem operation. This function
 * handles that, including setting the filesystem type if a block device is not
 * being used
 *
 * @bflow: Information about file to try
 * @desc: Block descriptor to read from (NULL if not a block device)
 * Return: 0 if OK, -ve on error
 */
static int setup_fs(struct bootflow *bflow, struct blk_desc *desc)
{
	int ret;

	if (desc) {
		ret = fs_set_blk_dev_with_part(desc, bflow->part);
		if (ret)
			return log_msg_ret("set", ret);
	} else if (IS_ENABLED(CONFIG_BOOTSTD_FULL) && bflow->fs_type) {
		fs_set_type(bflow->fs_type);
	}

	return 0;
}

int bootmeth_try_file(struct bootflow *bflow, struct blk_desc *desc,
		      const char *prefix, const char *fname)
{
	char path[200];
	loff_t size;
	int ret, ret2;

	snprintf(path, sizeof(path), "%s%s", prefix ? prefix : "", fname);
	log_debug("trying: %s\n", path);

	free(bflow->fname);
	bflow->fname = strdup(path);
	if (!bflow->fname)
		return log_msg_ret("name", -ENOMEM);

	if (IS_ENABLED(CONFIG_BOOTSTD_FULL) && bflow->fs_type)
		fs_set_type(bflow->fs_type);

	ret = fs_size(path, &size);
	log_debug("   %s - err=%d\n", path, ret);

	/* Sadly FS closes the file after fs_size() so we must redo this */
	ret2 = setup_fs(bflow, desc);
	if (ret2)
		return log_msg_ret("fs", ret2);

	if (ret)
		return log_msg_ret("size", ret);

	bflow->size = size;
	bflow->state = BOOTFLOWST_FILE;

	return 0;
}

int bootmeth_alloc_file(struct bootflow *bflow, uint size_limit, uint align)
{
	loff_t bytes_read;
	ulong addr;
	char *buf;
	uint size;
	int ret;

	size = bflow->size;
	log_debug("   - script file size %x\n", size);
	if (size > size_limit)
		return log_msg_ret("chk", -E2BIG);

	buf = memalign(align, size + 1);
	if (!buf)
		return log_msg_ret("buf", -ENOMEM);
	addr = map_to_sysmem(buf);

	ret = fs_read(bflow->fname, addr, 0, 0, &bytes_read);
	if (ret) {
		free(buf);
		return log_msg_ret("read", ret);
	}
	if (size != bytes_read)
		return log_msg_ret("bread", -EINVAL);
	buf[size] = '\0';
	bflow->state = BOOTFLOWST_READY;
	bflow->buf = buf;

	return 0;
}

int bootmeth_common_read_file(struct udevice *dev, struct bootflow *bflow,
			      const char *file_path, ulong addr, ulong *sizep)
{
	struct blk_desc *desc = NULL;
	loff_t len_read;
	loff_t size;
	int ret;

	if (bflow->blk)
		desc = dev_get_uclass_plat(bflow->blk);

	ret = setup_fs(bflow, desc);
	if (ret)
		return log_msg_ret("fs", ret);

	ret = fs_size(file_path, &size);
	if (ret)
		return log_msg_ret("size", ret);
	if (size > *sizep)
		return log_msg_ret("spc", -ENOSPC);

	ret = setup_fs(bflow, desc);
	if (ret)
		return log_msg_ret("fs", ret);

	ret = fs_read(file_path, addr, 0, 0, &len_read);
	if (ret)
		return ret;
	*sizep = len_read;

	return 0;
}

#ifdef CONFIG_BOOTSTD_FULL
/**
 * on_bootmeths() - Update the bootmeth order
 *
 * This will check for a valid baudrate and only apply it if valid.
 */
static int on_bootmeths(const char *name, const char *value, enum env_op op,
			int flags)
{
	int ret;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:
		ret = bootmeth_set_order(value);
		if (ret)
			return 1;
		return 0;
	case env_op_delete:
		bootmeth_set_order(NULL);
		fallthrough;
	default:
		return 0;
	}
}
U_BOOT_ENV_CALLBACK(bootmeths, on_bootmeths);
#endif /* CONFIG_BOOTSTD_FULL */

UCLASS_DRIVER(bootmeth) = {
	.id		= UCLASS_BOOTMETH,
	.name		= "bootmeth",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.per_device_plat_auto	= sizeof(struct bootmeth_uc_plat),
};
