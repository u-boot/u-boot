#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <syscall.h>

#ifdef CONFIG_ARM /* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
#warning ARM version not implemented yet /* XXXXXXXXXXXXXXXXXXXXXXXXXXXX */
	.global	mon_getc
	.type	mon_getc,function
mon_getc:
	.global	mon_tstc
	.type	mon_tstc,function
mon_tstc:
	.global	mon_putc
	.type	mon_putc,function
mon_putc:
	.global	mon_puts
	.type	mon_puts,function
mon_puts:
	.global	mon_printf
	.type	mon_printf,function
mon_printf:
	.global	mon_install_hdlr
	.type	mon_install_hdlr,function
mon_install_hdlr:
	.global	mon_free_hdlr
	.type	mon_free_hdlr,function
mon_free_hdlr:
	.global	mon_malloc
	.type	mon_malloc,function
mon_malloc:
	.global	mon_free
	.type	mon_free,function
mon_free:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, current_function_anonymous_args = 0
	mov     ip, sp
	stmfd   sp!, {fp, ip, lr, pc}
	sub     fp, ip, #4
	ldmea   fp, {fp, sp, pc}
#elif defined(CONFIG_MIPS)/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
#warning MIPS version not implemented yet
	.global	mon_getc
	.type	mon_getc,function
mon_getc:
	.global	mon_tstc
	.type	mon_tstc,function
mon_tstc:
	.global	mon_putc
	.type	mon_putc,function
mon_putc:
	.global	mon_puts
	.type	mon_puts,function
mon_puts:
	.global	mon_printf
	.type	mon_printf,function
mon_printf:
	.global	mon_install_hdlr
	.type	mon_install_hdlr,function
mon_install_hdlr:
	.global	mon_free_hdlr
	.type	mon_free_hdlr,function
mon_free_hdlr:
	.global	mon_malloc
	.type	mon_malloc,function
mon_malloc:
	.global	mon_free
	.type	mon_free,function
mon_free:

#else

#ifdef CONFIG_I386 /* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
#define SYMBOL_NAME(X) 	X
#define SYMBOL_NAME_LABEL(X) 	X##:
	
#define SYSCALL(name,n) \
	.globl SYMBOL_NAME(name) ; \
SYMBOL_NAME_LABEL(name)     		 ; \
	ret

#endif	/* CONFIG_I386 */

#ifdef CONFIG_PPC
#define SYSCALL(name,n) \
	.globl name		; \
name:				; \
	li r0,n			; \
	sc			; \
	blr
#endif	/* CONFIG_PPC */

	.text

	/*
         * Make sure these functions are in the same order as they
         * appear in the "include/syscall.h" header file !!!
	 */

	SYSCALL(mon_getc,SYSCALL_GETC)
	SYSCALL(mon_tstc,SYSCALL_TSTC)
	SYSCALL(mon_putc,SYSCALL_PUTC)
	SYSCALL(mon_puts,SYSCALL_PUTS)
	SYSCALL(mon_printf,SYSCALL_PRINTF)
	SYSCALL(mon_install_hdlr,SYSCALL_INSTALL_HDLR)
	SYSCALL(mon_free_hdlr,SYSCALL_FREE_HDLR)
	SYSCALL(mon_malloc,SYSCALL_MALLOC)
	SYSCALL(mon_free,SYSCALL_FREE)
	SYSCALL(mon_udelay,SYSCALL_UDELAY)
	SYSCALL(mon_get_timer,SYSCALL_GET_TIMER)
#endif /* CONFIG_ARM XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
