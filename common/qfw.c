// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#include <dm.h>
#include <dm/uclass.h>
#include <qfw.h>
#include <stdlib.h>

int qfw_get_dev(struct udevice **devp)
{
	return uclass_first_device_err(UCLASS_QFW, devp);
}

int qfw_online_cpus(struct udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	return le16_to_cpu(nb_cpus);
}

int qfw_read_firmware_list(struct udevice *dev)
{
	int i;
	u32 count;
	struct fw_file *file;
	struct list_head *entry;

	struct qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	if (!list_empty(&qdev->fw_list))
		return 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &count);
	if (!count)
		return 0;

	count = be32_to_cpu(count);
	for (i = 0; i < count; i++) {
		file = malloc(sizeof(*file));
		if (!file) {
			printf("error: allocating resource\n");
			goto err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       sizeof(struct fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	return 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, struct fw_file, list);
		free(file);
	}

	return -ENOMEM;
}

struct fw_file *qfw_find_file(struct udevice *dev, const char *name)
{
	struct list_head *entry;
	struct fw_file *file;

	struct qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, struct fw_file, list);
		if (!strcmp(file->cfg.name, name))
			return file;
	}

	return NULL;
}

struct fw_file *qfw_file_iter_init(struct udevice *dev,
				   struct fw_cfg_file_iter *iter)
{
	struct qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	return list_entry((struct list_head *)iter->entry,
			  struct fw_file, list);
}

struct fw_file *qfw_file_iter_next(struct fw_cfg_file_iter *iter)
{
	iter->entry = ((struct list_head *)iter->entry)->next;
	return list_entry((struct list_head *)iter->entry,
			  struct fw_file, list);
}

bool qfw_file_iter_end(struct fw_cfg_file_iter *iter)
{
	return iter->entry == iter->end;
}
