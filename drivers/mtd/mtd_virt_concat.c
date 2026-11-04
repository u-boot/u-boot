// SPDX-License-Identifier: GPL-2.0+
/*
 * Virtual concat MTD device driver for U-Boot
 *
 * Copyright (C) 2018 Bernhard Frauendienst
 * Author: Bernhard Frauendienst <kernel@nospam.obeliks.de>
 *
 * Copyright (C) 2026 Advanced Micro Devices, Inc.
 * Author: Amit Kumar Mahapatra <amit.kumar-mahapatra@amd.com>
 *
 * This driver enables transparent concatenation of MTD partitions
 * based on the "part-concat-next" Device Tree property. It is used
 * for QSPI/OSPI flash devices on separate chip selects, where a single
 * filesystem spans multiple flash devices.
 */

#define LOG_CATEGORY UCLASS_MTD

#include <log.h>

#include <dm/ofnode.h>
#include <linux/compat.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/mtd/concat.h>
#include <linux/mtd/mtd.h>

#define CONCAT_PROP "part-concat-next"
#define CONCAT_POSTFIX "concat"
#define MIN_DEV_PER_CONCAT 2

static LIST_HEAD(concat_node_list);

/**
 * struct mtd_virt_concat_node - components of a concatenation
 * @head:       List handle
 * @count:      Number of partitions in this concatenation group
 * @num_subdev: Number of partitions collected so far
 * @nodes:      DT nodes of the partitions, in concatenation order
 * @subdev:     Collected partitions, at the same index as their DT node.
 *              Each holds a reference taken by mtd_virt_concat_add() and
 *              is not on its parent's partition list.
 * @mtd:        Concatenated MTD device, once it has been registered
 */
struct mtd_virt_concat_node {
	struct list_head head;
	unsigned int count;
	unsigned int num_subdev;
	ofnode *nodes;
	struct mtd_info **subdev;
	struct mtd_info *mtd;
};

/**
 * mtd_virt_concat_find - Find the group that contains a DT node
 * @node: DT node of a partition
 * @idxp: returns the index of @node in the group
 *
 * Return: the group, or NULL if @node is not part of any group
 */
static struct mtd_virt_concat_node *mtd_virt_concat_find(ofnode node,
							 int *idxp)
{
	struct mtd_virt_concat_node *item;
	int idx;

	list_for_each_entry(item, &concat_node_list, head) {
		for (idx = 0; idx < item->count; idx++) {
			if (ofnode_equal(item->nodes[idx], node)) {
				*idxp = idx;
				return item;
			}
		}
	}

	return NULL;
}

/**
 * mtd_virt_concat_free_part - Free a partition held by a group
 * @part: partition collected by mtd_virt_concat_add()
 */
static void mtd_virt_concat_free_part(struct mtd_info *part)
{
	__put_mtd_device(part);
	kfree(part->name);
	kfree(part);
}

/**
 * mtd_virt_concat_free_item - Destroy a group and everything it holds
 * @item: group to destroy
 *
 * Unregister and destroy the concatenated device, if there is one, then free
 * the partitions held by the group and the group itself.
 *
 * Return: 0 if OK, -EBUSY if the concatenated device is still in use, in
 * which case nothing is freed
 */
static int mtd_virt_concat_free_item(struct mtd_virt_concat_node *item)
{
	int idx, ret;

	if (item->mtd) {
		ret = del_mtd_device(item->mtd);
		if (ret)
			return ret;
		kfree(item->mtd->name);
		mtd_concat_destroy(item->mtd);
	}

	for (idx = 0; idx < item->count; idx++) {
		if (item->subdev[idx])
			mtd_virt_concat_free_part(item->subdev[idx]);
	}

	list_del(&item->head);
	kfree(item->subdev);
	kfree(item->nodes);
	kfree(item);

	return 0;
}

/**
 * mtd_virt_concat_free_pending - Free the groups which are not concatenated
 *
 * Called before every DT scan. A group which has been concatenated is kept,
 * along with its concatenated device, since the next scan would only create
 * the same device again.
 */
static void mtd_virt_concat_free_pending(void)
{
	struct mtd_virt_concat_node *item, *tmp;

	list_for_each_entry_safe(item, tmp, &concat_node_list, head) {
		if (!item->mtd)
			mtd_virt_concat_free_item(item);
	}
}

/**
 * mtd_virt_concat_uses - Check if a group has a partition of an MTD device
 * @item: group
 * @mtd: MTD device, which is the parent of the partitions
 *
 * Return: true if a partition collected by @item is a partition of @mtd
 */
static bool mtd_virt_concat_uses(struct mtd_virt_concat_node *item,
				 struct mtd_info *mtd)
{
	int idx;

	for (idx = 0; idx < item->count; idx++) {
		if (item->subdev[idx] && item->subdev[idx]->parent == mtd)
			return true;
	}

	return false;
}

/**
 * mtd_virt_concat_add_parts - Register the partitions of a group individually
 * @item: group which could not be concatenated
 *
 * Register each partition that the group holds as a normal partition of its
 * parent, then free the group.
 */
static void mtd_virt_concat_add_parts(struct mtd_virt_concat_node *item)
{
	struct mtd_info *part;
	int idx;

	for (idx = 0; idx < item->count; idx++) {
		part = item->subdev[idx];
		if (!part)
			continue;
		item->subdev[idx] = NULL;

		__put_mtd_device(part);
		list_add_tail(&part->node, &part->parent->partitions);
		add_mtd_device(part);
	}

	mtd_virt_concat_free_item(item);
}

int mtd_virt_concat_destroy(struct mtd_info *mtd)
{
	struct mtd_virt_concat_node *item, *tmp;
	int ret, err = 0;

	list_for_each_entry_safe(item, tmp, &concat_node_list, head) {
		if (!mtd_virt_concat_uses(item, mtd))
			continue;

		/* Carry on, so that every group which is not in use goes */
		ret = mtd_virt_concat_free_item(item);
		if (ret) {
			log_warning("%s is in use, keeping it\n",
				    item->mtd->name);
			if (!err)
				err = ret;
		}
	}

	return err;
}

bool mtd_virt_concat_used(struct mtd_info *mtd)
{
	struct mtd_virt_concat_node *item;

	list_for_each_entry(item, &concat_node_list, head) {
		if (!item->mtd || !mtd_virt_concat_uses(item, mtd))
			continue;
		if (item->mtd->usecount || mtd_partitions_used(item->mtd))
			return true;
	}

	return false;
}

bool mtd_virt_concat_is_joined(ofnode node)
{
	struct mtd_virt_concat_node *item;
	int idx;

	item = mtd_virt_concat_find(node, &idx);

	return item && item->mtd;
}

/**
 * mtd_virt_concat_create_item - Create a group
 * @parts:  DT node of the partition that has the part-concat-next property
 * @count:  number of partitions in the group, including @parts
 *
 * Return: 0 if OK (including when the group is ignored because of an invalid
 * part-concat-next entry), -ENOMEM if out of memory
 */
static int mtd_virt_concat_create_item(ofnode parts, unsigned int count)
{
	struct mtd_virt_concat_node *item;
	int i, idx, ret = -ENOMEM;
	ofnode node;

	item = kzalloc(sizeof(*item), GFP_KERNEL);
	if (!item)
		return -ENOMEM;
	item->count = count;
	item->nodes = kcalloc(count, sizeof(*item->nodes), GFP_KERNEL);
	item->subdev = kcalloc(count, sizeof(*item->subdev), GFP_KERNEL);
	if (!item->nodes || !item->subdev)
		goto err;

	/* From here on, ignore the group on error rather than failing */
	ret = 0;

	/*
	 * The partition in which "part-concat-next" property
	 * is defined is the first device in the list of concat
	 * devices.
	 */
	item->nodes[0] = parts;

	for (i = 1; i < count; i++) {
		node = ofnode_parse_phandle(parts, CONCAT_PROP, i - 1);
		if (!ofnode_valid(node)) {
			log_warning("%s: invalid %s entry %d, not concatenating\n",
				    ofnode_get_name(parts), CONCAT_PROP, i - 1);
			goto err;
		}
		if (mtd_virt_concat_find(node, &idx)) {
			log_warning("%s: %s is already concatenated, not concatenating\n",
				    ofnode_get_name(parts), ofnode_get_name(node));
			goto err;
		}
		item->nodes[i] = node;
	}

	list_add_tail(&item->head, &concat_node_list);

	return 0;

err:
	kfree(item->subdev);
	kfree(item->nodes);
	kfree(item);

	return ret;
}

int mtd_virt_concat_node_create(void)
{
	ofnode parts = ofnode_null();
	int count, idx, ret;

	/* Discard any group which the previous scan did not complete */
	mtd_virt_concat_free_pending();

	/* List all the concatenations found in DT */
	while (true) {
		parts = ofnode_find_node_with_property(parts, CONCAT_PROP);
		if (!ofnode_valid(parts))
			break;

		if (!ofnode_is_enabled(parts))
			continue;

		/* A group kept from a previous scan has the node first */
		if (mtd_virt_concat_find(parts, &idx)) {
			if (idx)
				log_warning("%s is already concatenated, ignoring its %s\n",
					    ofnode_get_name(parts), CONCAT_PROP);
			continue;
		}

		count = ofnode_count_phandle_with_args(parts, CONCAT_PROP,
						       NULL, 0);
		/*
		 * The partition in which "part-concat-next" property is
		 * defined is also part of the concat device, so increment
		 * count by 1.
		 */
		count++;
		if (count < MIN_DEV_PER_CONCAT) {
			log_warning("%s: invalid %s, not concatenating\n",
				    ofnode_get_name(parts), CONCAT_PROP);
			continue;
		}

		ret = mtd_virt_concat_create_item(parts, count);
		if (ret) {
			mtd_virt_concat_free_pending();
			return ret;
		}
	}

	return 0;
}

bool mtd_virt_concat_add(struct mtd_info *mtd)
{
	struct mtd_virt_concat_node *item;
	int idx;

	if (!ofnode_valid(mtd->flash_node))
		return false;

	item = mtd_virt_concat_find(mtd->flash_node, &idx);
	if (!item)
		return false;

	/*
	 * A partition which is out of reach has no size, so it cannot be
	 * concatenated. Leave the group incomplete, so that the partitions
	 * which were found are registered by themselves.
	 */
	if (!mtd->size || !mtd->erasesize)
		return false;

	/* Only one partition can take the place of a node in the group */
	if (item->subdev[idx]) {
		log_warning("%s is found more than once, ignoring the extra one\n",
			    ofnode_get_name(mtd->flash_node));
		return false;
	}

	if (__get_mtd_device(mtd))
		return false;
	item->subdev[idx] = mtd;
	item->num_subdev++;

	return true;
}

/**
 * mtd_virt_concat_join - Create and register the concatenated device of a group
 * @item: group with all its partitions collected
 *
 * Return: 0 if OK, -ve on error
 */
static int mtd_virt_concat_join(struct mtd_virt_concat_node *item)
{
	struct mtd_info *mtd;
	int idx, offset, ret;
	size_t name_sz;
	char *name;

	/* Calculate the length of the name of the virtual device */
	name_sz = 0;
	for (idx = 0; idx < item->count; idx++)
		name_sz += strlen(item->subdev[idx]->name) + 1;
	name_sz += strlen(CONCAT_POSTFIX);

	name = kmalloc(name_sz + 1, GFP_KERNEL);
	if (!name)
		return -ENOMEM;

	offset = 0;
	for (idx = 0; idx < item->count; idx++)
		offset += snprintf(name + offset, name_sz + 1 - offset,
				   "%s-", item->subdev[idx]->name);
	snprintf(name + offset, name_sz + 1 - offset, CONCAT_POSTFIX);

	mtd = mtd_concat_create(item->subdev, item->count, name);
	if (!mtd) {
		kfree(name);
		return -ENXIO;
	}

	ret = add_mtd_device(mtd);
	if (ret) {
		kfree(name);
		mtd_concat_destroy(mtd);
		return ret;
	}
	item->mtd = mtd;

	return 0;
}

int mtd_virt_concat_create_join(void)
{
	struct mtd_virt_concat_node *item, *tmp;
	int ret = 0, err;

	list_for_each_entry_safe(item, tmp, &concat_node_list, head) {
		/* Skip a group kept from a previous scan */
		if (item->mtd)
			continue;

		/*
		 * Everything has been probed by now, so nothing will
		 * complete this group later
		 */
		if (item->num_subdev != item->count) {
			log_warning("%s: found %u of %u partitions, not concatenating\n",
				    ofnode_get_name(item->nodes[0]),
				    item->num_subdev, item->count);
			mtd_virt_concat_add_parts(item);
			continue;
		}

		err = mtd_virt_concat_join(item);
		if (err) {
			log_warning("%s: failed to concatenate partitions (err=%d)\n",
				    ofnode_get_name(item->nodes[0]), err);
			mtd_virt_concat_add_parts(item);
			if (!ret)
				ret = err;
		}
	}

	return ret;
}
