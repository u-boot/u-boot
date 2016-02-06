/*
 * (C) Copyright 2000 - 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __U_BOOT_H__
#define __U_BOOT_H__

/* Currently, this board information is not passed to
 * Linux kernel from U-Boot, but may be passed to other
 * Operating systems. This is because U-Boot emulates
 * a SUN PROM loader (from Linux point of view).
 */
#include <asm-generic/u-boot.h>

/* For image.h:image_check_target_arch() */
#define IH_ARCH_DEFAULT IH_ARCH_SPARC

#endif
