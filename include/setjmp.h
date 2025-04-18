/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _SETJMP_H_
#define _SETJMP_H_ 1

/**
 * DOC: Overview
 *
 * The long jump API allows to perform nonlocal gotos, that is jump from one
 * function to another typically further down in the stack, while properly
 * restoring the stack's state (unwinding). The two functions needed to do this
 * are setjmp() and longjmp().
 *
 * In addition to these two standard POSIX.1-2001/C89 functions, a third one is
 * present in U-Boot: initjmp(). It is an extension which allows to implement
 * user-mode threads.
 */

#ifdef CONFIG_HAVE_SETJMP
#include <asm/setjmp.h>
#else
struct jmp_buf_data {
};
#endif
#include <linux/compiler_attributes.h>
#include <stddef.h>

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

/**
 * initjmp() - prepare for a long jump to a given function with a given stack
 *
 * This function sets up a jump buffer for later use with longjmp(). It allows
 * to branch to a specific function with a specific stack. Please note that
 * @func MUST NOT return. It shall typically restore the main stack and resume
 * execution by doing a long jump to a jump buffer initialized by setjmp()
 * before the long jump. initjmp() allows to implement multithreading.
 *
 * @env:	jump buffer
 * @func:	function to be called on longjmp(), MUST NOT RETURN
 * @stack_base:	the stack to be used by @func (lower address)
 * @stack_sz:	the stack size in bytes
 */
int initjmp(jmp_buf env, void __noreturn (*func)(void), void *stack_base,
	    size_t stack_sz);

#endif /* _SETJMP_H_ */
