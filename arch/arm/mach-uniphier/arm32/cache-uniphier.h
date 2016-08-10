/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CACHE_UNIPHIER_H
#define __CACHE_UNIPHIER_H

#include <linux/types.h>

void uniphier_cache_prefetch_range(u32 start, u32 end, u32 ways);
void uniphier_cache_touch_range(u32 start, u32 end, u32 ways);
void uniphier_cache_touch_zero_range(u32 start, u32 end, u32 ways);

#endif /* __CACHE_UNIPHIER_H */
