/**************************************
 *
 * copyright @ Motorola, 1999
 *
 **************************************/

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

	lwbrx	r3,r4,r3
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
