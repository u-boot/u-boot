/*
 * Core registration and callback routines for MTD
 * drivers and users.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/mtd/mtd.h>
#include <linux/mtd/compat.h>
#include <ubi_uboot.h>

struct mtd_info *mtd_table[MAX_MTD_DEVICES];

int add_mtd_device(struct mtd_info *mtd)
{
	int i;

	BUG_ON(mtd->writesize == 0);

	for (i = 0; i < MAX_MTD_DEVICES; i++)
		if (!mtd_table[i]) {
			mtd_table[i] = mtd;
			mtd->index = i;
			mtd->usecount = 0;

			/* No need to get a refcount on the module containing
			   the notifier, since we hold the mtd_table_mutex */

			/* We _know_ we aren't being removed, because
			   our caller is still holding us here. So none
			   of this try_ nonsense, and no bitching about it
			   either. :) */
			return 0;
		}

	return 1;
}

/**
 *      del_mtd_device - unregister an MTD device
 *      @mtd: pointer to MTD device info structure
 *
 *      Remove a device from the list of MTD devices present in the system,
 *      and notify each currently active MTD 'user' of its departure.
 *      Returns zero on success or 1 on failure, which currently will happen
 *      if the requested device does not appear to be present in the list.
 */
int del_mtd_device(struct mtd_info *mtd)
{
	int ret;

	if (mtd_table[mtd->index] != mtd) {
		ret = -ENODEV;
	} else if (mtd->usecount) {
		printk(KERN_NOTICE "Removing MTD device #%d (%s)"
				" with use count %d\n",
				mtd->index, mtd->name, mtd->usecount);
		ret = -EBUSY;
	} else {
		/* No need to get a refcount on the module containing
		 * the notifier, since we hold the mtd_table_mutex */
		mtd_table[mtd->index] = NULL;

		ret = 0;
	}

	return ret;
}

/**
 *	get_mtd_device - obtain a validated handle for an MTD device
 *	@mtd: last known address of the required MTD device
 *	@num: internal device number of the required MTD device
 *
 *	Given a number and NULL address, return the num'th entry in the device
 *      table, if any.  Given an address and num == -1, search the device table
 *      for a device with that address and return if it's still present. Given
 *      both, return the num'th driver only if its address matches. Return
 *      error code if not.
 */
struct mtd_info *get_mtd_device(struct mtd_info *mtd, int num)
{
	struct mtd_info *ret = NULL;
	int i, err = -ENODEV;

	if (num == -1) {
		for (i = 0; i < MAX_MTD_DEVICES; i++)
			if (mtd_table[i] == mtd)
				ret = mtd_table[i];
	} else if (num < MAX_MTD_DEVICES) {
		ret = mtd_table[num];
		if (mtd && mtd != ret)
			ret = NULL;
	}

	if (!ret)
		goto out_unlock;

	ret->usecount++;
	return ret;

out_unlock:
	return ERR_PTR(err);
}

/**
 *      get_mtd_device_nm - obtain a validated handle for an MTD device by
 *      device name
 *      @name: MTD device name to open
 *
 *      This function returns MTD device description structure in case of
 *      success and an error code in case of failure.
 */
struct mtd_info *get_mtd_device_nm(const char *name)
{
	int i, err = -ENODEV;
	struct mtd_info *mtd = NULL;

	for (i = 0; i < MAX_MTD_DEVICES; i++) {
		if (mtd_table[i] && !strcmp(name, mtd_table[i]->name)) {
			mtd = mtd_table[i];
			break;
		}
	}

	if (!mtd)
		goto out_unlock;

	mtd->usecount++;
	return mtd;

out_unlock:
	return ERR_PTR(err);
}

void put_mtd_device(struct mtd_info *mtd)
{
	int c;

	c = --mtd->usecount;
	BUG_ON(c < 0);
}
