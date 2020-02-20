/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Andes Technology Corporation
 * Ken Kuo, Andes Technology Corporation <ken_kuo@andestech.com>
 */
#ifndef __ASM_NDS_DMA_MAPPING_H
#define __ASM_NDS_DMA_MAPPING_H

#include <common.h>
#include <asm/cache.h>
#include <cpu_func.h>
#include <linux/dma-direction.h>
#include <linux/types.h>
#include <malloc.h>

static void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	*handle = (unsigned long)memalign(ARCH_DMA_MINALIGN, len);
	return (void *)*handle;
}

#endif /* __ASM_NDS_DMA_MAPPING_H */
