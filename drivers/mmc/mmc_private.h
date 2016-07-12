/*
 * Copyright 2008,2010 Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based (loosely) on the Linux code
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MMC_PRIVATE_H_
#define _MMC_PRIVATE_H_

#include <mmc.h>

extern int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data);
extern int mmc_send_status(struct mmc *mmc, int timeout);
extern int mmc_set_blocklen(struct mmc *mmc, int len);
#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
void mmc_adapter_card_type_ident(void);
#endif

#ifndef CONFIG_SPL_BUILD

unsigned long mmc_berase(struct blk_desc *block_dev, lbaint_t start,
			 lbaint_t blkcnt);

#ifdef CONFIG_BLK
ulong mmc_bwrite(struct udevice *dev, lbaint_t start, lbaint_t blkcnt,
		 const void *src);
#else
ulong mmc_bwrite(struct blk_desc *block_dev, lbaint_t start, lbaint_t blkcnt,
		 const void *src);
#endif

#else /* CONFIG_SPL_BUILD */

/* SPL will never write or erase, declare dummies to reduce code size. */

#ifdef CONFIG_BLK
static inline unsigned long mmc_berase(struct udevice *dev,
				       lbaint_t start, lbaint_t blkcnt)
{
	return 0;
}

static inline ulong mmc_bwrite(struct udevice *dev, lbaint_t start,
			       lbaint_t blkcnt, const void *src)
{
	return 0;
}
#else
static inline unsigned long mmc_berase(struct blk_desc *block_dev,
				       lbaint_t start, lbaint_t blkcnt)
{
	return 0;
}

static inline ulong mmc_bwrite(struct blk_desc *block_dev, lbaint_t start,
			       lbaint_t blkcnt, const void *src)
{
	return 0;
}
#endif

#endif /* CONFIG_SPL_BUILD */

/**
 * mmc_get_next_devnum() - Get the next available MMC device number
 *
 * @return next available device number (0 = first), or -ve on error
 */
int mmc_get_next_devnum(void);

/**
 * mmc_do_preinit() - Get an MMC device ready for use
 */
void mmc_do_preinit(void);

/**
 * mmc_list_init() - Set up the list of MMC devices
 */
void mmc_list_init(void);

/**
 * mmc_list_add() - Add a new MMC device to the list of devices
 *
 * @mmc:	Device to add
 */
void mmc_list_add(struct mmc *mmc);

#endif /* _MMC_PRIVATE_H_ */
