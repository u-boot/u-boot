/*
 * BedBug Functions
 */
#ifndef _CMD_BEDBUG_H
#define _CMD_BEDBUG_H

#if (CONFIG_COMMANDS & CFG_CMD_BEDBUG)

#define CMD_TBL_DIS     MK_CMD_TBL_ENTRY(                               \
        "ds",           2,      3,      1,      do_bedbug_dis,          \
        "ds      - disassemble memory\n",                               \
        "ds <address> [# instructions]\n"                               \
        ),

#define CMD_TBL_ASM     MK_CMD_TBL_ENTRY(                               \
        "as",           2,      2,      0,      do_bedbug_asm,          \
        "as      - assemble memory\n",                                  \
        "as <address>\n"                                                \
        ),

#define CMD_TBL_BREAK   MK_CMD_TBL_ENTRY(                               \
        "break",        2,      3,      0,      do_bedbug_break,        \
        "break   - set or clear a breakpoint\n",                        \
        " - Set or clear a breakpoint\n"                                \
        "break <address> - Break at an address\n"                       \
        "break off <bp#> - Disable breakpoint.\n"                       \
        "break show      - List breakpoints.\n"                         \
        ),

#define CMD_TBL_CONTINUE        MK_CMD_TBL_ENTRY(                       \
        "continue",     4,      1,      0,      do_bedbug_continue,     \
        "continue- continue from a breakpoint\n",                       \
        " - continue from a breakpoint.\n"                              \
        ),

#define CMD_TBL_STEP        MK_CMD_TBL_ENTRY(                           \
        "step", 4,      1,      1,      do_bedbug_step,                 \
        "step    - single step execution.\n",                           \
        " - single step execution.\n"                                   \
        ),

#define CMD_TBL_NEXT        MK_CMD_TBL_ENTRY(                           \
        "next", 4,      1,      1,      do_bedbug_next,                 \
        "next    - single step execution, stepping over subroutines.\n",\
        " - single step execution, stepping over subroutines.\n"        \
        ),

#define CMD_TBL_STACK        MK_CMD_TBL_ENTRY(                          \
        "where", 5,     1,      1,      do_bedbug_stack,                \
        "where   - Print the running stack.\n",                         \
        " - Print the running stack.\n"                                 \
        ),

#define CMD_TBL_RDUMP        MK_CMD_TBL_ENTRY(                          \
        "rdump", 5,     1,      1,      do_bedbug_rdump,                \
        "rdump   - Show registers.\n",                                  \
        " - Show registers.\n"                                          \
        ),

extern int do_bedbug_dis      (cmd_tbl_t *, int, int, char *[]);
extern int do_bedbug_asm      (cmd_tbl_t *, int, int, char *[]);
extern int do_bedbug_break    (cmd_tbl_t *, int, int, char *[]);
extern int do_bedbug_continue (cmd_tbl_t *, int, int, char *[]);
extern int do_bedbug_step     (cmd_tbl_t *, int, int, char *[]);
extern int do_bedbug_next     (cmd_tbl_t *, int, int, char *[]);
extern int do_bedbug_stack    (cmd_tbl_t *, int, int, char *[]);
extern int do_bedbug_rdump    (cmd_tbl_t *, int, int, char *[]);

/* Supporting routines */
extern int bedbug_puts (const char *);
extern void bedbug_init (void);
extern void do_bedbug_breakpoint (struct pt_regs *);
extern void bedbug_main_loop (unsigned long, struct pt_regs *);


typedef struct {
	int hw_debug_enabled;
	int stopped;
	int current_bp;
	struct pt_regs *regs;

	void (*do_break) (cmd_tbl_t *, int, int, char *[]);
	void (*break_isr) (struct pt_regs *);
	int (*find_empty) (void);
	int (*set) (int, unsigned long);
	int (*clear) (int);
} CPU_DEBUG_CTX;

#else /* ! CFG_CMD_BEDBUG */

#define CMD_TBL_DIS
#define CMD_TBL_ASM
#define CMD_TBL_BREAK
#define CMD_TBL_CONTINUE
#define CMD_TBL_STEP
#define CMD_TBL_NEXT
#define CMD_TBL_STACK
#define CMD_TBL_RDUMP

#endif /* CFG_CMD_BEDBUG */
#endif /* _CMD_BEDBUG_H */


/*
 * Copyright (c) 2001 William L. Pitts
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 */
