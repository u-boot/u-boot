/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _SETJMP_H_
#define _SETJMP_H_ 1

#ifdef CONFIG_HAVE_SETJMP
#include <asm/setjmp.h>
#else
struct jmp_buf_data {
};
#endif

/**
 * typedef jmp_buf - information needed to restore a calling environment
 */
typedef struct jmp_buf_data jmp_buf[1];

/**
 * setjmp() - prepare for a long jump
 *
 * Registers, the stack pointer, and the return address are saved in the
 * jump bufffer. The function returns zero afterwards. When longjmp() is
 * executed the function returns a second time with a non-zero value.
 *
 * @env:	jump buffer used to store register values
 * Return:	0 after setting up jump buffer, non-zero after longjmp()
 */
int setjmp(jmp_buf env);

/**
 * longjmp() - long jump
 *
 * Jump back to the address and the register state saved by setjmp().
 *
 * @env:	jump buffer
 * @val:	value to be returned by setjmp(), 0 is replaced by 1
 */
void longjmp(jmp_buf env, int val);

#endif /* _SETJMP_H_ */
