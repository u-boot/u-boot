/**************************************
 *
 * copyright @ Motorola, 1999
 *
 **************************************/

#include <ppc_asm.tmpl>
#include <ppc_defs.h>
#include <asm/processor.h>

/*********************************************
 * function: CoreExtIntEnable
 *
 * description: Enable 603e core external interrupt
 *
 * note: mtmsr is context-synchronization
 **********************************************/
		.text
		.align 2
	.global CoreExtIntEnable
CoreExtIntEnable:
	 mfmsr    r3

	 ori      r3,r3,0x8000         /* enable external interrupt */
	 mtmsr    r3

	 bclr 20, 0

/*******************************************
 * function: CoreExtIntDisable
 *
 * description: Disable 603e core external interrupt
 *
 * note:
 *******************************************/
		.text
		.align 2
	.global CoreExtIntDisable
CoreExtIntDisable:
	mfmsr    r4

	xor	r3,r3,r3
	or      r3,r3,r4

	andis.	r4,r4,0xffff
	andi.   r3,r3,0x7fff         /* disable external interrupt */

	or      r3,r3,r4
	mtmsr    r3

	bclr 20, 0

/*********************************************************
 * function: epicEOI
 *
 * description: signal the EOI and restore machine status
 *       Input: r3 - value of eumbbar
 *       Output: r3 - value of eumbbar
 *               r4 - ISR vector value
 * note:
 ********************************************************/
		.text
		.align 2
	.global epicEOI
epicEOI:
	lis	r5,0x0006	        /* Build End Of Interrupt Register offset */
	ori	r5,r5,0x00b0
	xor	r7,r7,r7	        /* Clear r7 */
	stwbrx	r7,r5,r3	    /* Save r7, writing to this register will
					     * intidate the end of processing the
					     * highest interrupt.
			     */
	sync

	/* ---RESTORE MACHINE STATE */
	mfmsr	r13		        /* Clear Recoverable Interrupt bit in MSR */
	or      r7,r7,r13

	andis.  r7,r7,0xffff
	andi.	r13,r13,0x7ffd	/* (and disable interrupts) */
	or      r13,r13,r7
	mtmsr	r13

	lwz   r13,0x1c(r1)      /* pull ctr */
	mtctr r13

	lwz   r13,0x18(r1)      /* pull xer */
	mtctr r13

	lwz   r13,0x14(r1)      /* pull lr */
	mtctr r13

	lwz	    r13,0x10(r1)	/* Pull SRR1 from stack */
	mtspr   SRR1,r13	    /* Restore SRR1 */

	lwz	    r13,0xc(r1)	    /* Pull SRR0 from stack */
	mtspr   SRR0,r13	    /* Restore SRR0 */

	lwz	    r13,0x8(r1)	    /* Pull User stack pointer from stack */
	mtspr   SPRG1,r13	    /* Restore SPRG1 */

	lwz	r4,0x4(r1)          /* vector value */
	lwz	r3,0x0(r1)          /* eumbbar */
	sync

	addi	r1,r1,0x20	/* Deallocate stack */
	mtspr   SPRG0,r1	/* Save updated Supervisor stack pointer */
	mfspr   r1,SPRG1	/* Restore User stack pointer */

	bclr     20,0

/***********************************************************
 * function: exception routine called by exception vector
 *           at 0x500, external interrupt
 *
 * description: Kahlua EPIC controller
 *
 * input:  r3 - content of eumbbar
 * output: r3 - ISR return value
 *         r4 - Interrupt vector number
 * note:
 ***********************************************************/

       .text
	   .align 2
       .global epic_exception

epic_exception:

	/*---SAVE MACHINE STATE TO A STACK */
	mtspr   SPRG1,r1	/* Save User stack pointer to SPRG1 */
	mfspr	r1,SPRG0	/* Load Supervisor stack pointer into r1 */

	stwu	r3,-0x20(r1)	/* Push the value of eumbbar onto stack */

	mfspr	r3,SPRG1	/* Push User stack pointer onto stack */
	stw	    r3,0x8(r1)
	mfspr	r3,SRR0	    /* Push SRR0 onto stack */
	stw	    r1,0xc(r1)
	mfspr	r3,SRR1	    /* Push SRR1 onto stack */
	stw	    r3,0x10(r1)
	mflr    r3
	stw     r3,0x14(r1) /* Push LR */
	mfxer   r3
	stw     r3,0x18(r1) /* Push Xer */
	mfctr   r3
	stw     r3,0x1c(r1) /* Push CTR */

	mtspr	SPRG0,r1	/* Save updated Supervisor stack pointer
					 * value to SPRG0
			 */
	mfmsr	r3
	ori	    r3,r3,0x0002	/* Set Recoverable Interrupt bit in MSR */
	mtmsr	r3

	/* ---READ IN THE EUMBAR REGISTER */
    lwz     r6,0(r1)       /* this is eumbbar */
    sync

	/* ---READ EPIC REGISTER:	PROCESSOR INTERRUPT ACKNOWLEDGE REGISTER */
	lis	r5,0x0006	        /* Build Interrupt Acknowledge Register
					     * offset
			     */
	ori	r5,r5,0x00a0
	lwbrx	r7,r5,r6    /* Load interrupt vector into r7 */
	sync

	/* --MASK OFF ALL BITS EXCEPT THE VECTOR */
	xor	r3,r3,r3
    xor r4,r4,r4
	or    r3, r3, r6        /*  eumbbar in r3 */
	andi. r4,r7,0x00ff	/* Mask off bits, vector in r4 */

    stw     r4,0x04(r1)     /* save the vector value */

    lis     r5,epicISR@ha
	ori     r5,r5,epicISR@l
	mtlr    r5
	blrl

    xor   r30,r30,r30
	or    r30,r30,r3        /* save the r3 which containts the return value from epicISR */

	/* ---READ IN THE EUMBAR REGISTER */
    lwz     r3,0(r1)
    sync

    lis     r5,epicEOI@ha
	ori     r5,r5,epicEOI@l
	mtlr    r5
	blrl

    xor  r3,r3,r3
	or   r3,r3,r30           /* restore the ISR return value  */

	bclr     20,0
