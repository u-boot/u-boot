#ifndef __ARM7_HW_H
#define __ARM7_HW_H

/*
 * Copyright (c) 2004	Cucy Systems (http://www.cucy.com)
 * Curt Brune <curt@cucy.com>
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

#if defined(CONFIG_NETARM)
#include <asm/arch-arm720t/netarm_registers.h>
#elif defined(CONFIG_IMPA7)
/* include IMPA7 specific hardware file if there was one */
#elif defined(CONFIG_EP7312)
/* include EP7312 specific hardware file if there was one */
#elif defined(CONFIG_ARMADILLO)
/* include armadillo specific hardware file if there was one */
#elif defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
/* include IntegratorCP/CM720T specific hardware file if there was one */
#else
#error No hardware file defined for this configuration
#endif

#endif /* __ARM7_HW_H */
