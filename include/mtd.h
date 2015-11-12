/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MTD_H_
#define _MTD_H_

#include <linux/mtd/mtd.h>

/*
 * Get mtd_info structure of the dev, which is stored as uclass private.
 *
 * @dev: The MTD device
 * @return: pointer to mtd_info, NULL on error
 */
static inline struct mtd_info *mtd_get_info(struct udevice *dev)
{
	return dev_get_uclass_priv(dev);
}

#endif	/* _MTD_H_ */
