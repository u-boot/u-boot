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

extern unsigned long mmc_berase(int dev_num, lbaint_t start, lbaint_t blkcnt);

extern ulong mmc_bwrite(int dev_num, lbaint_t start, lbaint_t blkcnt,
		const void *src);

#else /* CONFIG_SPL_BUILD */

/* SPL will never write or erase, declare dummies to reduce code size. */

static inline unsigned long mmc_berase(int dev_num, lbaint_t start,
		lbaint_t blkcnt)
{
	return 0;
}

static inline ulong mmc_bwrite(int dev_num, lbaint_t start, lbaint_t blkcnt,
		const void *src)
{
	return 0;
}

#endif /* CONFIG_SPL_BUILD */

#endif /* _MMC_PRIVATE_H_ */
