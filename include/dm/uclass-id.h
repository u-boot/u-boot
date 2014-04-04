/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_UCLASS_ID_H
#define _DM_UCLASS_ID_H

/* TODO(sjg@chromium.org): this could be compile-time generated */
enum uclass_id {
	/* These are used internally by driver model */
	UCLASS_ROOT = 0,
	UCLASS_DEMO,
	UCLASS_TEST,
	UCLASS_TEST_FDT,

	/* U-Boot uclasses start here */
	UCLASS_GPIO,

	UCLASS_COUNT,
	UCLASS_INVALID = -1,
};

#endif
