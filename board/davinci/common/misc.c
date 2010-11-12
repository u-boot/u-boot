/*
 * Miscelaneous DaVinci functions.
 *
 * Copyright (C) 2009 Nick Thompson, GE Fanuc Ltd, <nick.thompson@gefanuc.com>
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2008 Lyrtech <www.lyrtech.com>
 * Copyright (C) 2004 Texas Instruments.
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

#include <common.h>
#include <i2c.h>
#include <net.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include "misc.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size(
			(volatile void *)CONFIG_SYS_SDRAM_BASE,
			CONFIG_MAX_RAM_BANK_SIZE);
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;
}

#ifdef CONFIG_DRIVER_TI_EMAC

/* Read ethernet MAC address from EEPROM for DVEVM compatible boards.
 * Returns 1 if found, 0 otherwise.
 */
int dvevm_read_mac_address(uint8_t *buf)
{
#ifdef CONFIG_SYS_I2C_EEPROM_ADDR
	/* Read MAC address. */
	if (i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR, 0x7F00, CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
		     (uint8_t *) &buf[0], 6))
		goto i2cerr;

	/* Check that MAC address is valid. */
	if (!is_valid_ether_addr(buf))
		goto err;

	return 1; /* Found */

i2cerr:
	printf("Read from EEPROM @ 0x%02x failed\n", CONFIG_SYS_I2C_EEPROM_ADDR);
err:
#endif /* CONFIG_SYS_I2C_EEPROM_ADDR */

	return 0;
}

/*
 * If there is no MAC address in the environment, then it will be initialized
 * (silently) from the value in the EEPROM.
 */
void davinci_sync_env_enetaddr(uint8_t *rom_enetaddr)
{
	uint8_t env_enetaddr[6];

	eth_getenv_enetaddr_by_index(0, env_enetaddr);
	if (!memcmp(env_enetaddr, "\0\0\0\0\0\0", 6)) {
		/* There is no MAC address in the environment, so we initialize
		 * it from the value in the EEPROM. */
		debug("### Setting environment from EEPROM MAC address = "
			"\"%pM\"\n",
			env_enetaddr);
		eth_setenv_enetaddr("ethaddr", rom_enetaddr);
	}
}

#endif	/* DAVINCI_EMAC */

/*
 * Change the setting of a pin multiplexer field.
 *
 * Takes an array of pinmux settings similar to:
 *
 * struct pinmux_config uart_pins[] = {
 *	{ &davinci_syscfg_regs->pinmux[8], 2, 7 },
 *	{ &davinci_syscfg_regs->pinmux[9], 2, 0 }
 * };
 *
 * Stepping through the array, each pinmux[n] register has the given value
 * set in the pin mux field specified.
 *
 * The number of pins in the array must be passed (ARRAY_SIZE can provide
 * this value conveniently).
 *
 * Returns 0 if all field numbers and values are in the correct range,
 * else returns -1.
 */
int davinci_configure_pin_mux(const struct pinmux_config *pins,
			      const int n_pins)
{
	int i;

	/* check for invalid pinmux values */
	for (i = 0; i < n_pins; i++) {
		if (pins[i].field >= PIN_MUX_NUM_FIELDS ||
		    (pins[i].value & ~PIN_MUX_FIELD_MASK) != 0)
			return -1;
	}

	/* configure the pinmuxes */
	for (i = 0; i < n_pins; i++) {
		const int offset = pins[i].field * PIN_MUX_FIELD_SIZE;
		const unsigned int value = pins[i].value << offset;
		const unsigned int mask = PIN_MUX_FIELD_MASK << offset;
		const dv_reg *mux = pins[i].mux;

		writel(value | (readl(mux) & (~mask)), mux);
	}

	return 0;
}

/*
 * Configure multiple pinmux resources.
 *
 * Takes an pinmux_resource array of pinmux_config and pin counts:
 *
 * const struct pinmux_resource pinmuxes[] = {
 *	PINMUX_ITEM(uart_pins),
 *	PINMUX_ITEM(i2c_pins),
 * };
 *
 * The number of items in the array must be passed (ARRAY_SIZE can provide
 * this value conveniently).
 *
 * Each item entry is configured in the defined order. If configuration
 * of any item fails, -1 is returned and none of the following items are
 * configured. On success, 0 is returned.
 */
int davinci_configure_pin_mux_items(const struct pinmux_resource *item,
				    const int n_items)
{
	int i;

	for (i = 0; i < n_items; i++) {
		if (davinci_configure_pin_mux(item[i].pins,
					      item[i].n_pins) != 0)
			return -1;
	}

	return 0;
}
