/*
 * (C) Copyright 2002
 * John W. Linville, linville@tuxdriver.com
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include "i8259.h"

#undef  IRQ_DEBUG

#ifdef  IRQ_DEBUG
#define PRINTF(fmt,args...)     printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

static inline unsigned char read_byte(volatile unsigned char* from)
{
    int x;
    asm volatile ("lbz %0,%1\n eieio" : "=r" (x) : "m" (*from));
    return (unsigned char)x;
}

static inline void write_byte(volatile unsigned char *to, int x)
{
    asm volatile ("stb %1,%0\n eieio" : "=m" (*to) : "r" (x));
}

static inline unsigned long read_long_little(volatile unsigned long *from)
{
    unsigned long x;
    asm volatile ("lwbrx %0,0,%1\n eieio\n sync" : "=r" (x) : "r" (from), "m"(*from));
    return (unsigned long)x;
}

#ifdef out8
#undef out8
#endif

#ifdef in8
#undef in8
#endif

#define out8(addr, byte) write_byte(0xFE000000 | addr, byte)
#define in8(addr) read_byte(0xFE000000 | addr)

/*
 * This contains the irq mask for both 8259A irq controllers,
 */
static char cached_imr[2] = {0xff, 0xff};

#define cached_imr1	(cached_imr[0])
#define cached_imr2	(cached_imr[1])

void i8259_init(void)
{
	char dummy;
	PRINTF("Initializing Interrupt controller\n");
	/* init master interrupt controller */
	out8(0x20, 0x11); /* 0x19); /###* Start init sequence */
	out8(0x21, 0x00); /* Vector base */
	out8(0x21, 0x04); /* edge tiggered, Cascade (slave) on IRQ2 */
	out8(0x21, 0x11); /* was: 0x01); /###* Select 8086 mode */

	/* init slave interrupt controller */
	out8(0xA0, 0x11); /* 0x19); /###* Start init sequence */
	out8(0xA1, 0x08); /* Vector base */
	out8(0xA1, 0x02); /* edge triggered, Cascade (slave) on IRQ2 */
	out8(0xA1, 0x11); /* was: 0x01); /###* Select 8086 mode */

	/* always read ISR */
	out8(0x20, 0x0B);
	dummy = in8(ISR_1);
	out8(0xA0, 0x0B);
	dummy = in8(ISR_2);

/*     out8(0x43, 0x30); */
/*     out8(0x40, 0); */
/*     out8(0x40, 0); */
/*     out8(0x43, 0x70); */
/*     out8(0x41, 0); */
/*     out8(0x41, 0); */
/*     out8(0x43, 0xb0); */
/*     out8(0x42, 0); */
/*     out8(0x42, 0); */

	/* Mask all interrupts */
	out8(IMR_2, cached_imr2);
	out8(IMR_1, cached_imr1);

	i8259_unmask_irq(2);
#if 0
	{
	    int i;
	    for (i=0; i<16; i++)
	    {
		i8259_unmask_irq(i);
	    }
	}
#endif
}

static volatile char *pci_intack = (void *)0xFEF00000;

int i8259_irq(void)
{
	int irq;

	irq = read_long_little(pci_intack) & 0xff;
	if (irq==7) {
		/*
		 * This may be a spurious interrupt.
		 *
		 * Read the interrupt status register (ISR). If the most
		 * significant bit is not set then there is no valid
		 * interrupt.
		 */
		if(~in8(0x20)&0x80) {
			irq = -1;
		}
	}

	return irq;
}
int i8259_get_irq(struct pt_regs *regs)
{
	unsigned char irq;

	/*
	 * Perform an interrupt acknowledge cycle on controller 1
	 */
	out8(OCW3_1, 0x0C); /* prepare for poll */
	irq = in8(IPL_1) & 7;
	if (irq == 2) {
		/*
		 * Interrupt is cascaded so perform interrupt
		 * acknowledge on controller 2
		 */
		out8(OCW3_2, 0x0C); /* prepare for poll */
		irq = (in8(IPL_2) & 7) + 8;
		if (irq == 15) {
			/*
			 * This may be a spurious interrupt
			 *
			 * Read the interrupt status register. If the most
			 * significant bit is not set then there is no valid
			 * interrupt
			 */
			out8(OCW3_2, 0x0b);
			if (~(in8(ISR_2) & 0x80)) {
				return -1;
			}
		}
	} else if (irq == 7) {
		/*
		 * This may be a spurious interrupt
		 *
		 * Read the interrupt status register. If the most
		 * significant bit is not set then there is no valid
		 * interrupt
		 */
		out8(OCW3_1, 0x0b);
		if (~(in8(ISR_1) & 0x80)) {
			return -1;
		}
	}
	return irq;
}

/*
 * Careful! The 8259A is a fragile beast, it pretty
 * much _has_ to be done exactly like this (mask it
 * first, _then_ send the EOI, and the order of EOI
 * to the two 8259s is important!
 */
void i8259_mask_and_ack(int irq)
{
	if (irq > 7) {
		cached_imr2 |= (1 << (irq - 8));
		in8(IMR_2); /* DUMMY */
		out8(IMR_2, cached_imr2);
		out8(OCW2_2, 0x20); /* Non-specific EOI */
		out8(OCW2_1, 0x20); /* Non-specific EOI to cascade */
	} else {
		cached_imr1 |= (1 << irq);
		in8(IMR_1); /* DUMMY */
		out8(IMR_1, cached_imr1);
		out8(OCW2_1, 0x20); /* Non-specific EOI */
	}
}

void i8259_mask_irq(int irq)
{
	if (irq & 8) {
		cached_imr2 |= (1 << (irq & 7));
		out8(IMR_2, cached_imr2);
	} else {
		cached_imr1 |= (1 << irq);
		out8(IMR_1, cached_imr1);
	}
}

void i8259_unmask_irq(int irq)
{
	if (irq & 8) {
		cached_imr2 &= ~(1 << (irq & 7));
		out8(IMR_2, cached_imr2);
	} else {
		cached_imr1 &= ~(1 << irq);
		out8(IMR_1, cached_imr1);
	}
}
