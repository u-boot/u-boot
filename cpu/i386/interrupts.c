/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
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
 */

#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/i8259.h>
#include <asm/ibmpc.h>


struct idt_entry {
	u16	base_low;
	u16	selector;
	u8	res;
	u8	access;
	u16	base_high;
} __attribute__ ((packed));


struct idt_entry idt[256];


#define MAX_IRQ 16

typedef struct irq_handler {
	struct irq_handler *next;
	interrupt_handler_t* isr_func;
	void *isr_data;
} irq_handler_t;

#define IRQ_DISABLED   1

typedef struct {
	irq_handler_t *handler;
	unsigned long status;
} irq_desc_t;

static irq_desc_t irq_table[MAX_IRQ];

asm ("irq_return:\n"
     "     addl  $4, %esp\n"
     "     popa\n"
     "     iret\n");

asm ("exp_return:\n"
     "     addl  $12, %esp\n"
     "     pop   %esp\n"
     "     popa\n"
     "     iret\n");

char exception_stack[4096];

#define DECLARE_INTERRUPT(x) \
	asm(".globl irq_"#x"\n" \
		    "irq_"#x":\n" \
		    "pusha \n" \
		    "pushl $"#x"\n" \
		    "pushl $irq_return\n" \
		    "jmp   do_irq\n"); \
	void __attribute__ ((regparm(0))) irq_##x(void)

#define DECLARE_EXCEPTION(x, f) \
	asm(".globl exp_"#x"\n" \
		    "exp_"#x":\n" \
		    "pusha \n" \
		    "movl     %esp, %ebx\n" \
		    "movl     $exception_stack, %eax\n" \
		    "movl     %eax, %esp \n" \
		    "pushl    %ebx\n" \
		    "movl     32(%esp), %ebx\n" \
		    "xorl     %edx, %edx\n" \
		    "movw     36(%esp), %dx\n" \
		    "pushl    %edx\n" \
		    "pushl    %ebx\n" \
		    "pushl    $"#x"\n" \
		    "pushl    $exp_return\n" \
		    "jmp      "#f"\n"); \
	void __attribute__ ((regparm(0))) exp_##x(void)

DECLARE_EXCEPTION(0, divide_exception_entry);      /* Divide exception */
DECLARE_EXCEPTION(1, debug_exception_entry);       /* Debug exception */
DECLARE_EXCEPTION(2, nmi_entry);                   /* NMI */
DECLARE_EXCEPTION(3, unknown_exception_entry);     /* Breakpoint/Coprocessor Error */
DECLARE_EXCEPTION(4, unknown_exception_entry);     /* Overflow */
DECLARE_EXCEPTION(5, unknown_exception_entry);     /* Bounds */
DECLARE_EXCEPTION(6, invalid_instruction_entry);   /* Invalid instruction */
DECLARE_EXCEPTION(7, unknown_exception_entry);     /* Device not present */
DECLARE_EXCEPTION(8, double_fault_entry);          /* Double fault */
DECLARE_EXCEPTION(9, unknown_exception_entry);     /* Co-processor segment overrun */
DECLARE_EXCEPTION(10, invalid_tss_exception_entry);/* Invalid TSS */
DECLARE_EXCEPTION(11, seg_fault_entry);            /* Segment not present */
DECLARE_EXCEPTION(12, stack_fault_entry);          /* Stack overflow */
DECLARE_EXCEPTION(13, gpf_entry);                  /* GPF */
DECLARE_EXCEPTION(14, page_fault_entry);           /* PF */
DECLARE_EXCEPTION(15, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(16, fp_exception_entry);         /* Floating point */
DECLARE_EXCEPTION(17, alignment_check_entry);      /* alignment check */
DECLARE_EXCEPTION(18, machine_check_entry);        /* machine check */
DECLARE_EXCEPTION(19, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(20, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(21, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(22, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(23, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(24, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(25, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(26, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(27, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(28, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(29, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(30, unknown_exception_entry);    /* Reserved */
DECLARE_EXCEPTION(31, unknown_exception_entry);    /* Reserved */

DECLARE_INTERRUPT(0);
DECLARE_INTERRUPT(1);
DECLARE_INTERRUPT(3);
DECLARE_INTERRUPT(4);
DECLARE_INTERRUPT(5);
DECLARE_INTERRUPT(6);
DECLARE_INTERRUPT(7);
DECLARE_INTERRUPT(8);
DECLARE_INTERRUPT(9);
DECLARE_INTERRUPT(10);
DECLARE_INTERRUPT(11);
DECLARE_INTERRUPT(12);
DECLARE_INTERRUPT(13);
DECLARE_INTERRUPT(14);
DECLARE_INTERRUPT(15);

void __attribute__ ((regparm(0))) default_isr(void);
asm ("default_isr: iret\n");

void disable_irq(int irq)
{
	if (irq >= MAX_IRQ) {
		return;
	}
	irq_table[irq].status |= IRQ_DISABLED;

}

void enable_irq(int irq)
{
	if (irq >= MAX_IRQ) {
		return;
	}
	irq_table[irq].status &= ~IRQ_DISABLED;
}

/* masks one specific IRQ in the PIC */
static void unmask_irq(int irq)
{
	int imr_port;

	if (irq >= MAX_IRQ) {
		return;
	}
	if (irq > 7) {
		imr_port = SLAVE_PIC + IMR;
	} else {
		imr_port = MASTER_PIC + IMR;
	}

	outb(inb(imr_port)&~(1<<(irq&7)), imr_port);
}


/* unmasks one specific IRQ in the PIC */
static void mask_irq(int irq)
{
	int imr_port;

	if (irq >= MAX_IRQ) {
		return;
	}
	if (irq > 7) {
		imr_port = SLAVE_PIC + IMR;
	} else {
		imr_port = MASTER_PIC + IMR;
	}

	outb(inb(imr_port)|(1<<(irq&7)), imr_port);
}


/* issue a Specific End Of Interrupt instruciton */
static void specific_eoi(int irq)
{
	/* If it is on the slave PIC this have to be performed on
	 * both the master and the slave PICs */
	if (irq > 7) {
		outb(OCW2_SEOI|(irq&7), SLAVE_PIC + OCW2);
		irq = SEOI_IR2;               /* also do IR2 on master */
	}
	outb(OCW2_SEOI|irq, MASTER_PIC + OCW2);
}

void __attribute__ ((regparm(0))) do_irq(int irq)
{

	mask_irq(irq);

	if (irq_table[irq].status & IRQ_DISABLED) {
		unmask_irq(irq);
		specific_eoi(irq);
		return;
	}


	if (NULL != irq_table[irq].handler) {
		irq_handler_t *handler;
		for (handler = irq_table[irq].handler;
		     NULL!= handler; handler = handler->next) {
			handler->isr_func(handler->isr_data);
		}
	} else {
		if ((irq & 7) != 7) {
			printf("Spurious irq %d\n", irq);
		}
	}
	unmask_irq(irq);
	specific_eoi(irq);
}


void __attribute__ ((regparm(0))) unknown_exception_entry(int cause, int ip, int seg)
{
	printf("Unknown Exception %d at %04x:%08x\n", cause, seg, ip);
}

void __attribute__ ((regparm(0))) divide_exception_entry(int cause, int ip, int seg)
{
	printf("Divide Error (Division by zero) at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) debug_exception_entry(int cause, int ip, int seg)
{
	printf("Debug Interrupt (Single step) at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) nmi_entry(int cause, int ip, int seg)
{
	printf("NMI Interrupt at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) invalid_instruction_entry(int cause, int ip, int seg)
{
	printf("Invalid Instruction at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) double_fault_entry(int cause, int ip, int seg)
{
	printf("Double fault at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) invalid_tss_exception_entry(int cause, int ip, int seg)
{
	printf("Invalid TSS at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) seg_fault_entry(int cause, int ip, int seg)
{
	printf("Segmentation fault at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) stack_fault_entry(int cause, int ip, int seg)
{
	printf("Stack fault at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) gpf_entry(int cause, int ip, int seg)
{
	printf("General protection fault at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) page_fault_entry(int cause, int ip, int seg)
{
	printf("Page fault at %04x:%08x\n", seg, ip);
	while(1);
}

void __attribute__ ((regparm(0))) fp_exception_entry(int cause, int ip, int seg)
{
	printf("Floating point exception at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) alignment_check_entry(int cause, int ip, int seg)
{
	printf("Alignment check at %04x:%08x\n", seg, ip);
}

void __attribute__ ((regparm(0))) machine_check_entry(int cause, int ip, int seg)
{
	printf("Machine check exception at %04x:%08x\n", seg, ip);
}


void irq_install_handler(int ino, interrupt_handler_t *func, void *pdata)
{
	int status;

	if (ino>MAX_IRQ) {
		return;
	}

	if (NULL != irq_table[ino].handler) {
		return;
	}

	status = disable_interrupts();
	irq_table[ino].handler = malloc(sizeof(irq_handler_t));
	if (NULL == irq_table[ino].handler) {
		return;
	}

	memset(irq_table[ino].handler, 0, sizeof(irq_handler_t));

	irq_table[ino].handler->isr_func = func;
	irq_table[ino].handler->isr_data = pdata;
	if (status) {
		enable_interrupts();
	}

	unmask_irq(ino);

	return;
}

void irq_free_handler(int ino)
{
	int status;
	if (ino>MAX_IRQ) {
		return;
	}

	status = disable_interrupts();
	mask_irq(ino);
	if (NULL == irq_table[ino].handler) {
		return;
	}
	free(irq_table[ino].handler);
	irq_table[ino].handler=NULL;
	if (status) {
		enable_interrupts();
	}
	return;
}


asm ("idt_ptr:\n"
	".word	0x800\n" /* size of the table 8*256 bytes */
	".long	idt\n"	 /* offset */
	".word	0x18\n");/* data segment */

static void set_vector(int intnum, void *routine)
{
	idt[intnum].base_high = (u16)((u32)(routine)>>16);
	idt[intnum].base_low = (u16)((u32)(routine)&0xffff);
}


int interrupt_init(void)
{
	int i;

	/* Just in case... */
	disable_interrupts();

	/* Initialize the IDT and stuff */


	memset(irq_table, 0, sizeof(irq_table));

	/* Setup the IDT */
	for (i=0;i<256;i++) {
		idt[i].access = 0x8e;
		idt[i].res = 0;
		idt[i].selector = 0x10;
		set_vector(i, default_isr);
	}

	asm ("cs lidt idt_ptr\n");

	/* Setup exceptions */
	set_vector(0x00, exp_0);
	set_vector(0x01, exp_1);
	set_vector(0x02, exp_2);
	set_vector(0x03, exp_3);
	set_vector(0x04, exp_4);
	set_vector(0x05, exp_5);
	set_vector(0x06, exp_6);
	set_vector(0x07, exp_7);
	set_vector(0x08, exp_8);
	set_vector(0x09, exp_9);
	set_vector(0x0a, exp_10);
	set_vector(0x0b, exp_11);
	set_vector(0x0c, exp_12);
	set_vector(0x0d, exp_13);
	set_vector(0x0e, exp_14);
	set_vector(0x0f, exp_15);
	set_vector(0x10, exp_16);
	set_vector(0x11, exp_17);
	set_vector(0x12, exp_18);
	set_vector(0x13, exp_19);
	set_vector(0x14, exp_20);
	set_vector(0x15, exp_21);
	set_vector(0x16, exp_22);
	set_vector(0x17, exp_23);
	set_vector(0x18, exp_24);
	set_vector(0x19, exp_25);
	set_vector(0x1a, exp_26);
	set_vector(0x1b, exp_27);
	set_vector(0x1c, exp_28);
	set_vector(0x1d, exp_29);
	set_vector(0x1e, exp_30);
	set_vector(0x1f, exp_31);


	/* Setup interrupts */
	set_vector(0x20, irq_0);
	set_vector(0x21, irq_1);
	set_vector(0x23, irq_3);
	set_vector(0x24, irq_4);
	set_vector(0x25, irq_5);
	set_vector(0x26, irq_6);
	set_vector(0x27, irq_7);
	set_vector(0x28, irq_8);
	set_vector(0x29, irq_9);
	set_vector(0x2a, irq_10);
	set_vector(0x2b, irq_11);
	set_vector(0x2c, irq_12);
	set_vector(0x2d, irq_13);
	set_vector(0x2e, irq_14);
	set_vector(0x2f, irq_15);
	/* vectors 0x30-0x3f are reserved for irq 16-31 */


	/* Mask all interrupts */
	outb(0xff, MASTER_PIC + IMR);
	outb(0xff, SLAVE_PIC + IMR);

	/* Master PIC */
	outb(ICW1_SEL|ICW1_EICW4, MASTER_PIC + ICW1);
	outb(0x20, MASTER_PIC + ICW2);          /* Place master PIC interrupts at INT20 */
	outb(IR2, MASTER_PIC + ICW3);		/* ICW3, One slevc PIC is present */
	outb(ICW4_PM, MASTER_PIC + ICW4);

	for (i=0;i<8;i++) {
		outb(OCW2_SEOI|i, MASTER_PIC + OCW2);
	}

	/* Slave PIC */
	outb(ICW1_SEL|ICW1_EICW4, SLAVE_PIC + ICW1);
	outb(0x28, SLAVE_PIC + ICW2);	        /* Place slave PIC interrupts at INT28 */
	outb(0x02, SLAVE_PIC + ICW3);		/* Slave ID */
	outb(ICW4_PM, SLAVE_PIC + ICW4);

	for (i=0;i<8;i++) {
		outb(OCW2_SEOI|i, SLAVE_PIC + OCW2);
	}


	/* enable cascade interrerupt */
	outb(0xfb, MASTER_PIC + IMR);
	outb(0xff, SLAVE_PIC + IMR);

	/* It is now safe to enable interrupts */
	enable_interrupts();

	return 0;
}

void enable_interrupts(void)
{
	asm("sti\n");
}

int disable_interrupts(void)
{
	long flags;

	asm volatile ("pushfl ; popl %0 ; cli\n" : "=g" (flags) : );

	return (flags&0x200); /* IE flags is bit 9 */
}


#ifdef CFG_RESET_GENERIC

void __attribute__ ((regparm(0))) generate_gpf(void);
asm(".globl generate_gpf\n"
    "generate_gpf:\n"
    "ljmp   $0x70, $0x47114711\n"); /* segment 0x70 is an arbitrary segment which does not
				    * exist */
void reset_cpu(ulong addr)
{
	set_vector(13, generate_gpf);  /* general protection fault handler */
	set_vector(8, generate_gpf);   /* double fault handler */
	generate_gpf();                /* start the show */
}
#endif
