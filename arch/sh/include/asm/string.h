#ifndef __ASM_SH_STRING_H
#define __ASM_SH_STRING_H

/*
 * Copyright (C) 1999 Niibe Yutaka
 * But consider these trivial functions to be public domain.
 *
 * from linux kernel code.
 */

#define __HAVE_ARCH_STRCPY
static inline char *strcpy(char *__dest, const char *__src)
{
	register char *__xdest = __dest;
	unsigned long __dummy;

	__asm__ __volatile__("1:\n\t"
			     "mov.b	@%1+, %2\n\t"
			     "mov.b	%2, @%0\n\t"
			     "cmp/eq	#0, %2\n\t"
			     "bf/s	1b\n\t"
			     " add	#1, %0\n\t"
			     : "=r" (__dest), "=r" (__src), "=&z" (__dummy)
			     : "0" (__dest), "1" (__src)
			     : "memory", "t");

	return __xdest;
}

#define __HAVE_ARCH_STRNCPY
static inline char *strncpy(char *__dest, const char *__src, size_t __n)
{
	register char *__xdest = __dest;
	unsigned long __dummy;

	if (__n == 0)
		return __xdest;

	__asm__ __volatile__(
		"1:\n"
		"mov.b	@%1+, %2\n\t"
		"mov.b	%2, @%0\n\t"
		"cmp/eq	#0, %2\n\t"
		"bt/s	2f\n\t"
		" cmp/eq	%5,%1\n\t"
		"bf/s	1b\n\t"
		" add	#1, %0\n"
		"2:"
		: "=r" (__dest), "=r" (__src), "=&z" (__dummy)
		: "0" (__dest), "1" (__src), "r" (__src+__n)
		: "memory", "t");

	return __xdest;
}

#define __HAVE_ARCH_STRCMP
static inline int strcmp(const char *__cs, const char *__ct)
{
	register int __res;
	unsigned long __dummy;

	__asm__ __volatile__(
		"mov.b	@%1+, %3\n"
		"1:\n\t"
		"mov.b	@%0+, %2\n\t"
		"cmp/eq #0, %3\n\t"
		"bt	2f\n\t"
		"cmp/eq %2, %3\n\t"
		"bt/s	1b\n\t"
		" mov.b	@%1+, %3\n\t"
		"add	#-2, %1\n\t"
		"mov.b	@%1, %3\n\t"
		"sub	%3, %2\n"
		"2:"
		: "=r" (__cs), "=r" (__ct), "=&r" (__res), "=&z" (__dummy)
		: "0" (__cs), "1" (__ct)
		: "t");

	return __res;
}

#endif /* __ASM_SH_STRING_H */
