/*
 * Tegra pulse width frequency modulator definitions
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_TEGRA_PWM_H
#define __ASM_ARCH_TEGRA_PWM_H

/* This is a single PWM channel */
struct pwm_ctlr {
	uint control;		/* Control register */
	uint reserved[3];	/* Space space */
};

#define PWM_NUM_CHANNELS	4

/* PWM_CONTROLLER_PWM_CSR_0/1/2/3_0 */
#define PWM_ENABLE_SHIFT	31
#define PWM_ENABLE_MASK	(0x1 << PWM_ENABLE_SHIFT)

#define PWM_WIDTH_SHIFT	16
#define PWM_WIDTH_MASK		(0x7FFF << PWM_WIDTH_SHIFT)

#define PWM_DIVIDER_SHIFT	0
#define PWM_DIVIDER_MASK	(0x1FFF << PWM_DIVIDER_SHIFT)

/**
 * Program the PWM with the given parameters.
 *
 * @param channel	PWM channel to update
 * @param rate		Clock rate to use for PWM
 * @param pulse_width	high pulse width: 0=always low, 1=1/256 pulse high,
 *			n = n/256 pulse high
 * @param freq_divider	frequency divider value (1 to use rate as is)
 */
void pwm_enable(unsigned channel, int rate, int pulse_width, int freq_divider);

/**
 * Request a pwm channel as referenced by a device tree node.
 *
 * This channel can then be passed to pwm_enable().
 *
 * @param blob		Device tree blob
 * @param node		Node containing reference to pwm
 * @param prop_name	Property name of pwm reference
 * @return channel number, if ok, else -1
 */
int pwm_request(const void *blob, int node, const char *prop_name);

/**
 * Set up the pwm controller, by looking it up in the fdt.
 *
 * @return 0 if ok, -1 if the device tree node was not found or invalid.
 */
int pwm_init(const void *blob);

#endif	/* __ASM_ARCH_TEGRA_PWM_H */
