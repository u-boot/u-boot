/*
 * Pinmux configurations for the DAxxx SoCs
 *
 * Copyright (C) 2011 OMICRON electronics GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __ASM_ARCH_PINMUX_DEFS_H
#define __ASM_ARCH_PINMUX_DEFS_H

#include <asm/arch/davinci_misc.h>
#include <config.h>

/* SPI0 pin muxer settings */
extern const struct pinmux_config spi0_pins_base[3];
extern const struct pinmux_config spi0_pins_scs0[1];
extern const struct pinmux_config spi0_pins_ena[1];

/* SPI1 pin muxer settings */
extern const struct pinmux_config spi1_pins_base[3];
extern const struct pinmux_config spi1_pins_scs0[1];

/* UART pin muxer settings */
extern const struct pinmux_config uart0_pins_txrx[2];
extern const struct pinmux_config uart1_pins_txrx[2];
extern const struct pinmux_config uart2_pins_txrx[2];
extern const struct pinmux_config uart2_pins_rtscts[2];

/* EMAC pin muxer settings*/
extern const struct pinmux_config emac_pins_rmii[7];
extern const struct pinmux_config emac_pins_rmii_clk_source[1];
extern const struct pinmux_config emac_pins_mii[15];
extern const struct pinmux_config emac_pins_mdio[2];

/* I2C pin muxer settings */
extern const struct pinmux_config i2c0_pins[2];
extern const struct pinmux_config i2c1_pins[2];

/* EMIFA pin muxer settings */
extern const struct pinmux_config emifa_pins[40];
extern const struct pinmux_config emifa_pins_cs0[1];
extern const struct pinmux_config emifa_pins_cs2[1];
extern const struct pinmux_config emifa_pins_cs3[1];
extern const struct pinmux_config emifa_pins_cs4[1];
extern const struct pinmux_config emifa_pins_nand[12];
extern const struct pinmux_config emifa_pins_nor[43];

/* USB pin mux setting */
extern const struct pinmux_config usb_pins[1];

/* MMC pin muxer settings */
extern const struct pinmux_config mmc0_pins_8bit[10];
extern const struct pinmux_config mmc0_pins[6];

#endif
