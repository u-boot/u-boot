/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * Copyright (c) 2023, Svyatoslav Ryhel <clamor95@gmail.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "tegra114-common.h"

#ifdef CONFIG_TEGRA_SUPPORT_NON_SECURE
  #define CFG_PRAM			0x21c00	/* 135 MB */
#endif

#include "tegra-common-post.h"

#endif /* __CONFIG_H */
