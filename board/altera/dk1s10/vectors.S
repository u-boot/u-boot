/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 * Stephan Linz <linz@li-pro.net>
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

#include <config.h>


/*************************************************************************
 * Exception Vector Table
 *
 * This could have gone in the cpu soure tree, but the whole point of
 * Nios is customization -- and polluting the cpu source tree with
 * board-specific ifdef's really defeats the purpose, no? With this in
 * the board-specific tree, each board has the freedom to organize
 * vectors/traps, etc anyway it wants. The init code copies this table
 * to the proper location.
 *
 * Each board can do what it likes here. But there are four "standard"
 * handlers availble:
 *
 *	_cwp_lolimit	-Handles register window underflows.
 *	_cwp_hilimit	-Handles register window overflows.
 *	_timebase_int	-Increments the timebase.
 *	_def_xhandler	-Default exception handler.
 *
 * _timebase_int handles a Nios Timer interrupt and increments the
 * timestamp used for the get_timer(), reset_timer(), etc. routines. It
 * expects the timer to be configured like the standard-32 low priority
 * timer.
 *
 * _def_xhandler dispatches exceptions/traps via the external_interrupt()
 * routine. This lets you use the irq_install_handler() and handle your
 * interrupts/traps with code written in C.
 ************************************************************************/

	.data
	.global _vectors
	.align	4
_vectors:

#if	defined(CONFIG_SYS_NIOS_CPU_OCI_BASE)
	/* OCI does the reset job */
	.long	_def_xhandler@h		/* Vector 0  - NMI / Reset */
#else
	/* there is no OCI, so we have to do a direct reset jump here */
	.long	CONFIG_SYS_NIOS_CPU_RST_VECT	/* Vector 0  - Reset to GERMS */
#endif
	.long	_cwp_lolimit@h		/* Vector 1  - underflow */
	.long	_cwp_hilimit@h		/* Vector 2  - overflow	*/

	.long	_def_xhandler@h		/* Vector 3 - GNUPro debug */
	.long	_def_xhandler@h		/* Vector 4 - GNUPro debug */
	.long	_def_xhandler@h		/* Vector 5 - GNUPro debug */
	.long	_def_xhandler@h		/* Vector 6 - future reserved */
	.long	_def_xhandler@h		/* Vector 7 - future reserved */
	.long	_def_xhandler@h		/* Vector 8 - future reserved */
	.long	_def_xhandler@h		/* Vector 9 - future reserved */
	.long	_def_xhandler@h		/* Vector 10 - future reserved */
	.long	_def_xhandler@h		/* Vector 11 - future reserved */
	.long	_def_xhandler@h		/* Vector 12 - future reserved */
	.long	_def_xhandler@h		/* Vector 13 - future reserved */
	.long	_def_xhandler@h		/* Vector 14 - future reserved */
	.long	_def_xhandler@h		/* Vector 15 - future reserved */
#if	(CONFIG_SYS_NIOS_TMRIRQ == 16)
	.long	_timebase_int@h		/* Vector 16 - lopri timer*/
#else
	.long	_def_xhandler@h		/* Vector 16 */
#endif
	.long	_def_xhandler@h		/* Vector 17 */
	.long	_def_xhandler@h		/* Vector 18 */
	.long	_def_xhandler@h		/* Vector 19 */
	.long	_def_xhandler@h		/* Vector 20 */
	.long	_def_xhandler@h		/* Vector 21 */
	.long	_def_xhandler@h		/* Vector 22 */
	.long	_def_xhandler@h		/* Vector 23 */
	.long	_def_xhandler@h		/* Vector 24 */
	.long	_def_xhandler@h		/* Vector 25 */
	.long	_def_xhandler@h		/* Vector 26 */
	.long	_def_xhandler@h		/* Vector 27 */
	.long	_def_xhandler@h		/* Vector 28 */
	.long	_def_xhandler@h		/* Vector 29 */
	.long	_def_xhandler@h		/* Vector 30 */
	.long	_def_xhandler@h		/* Vector 31 */
	.long	_def_xhandler@h		/* Vector 32 */
	.long	_def_xhandler@h		/* Vector 33 */
	.long	_def_xhandler@h		/* Vector 34 */
	.long	_def_xhandler@h		/* Vector 35 */
	.long	_def_xhandler@h		/* Vector 36 */
	.long	_def_xhandler@h		/* Vector 37 */
	.long	_def_xhandler@h		/* Vector 38 */
	.long	_def_xhandler@h		/* Vector 39 */
	.long	_def_xhandler@h		/* Vector 40 */
	.long	_def_xhandler@h		/* Vector 41 */
	.long	_def_xhandler@h		/* Vector 42 */
	.long	_def_xhandler@h		/* Vector 43 */
	.long	_def_xhandler@h		/* Vector 44 */
	.long	_def_xhandler@h		/* Vector 45 */
	.long	_def_xhandler@h		/* Vector 46 */
	.long	_def_xhandler@h		/* Vector 47 */
	.long	_def_xhandler@h		/* Vector 48 */
	.long	_def_xhandler@h		/* Vector 49 */
#if	(CONFIG_SYS_NIOS_TMRIRQ == 50)
	.long	_timebase_int@h		/* Vector 50 - lopri timer*/
#else
	.long	_def_xhandler@h		/* Vector 50 */
#endif
	.long	_def_xhandler@h		/* Vector 51 */
	.long	_def_xhandler@h		/* Vector 52 */
	.long	_def_xhandler@h		/* Vector 53 */
	.long	_def_xhandler@h		/* Vector 54 */
	.long	_def_xhandler@h		/* Vector 55 */
	.long	_def_xhandler@h		/* Vector 56 */
	.long	_def_xhandler@h		/* Vector 57 */
	.long	_def_xhandler@h		/* Vector 58 */
	.long	_def_xhandler@h		/* Vector 59 */
	.long	_def_xhandler@h		/* Vector 60 */
	.long	_def_xhandler@h		/* Vector 61 */
	.long	_def_xhandler@h		/* Vector 62 */
	.long	_def_xhandler@h		/* Vector 63 */
