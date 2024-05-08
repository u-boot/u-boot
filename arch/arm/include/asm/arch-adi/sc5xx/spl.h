/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */
#ifndef ARCH_ADI_SC5XX_SPL_H
#define ARCH_ADI_SC5XX_SPL_H

#include <linux/types.h>

struct adi_boot_args {
	phys_addr_t addr;
	u32 flags;
	u32 cmd;
};

extern u32 bmode;

/**
 * This table stores the arguments to the rom boot function per bootmode,
 * and it is populated per SoC in the corresponding SoC support file (sc7x, sc58x,
 * and so on).
 */
extern const struct adi_boot_args adi_rom_boot_args[8];

/**
 * Struct layout for the boot config is also specific to an SoC, so you should
 * only access it inside an SoC-specific boot hook function, which will be called
 * from the boot rom while going from SPL to proper u-boot
 */
struct ADI_ROM_BOOT_CONFIG;
int32_t adi_rom_boot_hook(struct ADI_ROM_BOOT_CONFIG *cfg, int32_t cause);

typedef void (*adi_rom_boot_fn)(void *address, uint32_t flags, int32_t count,
				void *hook, uint32_t command);

extern adi_rom_boot_fn adi_rom_boot;

#endif
