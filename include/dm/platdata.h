/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_PLATDATA_H
#define _DM_PLATDATA_H

struct driver_info {
	const char	*name;
	const void	*platdata;
};

#define U_BOOT_DEVICE(__name)						\
	ll_entry_declare(struct driver_info, __name, driver_info)

#endif
