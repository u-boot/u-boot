/**************************************
 *
 * copyright @ Motorola, 1999
 *
 **************************************/

#include <config.h>
#ifdef CONFIG_HARD_I2C
#include <ppc_asm.tmpl>
#include <asm/mmu.h>
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

/*		  xor r5,r5,r5
*         or  r5,r5,r3
*
*	      lwbrx	r3,r4,r5
*/
	      lwbrx r3,r4,r3
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

	      stwbrx r5,  r4, r3
	      sync

              bclr   20,0



#endif /* CONFIG_HARD_I2C */
