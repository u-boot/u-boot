/*
 * UBIFS u-boot wrapper functions header
 *
 * Copyright (C) 2006-2008 Nokia Corporation
 *
 * (C) Copyright 2008-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Authors: Artem Bityutskiy (Битюцкий Артём)
 *          Adrian Hunter
 */

#ifndef __UBIFS_UBOOT_H__
#define __UBIFS_UBOOT_H__

int ubifs_init(void);
int uboot_ubifs_mount(char *vol_name);
void uboot_ubifs_umount(void);
int ubifs_is_mounted(void);
int ubifs_load(char *filename, u32 addr, u32 size);

int ubifs_ls(const char *dir_name);
int ubifs_read(const char *filename, void *buf, loff_t offset,
	       loff_t size, loff_t *actread);

#endif /* __UBIFS_UBOOT_H__ */
