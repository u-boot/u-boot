#ifndef __MON_SYS_CALL_H__
#define __MON_SYS_CALL_H__

#ifndef __ASSEMBLY__

#include <common.h>

/* These are declarations of system calls available in C code */
int  mon_getc(void);
int  mon_tstc(void);
void mon_putc(const char);
void mon_puts(const char*);
void mon_printf(const char* fmt, ...);
void mon_install_hdlr(int, interrupt_handler_t*, void*);
void mon_free_hdlr(int);
void *mon_malloc(size_t);
void mon_free(void*);
void mon_udelay(unsigned long);
unsigned long mon_get_timer(unsigned long);

#endif    /* ifndef __ASSEMBLY__ */

#define NR_SYSCALLS            11        /* number of syscalls */


/*
 * Make sure these functions are in the same order as they
 * appear in the "examples/syscall.S" file !!!
 */
#define SYSCALL_GETC           0
#define SYSCALL_TSTC           1
#define SYSCALL_PUTC           2
#define SYSCALL_PUTS           3
#define SYSCALL_PRINTF         4
#define SYSCALL_INSTALL_HDLR   5
#define SYSCALL_FREE_HDLR      6
#define SYSCALL_MALLOC         7
#define SYSCALL_FREE           8
#define SYSCALL_UDELAY         9
#define SYSCALL_GET_TIMER     10

#endif
