/* LEON3 header file. LEON3 is a free GPL SOC processor available
 * at www.gaisler.com.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
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
 *
 */

#ifndef __LEON3_H__
#define __LEON3_H__

#ifndef CONFIG_LEON3
#error Include LEON3 header file only if LEON3 processor
#endif

/* Not much to define, most is Plug and Play and GRLIB dependent
 * not LEON3 dependent. See <ambapp.h> for GRLIB timers, interrupt
 * ctrl, memory controllers etc.
 */

#endif
