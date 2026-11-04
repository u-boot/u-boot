/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MTD device concatenation layer definitions
 *
 * Copyright © 2002      Robert Kaiser <rkaiser@sysgo.de>
 *
 */

#ifndef MTD_CONCAT_H
#define MTD_CONCAT_H

#include <linux/mtd/mtd.h>

struct mtd_info *mtd_concat_create(
    struct mtd_info *subdev[],  /* subdevices to concatenate */
    int num_devs,               /* number of subdevices      */
#ifndef __UBOOT__
    const char *name);          /* name for the new device   */
#else
    char *name);          /* name for the new device   */
#endif

void mtd_concat_destroy(struct mtd_info *mtd);

#if IS_ENABLED(CONFIG_MTD_VIRT_CONCAT)

/**
 * mtd_virt_concat_node_create - List all concatenations found in DT
 *
 * Scans the device tree for nodes containing the "part-concat-next"
 * property and creates internal tracking structures for each
 * concatenation group which does not have one yet. A group which has been
 * concatenated by an earlier scan is kept, along with its concatenated
 * device, until a device it uses is removed (see mtd_virt_concat_destroy())
 * or the partitions of that device come from mtdparts instead.
 *
 * Return: 0 on success, -error otherwise.
 */
int mtd_virt_concat_node_create(void);

/**
 * mtd_virt_concat_is_joined - Check if a partition is in a concatenated device
 * @node: DT node of a partition
 *
 * Return: true if @node is in a group which has already been concatenated, so
 * that its partition does not need to be created again.
 */
bool mtd_virt_concat_is_joined(ofnode node);

/**
 * mtd_virt_concat_add - Add a partition to its concatenation group
 * @mtd: partition, with its DT node in @mtd->flash_node
 *
 * Checks if the partition's DT node is part of a concatenation group. If
 * so, the group takes the partition, which must then not be registered by
 * the caller. A partition which is out of reach, or which is not the first
 * one found for its node, is not taken.
 *
 * Return: true if the partition was taken by a group, false otherwise.
 */
bool mtd_virt_concat_add(struct mtd_info *mtd);

/**
 * mtd_virt_concat_create_join - Create and register concatenated MTD devices
 *
 * For each concatenation group that has all its partitions present,
 * creates and registers the concatenated MTD device. The partitions of a
 * group which is incomplete, or which cannot be concatenated, are
 * registered individually instead, with a warning.
 *
 * Return: 0 on success, -error otherwise.
 */
int mtd_virt_concat_create_join(void);

/**
 * mtd_virt_concat_destroy - Destroy concats which use an MTD device
 * @mtd: MTD device which is being removed, or whose partitions are replaced
 *
 * Destroys each concatenated device with a partition on @mtd, and frees its
 * partitions. They are found again the next time the partitions are
 * scanned. A concatenated device which is still in use is kept, but the
 * others are still destroyed.
 *
 * Return: 0 on success, -EBUSY if a concatenated device is still in use.
 */
int mtd_virt_concat_destroy(struct mtd_info *mtd);

/**
 * mtd_virt_concat_used - Check if a concat which uses an MTD device is in use
 * @mtd: MTD device
 *
 * Return: true if a concatenated device with a partition on @mtd is in use,
 * or any of its own partitions is.
 */
bool mtd_virt_concat_used(struct mtd_info *mtd);

#else

static inline int mtd_virt_concat_node_create(void) { return 0; }
static inline bool mtd_virt_concat_is_joined(ofnode node) { return false; }
static inline bool mtd_virt_concat_add(struct mtd_info *mtd) { return false; }
static inline int mtd_virt_concat_create_join(void) { return 0; }
static inline int mtd_virt_concat_destroy(struct mtd_info *mtd) { return 0; }
static inline bool mtd_virt_concat_used(struct mtd_info *mtd) { return false; }

#endif /* CONFIG_MTD_VIRT_CONCAT */
#endif /* MTD_CONCAT_H */
