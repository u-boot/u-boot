/*
 *  linux/include/asm-arm/arch-pxa/hardware.h
 *
 *  Author:	Nicolas Pitre
 *  Created:	Jun 15, 2001
 *  Copyright:	MontaVista Software Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <linux/config.h>
#include <asm/mach-types.h>

/*
 * We requires absolute addresses.
 */
#define PCIO_BASE		0

/*
 * Workarounds for at least 2 errata so far require this.
 * The mapping is set in mach-pxa/generic.c.
 */
#define UNCACHED_PHYS_0		0xff000000
#define UNCACHED_ADDR		UNCACHED_PHYS_0

/*
 * Intel PXA2xx internal register mapping:
 *
 * 0x40000000 - 0x41ffffff <--> 0xf2000000 - 0xf3ffffff
 * 0x44000000 - 0x45ffffff <--> 0xf4000000 - 0xf5ffffff
 * 0x48000000 - 0x49ffffff <--> 0xf6000000 - 0xf7ffffff
 * 0x4c000000 - 0x4dffffff <--> 0xf8000000 - 0xf9ffffff
 * 0x50000000 - 0x51ffffff <--> 0xfa000000 - 0xfbffffff
 * 0x54000000 - 0x55ffffff <--> 0xfc000000 - 0xfdffffff
 * 0x58000000 - 0x59ffffff <--> 0xfe000000 - 0xffffffff
 *
 * Note that not all PXA2xx chips implement all those addresses, and the
 * kernel only maps the minimum needed range of this mapping.
 */
#ifndef CONFIG_CPU_MONAHANS
#define io_p2v(x) (0xf2000000 + ((x) & 0x01ffffff) + (((x) & 0x1c000000) >> 1))
#define io_v2p(x) (0x3c000000 + ((x) & 0x01ffffff) + (((x) & 0x0e000000) << 1))
#else 

/* There are too many IO area needed to map, so I divide them into 3 areas
 * 0x40000000 - 0x41ffffff <--> 0xf6000000 - 0xf7ffffff  Devs
 */
#define io_p2v(x)  ((((x) & 0xfc000000)>>4) + 0xf2000000 + ((x)&0x01ffffff))
#define io_v2p(x)  (((((x) - 0xf2000000)&0xfc000000)<<4) + ((x)&0x01ffffff))

/*
 * 0x42000000 - 0x421fffff <--> 0xf8000000 - 0xf81fffff  MMC2 & USIM2
 * 0x43000000 - 0x430fffff <--> 0xf8200000 - 0xf82fffff  Caddo
 * 0x43100000 - 0x431fffff <--> 0xf8300000 - 0xf83fffff  NAND
 * 0x44000000 - 0x440fffff <--> 0xf8400000 - 0xf84fffff  LCD
 * 0x46000000 - 0x460fffff <--> 0xf8800000 - 0xf88fffff  Mini LCD
 * 0x48100000 - 0x481fffff <--> 0xf8d00000 - 0xf8dfffff  Dynamic Mem Ctl
 * 0x4a000000 - 0x4a0fffff <--> 0xf9000000 - 0xf90fffff  Static Mem Ctl
 * 0x4c000000 - 0x4c0fffff <--> 0xf9400000 - 0xf94fffff  USB Host
 */

#define io_p2v_2(x)	(((((x) - 0x42000000) & 0xff000000) >> 3) + 0xf8000000\
 			+ ((x) & 0x001fffff))
#define io_v2p_2(x)	(((((x) & 0xffe00000) - 0xf8000000) << 3) + 0x42000000\
				+ (x & 0x001fffff)) 
/*
 * 0x50000000 - 0x500fffff <--> 0xfa000000 - 0xfa0fffff  Camera Interface
 * 0x54000000 - 0x540fffff <--> 0xfa400000 - 0xfa4fffff  2D Graphics Ctrl
 * 0x54100000 - 0x541fffff <--> 0xfa500000 - 0xfa5fffff  USB Device 2.0 Ctrl
 * 0x58000000 - 0x580fffff <--> 0xfa800000 - 0xfa8fffff  Internal SRAM Ctrl
 */

#define io_p2v_3(x)	((((x) & 0xfc000000) >> 4) + 0xf5000000 + \
				((x) & 0x001fffff)) 
#define io_v2p_3(x)	(((((x) - 0xf5000000) & 0x0fc00000) << 4) + \
				((x) & 0x001fffff)) 
#endif /* CONFIG_CPU_MONAHANS */

#ifndef __ASSEMBLY__

#if 0
# define __REG(x)	(*((volatile u32 *)io_p2v(x)))
#else
/*
 * This __REG() version gives the same results as the one above,  except
 * that we are fooling gcc somehow so it generates far better and smaller
 * assembly code for access to contigous registers.  It's a shame that gcc
 * doesn't guess this by itself.
 */
#include <asm/types.h>
typedef struct { volatile u32 offset[4096]; } __regbase;
# define __REGP(x)	((__regbase *)((x)&~4095))->offset[((x)&4095)>>2]
# define __REG(x)	__REGP(io_p2v(x))

/* __REG_2 is for NAND, LCD etc.
 * __REG_3 is for Camera Interface, 2D Graphics, U2D etc.*/
#ifdef CONFIG_CPU_MONAHANS
#define __REG_2(x)	__REGP(io_p2v_2(x))
#define __REG_3(x)	__REGP(io_p2v_3(x))

#endif /* CONFIG_CPU_MONAHANS */
#endif /* if 0 */

/* With indexed regs we don't want to feed the index through io_p2v()
   especially if it is a variable, otherwise horrible code will result. */
# define __REG2(x,y)     (*(volatile u32 *)((u32)&__REG(x) + (y)))

# define __PREG(x)	(io_v2p((u32)&(x)))

#else /* ifndef __ASSEMBLY__ */

# define __REG(x)	io_p2v(x)
# define __PREG(x)	io_v2p(x)

#ifdef CONFIG_CPU_MONAHANS
# define __REG_2(x)	io_p2v(x)
# define __REG_3(x)	io_p2v(x)
#endif /* CONFIG_CPU_MONAHANS */

#endif /* ifndef __ASSEMBLY__ */

#ifndef __ASSEMBLY__

#ifdef CONFIG_MACH_ZYLONITE
#include "zylonite.h"
#endif

/*
 * Handy routine to set GPIO alternate functions
 */
extern void pxa_gpio_mode( int gpio_mode );

/*
 * Routine to enable or disable CKEN
 */
extern void pxa_set_cken(int clock, int enable);

/*
 * return current memory and LCD clock frequency in units of 10kHz
 */
extern unsigned int get_memclk_frequency_10khz(void);
extern unsigned int get_lcdclk_frequency_10khz(void);
#endif /* __ASSEMBLY__ */

#ifdef CONFIG_ARCH_LUBBOCK
#include "lubbock.h"
#endif

#ifdef CONFIG_ARCH_PXA_IDP
#include "idp.h"
#endif

#ifdef CONFIG_ARCH_PXA_CERF
#include "cerf.h"
#endif

#if CONFIG_CPU_MONAHANS_L2CACHE
#define	__cpuc_flush_l2cache_all	xscale_flush_l2cache_all
extern void __cpuc_flush_l2cache_all(void);
#define	flush_l2cache_all		__cpuc_flush_l2cache_all
#else
#define	__cpuc_flush_l2cache_all()	do {} while (0)
#define	flush_l2cache_all()		do {} while (0)
#endif

#ifdef CONFIG_ARCH_CSB226
#include "csb226.h"
#endif

#ifdef CONFIG_ARCH_INNOKOM
#include "innokom.h"
#endif

#ifdef CONFIG_ARCH_PLEB
#include "pleb.h"
#endif

#ifdef CONFIG_MACH_MAINSTONE
#include "mainstone.h"
#endif

#include "pxa-regs.h"

#endif  /* _ASM_ARCH_HARDWARE_H */
