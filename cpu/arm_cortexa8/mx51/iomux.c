/*
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
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
#include <asm/arch/mx51_pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>

/* IOMUX register (base) addresses */
enum iomux_reg_addr {
	IOMUXGPR0 = IOMUXC_BASE_ADDR,
	IOMUXGPR1 = IOMUXC_BASE_ADDR + 0x004,
	IOMUXSW_MUX_CTL = IOMUXC_BASE_ADDR,
	IOMUXSW_MUX_END = IOMUXC_BASE_ADDR + MUX_I_END,
	IOMUXSW_PAD_CTL = IOMUXC_BASE_ADDR + PAD_I_START,
	IOMUXSW_INPUT_CTL = IOMUXC_BASE_ADDR,
};

#define MUX_PIN_NUM_MAX (((MUX_I_END - MUX_I_START) >> 2) + 1)

/* Get the iomux register address of this pin */
static inline u32 get_mux_reg(iomux_pin_name_t pin)
{
	u32 mux_reg = PIN_TO_IOMUX_MUX(pin);

	if (is_soc_rev(CHIP_REV_2_0) < 0) {
		/*
		 * Fixup register address:
		 *	i.MX51 TO1 has offset with the register
		 * 	which is define as TO2.
		 */
		if ((pin == MX51_PIN_NANDF_RB5) ||
			(pin == MX51_PIN_NANDF_RB6) ||
			(pin == MX51_PIN_NANDF_RB7))
			; /* Do nothing */
		else if (mux_reg >= 0x2FC)
			mux_reg += 8;
		else if (mux_reg >= 0x130)
			mux_reg += 0xC;
	}
	mux_reg += IOMUXSW_MUX_CTL;
	return mux_reg;
}

/* Get the pad register address of this pin */
static inline u32 get_pad_reg(iomux_pin_name_t pin)
{
	u32 pad_reg = PIN_TO_IOMUX_PAD(pin);

	if (is_soc_rev(CHIP_REV_2_0) < 0) {
		/*
		 * Fixup register address:
		 *	i.MX51 TO1 has offset with the register
		 * 	which is define as TO2.
		 */
		if ((pin == MX51_PIN_NANDF_RB5) ||
			(pin == MX51_PIN_NANDF_RB6) ||
			(pin == MX51_PIN_NANDF_RB7))
			; /* Do nothing */
		else if (pad_reg == 0x4D0 - PAD_I_START)
			pad_reg += 0x4C;
		else if (pad_reg == 0x860 - PAD_I_START)
			pad_reg += 0x9C;
		else if (pad_reg >= 0x804 - PAD_I_START)
			pad_reg += 0xB0;
		else if (pad_reg >= 0x7FC - PAD_I_START)
			pad_reg += 0xB4;
		else if (pad_reg >= 0x4E4 - PAD_I_START)
			pad_reg += 0xCC;
		else
			pad_reg += 8;
	}
	pad_reg += IOMUXSW_PAD_CTL;
	return pad_reg;
}

/* Get the last iomux register address */
static inline u32 get_mux_end(void)
{
	if (is_soc_rev(CHIP_REV_2_0) < 0)
		return IOMUXC_BASE_ADDR + (0x3F8 - 4);
	else
		return IOMUXC_BASE_ADDR + (0x3F0 - 4);
}

/*
 * This function is used to configure a pin through the IOMUX module.
 * @param  pin		a pin number as defined in iomux_pin_name_t
 * @param  cfg		an output function as defined in iomux_pin_cfg_t
 *
 * @return 		0 if successful; Non-zero otherwise
 */
static void iomux_config_mux(iomux_pin_name_t pin, iomux_pin_cfg_t cfg)
{
	u32 mux_reg = get_mux_reg(pin);

	if ((mux_reg > get_mux_end()) || (mux_reg < IOMUXSW_MUX_CTL))
		return ;
	if (cfg == IOMUX_CONFIG_GPIO)
		writel(PIN_TO_ALT_GPIO(pin), mux_reg);
	else
		writel(cfg, mux_reg);
}

/*
 * Request ownership for an IO pin. This function has to be the first one
 * being called before that pin is used. The caller has to check the
 * return value to make sure it returns 0.
 *
 * @param  pin		a name defined by iomux_pin_name_t
 * @param  cfg		an input function as defined in iomux_pin_cfg_t
 *
 */
void mxc_request_iomux(iomux_pin_name_t pin, iomux_pin_cfg_t cfg)
{
	iomux_config_mux(pin, cfg);
}

/*
 * Release ownership for an IO pin
 *
 * @param  pin		a name defined by iomux_pin_name_t
 * @param  cfg		an input function as defined in iomux_pin_cfg_t
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
	u32 pad_reg = get_pad_reg(pin);
	writel(config, pad_reg);
}

unsigned int mxc_iomux_get_pad(iomux_pin_name_t pin)
{
	u32 pad_reg = get_pad_reg(pin);
	return readl(pad_reg);
}
