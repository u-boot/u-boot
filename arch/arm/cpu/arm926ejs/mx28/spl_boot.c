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
#include <asm/arch/iomux-mx28.h>
#include <asm/arch/imx-regs.h>

#include "mx28_init.h"

/*
 * This delay function is intended to be used only in early stage of boot, where
 * clock are not set up yet. The timer used here is reset on every boot and
 * takes a few seconds to roll. The boot doesn't take that long, so to keep the
 * code simple, it doesn't take rolling into consideration.
 */
#define	HW_DIGCTRL_MICROSECONDS	0x8001c0c0
void early_delay(int delay)
{
	uint32_t st = readl(HW_DIGCTRL_MICROSECONDS);
	st += delay;
	while (st > readl(HW_DIGCTRL_MICROSECONDS))
		;
}

void mx28_common_spl_init(const iomux_cfg_t *iomux_setup,
			const unsigned int iomux_size)
{
	mxs_iomux_setup_multiple_pads(iomux_setup, iomux_size);
	mx28_power_init();
	mx28_mem_init();
	mx28_power_wait_pswitch();
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

void serial_putc(const char c) {}
void serial_puts(const char *s) {}
void hang(void) __attribute__ ((noreturn));
void hang(void)
{
	for (;;)
		;
}
