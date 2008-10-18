/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
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

/*
 * W7O board level hardware watchdog.
 */
#include <common.h>
#include <config.h>

#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>

void hw_watchdog_reset(void)
{
    volatile ushort *hwd = (ushort *)(CONFIG_SYS_W7O_EBC_PB7CR & 0xfff00000);

    /*
     * Read the LMG's hwd register and toggle the
     * watchdog bit to reset it.   On the LMC, just
     * reading it is enough, but toggling the bit
     * doen't hurt either.
     */
    *hwd = *hwd ^ 0x8000;

} /* hw_watchdog_reset() */

#endif /* CONFIG_HW_WATCHDOG */
