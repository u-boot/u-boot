/*
 * Based on Linux i.MX iomux-v3.h file:
 * Copyright (C) 2009 by Jan Weitzel Phytec Messtechnik GmbH,
 *			<armlinux@phytec.de>
 *
 * Copyright (C) 2011 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef __MACH_IOMUX_V3_H__
#define __MACH_IOMUX_V3_H__

/*
 *	build IOMUX_PAD structure
 *
 * This iomux scheme is based around pads, which are the physical balls
 * on the processor.
 *
 * - Each pad has a pad control register (IOMUXC_SW_PAD_CTRL_x) which controls
 *   things like driving strength and pullup/pulldown.
 * - Each pad can have but not necessarily does have an output routing register
 *   (IOMUXC_SW_MUX_CTL_PAD_x).
 * - Each pad can have but not necessarily does have an input routing register
 *   (IOMUXC_x_SELECT_INPUT)
 *
 * The three register sets do not have a fixed offset to each other,
 * hence we order this table by pad control registers (which all pads
 * have) and put the optional i/o routing registers into additional
 * fields.
 *
 * The naming convention for the pad modes is SOC_PAD_<padname>__<padmode>
 * If <padname> or <padmode> refers to a GPIO, it is named GPIO_<unit>_<num>
 *
 * IOMUX/PAD Bit field definitions
 *
 * MUX_CTRL_OFS:	    0..11 (12)
 * PAD_CTRL_OFS:	   12..23 (12)
 * SEL_INPUT_OFS:	   24..35 (12)
 * MUX_MODE + SION:	   36..40  (5)
 * PAD_CTRL + NO_PAD_CTRL: 41..58 (18)
 * SEL_INP:		   59..62  (4)
 * reserved:		     63    (1)
*/

typedef u64 iomux_v3_cfg_t;

#define MUX_CTRL_OFS_SHIFT	0
#define MUX_CTRL_OFS_MASK	((iomux_v3_cfg_t)0xfff << MUX_CTRL_OFS_SHIFT)
#define MUX_PAD_CTRL_OFS_SHIFT	12
#define MUX_PAD_CTRL_OFS_MASK	((iomux_v3_cfg_t)0xfff << \
	MUX_PAD_CTRL_OFS_SHIFT)
#define MUX_SEL_INPUT_OFS_SHIFT	24
#define MUX_SEL_INPUT_OFS_MASK	((iomux_v3_cfg_t)0xfff << \
	MUX_SEL_INPUT_OFS_SHIFT)

#define MUX_MODE_SHIFT		36
#define MUX_MODE_MASK		((iomux_v3_cfg_t)0x1f << MUX_MODE_SHIFT)
#define MUX_PAD_CTRL_SHIFT	41
#define MUX_PAD_CTRL_MASK	((iomux_v3_cfg_t)0x3ffff << MUX_PAD_CTRL_SHIFT)
#define MUX_SEL_INPUT_SHIFT	59
#define MUX_SEL_INPUT_MASK	((iomux_v3_cfg_t)0xf << MUX_SEL_INPUT_SHIFT)

#define MUX_PAD_CTRL(x)		((iomux_v3_cfg_t)(x) << MUX_PAD_CTRL_SHIFT)

#define IOMUX_PAD(pad_ctrl_ofs, mux_ctrl_ofs, mux_mode, sel_input_ofs,	\
		sel_input, pad_ctrl)					\
	(((iomux_v3_cfg_t)(mux_ctrl_ofs) << MUX_CTRL_OFS_SHIFT)     |	\
	((iomux_v3_cfg_t)(mux_mode)      << MUX_MODE_SHIFT)         |	\
	((iomux_v3_cfg_t)(pad_ctrl_ofs)  << MUX_PAD_CTRL_OFS_SHIFT) |	\
	((iomux_v3_cfg_t)(pad_ctrl)      << MUX_PAD_CTRL_SHIFT)     |	\
	((iomux_v3_cfg_t)(sel_input_ofs) << MUX_SEL_INPUT_OFS_SHIFT)|	\
	((iomux_v3_cfg_t)(sel_input)     << MUX_SEL_INPUT_SHIFT))

#define NO_PAD_CTRL		(1 << 17)
#define GPIO_PIN_MASK		0x1f
#define GPIO_PORT_SHIFT		5
#define GPIO_PORT_MASK		(0x7 << GPIO_PORT_SHIFT)
#define GPIO_PORTA		(0 << GPIO_PORT_SHIFT)
#define GPIO_PORTB		(1 << GPIO_PORT_SHIFT)
#define GPIO_PORTC		(2 << GPIO_PORT_SHIFT)
#define GPIO_PORTD		(3 << GPIO_PORT_SHIFT)
#define GPIO_PORTE		(4 << GPIO_PORT_SHIFT)
#define GPIO_PORTF		(5 << GPIO_PORT_SHIFT)

#define MUX_CONFIG_SION		(0x1 << 4)

int imx_iomux_v3_setup_pad(iomux_v3_cfg_t pad);
int imx_iomux_v3_setup_multiple_pads(iomux_v3_cfg_t *pad_list, unsigned count);

#endif	/* __MACH_IOMUX_V3_H__*/
