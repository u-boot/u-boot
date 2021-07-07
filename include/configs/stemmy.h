/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019 Stephan Gerhold <stephan@gerhold.net>
 */
#ifndef __CONFIGS_STEMMY_H
#define __CONFIGS_STEMMY_H

#include <linux/sizes.h>

/*
 * The "stemmy" U-Boot port is designed to be chainloaded by the Samsung
 * bootloader on devices based on ST-Ericsson Ux500. Therefore, we skip most
 * low-level initialization and rely on configuration provided by the Samsung
 * bootloader. New images are loaded at the same address for compatibility.
 */
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_MALLOC_LEN		SZ_2M

/* FIXME: This should be loaded from device tree... */
#define CONFIG_SYS_L2_PL310
#define CONFIG_SYS_PL310_BASE		0xa0412000

/* Generate initrd atag for downstream kernel (others are copied in stemmy.c) */
#define CONFIG_INITRD_TAG

#endif
