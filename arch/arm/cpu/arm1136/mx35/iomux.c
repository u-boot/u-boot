/*
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx35_pins.h>
#include <asm/arch/iomux.h>

/*
 * IOMUX register (base) addresses
 */
enum iomux_reg_addr {
	IOMUXGPR = IOMUXC_BASE_ADDR,			/* General purpose */
	IOMUXSW_MUX_CTL = IOMUXC_BASE_ADDR + 4,		/* MUX control */
	IOMUXSW_MUX_END = IOMUXC_BASE_ADDR + 0x324,	/* last MUX control */
	IOMUXSW_PAD_CTL = IOMUXC_BASE_ADDR + 0x328,	/* Pad control */
	IOMUXSW_PAD_END = IOMUXC_BASE_ADDR + 0x794,	/* last Pad control */
	IOMUXSW_INPUT_CTL = IOMUXC_BASE_ADDR + 0x7AC,	/* input select */
	IOMUXSW_INPUT_END = IOMUXC_BASE_ADDR + 0x9F4,	/* last input select */
};

#define MUX_PIN_NUM_MAX		\
		(((IOMUXSW_PAD_END - IOMUXSW_PAD_CTL) >> 2) + 1)
#define MUX_INPUT_NUM_MUX	\
		(((IOMUXSW_INPUT_END - IOMUXSW_INPUT_CTL) >> 2) + 1)

/*
 * Request ownership for an IO pin. This function has to be the first one
 * being called before that pin is used.
 */
void mxc_request_iomux(iomux_pin_name_t pin, iomux_pin_cfg_t cfg)
{
	u32 mux_reg = PIN_TO_IOMUX_MUX(pin);

	if (mux_reg != NON_MUX_I) {
		mux_reg += IOMUXGPR;
		writel(cfg, mux_reg);
	}
}

/*
 * Release ownership for an IO pin
 */
void mxc_free_iomux(iomux_pin_name_t pin, iomux_pin_cfg_t cfg)
{
}

/*
 * This function configures the pad value for a IOMUX pin.
 *
 * @param  pin     a pin number as defined in iomux_pin_name_t
 * @param  config  the ORed value of elements defined in iomux_pad_config_t
 */
void mxc_iomux_set_pad(iomux_pin_name_t pin, u32 config)
{
	u32 pad_reg = IOMUXGPR + PIN_TO_IOMUX_PAD(pin);

	writel(config, pad_reg);
}

/*
 * This function enables/disables the general purpose function for a particular
 * signal.
 *
 * @param  gp   one signal as defined in iomux_gp_func_t
 * @param  en   enable/disable
 */
void mxc_iomux_set_gpr(iomux_gp_func_t gp, int en)
{
	u32 l;

	l = readl(IOMUXGPR);
	if (en)
		l |= gp;
	else
		l &= ~gp;

	writel(l, IOMUXGPR);
}

/*
 * This function configures input path.
 *
 * @param input index of input select register as defined in
 *			iomux_input_select_t
 * @param config the binary value of elements defined in
 *			iomux_input_config_t
 */
void mxc_iomux_set_input(iomux_input_select_t input, u32 config)
{
	u32 reg = IOMUXSW_INPUT_CTL + (input << 2);

	writel(config, reg);
}
