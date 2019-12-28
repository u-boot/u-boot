/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common header file for U-Boot
 *
 * This file still includes quite a bit of stuff that should be in separate
 * headers. Please think before adding more things.
 * Patches to remove things are welcome.
 *
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __COMMON_H_
#define __COMMON_H_	1

#ifndef __ASSEMBLY__		/* put C only stuff in this section */

#include <config.h>
#include <errno.h>
#include <time.h>
#include <asm-offsets.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/stringify.h>
#include <asm/ptrace.h>
#include <stdarg.h>
#include <stdio.h>
#include <linux/kernel.h>

#include <part.h>
#include <flash.h>
#include <image.h>

#ifdef __LP64__
#define CONFIG_SYS_SUPPORT_64BIT_DATA
#endif

#include <log.h>

#include <asm/u-boot.h> /* boot information for Linux kernel */
#include <asm/global_data.h>	/* global data used for startup functions */

/* startup functions, used in:
 * common/board_f.c
 * common/init/board_init.c
 * common/board_r.c
 * common/board_info.c
 */
#include <init.h>

/*
 * Function Prototypes
 */
void	hang		(void) __attribute__ ((noreturn));

#include <display_options.h>

/* common/cmd_source.c */
int	source (ulong addr, const char *fit_uname);

extern ulong load_addr;		/* Default Load Address */
extern ulong save_addr;		/* Default Save Address */
extern ulong save_size;		/* Default Save Size */

/* common/memsize.c */
long	get_ram_size  (long *, long);
phys_size_t get_effective_memsize(void);

#if defined(CONFIG_SYS_DRAM_TEST)
int testdram(void);
#endif /* CONFIG_SYS_DRAM_TEST */

void	upmconfig     (unsigned int, unsigned int *, unsigned int);
ulong	get_tbclk     (void);
void	reset_misc    (void);
void	reset_cpu     (ulong addr);

/* $(CPU)/speed.c */
int	get_clocks (void);
ulong	get_bus_freq  (ulong);
int get_serial_clock(void);

/* lib/uuid.c */
#include <uuid.h>

/* lib/vsprintf.c */
#include <vsprintf.h>

/* lib/net_utils.c */
#include <net.h>

#include <bootstage.h>

#else	/* __ASSEMBLY__ */

#endif	/* __ASSEMBLY__ */

/* Put only stuff here that the assembler can digest */

#define ROUND(a,b)		(((a) + (b) - 1) & ~((b) - 1))

/*
 * check_member() - Check the offset of a structure member
 *
 * @structure:	Name of structure (e.g. global_data)
 * @member:	Name of member (e.g. baudrate)
 * @offset:	Expected offset in bytes
 */
#define check_member(structure, member, offset) _Static_assert( \
	offsetof(struct structure, member) == offset, \
	"`struct " #structure "` offset for `" #member "` is not " #offset)

/* Avoid using CONFIG_EFI_STUB directly as we may boot from other loaders */
#ifdef CONFIG_EFI_STUB
#define ll_boot_init()	false
#else
#define ll_boot_init()	true
#endif

/* Pull in stuff for the build system */
#ifdef DO_DEPS_ONLY
# include <env_internal.h>
#endif

#endif	/* __COMMON_H_ */
