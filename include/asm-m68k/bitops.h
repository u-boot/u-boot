/*
 * bitops.h: Bit string operations on the m68k
 */

#ifndef _M68K_BITOPS_H
#define _M68K_BITOPS_H

#include <linux/config.h>
#include <asm/byteorder.h>

extern void set_bit(int nr, volatile void *addr);
extern void clear_bit(int nr, volatile void *addr);
extern void change_bit(int nr, volatile void *addr);
extern int test_and_set_bit(int nr, volatile void *addr);
extern int test_and_clear_bit(int nr, volatile void *addr);
extern int test_and_change_bit(int nr, volatile void *addr);

#endif /* _M68K_BITOPS_H */
