/* LEON3 header file. LEON3 is a free GPL SOC processor available
 * at www.gaisler.com.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
