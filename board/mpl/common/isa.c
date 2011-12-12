/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 *
 * TODO: clean-up
 */

#include <common.h>
#include <asm/processor.h>
#include <stdio_dev.h>
#include "isa.h"
#include "piix4_pci.h"
#include "kbd.h"
#include "video.h"


#undef	ISA_DEBUG

#ifdef	ISA_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#ifndef	TRUE
#define TRUE            1
#endif
#ifndef FALSE
#define FALSE           0
#endif

#if defined(CONFIG_PIP405)

extern int drv_isa_kbd_init (void);

/* fdc (logical device 0) */
const SIO_LOGDEV_TABLE sio_fdc[] = {
	{0x60, 3},			/* set IO to FDPort (3F0) */
	{0x61, 0xF0},		/* set IO to FDPort (3F0) */
	{0x70, 06},			/* set IRQ 6 for FDPort */
	{0x74, 02},			/* set DMA 2 for FDPort */
	{0xF0, 0x05},		/* set to PS2 type */
	{0xF1, 0x00},	  /* default value */
	{0x30, 1},			/* and activate the device */
	{0xFF, 0}				/* end of device table */
};
/* paralell port (logical device 3) */
const SIO_LOGDEV_TABLE sio_pport[] = {
	{0x60, 3},			/* set IO to PPort (378) */
	{0x61, 0x78},		/* set IO to PPort (378) */
	{0x70, 07},			/* set IRQ 7 for PPort */
	{0xF1, 00},			/* set PPort to normal */
	{0x30, 1},			/* and activate the device */
	{0xFF, 0}				/* end of device table */
};
/* paralell port (logical device 3) Floppy assigned to lpt */
const SIO_LOGDEV_TABLE sio_pport_fdc[] = {
	{0x60, 3},			/* set IO to PPort (378) */
	{0x61, 0x78},		/* set IO to PPort (378) */
	{0x70, 07},			/* set IRQ 7 for PPort */
	{0xF1, 02},			/* set PPort to Floppy */
	{0x30, 1},			/* and activate the device */
	{0xFF, 0}				/* end of device table */
};
/* uart 1 (logical device 4) */
const SIO_LOGDEV_TABLE sio_com1[] = {
	{0x60, 3},			/* set IO to COM1 (3F8) */
	{0x61, 0xF8},		/* set IO to COM1 (3F8) */
	{0x70, 04},			/* set IRQ 4 for COM1 */
	{0x30, 1},			/* and activate the device */
	{0xFF, 0}				/* end of device table */
};
/* uart 2 (logical device 5) */
const SIO_LOGDEV_TABLE sio_com2[] = {
	{0x60, 2},			/* set IO to COM2 (2F8) */
	{0x61, 0xF8},		/* set IO to COM2 (2F8) */
	{0x70, 03},			/* set IRQ 3 for COM2 */
	{0x30, 1},			/* and activate the device */
	{0xFF, 0}				/* end of device table */
};

/* keyboard controller (logical device 7) */
const SIO_LOGDEV_TABLE sio_keyboard[] = {
	{0x70, 1},			/* set IRQ 1 for keyboard */
	{0x72, 12},			/* set IRQ 12 for mouse */
	{0xF0, 0},			/* disable Port92 (this is a PowerPC!!) */
	{0x30, 1},			/* and activate the device */
	{0xFF, 0}				/* end of device table */
};


/*******************************************************************************
* Config SuperIO FDC37C672
********************************************************************************/
unsigned char open_cfg_super_IO(int address)
{
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address,0x55); /* open config */
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address,0x20); /* set address to DEV ID */
	if(in8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address | 0x1)==0x40) /* ok Device ID is correct */
		return TRUE;
	else
		return FALSE;
}

void close_cfg_super_IO(int address)
{
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address,0xAA); /* close config */
}


unsigned char read_cfg_super_IO(int address, unsigned char function, unsigned char regaddr)
{
	/* assuming config reg is open */
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address,0x7); /* points to the function reg */
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address | 1,function); /* set the function no */
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address,regaddr); /* sets the address in the function */
	return in8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address | 1);
}

void write_cfg_super_IO(int address, unsigned char function, unsigned char regaddr, unsigned char data)
{
	/* assuming config reg is open */
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address,0x7); /* points to the function reg */
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address | 1,function); /* set the function no */
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address,regaddr); /* sets the address in the function */
	out8(CONFIG_SYS_ISA_IO_BASE_ADDRESS | address | 1,data); /* writes the data */
}

void isa_write_table(SIO_LOGDEV_TABLE *ldt,unsigned char ldev)
{
	while (ldt->index != 0xFF) {
		write_cfg_super_IO(SIO_CFG_PORT, ldev, ldt->index, ldt->val);
		ldt++;
	} /* endwhile */
}

void isa_sio_loadtable(void)
{
	char *s = getenv("floppy");
	/* setup Floppy device 0*/
	isa_write_table((SIO_LOGDEV_TABLE *)&sio_fdc,0);
	/* setup parallel port device 3 */
	if(s && !strncmp(s, "lpt", 3)) {
		printf("SIO:   Floppy assigned to LPT\n");
		/* floppy is assigned to the LPT */
		isa_write_table((SIO_LOGDEV_TABLE *)&sio_pport_fdc,3);
	}
	else {
		/*printf("Floppy assigned to internal port\n");*/
		isa_write_table((SIO_LOGDEV_TABLE *)&sio_pport,3);
	}
	/* setup Com1 port device 4 */
	isa_write_table((SIO_LOGDEV_TABLE *)&sio_com1,4);
	/* setup Com2 port device 5 */
	isa_write_table((SIO_LOGDEV_TABLE *)&sio_com2,5);
	/* setup keyboards device 7 */
	isa_write_table((SIO_LOGDEV_TABLE *)&sio_keyboard,7);
}


void isa_sio_setup(void)
{
	if(open_cfg_super_IO(SIO_CFG_PORT)==TRUE)
	{
		isa_sio_loadtable();
		close_cfg_super_IO(0x3F0);
	}
}
#endif

/******************************************************************************
 * IRQ Controller
 * we use the Vector mode
 */

struct	isa_irq_action {
	 interrupt_handler_t *handler;
	 void *arg;
	 int count;
};

static struct isa_irq_action isa_irqs[16];


/*
 * This contains the irq mask for both 8259A irq controllers,
 */
static unsigned int cached_irq_mask = 0xfff9;

#define cached_imr1	(unsigned char)cached_irq_mask
#define cached_imr2	(unsigned char)(cached_irq_mask>>8)
#define IMR_1		CONFIG_SYS_ISA_IO_BASE_ADDRESS + PIIX4_ISA_INT1_OCW1
#define IMR_2		CONFIG_SYS_ISA_IO_BASE_ADDRESS + PIIX4_ISA_INT2_OCW1
#define ICW1_1	CONFIG_SYS_ISA_IO_BASE_ADDRESS + PIIX4_ISA_INT1_ICW1
#define ICW1_2	CONFIG_SYS_ISA_IO_BASE_ADDRESS + PIIX4_ISA_INT2_ICW1
#define ICW2_1	CONFIG_SYS_ISA_IO_BASE_ADDRESS + PIIX4_ISA_INT1_ICW2
#define ICW2_2	CONFIG_SYS_ISA_IO_BASE_ADDRESS + PIIX4_ISA_INT2_ICW2
#define ICW3_1	ICW2_1
#define ICW3_2	ICW2_2
#define ICW4_1	ICW2_1
#define ICW4_2	ICW2_2
#define ISR_1		ICW1_1
#define ISR_2		ICW1_2


void disable_8259A_irq(unsigned int irq)
{
	unsigned int mask = 1 << irq;

	cached_irq_mask |= mask;
	if (irq & 8)
		out8(IMR_2,cached_imr2);
	else
		out8(IMR_1,cached_imr1);
}

void enable_8259A_irq(unsigned int irq)
{
	unsigned int mask = ~(1 << irq);

	cached_irq_mask &= mask;
	if (irq & 8)
		out8(IMR_2,cached_imr2);
	else
		out8(IMR_1,cached_imr1);
}
/*
int i8259A_irq_pending(unsigned int irq)
{
	unsigned int mask = 1<<irq;
	int ret;

	if (irq < 8)
		ret = inb(0x20) & mask;
	else
		ret = inb(0xA0) & (mask >> 8);
	spin_unlock_irqrestore(&i8259A_lock, flags);

	return ret;
}
*/

/*
 * This function assumes to be called rarely. Switching between
 * 8259A registers is slow.
 */
int i8259A_irq_real(unsigned int irq)
{
	int value;
	int irqmask = 1<<irq;

	if (irq < 8) {
		out8(ISR_1,0x0B);		/* ISR register */
		value = in8(ISR_1) & irqmask;
		out8(ISR_1,0x0A);		/* back to the IRR register */
		return value;
	}
	out8(ISR_2,0x0B);		/* ISR register */
	value = in8(ISR_2) & (irqmask >> 8);
	out8(ISR_2,0x0A);		/* back to the IRR register */
	return value;
}

/*
 * Careful! The 8259A is a fragile beast, it pretty
 * much _has_ to be done exactly like this (mask it
 * first, _then_ send the EOI, and the order of EOI
 * to the two 8259s is important!
 */
void mask_and_ack_8259A(unsigned int irq)
{
	unsigned int irqmask = 1 << irq;
	unsigned int temp_irqmask = cached_irq_mask;
	/*
	 * Lightweight spurious IRQ detection. We do not want
	 * to overdo spurious IRQ handling - it's usually a sign
	 * of hardware problems, so we only do the checks we can
	 * do without slowing down good hardware unnecesserily.
	 *
	 * Note that IRQ7 and IRQ15 (the two spurious IRQs
	 * usually resulting from the 8259A-1|2 PICs) occur
	 * even if the IRQ is masked in the 8259A. Thus we
	 * can check spurious 8259A IRQs without doing the
	 * quite slow i8259A_irq_real() call for every IRQ.
	 * This does not cover 100% of spurious interrupts,
	 * but should be enough to warn the user that there
	 * is something bad going on ...
	 */
	if (temp_irqmask & irqmask)
		goto spurious_8259A_irq;
	temp_irqmask |= irqmask;

handle_real_irq:
	if (irq & 8) {
		in8(IMR_2);		/* DUMMY - (do we need this?) */
		out8(IMR_2,(unsigned char)(temp_irqmask>>8));
		out8(ISR_2,0x60+(irq&7));/* 'Specific EOI' to slave */
		out8(ISR_1,0x62);	/* 'Specific EOI' to master-IRQ2 */
		out8(IMR_2,cached_imr2); /* turn it on again */
	} else {
		in8(IMR_1);		/* DUMMY - (do we need this?) */
		out8(IMR_1,(unsigned char)temp_irqmask);
		out8(ISR_1,0x60+irq);	/* 'Specific EOI' to master */
		out8(IMR_1,cached_imr1); /* turn it on again */
	}

	return;

spurious_8259A_irq:
	/*
	 * this is the slow path - should happen rarely.
	 */
	if (i8259A_irq_real(irq))
		/*
		 * oops, the IRQ _is_ in service according to the
		 * 8259A - not spurious, go handle it.
		 */
		goto handle_real_irq;

	{
		static int spurious_irq_mask;
		/*
		 * At this point we can be sure the IRQ is spurious,
		 * lets ACK and report it. [once per IRQ]
		 */
		if (!(spurious_irq_mask & irqmask)) {
			PRINTF("spurious 8259A interrupt: IRQ%d.\n", irq);
			spurious_irq_mask |= irqmask;
		}
		/* irq_err_count++; */
		/*
		 * Theoretically we do not have to handle this IRQ,
		 * but in Linux this does not cause problems and is
		 * simpler for us.
		 */
		goto handle_real_irq;
	}
}

void init_8259A(void)
{
	out8(IMR_1,0xff);	/* mask all of 8259A-1 */
	out8(IMR_2,0xff);	/* mask all of 8259A-2 */

	out8(ICW1_1,0x11);	/* ICW1: select 8259A-1 init */
	out8(ICW2_1,0x20 + 0);	/* ICW2: 8259A-1 IR0-7 mapped to 0x20-0x27 */
	out8(ICW3_1,0x04);	/* 8259A-1 (the master) has a slave on IR2 */
	out8(ICW4_1,0x01);	/* master expects normal EOI */
	out8(ICW1_2,0x11);	/* ICW2: select 8259A-2 init */
	out8(ICW2_2,0x20 + 8);	/* ICW2: 8259A-2 IR0-7 mapped to 0x28-0x2f */
	out8(ICW3_2,0x02);	/* 8259A-2 is a slave on master's IR2 */
	out8(ICW4_2,0x01);	/* (slave's support for AEOI in flat mode
				    is to be investigated) */
	udelay(10000);		/* wait for 8259A to initialize */
	out8(IMR_1,cached_imr1);	/* restore master IRQ mask */
	udelay(10000);		/* wait for 8259A to initialize */
	out8(IMR_2,cached_imr2);	/* restore slave IRQ mask */
}


#define PCI_INT_ACK_ADDR 0xEED00000

int handle_isa_int(void)
{
	unsigned long irqack;
	unsigned char irq;
	/* first we acknokledge the int via the PCI bus */
	irqack=in32(PCI_INT_ACK_ADDR);
	/* now we get the ISRs */
	in8(ISR_2);
	in8(ISR_1);
	irq=(unsigned char)irqack;
	irq-=32;
/*	if((irq==7)&&((isr1&0x80)==0)) {
		PRINTF("IRQ7 detected but not in ISR\n");
	}
	else {
*/		/* we should handle cascaded interrupts here also */
	{
/*		printf("ISA Irq %d\n",irq); */
		isa_irqs[irq].count++;
		if(irq!=2) { /* just swallow the cascade irq 2 */
			if (isa_irqs[irq].handler != NULL)
				(*isa_irqs[irq].handler)(isa_irqs[irq].arg);      /* call isr */
			else {
				PRINTF ("bogus interrupt vector 0x%x\n", irq);
			}
		}
	}
	/* issue EOI instruction to clear the IRQ */
	mask_and_ack_8259A(irq);
	return 0;
}


/******************************************************************
 * Install and free an ISA interrupt handler.
 */

void isa_irq_install_handler(int vec, interrupt_handler_t *handler, void *arg)
{
	if (isa_irqs[vec].handler != NULL) {
		printf ("ISA Interrupt vector %d: handler 0x%x replacing 0x%x\n",
			vec, (uint)handler, (uint)isa_irqs[vec].handler);
	}
	isa_irqs[vec].handler = handler;
	isa_irqs[vec].arg     = arg;
	enable_8259A_irq(vec);
	PRINTF ("Install ISA IRQ %d ==> %p, @ %p mask=%04x\n", vec, handler, &isa_irqs[vec].handler,cached_irq_mask);

}

void isa_irq_free_handler(int vec)
{
	disable_8259A_irq(vec);
	isa_irqs[vec].handler = NULL;
	isa_irqs[vec].arg     = NULL;
	PRINTF ("Free ISA IRQ %d mask=%04x\n", vec, cached_irq_mask);

}

/****************************************************************************/
void isa_init_irq_contr(void)
{
	int i;
	/* disable all Interrupts */
	/* first write icws controller 1 */
	for(i=0;i<16;i++)
	{
		isa_irqs[i].handler=NULL;
		isa_irqs[i].arg=NULL;
		isa_irqs[i].count=0;
	}
	init_8259A();
	out8(IMR_2,0xFF);
}
/*************************************************************************/

void isa_show_irq(void)
{
	int vec;

	printf ("\nISA Interrupt-Information:\n");
	printf ("Nr  Routine   Arg       Count\n");

	for (vec=0; vec<16; vec++) {
		if (isa_irqs[vec].handler != NULL) {
			printf ("%02d  %08lx  %08lx  %d\n",
				vec,
				(ulong)isa_irqs[vec].handler,
				(ulong)isa_irqs[vec].arg,
				isa_irqs[vec].count);
		}
	}
}

int isa_irq_get_count(int vec)
{
	return(isa_irqs[vec].count);
}

/******************************************************************
 * Init the ISA bus and devices.
 */

#if defined(CONFIG_PIP405)

int isa_init(void)
{
	isa_sio_setup();
	isa_init_irq_contr();
	drv_isa_kbd_init();
	return 0;
}
#endif
