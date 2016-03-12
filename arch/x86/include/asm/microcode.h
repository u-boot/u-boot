/*
 * Copyright (c) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_MICROCODE_H
#define __ASM_ARCH_MICROCODE_H

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

/**
 * microcode_read_rev() - Read the microcode version
 *
 * This reads the microcode version of the currently running CPU
 *
 * @return microcode version number
 */
int microcode_read_rev(void);
#endif /* __ASSEMBLY__ */

#endif
