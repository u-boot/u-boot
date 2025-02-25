// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */
#include <wait_bit.h>
#include <asm/arch/base_addr_soc64.h>
#include <linux/bitfield.h>

#define CCU_DMI0_DMIUSMCTCR				SOCFPGA_CCU_ADDRESS + 0x7300
#define CCU_DMI0_DMIUSMCMCR				SOCFPGA_CCU_ADDRESS + 0x7340
#define CCU_DMI0_DMIUSMCMAR				SOCFPGA_CCU_ADDRESS + 0x7344
#define CCU_DMI0_DMIUSMCMCR_MNTOP		GENMASK(3, 0)
#define MAX_DISTRIBUTED_MEM_INTERFACE	2
#define FLUSH_ALL_ENTRIES				0x4
#define CCU_DMI0_DMIUSMCMCR_ARRAY_ID	GENMASK(21, 16)
#define ARRAY_ID_TAG					0x0
#define ARRAY_ID_DATA					0x1
#define CACHE_OPERATION_DONE			BIT(0)
#define TIMEOUT_200MS					200

int __asm_flush_l3_dcache(void)
{
	int i;
	int ret = 0;

	/* Flushing all entries in CCU system memory cache */
	for (i = 0; i < MAX_DISTRIBUTED_MEM_INTERFACE; i++) {
		/*
		 * Skipping if the system memory cache is not enabled for
		 * particular DMI
		 */
		if (!readl((uintptr_t)(CCU_DMI0_DMIUSMCTCR + (i * 0x1000))))
			continue;

		writel(FIELD_PREP(CCU_DMI0_DMIUSMCMCR_MNTOP, FLUSH_ALL_ENTRIES) |
			   FIELD_PREP(CCU_DMI0_DMIUSMCMCR_ARRAY_ID, ARRAY_ID_TAG),
			   (uintptr_t)(CCU_DMI0_DMIUSMCMCR + (i * 0x1000)));

		/* Wait for cache maintenance operation done */
		ret = wait_for_bit_le32((const void *)(uintptr_t)(CCU_DMI0_DMIUSMCMAR +
				(i * 0x1000)), CACHE_OPERATION_DONE, false, TIMEOUT_200MS,
				false);
		if (ret) {
			debug("%s: Timeout while waiting for flushing tag in DMI%d done\n",
			      __func__, i);
			return ret;
		}

		writel(FIELD_PREP(CCU_DMI0_DMIUSMCMCR_MNTOP, FLUSH_ALL_ENTRIES) |
			   FIELD_PREP(CCU_DMI0_DMIUSMCMCR_ARRAY_ID, ARRAY_ID_DATA),
			   (uintptr_t)(CCU_DMI0_DMIUSMCMCR + (i * 0x1000)));

		/* Wait for cache maintenance operation done */
		ret = wait_for_bit_le32((const void *)(uintptr_t)(CCU_DMI0_DMIUSMCMAR +
				(i * 0x1000)), CACHE_OPERATION_DONE, false, TIMEOUT_200MS,
				false);
		if (ret)
			debug("%s: Timeout waiting for flushing data in DMI%d done\n",
			      __func__, i);
	}

	return ret;
}
