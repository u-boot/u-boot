/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Common code for Intel CPUs
 *
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

/**
 * cpu_set_perf_control() - Set the nominal CPU clock speed
 *
 * This sets the clock speed as a multiplier of BCLK
 *
 * @clk_ratio: Ratio to use
 */
void cpu_set_perf_control(uint clk_ratio);

/**
 * cpu_config_tdp_levels() - Check for configurable TDP option
 *
 * @return true if the CPU has configurable TDP (Thermal-design power)
 */
bool cpu_config_tdp_levels(void);

/** enum burst_mode_t - Burst-mode states */
enum burst_mode_t {
	BURST_MODE_UNKNOWN,
	BURST_MODE_UNAVAILABLE,
	BURST_MODE_DISABLED,
	BURST_MODE_ENABLED
};

/*
 * cpu_get_burst_mode_state() - Get the Burst/Turbo Mode State
 *
 * This reads MSR IA32_MISC_ENABLE 0x1A0
 * Bit 38 - TURBO_MODE_DISABLE Bit to get state ENABLED / DISABLED.
 * Also checks cpuid 0x6 to see whether burst mode is supported.
 *
 * @return current burst mode status
 */
enum burst_mode_t cpu_get_burst_mode_state(void);

/**
 * cpu_set_burst_mode() - Set CPU burst mode
 *
 * @burst_mode: true to enable burst mode, false to disable
 */
void cpu_set_burst_mode(bool burst_mode);

/**
 * cpu_set_eist() - Enable Enhanced Intel Speed Step Technology
 *
 * @eist_status: true to enable EIST, false to disable
 */
void cpu_set_eist(bool eist_status);

/**
 * cpu_set_p_state_to_turbo_ratio() - Set turbo ratio
 *
 * TURBO_RATIO_LIMIT MSR (0x1AD) Bits 31:0 indicates the
 * factory configured values for of 1-core, 2-core, 3-core
 * and 4-core turbo ratio limits for all processors.
 *
 * 7:0 -	MAX_TURBO_1_CORE
 * 15:8 -	MAX_TURBO_2_CORES
 * 23:16 -	MAX_TURBO_3_CORES
 * 31:24 -	MAX_TURBO_4_CORES
 *
 * Set PERF_CTL MSR (0x199) P_Req with that value.
 */
void cpu_set_p_state_to_turbo_ratio(void);

#endif
