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

/*
 * IOMUXC_GPR13 bit fields
 */
#define IOMUXC_GPR13_SDMA_STOP_REQ	(1<<30)
#define IOMUXC_GPR13_CAN2_STOP_REQ	(1<<29)
#define IOMUXC_GPR13_CAN1_STOP_REQ	(1<<28)
#define IOMUXC_GPR13_ENET_STOP_REQ	(1<<27)
#define IOMUXC_GPR13_SATA_PHY_8_MASK	(7<<24)
#define IOMUXC_GPR13_SATA_PHY_7_MASK	(0x1f<<19)
#define IOMUXC_GPR13_SATA_PHY_6_SHIFT	16
#define IOMUXC_GPR13_SATA_PHY_6_MASK	(7<<IOMUXC_GPR13_SATA_PHY_6_SHIFT)
#define IOMUXC_GPR13_SATA_SPEED_MASK	(1<<15)
#define IOMUXC_GPR13_SATA_PHY_5_MASK	(1<<14)
#define IOMUXC_GPR13_SATA_PHY_4_MASK	(7<<11)
#define IOMUXC_GPR13_SATA_PHY_3_MASK	(0x1f<<7)
#define IOMUXC_GPR13_SATA_PHY_2_MASK	(0x1f<<2)
#define IOMUXC_GPR13_SATA_PHY_1_MASK	(3<<0)

#define IOMUXC_GPR13_SATA_PHY_8_RXEQ_0P5DB	(0b000<<24)
#define IOMUXC_GPR13_SATA_PHY_8_RXEQ_1P0DB	(0b001<<24)
#define IOMUXC_GPR13_SATA_PHY_8_RXEQ_1P5DB	(0b010<<24)
#define IOMUXC_GPR13_SATA_PHY_8_RXEQ_2P0DB	(0b011<<24)
#define IOMUXC_GPR13_SATA_PHY_8_RXEQ_2P5DB	(0b100<<24)
#define IOMUXC_GPR13_SATA_PHY_8_RXEQ_3P0DB	(0b101<<24)
#define IOMUXC_GPR13_SATA_PHY_8_RXEQ_3P5DB	(0b110<<24)
#define IOMUXC_GPR13_SATA_PHY_8_RXEQ_4P0DB	(0b111<<24)

#define IOMUXC_GPR13_SATA_PHY_7_SATA1I	(0b10000<<19)
#define IOMUXC_GPR13_SATA_PHY_7_SATA1M	(0b10000<<19)
#define IOMUXC_GPR13_SATA_PHY_7_SATA1X	(0b11010<<19)
#define IOMUXC_GPR13_SATA_PHY_7_SATA2I	(0b10010<<19)
#define IOMUXC_GPR13_SATA_PHY_7_SATA2M	(0b10010<<19)
#define IOMUXC_GPR13_SATA_PHY_7_SATA2X	(0b11010<<19)

#define IOMUXC_GPR13_SATA_SPEED_1P5G	(0<<15)
#define IOMUXC_GPR13_SATA_SPEED_3G	(1<<15)

#define IOMUXC_GPR13_SATA_SATA_PHY_5_SS_DISABLED	(0<<14)
#define IOMUXC_GPR13_SATA_SATA_PHY_5_SS_ENABLED		(1<<14)

#define IOMUXC_GPR13_SATA_SATA_PHY_4_ATTEN_16_16	(0<<11)
#define IOMUXC_GPR13_SATA_SATA_PHY_4_ATTEN_14_16	(1<<11)
#define IOMUXC_GPR13_SATA_SATA_PHY_4_ATTEN_12_16	(2<<11)
#define IOMUXC_GPR13_SATA_SATA_PHY_4_ATTEN_10_16	(3<<11)
#define IOMUXC_GPR13_SATA_SATA_PHY_4_ATTEN_9_16		(4<<11)
#define IOMUXC_GPR13_SATA_SATA_PHY_4_ATTEN_8_16		(5<<11)

#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_0P00_DB	(0b0000<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_0P37_DB	(0b0001<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_0P74_DB	(0b0010<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_1P11_DB	(0b0011<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_1P48_DB	(0b0100<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_1P85_DB	(0b0101<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_2P22_DB	(0b0110<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_2P59_DB	(0b0111<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_2P96_DB	(0b1000<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_3P33_DB	(0b1001<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_3P70_DB	(0b1010<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_4P07_DB	(0b1011<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_4P44_DB	(0b1100<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_4P81_DB	(0b1101<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_5P28_DB	(0b1110<<7)
#define IOMUXC_GPR13_SATA_PHY_3_TXBOOST_5P75_DB	(0b1111<<7)

#define IOMUXC_GPR13_SATA_PHY_2_TX_0P937V	(0b00000<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_0P947V	(0b00001<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_0P957V	(0b00010<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_0P966V	(0b00011<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_0P976V	(0b00100<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_0P986V	(0b00101<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_0P996V	(0b00110<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P005V	(0b00111<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P015V	(0b01000<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P025V	(0b01001<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P035V	(0b01010<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P045V	(0b01011<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P054V	(0b01100<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P064V	(0b01101<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P074V	(0b01110<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P084V	(0b01111<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P094V	(0b10000<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P104V	(0b10001<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P113V	(0b10010<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P123V	(0b10011<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P133V	(0b10100<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P143V	(0b10101<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P152V	(0b10110<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P162V	(0b10111<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P172V	(0b11000<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P182V	(0b11001<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P191V	(0b11010<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P201V	(0b11011<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P211V	(0b11100<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P221V	(0b11101<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P230V	(0b11110<<2)
#define IOMUXC_GPR13_SATA_PHY_2_TX_1P240V	(0b11111<<2)

#define IOMUXC_GPR13_SATA_PHY_1_FAST	0
#define IOMUXC_GPR13_SATA_PHY_1_MEDIUM	1
#define IOMUXC_GPR13_SATA_PHY_1_SLOW	2

#define IOMUXC_GPR13_SATA_MASK (IOMUXC_GPR13_SATA_PHY_8_MASK \
				|IOMUXC_GPR13_SATA_PHY_7_MASK \
				|IOMUXC_GPR13_SATA_PHY_6_MASK \
				|IOMUXC_GPR13_SATA_SPEED_MASK \
				|IOMUXC_GPR13_SATA_PHY_5_MASK \
				|IOMUXC_GPR13_SATA_PHY_4_MASK \
				|IOMUXC_GPR13_SATA_PHY_3_MASK \
				|IOMUXC_GPR13_SATA_PHY_2_MASK \
				|IOMUXC_GPR13_SATA_PHY_1_MASK)

#endif	/* __MACH_IOMUX_V3_H__*/
