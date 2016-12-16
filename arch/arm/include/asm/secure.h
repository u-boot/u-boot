#ifndef __ASM_SECURE_H
#define __ASM_SECURE_H

#include <config.h>

#define __secure __attribute__ ((section ("._secure.text")))
#define __secure_data __attribute__ ((section ("._secure.data")))

#if defined(CONFIG_ARMV7_SECURE_BASE) || defined(CONFIG_ARMV8_SECURE_BASE)
/*
 * Warning, horror ahead.
 *
 * The target code lives in our "secure ram", but u-boot doesn't know
 * that, and has blindly added reloc_off to every relocation
 * entry. Gahh. Do the opposite conversion. This hack also prevents
 * GCC from generating code veeners, which u-boot doesn't relocate at
 * all...
 */
#define secure_ram_addr(_fn) ({						\
			DECLARE_GLOBAL_DATA_PTR;			\
			void *__fn = _fn;				\
			typeof(_fn) *__tmp = (__fn - gd->reloc_off);	\
			__tmp;						\
		})
#else
#define secure_ram_addr(_fn)	(_fn)
#endif

#endif
