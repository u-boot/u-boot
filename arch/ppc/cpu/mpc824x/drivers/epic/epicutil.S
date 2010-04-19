/**************************************
 *
 * copyright @ Motorola, 1999
 *
 *
 * This file contains two commonly used
 * lower level utility routines.
 *
 * The utility routines are also in other
 * Kahlua device driver libraries. The
 * need to be linked in only once.
 **************************************/

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

/**********************************************************
 * function: load_runtime_reg
 *
 * input:  r3 - value of eumbbar
 *         r4 - register offset in embedded utility space
 *
 * output: r3 - register content
 **********************************************************/
      .text
      .align 2
      .global load_runtime_reg

load_runtime_reg:

		  xor r5,r5,r5
	  or  r5,r5,r3       /* save eumbbar */

	      lwbrx	r3,r4,r5
	      sync

	      bclr 20, 0

/****************************************************************
 * function: store_runtime_reg
 *
 * input: r3 - value of eumbbar
 *        r4 - register offset in embedded utility space
 *        r5 - new value to be stored
 *
 ****************************************************************/
	   .text
	   .align 2
	   .global store_runtime_reg
store_runtime_reg:

		  xor r0,r0,r0

	      stwbrx r5,  r4, r3
	      sync

		  bclr   20,0
