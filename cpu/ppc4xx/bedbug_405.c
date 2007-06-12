/*
 * Bedbug Functions specific to the PPC405 chip
 */

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <bedbug/type.h>
#include <bedbug/bedbug.h>
#include <bedbug/regs.h>
#include <bedbug/ppc.h>

#if ((CONFIG_COMMANDS & CFG_CMD_BEDBUG) || defined(CONFIG_CMD_BEDBUG)) && defined(CONFIG_4xx)

#define MAX_BREAK_POINTS 4

extern CPU_DEBUG_CTX bug_ctx;

void bedbug405_init __P ((void));
void bedbug405_do_break __P ((cmd_tbl_t *, int, int, char *[]));
void bedbug405_break_isr __P ((struct pt_regs *));
int bedbug405_find_empty __P ((void));
int bedbug405_set __P ((int, unsigned long));
int bedbug405_clear __P ((int));


/* ======================================================================
 * Initialize the global bug_ctx structure for the AMCC PPC405.	Clear all
 * of the breakpoints.
 * ====================================================================== */

void bedbug405_init (void)
{
	int i;

	/* -------------------------------------------------- */

	bug_ctx.hw_debug_enabled = 0;
	bug_ctx.stopped = 0;
	bug_ctx.current_bp = 0;
	bug_ctx.regs = NULL;

	bug_ctx.do_break = bedbug405_do_break;
	bug_ctx.break_isr = bedbug405_break_isr;
	bug_ctx.find_empty = bedbug405_find_empty;
	bug_ctx.set = bedbug405_set;
	bug_ctx.clear = bedbug405_clear;

	for (i = 1; i <= MAX_BREAK_POINTS; ++i)
		(*bug_ctx.clear) (i);

	puts ("BEDBUG:ready\n");
	return;
}	/* bedbug_init_breakpoints */



/* ======================================================================
 * Set/clear/show one of the hardware breakpoints for the 405.	The "off"
 * string will disable a specific breakpoint.  The "show" string will
 * display the current breakpoints.  Otherwise an address will set a
 * breakpoint at that address.	Setting a breakpoint uses the CPU-specific
 * set routine which will assign a breakpoint number.
 * ====================================================================== */

void bedbug405_do_break (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	long addr = 0;		/* Address to break at  */
	int which_bp;		/* Breakpoint number    */

	/* -------------------------------------------------- */

	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return;
	}

	/* Turn off a breakpoint */

	if (strcmp (argv[1], "off") == 0) {
		if (bug_ctx.hw_debug_enabled == 0) {
			printf ("No breakpoints enabled\n");
			return;
		}

		which_bp = simple_strtoul (argv[2], NULL, 10);

		if (bug_ctx.clear)
			(*bug_ctx.clear) (which_bp);

		printf ("Breakpoint %d removed\n", which_bp);
		return;
	}

	/* Show a list of breakpoints */

	if (strcmp (argv[1], "show") == 0) {
		for (which_bp = 1; which_bp <= MAX_BREAK_POINTS; ++which_bp) {

			switch (which_bp) {
			case 1:
				addr = GET_IAC1 ();
				break;
			case 2:
				addr = GET_IAC2 ();
				break;
			case 3:
				addr = GET_IAC3 ();
				break;
			case 4:
				addr = GET_IAC4 ();
				break;
			}

			printf ("Breakpoint [%d]: ", which_bp);
			if (addr == 0)
				printf ("NOT SET\n");
			else
				disppc ((unsigned char *) addr, 0, 1, bedbug_puts,
						F_RADHEX);
		}
		return;
	}

	/* Set a breakpoint at the address */

	if (!isdigit (argv[1][0])) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return;
	}

	addr = simple_strtoul (argv[1], NULL, 16) & 0xfffffffc;

	if ((bug_ctx.set) && (which_bp = (*bug_ctx.set) (0, addr)) > 0) {
		printf ("Breakpoint [%d]: ", which_bp);
		disppc ((unsigned char *) addr, 0, 1, bedbug_puts, F_RADHEX);
	}

	return;
}	/* bedbug405_do_break */



/* ======================================================================
 * Handle a breakpoint.	 First determine which breakpoint was hit by
 * looking at the DeBug Status Register (DBSR), clear the breakpoint
 * and enter a mini main loop.	Stay in the loop until the stopped flag
 * in the debug context is cleared.
 * ====================================================================== */

void bedbug405_break_isr (struct pt_regs *regs)
{
	unsigned long dbsr_val;		/* Value of the DBSR    */
	unsigned long addr = 0;		/* Address stopped at   */

	/* -------------------------------------------------- */

	dbsr_val = GET_DBSR ();

	if (dbsr_val & DBSR_IA1) {
		bug_ctx.current_bp = 1;
		addr = GET_IAC1 ();
		SET_DBSR (DBSR_IA1);	/* Write a 1 to clear */
	} else if (dbsr_val & DBSR_IA2) {
		bug_ctx.current_bp = 2;
		addr = GET_IAC2 ();
		SET_DBSR (DBSR_IA2);	/* Write a 1 to clear */
	} else if (dbsr_val & DBSR_IA3) {
		bug_ctx.current_bp = 3;
		addr = GET_IAC3 ();
		SET_DBSR (DBSR_IA3);	/* Write a 1 to clear */
	} else if (dbsr_val & DBSR_IA4) {
		bug_ctx.current_bp = 4;
		addr = GET_IAC4 ();
		SET_DBSR (DBSR_IA4);	/* Write a 1 to clear */
	}

	bedbug_main_loop (addr, regs);
	return;
}	/* bedbug405_break_isr */



/* ======================================================================
 * Look through all of the hardware breakpoints available to see if one
 * is unused.
 * ====================================================================== */

int bedbug405_find_empty (void)
{
	/* -------------------------------------------------- */

	if (GET_IAC1 () == 0)
		return 1;

	if (GET_IAC2 () == 0)
		return 2;

	if (GET_IAC3 () == 0)
		return 3;

	if (GET_IAC4 () == 0)
		return 4;

	return 0;
}	/* bedbug405_find_empty */



/* ======================================================================
 * Set a breakpoint.  If 'which_bp' is zero then find an unused breakpoint
 * number, otherwise reassign the given breakpoint.  If hardware debugging
 * is not enabled, then turn it on via the MSR and DBCR0.  Set the break
 * address in the appropriate IACx register and enable proper address
 * beakpoint in DBCR0.
 * ====================================================================== */

int bedbug405_set (int which_bp, unsigned long addr)
{
	/* -------------------------------------------------- */

	/* Only look if which_bp == 0, else use which_bp */
	if ((bug_ctx.find_empty) && (!which_bp) &&
		(which_bp = (*bug_ctx.find_empty) ()) == 0) {
		printf ("All breakpoints in use\n");
		return 0;
	}

	if (which_bp < 1 || which_bp > MAX_BREAK_POINTS) {
		printf ("Invalid break point # %d\n", which_bp);
		return 0;
	}

	if (!bug_ctx.hw_debug_enabled) {
		SET_MSR (GET_MSR () | 0x200);	/* set MSR[ DE ] */
		SET_DBCR0 (GET_DBCR0 () | DBCR0_IDM);
		bug_ctx.hw_debug_enabled = 1;
	}

	switch (which_bp) {
	case 1:
		SET_IAC1 (addr);
		SET_DBCR0 (GET_DBCR0 () | DBCR0_IA1);
		break;

	case 2:
		SET_IAC2 (addr);
		SET_DBCR0 (GET_DBCR0 () | DBCR0_IA2);
		break;

	case 3:
		SET_IAC3 (addr);
		SET_DBCR0 (GET_DBCR0 () | DBCR0_IA3);
		break;

	case 4:
		SET_IAC4 (addr);
		SET_DBCR0 (GET_DBCR0 () | DBCR0_IA4);
		break;
	}

	return which_bp;
}	/* bedbug405_set */



/* ======================================================================
 * Disable a specific breakoint by setting the appropriate IACx register
 * to zero and claring the instruction address breakpoint in DBCR0.
 * ====================================================================== */

int bedbug405_clear (int which_bp)
{
	/* -------------------------------------------------- */

	if (which_bp < 1 || which_bp > MAX_BREAK_POINTS) {
		printf ("Invalid break point # (%d)\n", which_bp);
		return -1;
	}

	switch (which_bp) {
	case 1:
		SET_IAC1 (0);
		SET_DBCR0 (GET_DBCR0 () & ~DBCR0_IA1);
		break;

	case 2:
		SET_IAC2 (0);
		SET_DBCR0 (GET_DBCR0 () & ~DBCR0_IA2);
		break;

	case 3:
		SET_IAC3 (0);
		SET_DBCR0 (GET_DBCR0 () & ~DBCR0_IA3);
		break;

	case 4:
		SET_IAC4 (0);
		SET_DBCR0 (GET_DBCR0 () & ~DBCR0_IA4);
		break;
	}

	return 0;
}	/* bedbug405_clear */


/* ====================================================================== */
#endif
