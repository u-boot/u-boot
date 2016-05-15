/*
 * Copyright (C) 2004-2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_U_BOOT_H__
#define __ASM_U_BOOT_H__ 1

/* Use the generic board which requires a unified bd_info */
#include <asm-generic/u-boot.h>

/* For image.h:image_check_target_arch() */
#define IH_ARCH_DEFAULT IH_ARCH_AVR32

int arch_cpu_init(void);
int dram_init(void);

#endif /* __ASM_U_BOOT_H__ */
