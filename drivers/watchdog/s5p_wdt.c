/*
 * Copyright (C) 2012 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/watchdog.h>

#define PRESCALER_VAL 255

void wdt_stop(void)
{
	struct s5p_watchdog *wdt =
		(struct s5p_watchdog *)samsung_get_base_watchdog();
	unsigned int wtcon;

	wtcon = readl(&wdt->wtcon);
	wtcon &= ~(WTCON_EN | WTCON_INT | WTCON_RESET);

	writel(wtcon, &wdt->wtcon);
}

void wdt_start(unsigned int timeout)
{
	struct s5p_watchdog *wdt =
		(struct s5p_watchdog *)samsung_get_base_watchdog();
	unsigned int wtcon;

	wdt_stop();

	wtcon = readl(&wdt->wtcon);
	wtcon |= (WTCON_EN | WTCON_CLK(WTCON_CLK_128));
	wtcon &= ~WTCON_INT;
	wtcon |= WTCON_RESET;
	wtcon |= WTCON_PRESCALER(PRESCALER_VAL);

	writel(timeout, &wdt->wtdat);
	writel(timeout, &wdt->wtcnt);
	writel(wtcon, &wdt->wtcon);
}
