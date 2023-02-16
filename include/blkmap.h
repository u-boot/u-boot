/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2023 Addiva Elektronik
 * Author: Tobias Waldekranz <tobias@waldekranz.com>
 */

#ifndef _BLKMAP_H
#define _BLKMAP_H

/**
 * blkmap_from_label() - Find blkmap from label
 *
 * @label: Label of the requested blkmap
 * Returns: A pointer to the blkmap on success, NULL on failure
 */
struct udevice *blkmap_from_label(const char *label);

/**
 * blkmap_create() - Create new blkmap
 *
 * @label: Label of the new blkmap
 * @devp: If not NULL, updated with the address of the resulting device
 * Returns: 0 on success, negative error code on failure
 */
int blkmap_create(const char *label, struct udevice **devp);

/**
 * blkmap_destroy() - Destroy blkmap
 *
 * @dev: The blkmap to be destroyed
 * Returns: 0 on success, negative error code on failure
 */
int blkmap_destroy(struct udevice *dev);

#endif	/* _BLKMAP_H */
