/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SpacemiT reset controller — common definitions (U-Boot)
 */

#ifndef _RESET_SPACEMIT_COMMON_H_
#define _RESET_SPACEMIT_COMMON_H_

#include <linux/types.h>

struct udevice;
struct reset_ops;

struct spacemit_reset_data {
	u32 offset;
	u32 assert_mask;
	u32 deassert_mask;
};

struct spacemit_reset_priv {
	void *base;
	const struct spacemit_reset_data *table;
	size_t table_size;
};

#define RESET_DATA(_offset, _assert_mask, _deassert_mask)	\
	{							\
		.offset		= (_offset),			\
		.assert_mask	= (_assert_mask),		\
		.deassert_mask	= (_deassert_mask),		\
	}

extern const struct reset_ops spacemit_reset_ops;

int spacemit_reset_probe(struct udevice *dev);
int spacemit_reset_bind(struct udevice *parent, const char *drv_name,
			const struct spacemit_reset_data *table,
			size_t table_size);

#endif /* _RESET_SPACEMIT_COMMON_H_ */
