/*
 * Copyright (C) 2008 Lyrtech <www.lyrtech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#ifndef __MISC_H
#define __MISC_H

/* pin muxer definitions */
#define PIN_MUX_NUM_FIELDS	8	/* Per register */
#define PIN_MUX_FIELD_SIZE	4	/* n in bits */
#define PIN_MUX_FIELD_MASK	((1 << PIN_MUX_FIELD_SIZE) - 1)

/* pin definition */
struct pinmux_config {
	dv_reg		*mux;		/* Address of mux register */
	unsigned char	value;		/* Value to set in field */
	unsigned char	field;		/* field number */
};

/* pin table definition */
struct pinmux_resource {
	const struct pinmux_config	*pins;
	const int 			n_pins;
};

#define PINMUX_ITEM(item) { \
				.pins = item, \
				.n_pins = ARRAY_SIZE(item) \
			  }

struct lpsc_resource {
	const int	lpsc_no;
};

int dvevm_read_mac_address(uint8_t *buf);
void davinci_sync_env_enetaddr(uint8_t *rom_enetaddr);
int davinci_configure_pin_mux(const struct pinmux_config *pins, int n_pins);
int davinci_configure_pin_mux_items(const struct pinmux_resource *item,
				    int n_items);
#if defined(CONFIG_DRIVER_TI_EMAC) && defined(CONFIG_SOC_DA8XX)
void davinci_emac_mii_mode_sel(int mode_sel);
#endif
#if defined(CONFIG_SOC_DA8XX)
void irq_init(void);
int da8xx_configure_lpsc_items(const struct lpsc_resource *item,
				    const int n_items);
#endif

#endif /* __MISC_H */
