#ifndef _MPC8260_IRQ_H
#define _MPC8260_IRQ_H

/****************************************************************************/
/* most of this was ripped out of include/asm-ppc/irq.h from the Linux/PPC  */
/* source. There was no copyright information in the file.		    */

/*
 * this is the # irq's for all ppc arch's (pmac/chrp/prep)
 * so it is the max of them all
 *
 * [let's just worry about 8260 for now - mjj]
 */
#define NR_IRQS			64

/* The 8260 has an internal interrupt controller with a maximum of
 * 64 IRQs.  We will use NR_IRQs from above since it is large enough.
 * Don't be confused by the 8260 documentation where they list an
 * "interrupt number" and "interrupt vector".  We are only interested
 * in the interrupt vector.  There are "reserved" holes where the
 * vector number increases, but the interrupt number in the table does not.
 * (Document errata updates have fixed this...make sure you have up to
 * date processor documentation -- Dan).
 */
#define NR_SIU_INTS	64

/* There are many more than these, we will add them as we need them.
*/
#define	SIU_INT_SMC1		((uint)0x04)
#define	SIU_INT_SMC2		((uint)0x05)
#define	SIU_INT_FCC1		((uint)0x20)
#define	SIU_INT_FCC2		((uint)0x21)
#define	SIU_INT_FCC3		((uint)0x22)
#define	SIU_INT_SCC1		((uint)0x28)
#define	SIU_INT_SCC2		((uint)0x29)
#define	SIU_INT_SCC3		((uint)0x2a)
#define	SIU_INT_SCC4		((uint)0x2b)

#define NR_MASK_WORDS	((NR_IRQS + 31) / 32)

#endif /* _MPC8260_IRQ_H */
