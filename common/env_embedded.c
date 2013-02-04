/*
 * (C) Copyright 2001
 * Erik Theisen,  Wave 7 Optics, etheisen@mindspring.com.
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

#ifndef __ASSEMBLY__
#define	__ASSEMBLY__			/* Dirty trick to get only #defines */
#endif
#define	__ASM_STUB_PROCESSOR_H__	/* don't include asm/processor. */
#include <config.h>
#undef	__ASSEMBLY__
#include <environment.h>
#include <linux/stringify.h>

/* Handle HOSTS that have prepended crap on symbol names, not TARGETS. */
#if defined(__APPLE__)
/* Leading underscore on symbols */
#  define SYM_CHAR "_"
#else /* No leading character on symbols */
#  define SYM_CHAR
#endif

/*
 * Generate embedded environment table
 * inside U-Boot image, if needed.
 */
#if defined(ENV_IS_EMBEDDED) || defined(CONFIG_BUILD_ENVCRC)
/*
 * Only put the environment in it's own section when we are building
 * U-Boot proper.  The host based program "tools/envcrc" does not need
 * a seperate section.  Note that ENV_CRC is only defined when building
 * U-Boot itself.
 */
#if (defined(CONFIG_SYS_USE_PPCENV) || defined(CONFIG_NAND_U_BOOT)) && \
	defined(ENV_CRC) /* Environment embedded in U-Boot .ppcenv section */
/* XXX - This only works with GNU C */
#  define __PPCENV__	__attribute__ ((section(".ppcenv")))
#  define __PPCTEXT__	__attribute__ ((section(".text")))

#elif defined(USE_HOSTCC) /* Native for 'tools/envcrc' */
#  define __PPCENV__	/*XXX DO_NOT_DEL_THIS_COMMENT*/
#  define __PPCTEXT__	/*XXX DO_NOT_DEL_THIS_COMMENT*/

#else /* Environment is embedded in U-Boot's .text section */
/* XXX - This only works with GNU C */
#  define __PPCENV__	__attribute__ ((section(".text")))
#  define __PPCTEXT__	__attribute__ ((section(".text")))
#endif

/*
 * Macros to generate global absolutes.
 */
#if defined(__bfin__)
# define GEN_SET_VALUE(name, value)	\
	asm(".set " GEN_SYMNAME(name) ", " GEN_VALUE(value))
#else
# define GEN_SET_VALUE(name, value)	\
	asm(GEN_SYMNAME(name) " = " GEN_VALUE(value))
#endif
#define GEN_SYMNAME(str)	SYM_CHAR #str
#define GEN_VALUE(str)		#str
#define GEN_ABS(name, value)			\
	asm(".globl " GEN_SYMNAME(name));	\
	GEN_SET_VALUE(name, value)

/*
 * Check to see if we are building with a
 * computed CRC.  Otherwise define it as ~0.
 */
#if !defined(ENV_CRC)
#  define ENV_CRC	(~0)
#endif

#define DEFAULT_ENV_INSTANCE_EMBEDDED
#include <env_default.h>

#ifdef CONFIG_ENV_ADDR_REDUND
env_t redundand_environment __PPCENV__ = {
	0,		/* CRC Sum: invalid */
	0,		/* Flags:   invalid */
	{
	"\0"
	}
};
#endif	/* CONFIG_ENV_ADDR_REDUND */

/*
 * These will end up in the .text section
 * if the environment strings are embedded
 * in the image.  When this is used for
 * tools/envcrc, they are placed in the
 * .data/.sdata section.
 *
 */
unsigned long env_size __PPCTEXT__ = sizeof(env_t);

/*
 * Add in absolutes.
 */
GEN_ABS(env_offset, CONFIG_ENV_OFFSET);

#endif /* ENV_IS_EMBEDDED */
