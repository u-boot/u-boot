/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARC_CACHE_H
#define __ASM_ARC_CACHE_H

#include <config.h>

#ifdef CONFIG_ARC_CACHE_LINE_SHIFT
#define CONFIG_SYS_CACHELINE_SIZE	(1 << CONFIG_ARC_CACHE_LINE_SHIFT)
#define ARCH_DMA_MINALIGN		CONFIG_SYS_CACHELINE_SIZE
#else
/* Satisfy users of ARCH_DMA_MINALIGN */
#define ARCH_DMA_MINALIGN		128
#endif

#if defined(ARC_MMU_ABSENT)
#define CONFIG_ARC_MMU_VER 0
#elif defined(CONFIG_ARC_MMU_V2)
#define CONFIG_ARC_MMU_VER 2
#elif defined(CONFIG_ARC_MMU_V3)
#define CONFIG_ARC_MMU_VER 3
#elif defined(CONFIG_ARC_MMU_V4)
#define CONFIG_ARC_MMU_VER 4
#endif

#endif /* __ASM_ARC_CACHE_H */
