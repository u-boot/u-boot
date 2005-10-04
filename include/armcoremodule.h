/*
 * (C) Copyright 2005
 * ARM Ltd.
 * Peter Pearse, <Peter.Pearse@arm.com>
 * Configuration for ARM Core Modules.
 * No standalonw port yet available
 * - this file is included by both integratorap.h & integratorcp.h
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

#ifndef __ARMCOREMODULE_H
#define __ARMCOREMODULE_H

#define CM_BASE		0x10000000

/* CM registers common to all CMs */
/* Note that observed values after reboot into the ARM Boot Monitor
   have been used as defaults, rather than the POR values */
#define OS_CTRL     		0x0000000C
#define CMMASK_REMAP		0x00000005	/* set remap & led           */
#define CMMASK_RESET		0x00000008
#define OS_LOCK             	0x00000014
#define CMVAL_LOCK1	     	0x0000A000	/* locking value             */
#define CMVAL_LOCK2		0x0000005F	/* locking value             */
#define CMVAL_UNLOCK		0x00000000	/* any value != CM_LOCKVAL   */
#define OS_SDRAM		0x00000020
#define OS_INIT     		0x00000024
#define CMMASK_MAP_SIMPLE	0xFFFDFFFF	/* simple mapping */
#define CMMASK_TCRAM_DISABLE	0xFFFEFFFF	/* TCRAM disabled */
#define CMMASK_LOWVEC		0x00000000	/* vectors @ 0x00000000 */
#define CMMASK_LE		0xFFFFFFF7	/* little endian */
#define CMMASK_CMxx6_COMMON	0x00000013      /* Common value for CMxx6 */
						/* - observed reset value of */
						/*   CM926EJ-S */
						/*   CM1136-EJ-S */

#if defined (CONFIG_CM10200E) || defined (CONFIG_CM10220E)
#define CMMASK_INIT_102	0x00000300		/* see CM102xx ref manual */
						/* - PLL test clock bypassed */
						/* - bus clock ratio 2 */
						/* - little endian */
						/* - vectors at zero */
#endif /* CM1022xx */

/* Determine CM characteristics */

#undef	CONFIG_CM_MULTIPLE_SSRAM
#undef	CONFIG_CM_SPD_DETECT
#undef	CONFIG_CM_REMAP
#undef	CONFIG_CM_INIT
#undef	CONFIG_CM_TCRAM

#if defined (CONFIG_CM946E_S) || defined (CONFIG_CM966E_S)
#define	CONFIG_CM_MULTIPLE_SSRAM	/* CM has multiple SSRAM mapping */
#endif

/* Excalibur core module has reduced functionality */
#ifndef	CONFIG_CM922T_XA10
#define CONFIG_CM_SPD_DETECT			/* CM supports SPD query      */
#define OS_SPD			0x00000100	/* Address of SPD data        */
#define CONFIG_CM_REMAP				/* CM supports remapping      */
#define CONFIG_CM_INIT				/* CM has initialization reg  */
#endif	/* NOT EXCALIBUR */

#if defined(CONFIG_CM926EJ_S)   || defined (CONFIG_CM946E_S)	|| \
    defined(CONFIG_CM966E_S)    || defined (CONFIG_CM1026EJ_S)	|| \
    defined(CONFIG_CM1136JF_S)
#define CONFIG_CM_TCRAM				/* CM has TCRAM  */
#endif

#ifdef CONFIG_CM_SPD_DETECT
#define OS_SPD		0x00000100	/* The SDRAM SPD data is copied here */
#endif

#endif /* __ARMCOREMODULE_H */
