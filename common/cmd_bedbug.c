/*
 * BedBug Functions
 */

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <net.h>
#include <bedbug/type.h>
#include <bedbug/bedbug.h>
#include <bedbug/regs.h>
#include <bedbug/ppc.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_BEDBUG)

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

extern void show_regs __P ((struct pt_regs *));
extern int run_command __P ((const char *, int));
extern char console_buffer[];

ulong dis_last_addr = 0;	/* Last address disassembled   */
ulong dis_last_len = 20;	/* Default disassembler length */
CPU_DEBUG_CTX bug_ctx;		/* Bedbug context structure    */


/* ======================================================================
 * U-Boot's puts function does not append a newline, so the bedbug stuff
 * will use this for the output of the dis/assembler.
 * ====================================================================== */

int bedbug_puts (const char *str)
{
	/* -------------------------------------------------- */

	printf ("%s\r\n", str);
	return 0;
}				/* bedbug_puts */



/* ======================================================================
 * Initialize the bug_ctx structure used by the bedbug debugger.  This is
 * specific to the CPU since each has different debug registers and
 * settings.
 * ====================================================================== */

void bedbug_init (void)
{
	/* -------------------------------------------------- */

#if defined(CONFIG_4xx)
	void bedbug405_init (void);

	bedbug405_init ();
#elif defined(CONFIG_8xx)
	void bedbug860_init (void);

	bedbug860_init ();
#endif

#if defined(CONFIG_MPC824X) || defined(CONFIG_MPC8260)
	/* Processors that are 603e core based */
	void bedbug603e_init (void);

	bedbug603e_init ();
#endif

	return;
}				/* bedbug_init */



/* ======================================================================
 * Entry point from the interpreter to the disassembler.  Repeated calls
 * will resume from the last disassembled address.
 * ====================================================================== */
int do_bedbug_dis (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong addr;		/* Address to start disassembly from    */
	ulong len;		/* # of instructions to disassemble     */

	/* -------------------------------------------------- */

	/* Setup to go from the last address if none is given */
	addr = dis_last_addr;
	len = dis_last_len;

	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/* New command */
		addr = simple_strtoul (argv[1], NULL, 16);

		/* If an extra param is given then it is the length */
		if (argc > 2)
			len = simple_strtoul (argv[2], NULL, 16);
	}

	/* Run the disassembler */
	disppc ((unsigned char *) addr, 0, len, bedbug_puts, F_RADHEX);

	dis_last_addr = addr + (len * 4);
	dis_last_len = len;
	return 0;
}				/* do_bedbug_dis */

U_BOOT_CMD (ds, 3, 1, do_bedbug_dis,
	    "ds      - disassemble memory\n",
	    "ds <address> [# instructions]\n");

/* ======================================================================
 * Entry point from the interpreter to the assembler.  Assembles
 * instructions in consecutive memory locations until a '.' (period) is
 * entered on a line by itself.
 * ====================================================================== */
int do_bedbug_asm (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	long mem_addr;		/* Address to assemble into     */
	unsigned long instr;	/* Machine code for text        */
	char prompt[15];	/* Prompt string for user input */
	int asm_err;		/* Error code from the assembler */

	/* -------------------------------------------------- */
	int rcode = 0;

	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	printf ("\nEnter '.' when done\n");
	mem_addr = simple_strtoul (argv[1], NULL, 16);

	while (1) {
		putc ('\n');
		disppc ((unsigned char *) mem_addr, 0, 1, bedbug_puts,
			F_RADHEX);

		sprintf (prompt, "%08lx:    ", mem_addr);
		readline (prompt);

		if (console_buffer[0] && strcmp (console_buffer, ".")) {
			if ((instr =
			     asmppc (mem_addr, console_buffer,
				     &asm_err)) != 0) {
				*(unsigned long *) mem_addr = instr;
				mem_addr += 4;
			} else {
				printf ("*** Error: %s ***\n",
					asm_error_str (asm_err));
				rcode = 1;
			}
		} else {
			break;
		}
	}
	return rcode;
}				/* do_bedbug_asm */

U_BOOT_CMD (as, 2, 0, do_bedbug_asm,
	    "as      - assemble memory\n", "as <address>\n");

/* ======================================================================
 * Used to set a break point from the interpreter.  Simply calls into the
 * CPU-specific break point set routine.
 * ====================================================================== */

int do_bedbug_break (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	/* -------------------------------------------------- */
	if (bug_ctx.do_break)
		(*bug_ctx.do_break) (cmdtp, flag, argc, argv);
	return 0;

}				/* do_bedbug_break */

U_BOOT_CMD (break, 3, 0, do_bedbug_break,
	    "break   - set or clear a breakpoint\n",
	    " - Set or clear a breakpoint\n"
	    "break <address> - Break at an address\n"
	    "break off <bp#> - Disable breakpoint.\n"
	    "break show      - List breakpoints.\n");

/* ======================================================================
 * Called from the debug interrupt routine.  Simply calls the CPU-specific
 * breakpoint handling routine.
 * ====================================================================== */

void do_bedbug_breakpoint (struct pt_regs *regs)
{
	/* -------------------------------------------------- */

	if (bug_ctx.break_isr)
		(*bug_ctx.break_isr) (regs);

	return;
}				/* do_bedbug_breakpoint */



/* ======================================================================
 * Called from the CPU-specific breakpoint handling routine.  Enter a
 * mini main loop until the stopped flag is cleared from the breakpoint
 * context.
 *
 * This handles the parts of the debugger that are common to all CPU's.
 * ====================================================================== */

void bedbug_main_loop (unsigned long addr, struct pt_regs *regs)
{
	int len;		/* Length of command line */
	int flag;		/* Command flags          */
	int rc = 0;		/* Result from run_command */
	char prompt_str[20];	/* Prompt string          */
	static char lastcommand[CFG_CBSIZE] = { 0 };	/* previous command */
	/* -------------------------------------------------- */

	if (bug_ctx.clear)
		(*bug_ctx.clear) (bug_ctx.current_bp);

	printf ("Breakpoint %d: ", bug_ctx.current_bp);
	disppc ((unsigned char *) addr, 0, 1, bedbug_puts, F_RADHEX);

	bug_ctx.stopped = 1;
	bug_ctx.regs = regs;

	sprintf (prompt_str, "BEDBUG.%d =>", bug_ctx.current_bp);

	/* A miniature main loop */
	while (bug_ctx.stopped) {
		len = readline (prompt_str);

		flag = 0;	/* assume no special flags for now */

		if (len > 0)
			strcpy (lastcommand, console_buffer);
		else if (len == 0)
			flag |= CMD_FLAG_REPEAT;

		if (len == -1)
			printf ("<INTERRUPT>\n");
		else
			rc = run_command (lastcommand, flag);

		if (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}

	bug_ctx.regs = NULL;
	bug_ctx.current_bp = 0;

	return;
}				/* bedbug_main_loop */



/* ======================================================================
 * Interpreter command to continue from a breakpoint.  Just clears the
 * stopped flag in the context so that the breakpoint routine will
 * return.
 * ====================================================================== */
int do_bedbug_continue (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	/* -------------------------------------------------- */

	if (!bug_ctx.stopped) {
		printf ("Not at a breakpoint\n");
		return 1;
	}

	bug_ctx.stopped = 0;
	return 0;
}				/* do_bedbug_continue */

U_BOOT_CMD (continue, 1, 0, do_bedbug_continue,
	    "continue- continue from a breakpoint\n",
	    " - continue from a breakpoint.\n");

/* ======================================================================
 * Interpreter command to continue to the next instruction, stepping into
 * subroutines.  Works by calling the find_next_addr() routine to compute
 * the address passes control to the CPU-specific set breakpoint routine
 * for the current breakpoint number.
 * ====================================================================== */
int do_bedbug_step (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned long addr;	/* Address to stop at */

	/* -------------------------------------------------- */

	if (!bug_ctx.stopped) {
		printf ("Not at a breakpoint\n");
		return 1;
	}

	if (!find_next_address ((unsigned char *) &addr, FALSE, bug_ctx.regs))
		return 1;

	if (bug_ctx.set)
		(*bug_ctx.set) (bug_ctx.current_bp, addr);

	bug_ctx.stopped = 0;
	return 0;
}				/* do_bedbug_step */

U_BOOT_CMD (step, 1, 1, do_bedbug_step,
	    "step    - single step execution.\n",
	    " - single step execution.\n");

/* ======================================================================
 * Interpreter command to continue to the next instruction, stepping over
 * subroutines.  Works by calling the find_next_addr() routine to compute
 * the address passes control to the CPU-specific set breakpoint routine
 * for the current breakpoint number.
 * ====================================================================== */
int do_bedbug_next (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned long addr;	/* Address to stop at */

	/* -------------------------------------------------- */

	if (!bug_ctx.stopped) {
		printf ("Not at a breakpoint\n");
		return 1;
	}

	if (!find_next_address ((unsigned char *) &addr, TRUE, bug_ctx.regs))
		return 1;

	if (bug_ctx.set)
		(*bug_ctx.set) (bug_ctx.current_bp, addr);

	bug_ctx.stopped = 0;
	return 0;
}				/* do_bedbug_next */

U_BOOT_CMD (next, 1, 1, do_bedbug_next,
	    "next    - single step execution, stepping over subroutines.\n",
	    " - single step execution, stepping over subroutines.\n");

/* ======================================================================
 * Interpreter command to print the current stack.  This assumes an EABI
 * architecture, so it starts with GPR R1 and works back up the stack.
 * ====================================================================== */
int do_bedbug_stack (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned long sp;	/* Stack pointer                */
	unsigned long func;	/* LR from stack                */
	int depth;		/* Stack iteration level        */
	int skip = 1;		/* Flag to skip the first entry */
	unsigned long top;	/* Top of memory address        */

	/* -------------------------------------------------- */

	if (!bug_ctx.stopped) {
		printf ("Not at a breakpoint\n");
		return 1;
	}

	top = gd->bd->bi_memstart + gd->bd->bi_memsize;
	depth = 0;

	printf ("Depth     PC\n");
	printf ("-----  --------\n");
	printf ("%5d  %08lx\n", depth++, bug_ctx.regs->nip);

	sp = bug_ctx.regs->gpr[1];
	func = *(unsigned long *) (sp + 4);

	while ((func < top) && (sp < top)) {
		if (!skip)
			printf ("%5d  %08lx\n", depth++, func);
		else
			--skip;

		sp = *(unsigned long *) sp;
		func = *(unsigned long *) (sp + 4);
	}
	return 0;
}				/* do_bedbug_stack */

U_BOOT_CMD (where, 1, 1, do_bedbug_stack,
	    "where   - Print the running stack.\n",
	    " - Print the running stack.\n");

/* ======================================================================
 * Interpreter command to dump the registers.  Calls the CPU-specific
 * show registers routine.
 * ====================================================================== */
int do_bedbug_rdump (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	/* -------------------------------------------------- */

	if (!bug_ctx.stopped) {
		printf ("Not at a breakpoint\n");
		return 1;
	}

	show_regs (bug_ctx.regs);
	return 0;
}				/* do_bedbug_rdump */

U_BOOT_CMD (rdump, 1, 1, do_bedbug_rdump,
	    "rdump   - Show registers.\n", " - Show registers.\n");
/* ====================================================================== */
#endif


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
