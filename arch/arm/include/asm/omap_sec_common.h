/*
 * (C) Copyright 2016
 * Texas Instruments, <www.ti.com>
 *
 * Andreas Dannenberg <dannenberg@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef	_OMAP_SEC_COMMON_H_
#define	_OMAP_SEC_COMMON_H_

#include <common.h>

/*
 * Invoke secure ROM API on high-security (HS) device variants. It formats
 * the variable argument list into the format expected by the ROM code before
 * triggering the actual low-level smc entry.
 */
u32 secure_rom_call(u32 service, u32 proc_id, u32 flag, ...);

/*
 * Invoke a secure ROM API on high-secure (HS) device variants that can be used
 * to verify a secure blob by authenticating and optionally decrypting it. The
 * exact operation performed depends on how the certificate that was embedded
 * into the blob during the signing/encryption step when the secure blob was
 * first created.
 */
int secure_boot_verify_image(void **p_image, size_t *p_size);

#endif /* _OMAP_SEC_COMMON_H_ */
