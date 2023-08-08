/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:       GPL-2.0+
 * https://spdx.org/licenses
 */

#ifndef _VAR_H_
#define _VAR_H_

#include <common.h>
#include <linux/compiler.h>

#define INVALID_KEY	0xFF
#define MAX_VAR_OPTIONS	10

#define VAR_IS_DEFAULT	0x1
#define VAR_IS_LAST	0x2

struct var_opts {
	u8 value;
	char *desc;
	u8 flags;
};

struct var_desc {
	char *key;
	char *description;
};

#endif /* _VAR_H_ */
