/*
 * Copyright (c) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_MICROCODE_H
#define __ASM_ARCH_MICROCODE_H

/* Length of the public header on Intel microcode blobs */
#define UCODE_HEADER_LEN	0x30

#ifndef __ASSEMBLY__

/**
 * microcode_update_intel() - Apply microcode updates
 *
 * Applies any microcode updates in the device tree.
 *
 * @return 0 if OK, -EEXIST if the updates were already applied, -ENOENT if
 * not updates were found, -EINVAL if an update was invalid
 */
int microcode_update_intel(void);
#endif /* __ASSEMBLY__ */

#endif
