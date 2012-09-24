/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <clps7111.h>
#include <asm/hardware.h>
#include <asm/system.h>

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 * and we set the CPU-speed to 73 MHz - see start.S for details
	 */

#if defined(CONFIG_NETARM) || defined(CONFIG_S3C4510B) || defined(CONFIG_LPC2292)
	disable_interrupts ();
	/* Nothing more needed */
#elif defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* No cleanup before linux for IntegratorAP/CM720T as yet */
#elif defined(CONFIG_TEGRA)
	/* No cleanup before linux for tegra as yet */
#else
#error No cleanup_before_linux() defined for this CPU type
#endif
	return 0;
}
