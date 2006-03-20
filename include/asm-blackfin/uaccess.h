/*
 * U-boot - uaccess.h
 *
 * Copyright (c) 2005 blackfin.uclinux.org
 *
 * This file is based on
 * Based on: include/asm-m68knommu/uaccess.h
 * Changes made by Lineo Inc.    May 2001
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

#ifndef __BLACKFIN_UACCESS_H
#define __BLACKFIN_UACCESS_H

/*
 * User space memory access functions
 */
#include <asm/segment.h>
#include <asm/errno.h>

#define VERIFY_READ	0
#define VERIFY_WRITE	1

/* We let the MMU do all checking */
static inline int access_ok(int type, const void *addr, unsigned long size)
{
	return ((unsigned long) addr < 0x10f00000);	/* need final decision - Tony */
}

static inline int verify_area(int type, const void *addr,
			      unsigned long size)
{
	return access_ok(type, addr, size) ? 0 : -EFAULT;
}

/*
 * The exception table consists of pairs of addresses: the first is the
 * address of an instruction that is allowed to fault, and the second is
 * the address at which the program should continue.  No registers are
 * modified, so it is entirely up to the continuation code to figure out
 * what to do.
 *
 * All the routines below use bits of fixup code that are out of line
 * with the main instruction path.  This means when everything is well,
 * we don't even have to jump over them.  Further, they do not intrude
 * on our cache or tlb entries.
 */

struct exception_table_entry {
	unsigned long insn, fixup;
};

/* Returns 0 if exception not found and fixup otherwise.  */
extern unsigned long search_exception_table(unsigned long);

/*
 * These are the main single-value transfer routines.  They automatically
 * use the right size if we just have the right pointer type.
 */

#define put_user(x, ptr)				\
({							\
    int __pu_err = 0;					\
    typeof(*(ptr)) __pu_val = (x);			\
    switch (sizeof (*(ptr))) {				\
    case 1:						\
	__put_user_asm(__pu_err, __pu_val, ptr, B);	\
	break;						\
    case 2:						\
	__put_user_asm(__pu_err, __pu_val, ptr, W);	\
	break;						\
    case 4:						\
	__put_user_asm(__pu_err, __pu_val, ptr,  );	\
	break;						\
    default:						\
	__pu_err = __put_user_bad();			\
	break;						\
    }							\
    __pu_err;						\
})
/*
 * [pregs] = dregs  ==> 32bits
 * H[pregs] = dregs  ==> 16bits
 * B[pregs] = dregs  ==> 8 bits
 */

#define __put_user(x, ptr) put_user(x, ptr)

static inline int bad_user_access_length(void)
{
	panic("bad_user_access_length");
	return -1;
}

#define __put_user_bad() (bad_user_access_length(), (-EFAULT))

/*
 * Tell gcc we read from memory instead of writing: this is because
 * we do not write to any memory gcc knows about, so there are no
 * aliasing issues.
 */

#define __ptr(x) ((unsigned long *)(x))

#define __put_user_asm(err,x,ptr,bhw)					\
	__asm__ (#bhw"[%1] = %0;\n\t"					\
		:	/* no outputs */				\
		:"d" (x),"a" (__ptr(ptr)) : "memory")

#define get_user(x, ptr)						\
({									\
	int __gu_err = 0;						\
	typeof(*(ptr)) __gu_val = 0;					\
	switch (sizeof(*(ptr))) {					\
	case 1:								\
		__get_user_asm(__gu_err, __gu_val, ptr, B, "=d",(Z));	\
		break;							\
	case 2:								\
		__get_user_asm(__gu_err, __gu_val, ptr, W, "=r",(Z));	\
		break;							\
	case 4:								\
		__get_user_asm(__gu_err, __gu_val, ptr,  , "=r",);	\
		break;							\
	default:							\
		__gu_val = 0;						\
		__gu_err = __get_user_bad();				\
		break;							\
	}								\
	(x) = __gu_val;							\
	__gu_err;							\
})

/* dregs = [pregs] ==> 32bits
 * H[pregs]   ==> 16bits
 * B[pregs]   ==> 8 bits
 */

#define __get_user(x, ptr)	get_user(x, ptr)
#define __get_user_bad()	(bad_user_access_length(), (-EFAULT))

#define __get_user_asm(err,x,ptr,bhw,reg,option)		\
	__asm__ ("%0 =" #bhw "[%1]"#option";\n\t"		\
		: "=d" (x)					\
		: "a" (__ptr(ptr)))

#define copy_from_user(to, from, n)	(memcpy(to, from, n), 0)
#define copy_to_user(to, from, n)	(memcpy(to, from, n), 0)

#define __copy_from_user(to, from, n)	copy_from_user(to, from, n)
#define __copy_to_user(to, from, n)	copy_to_user(to, from, n)

#define copy_to_user_ret(to,from,n,retval)	({ if (copy_to_user(to,from,n)) return retval; })
#define copy_from_user_ret(to,from,n,retval)	({ if (copy_from_user(to,from,n)) return retval; })

/*
 * Copy a null terminated string from userspace.
 */

static inline long strncpy_from_user(char *dst, const char *src,
				     long count)
{
	char *tmp;
	strncpy(dst, src, count);
	for (tmp = dst; *tmp && count > 0; tmp++, count--);
	return (tmp - dst);	/* DAVIDM should we count a NUL ?  check getname */
}

/*
 * Return the size of a string (including the ending 0)
 *
 * Return 0 on exception, a value greater than N if too long
 */
static inline long strnlen_user(const char *src, long n)
{
	return (strlen(src) + 1);	/* DAVIDM make safer */
}

#define strlen_user(str) strnlen_user(str, 32767)

/*
 * Zero Userspace
 */

static inline unsigned long clear_user(void *to, unsigned long n)
{
	memset(to, 0, n);
	return (0);
}

#endif
