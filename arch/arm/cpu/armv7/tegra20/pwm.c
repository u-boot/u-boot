/*
 * Tegra2 pulse width frequency modulator definitions
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/pwm.h>

struct pwm_info {
	struct pwm_ctlr *pwm;		/* Registers for our pwm controller */
	int pwm_node;			/* PWM device tree node */
} local;

void pwm_enable(unsigned channel, int rate, int pulse_width, int freq_divider)
{
	u32 reg;

	assert(channel < PWM_NUM_CHANNELS);

	/* TODO: Can we use clock_adjust_periph_pll_div() here? */
	clock_start_periph_pll(PERIPH_ID_PWM, CLOCK_ID_SFROM32KHZ, rate);

	reg = PWM_ENABLE_MASK;
	reg |= pulse_width << PWM_WIDTH_SHIFT;
	reg |= freq_divider << PWM_DIVIDER_SHIFT;
	writel(reg, &local.pwm[channel].control);
	debug("%s: channel=%d, rate=%d\n", __func__, channel, rate);
}

int pwm_request(const void *blob, int node, const char *prop_name)
{
	int pwm_node;
	u32 data[3];

	if (fdtdec_get_int_array(blob, node, prop_name, data,
			ARRAY_SIZE(data))) {
		debug("%s: Cannot decode PWM property '%s'\n", __func__,
		      prop_name);
		return -1;
	}

	pwm_node = fdt_node_offset_by_phandle(blob, data[0]);
	if (pwm_node != local.pwm_node) {
		debug("%s: PWM property '%s' phandle %d not recognised"
		      "- expecting %d\n", __func__, prop_name, data[0],
		      local.pwm_node);
		return -1;
	}
	if (data[1] >= PWM_NUM_CHANNELS) {
		debug("%s: PWM property '%s': invalid channel %u\n", __func__,
		      prop_name, data[1]);
		return -1;
	}

	/*
	 * TODO: We could maintain a list of requests, but it might not be
	 * worth it for U-Boot.
	 */
	return data[1];
}

int pwm_init(const void *blob)
{
	local.pwm_node = fdtdec_next_compatible(blob, 0,
						COMPAT_NVIDIA_TEGRA20_PWM);
	if (local.pwm_node < 0) {
		debug("%s: Cannot find device tree node\n", __func__);
		return -1;
	}

	local.pwm = (struct pwm_ctlr *)fdtdec_get_addr(blob, local.pwm_node,
						       "reg");
	if (local.pwm == (struct pwm_ctlr *)FDT_ADDR_T_NONE) {
		debug("%s: Cannot find pwm reg address\n", __func__);
		return -1;
	}
	debug("Tegra PWM at %p, node %d\n", local.pwm, local.pwm_node);

	return 0;
}
