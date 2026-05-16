// SPDX-License-Identifier: GPL-2.0

#ifndef __QCOM_PRIV_H__
#define __QCOM_PRIV_H__

#include <stdbool.h>

/**
 * enum qcom_boot_source - Track where we got loaded from.
 * Used for capsule update logic.
 *
 * @QCOM_BOOT_SOURCE_ANDROID: chainloaded (typically from ABL)
 * @QCOM_BOOT_SOURCE_XBL: flashed to the XBL or UEFI partition
 */
enum qcom_boot_source {
	QCOM_BOOT_SOURCE_ANDROID = 1,
	QCOM_BOOT_SOURCE_XBL,
};

extern enum qcom_boot_source qcom_boot_source;

/*
 * enum qcom_memmap_source - Track where we got the memory map from.
 * used for debugging and validation.
 */
enum qcom_memmap_source {
	QCOM_MEMMAP_SOURCE_INTERNAL_FDT = 1,
	QCOM_MEMMAP_SOURCE_EXTERNAL_FDT,
	QCOM_MEMMAP_SOURCE_SMEM,
};

/* Set by qcom_parse_memory() */
extern enum qcom_memmap_source qcom_memmap_source;

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
void qcom_configure_capsule_updates(void);
#else
static inline void qcom_configure_capsule_updates(void) {}
#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int qcom_parse_memory(const void *fdt, bool fdt_is_internal);

#endif /* __QCOM_PRIV_H__ */
