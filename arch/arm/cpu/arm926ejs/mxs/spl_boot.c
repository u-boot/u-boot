/*
 * Freescale i.MX28 Boot setup
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
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
#include <config.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>

#include "mxs_init.h"

/*
 * This delay function is intended to be used only in early stage of boot, where
 * clock are not set up yet. The timer used here is reset on every boot and
 * takes a few seconds to roll. The boot doesn't take that long, so to keep the
 * code simple, it doesn't take rolling into consideration.
 */
void early_delay(int delay)
{
	struct mxs_digctl_regs *digctl_regs =
		(struct mxs_digctl_regs *)MXS_DIGCTL_BASE;

	uint32_t st = readl(&digctl_regs->hw_digctl_microseconds);
	st += delay;
	while (st > readl(&digctl_regs->hw_digctl_microseconds))
		;
}

#define	MUX_CONFIG_BOOTMODE_PAD	(MXS_PAD_3V3 | MXS_PAD_4MA | MXS_PAD_NOPULL)
const iomux_cfg_t iomux_boot[] = {
	MX28_PAD_LCD_D00__GPIO_1_0 | MUX_CONFIG_BOOTMODE_PAD,
	MX28_PAD_LCD_D01__GPIO_1_1 | MUX_CONFIG_BOOTMODE_PAD,
	MX28_PAD_LCD_D02__GPIO_1_2 | MUX_CONFIG_BOOTMODE_PAD,
	MX28_PAD_LCD_D03__GPIO_1_3 | MUX_CONFIG_BOOTMODE_PAD,
	MX28_PAD_LCD_D04__GPIO_1_4 | MUX_CONFIG_BOOTMODE_PAD,
	MX28_PAD_LCD_D05__GPIO_1_5 | MUX_CONFIG_BOOTMODE_PAD,
};

uint8_t mxs_get_bootmode_index(void)
{
	uint8_t bootmode = 0;
	int i;
	uint8_t masked;

	/* Setup IOMUX of bootmode pads to GPIO */
	mxs_iomux_setup_multiple_pads(iomux_boot, ARRAY_SIZE(iomux_boot));

	/* Setup bootmode pins as GPIO input */
	gpio_direction_input(MX28_PAD_LCD_D00__GPIO_1_0);
	gpio_direction_input(MX28_PAD_LCD_D01__GPIO_1_1);
	gpio_direction_input(MX28_PAD_LCD_D02__GPIO_1_2);
	gpio_direction_input(MX28_PAD_LCD_D03__GPIO_1_3);
	gpio_direction_input(MX28_PAD_LCD_D04__GPIO_1_4);
	gpio_direction_input(MX28_PAD_LCD_D05__GPIO_1_5);

	/* Read bootmode pads */
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D00__GPIO_1_0) ? 1 : 0) << 0;
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D01__GPIO_1_1) ? 1 : 0) << 1;
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D02__GPIO_1_2) ? 1 : 0) << 2;
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D03__GPIO_1_3) ? 1 : 0) << 3;
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D04__GPIO_1_4) ? 1 : 0) << 4;
	bootmode |= (gpio_get_value(MX28_PAD_LCD_D05__GPIO_1_5) ? 1 : 0) << 5;

	for (i = 0; i < ARRAY_SIZE(mxs_boot_modes); i++) {
		masked = bootmode & mxs_boot_modes[i].boot_mask;
		if (masked == mxs_boot_modes[i].boot_pads)
			break;
	}

	return i;
}

void mxs_common_spl_init(const iomux_cfg_t *iomux_setup,
			const unsigned int iomux_size)
{
	struct mxs_spl_data *data = (struct mxs_spl_data *)
		((CONFIG_SYS_TEXT_BASE - sizeof(struct mxs_spl_data)) & ~0xf);
	uint8_t bootmode = mxs_get_bootmode_index();

	mxs_iomux_setup_multiple_pads(iomux_setup, iomux_size);
	mxs_power_init();

	mxs_mem_init();
	data->mem_dram_size = mxs_mem_get_size();

	data->boot_mode_idx = bootmode;

	mxs_power_wait_pswitch();
}

/* Support aparatus */
inline void board_init_f(unsigned long bootflag)
{
	for (;;)
		;
}

inline void board_init_r(gd_t *id, ulong dest_addr)
{
	for (;;)
		;
}

#ifndef CONFIG_SPL_SERIAL_SUPPORT
void serial_putc(const char c) {}
void serial_puts(const char *s) {}
#endif
void hang(void) __attribute__ ((noreturn));
void hang(void)
{
	for (;;)
		;
}
