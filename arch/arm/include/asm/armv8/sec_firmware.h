/*
 * Copyright 2016 NXP Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SEC_FIRMWARE_H_
#define __SEC_FIRMWARE_H_

#define PSCI_INVALID_VER		0xffffffff

int sec_firmware_init(const void *, u32 *, u32 *);
int _sec_firmware_entry(const void *, u32 *, u32 *);
bool sec_firmware_is_valid(const void *);
#ifdef CONFIG_SEC_FIRMWARE_ARMV8_PSCI
unsigned int sec_firmware_support_psci_version(void);
unsigned int _sec_firmware_support_psci_version(void);
#else
static inline unsigned int sec_firmware_support_psci_version(void)
{
	return PSCI_INVALID_VER;
}
#endif

#endif /* __SEC_FIRMWARE_H_ */
