/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2008
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SPARC_BYTEORDER_H
#define _SPARC_BYTEORDER_H

#include <asm/types.h>

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define __BYTEORDER_HAS_U64__
#define __SWAB_64_THRU_32__
#endif
#include <linux/byteorder/big_endian.h>
#endif				/* _SPARC_BYTEORDER_H */
