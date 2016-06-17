/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARC_CACHE_H
#define __ASM_ARC_CACHE_H

#include <config.h>

/*
 * As of today we may handle any L1 cache line length right in software.
 * For that essentially cache line length is a variable not constant.
 * And to satisfy users of ARCH_DMA_MINALIGN we just use largest line length
 * that may exist in either L1 or L2 (AKA SLC) caches on ARC.
 */
#define ARCH_DMA_MINALIGN	128

#if defined(ARC_MMU_ABSENT)
#define CONFIG_ARC_MMU_VER 0
#elif defined(CONFIG_ARC_MMU_V2)
#define CONFIG_ARC_MMU_VER 2
#elif defined(CONFIG_ARC_MMU_V3)
#define CONFIG_ARC_MMU_VER 3
#elif defined(CONFIG_ARC_MMU_V4)
#define CONFIG_ARC_MMU_VER 4
#endif

#ifndef __ASSEMBLY__

void cache_init(void);

#endif /* __ASSEMBLY__ */

#endif /* __ASM_ARC_CACHE_H */
