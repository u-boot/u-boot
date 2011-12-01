/*
 * Copyright 2011 Freescale Semiconductor, Inc.
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

#ifndef __P3060QDS_QIXIS_H__
#define __P3060QDS_QIXIS_H__

/* Definitions of QIXIS Registers for P3060QDS */

/* BRDCFG4[4:7]] select EC1 and EC2 as a pair */
#define BRDCFG4_EC_MODE_MASK		0x0F
#define BRDCFG4_EC2_MII_EC1_MII	0x00
#define BRDCFG4_EC2_MII_EC1_USB	0x03
#define BRDCFG4_EC2_USB_EC1_MII	0x0C
#define BRDCFG4_EC2_USB_EC1_USB	0x0F
#define BRDCFG4_EC2_USB_EC1_RGMII	0x0E
#define BRDCFG4_EC2_RGMII_EC1_USB	0x0B
#define BRDCFG4_EC2_RGMII_EC1_RGMII	0x0A
#define BRDCFG4_EMISEL_MASK		0xF0

#define BRDCFG5_ECLKS_MASK		0x80
#define BRDCFG5_USB1ID_MASK		0x40
#define BRDCFG5_USB2ID_MASK		0x20
#define BRDCFG5_GC2MX_MASK		0x0C
#define BRDCFG5_T15MX_MASK		0x03
#define BRDCFG5_ECLKS_IEEE1588_CM	0x80
#define BRDCFG5_USB1ID_CTRL		0x40
#define BRDCFG5_USB2ID_CTRL		0x20

#define BRDCFG6_SD1MX_A		0x01
#define BRDCFG6_SD1MX_B		0x00
#define BRDCFG6_SD2MX_A		0x02
#define BRDCFG6_SD2MX_B		0x00
#define BRDCFG6_SD3MX_A		0x04
#define BRDCFG6_SD3MX_B		0x00
#define BRDCFG6_SD4MX_A		0x08
#define BRDCFG6_SD4MX_B		0x00

#define BRDCFG7_JTAGMX_MASK		0xC0
#define BRDCFG7_IQ1MX_MASK		0x20
#define BRDCFG7_G1MX_MASK		0x10
#define BRDCFG7_D1MX_MASK		0x0C
#define BRDCFG7_I3MX_MASK		0x03
#define BRDCFG7_JTAGMX_AURORA		0x00
#define BRDCFG7_JTAGMX_FPGA		0x80
#define BRDCFG7_JTAGMX_COP_JTAG	0xC0
#define BRDCFG7_IQ1MX_IRQ_EVT		0x00
#define BRDCFG7_IQ1MX_USB2		0x20
#define BRDCFG7_G1MX_USB1		0x00
#define BRDCFG7_G1MX_TSEC3		0x10
#define BRDCFG7_D1MX_DMA		0x00
#define BRDCFG7_D1MX_TSEC3USB		0x04
#define BRDCFG7_D1MX_HDLC2		0x08
#define BRDCFG7_I3MX_UART2_I2C34	0x00
#define BRDCFG7_I3MX_GPIO_EVT		0x01
#define BRDCFG7_I3MX_USB1		0x02
#define BRDCFG7_I3MX_TSEC3		0x03

#endif
