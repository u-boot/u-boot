/*
 * Copyright 2011 Freescale Semiconductor, Inc.
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
 *
 */

#ifndef _ASM_MPC86xx_CONFIG_H_
#define _ASM_MPC86xx_CONFIG_H_

/* SoC specific defines for Freescale MPC86xx processors */

#if defined(CONFIG_MPC8610)
#define CONFIG_MAX_CPUS			1
#define CONFIG_SYS_FSL_NUM_LAWS		10

#elif defined(CONFIG_MPC8641)
#define CONFIG_MAX_CPUS			2
#define CONFIG_SYS_FSL_NUM_LAWS		10

#else
#error Processor type not defined for this platform
#endif

#endif /* _ASM_MPC85xx_CONFIG_H_ */
