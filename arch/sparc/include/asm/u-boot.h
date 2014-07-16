/*
 * (C) Copyright 2000 - 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007, From asm-ppc/u-boot.h
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef __U_BOOT_H__
#define __U_BOOT_H__

/*
 * Currently, this Board information is not passed to
 * Linux kernel from U-Boot, but may be passed to other
 * Operating systems. This is because U-boot emulates
 * a SUN PROM loader (from Linux point of view).
 *
 * include/asm-sparc/u-boot.h
 */

#ifndef __ASSEMBLY__

typedef struct bd_info {
	unsigned long bi_memstart;	/* start of DRAM memory */
	phys_size_t bi_memsize;		/* size  of DRAM memory in bytes */
	unsigned long bi_flashstart;	/* start of FLASH memory */
	unsigned long bi_flashsize;	/* size  of FLASH memory */
	unsigned long bi_flashoffset;	/* reserved area for startup monitor */
	unsigned long bi_sramstart;	/* start of SRAM memory */
	unsigned long bi_sramsize;	/* size  of SRAM memory */
	unsigned long bi_bootflags;	/* boot / reboot flag (for LynxOS) */
	unsigned short bi_ethspeed;	/* Ethernet speed in Mbps */
	unsigned long bi_intfreq;	/* Internal Freq, in MHz */
	unsigned long bi_busfreq;	/* Bus Freq, in MHz */
} bd_t;

#endif				/* __ASSEMBLY__ */

/* For image.h:image_check_target_arch() */
#define IH_ARCH_DEFAULT IH_ARCH_SPARC

#endif				/* __U_BOOT_H__ */
