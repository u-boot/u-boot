/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef _DMA_H_
#define _DMA_H_

/*
 * enum dma_direction - dma transfer direction indicator
 * @DMA_MEM_TO_MEM: Memcpy mode
 * @DMA_MEM_TO_DEV: From Memory to Device
 * @DMA_DEV_TO_MEM: From Device to Memory
 * @DMA_DEV_TO_DEV: From Device to Device
 */
enum dma_direction {
	DMA_MEM_TO_MEM,
	DMA_MEM_TO_DEV,
	DMA_DEV_TO_MEM,
	DMA_DEV_TO_DEV,
};

#define DMA_SUPPORTS_MEM_TO_MEM	BIT(0)
#define DMA_SUPPORTS_MEM_TO_DEV	BIT(1)
#define DMA_SUPPORTS_DEV_TO_MEM	BIT(2)
#define DMA_SUPPORTS_DEV_TO_DEV	BIT(3)

/*
 * struct dma_dev_priv - information about a device used by the uclass
 *
 * @supported: mode of transfers that DMA can support, should be
 *	       one/multiple of DMA_SUPPORTS_*
 */
struct dma_dev_priv {
	u32 supported;
};

/*
 * dma_get_device - get a DMA device which supports transfer
 * type of transfer_type
 *
 * @transfer_type - transfer type should be one/multiple of
 *		    DMA_SUPPORTS_*
 * @devp - udevice pointer to return the found device
 * @return - will return on success and devp will hold the
 *	     pointer to the device
 */
int dma_get_device(u32 transfer_type, struct udevice **devp);

/*
 * dma_memcpy - try to use DMA to do a mem copy which will be
 *		much faster than CPU mem copy
 *
 * @dst - destination pointer
 * @src - souce pointer
 * @len - data length to be copied
 * @return - on successful transfer returns no of bytes
	     transferred and on failure return error code.
 */
int dma_memcpy(void *dst, void *src, size_t len);

#endif	/* _DMA_H_ */
