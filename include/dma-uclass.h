/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (C) 2015 Texas Instruments Incorporated <www.ti.com>
 * Written by Mugunthan V N <mugunthanvnm@ti.com>
 *
 */

#ifndef _DMA_UCLASS_H
#define _DMA_UCLASS_H

/* See dma.h for background documentation. */

#include <dma.h>

/*
 * struct dma_ops - Driver model DMA operations
 *
 * The uclass interface is implemented by all DMA devices which use
 * driver model.
 */
struct dma_ops {
	/**
	 * transfer() - Issue a DMA transfer. The implementation must
	 *   wait until the transfer is done.
	 *
	 * @dev: The DMA device
	 * @direction: direction of data transfer (should be one from
	 *   enum dma_direction)
	 * @dst: The destination pointer.
	 * @src: The source pointer.
	 * @len: Length of the data to be copied (number of bytes).
	 * @return zero on success, or -ve error code.
	 */
	int (*transfer)(struct udevice *dev, int direction, void *dst,
			void *src, size_t len);
};

#endif /* _DMA_UCLASS_H */
