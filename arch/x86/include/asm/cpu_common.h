/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __ASM_CPU_COMMON_H
#define __ASM_CPU_COMMON_H

/* Standard Intel bus clock is fixed at 100MHz */
enum {
	INTEL_BCLK_MHZ		= 100
};

struct cpu_info;

/**
 * cpu_common_init() - Set up common CPU init
 *
 * This reports BIST failure, enables the LAPIC, updates microcode, enables
 * the upper 128-bytes of CROM RAM, probes the northbridge, PCH, LPC and SATA.
 *
 * @return 0 if OK, -ve on error
 */
int cpu_common_init(void);

/**
 * cpu_set_flex_ratio_to_tdp_nominal() - Set up the maximum non-turbo rate
 *
 * If a change is needed, this function will do a soft reset so it takes
 * effect.
 *
 * Some details are available here:
 * http://forum.hwbot.org/showthread.php?t=76092
 *
 * @return 0 if OK, -ve on error
 */
int cpu_set_flex_ratio_to_tdp_nominal(void);

/**
 * cpu_intel_get_info() - Obtain CPU info for Intel CPUs
 *
 * Most Intel CPUs use the same MSR to obtain the clock speed, and use the same
 * features. This function fills in these values, given the value of the base
 * clock in MHz (typically this should be set to 100).
 *
 * @info:	cpu_info struct to fill in
 * @bclk_mz:	the base clock in MHz
 *
 * @return 0 always
 */
int cpu_intel_get_info(struct cpu_info *info, int bclk_mz);

/**
 * cpu_configure_thermal_target() - Set the thermal target for a CPU
 *
 * This looks up the tcc-offset property and uses it to set the
 * MSR_TEMPERATURE_TARGET value.
 *
 * @dev: CPU device
 * @return 0 if OK, -ENOENT if no target is given in device tree
 */
int cpu_configure_thermal_target(struct udevice *dev);

#endif
