/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

#if !defined(CONFIG_DBGU) && !defined(CONFIG_USART0) && !defined(CONFIG_USART1)
#error must define one of CONFIG_DBGU or CONFIG_USART0 or CONFIG_USART1
#endif

/* read co-processor 15, register #1 (control register) */
static unsigned long read_p15_c1(void)
{
    unsigned long value;

    __asm__ __volatile__(
	"mrc     p15, 0, %0, c1, c0, 0   @ read control reg\n"
	: "=r" (value)
	:
	: "memory");
    /*printf("p15/c1 is = %08lx\n", value); */
    return value;
}

/* write to co-processor 15, register #1 (control register) */
static void write_p15_c1(unsigned long value)
{
    /*printf("write %08lx to p15/c1\n", value); */
    __asm__ __volatile__(
	"mcr     p15, 0, %0, c1, c0, 0   @ write it back\n"
	: "=r" (value)
	:
	: "memory");

    read_p15_c1();
}

static void cp_delay(void)
{
    volatile int i;

    /* copro seems to need some delay between reading and writing */
    for (i=0; i<100; i++);
}
/* See also ARM Ref. Man. */
#define C1_MMU		(1<<0)	/* mmu off/on */
#define C1_ALIGN	(1<<1)	/* alignment faults off/on */
#define C1_IDC		(1<<2)	/* icache and/or dcache off/on */
#define C1_WRITE_BUFFER	(1<<3)	/* write buffer off/on */
#define C1_BIG_ENDIAN	(1<<7)	/* big endian off/on */
#define C1_SYS_PROT	(1<<8)	/* system protection */
#define C1_ROM_PROT	(1<<9)	/* ROM protection */
#define C1_HIGH_VECTORS	(1<<13)	/* location of vectors: low/high addresses */

int cpu_init(void)
{
	/*
	 * setup up stacks if necessary
	 */
#ifdef CONFIG_USE_IRQ
	DECLARE_GLOBAL_DATA_PTR;

	IRQ_STACK_START = _armboot_start - CFG_MALLOC_LEN - CFG_GBL_DATA_SIZE - 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;
#endif
	return 0;
}

int cleanup_before_linux(void)
{
    /*
     * this function is called just before we call linux
     * it prepares the processor for linux
     *
     * we turn off caches etc ...
     * and we set the CPU-speed to 73 MHz - see start.S for details
     */

    disable_interrupts();
    return 0;
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

#ifdef CFG_SOFT_RESET
    extern void reset_cpu(ulong addr);

    disable_interrupts();
    reset_cpu(0);
#else
#ifdef CONFIG_DBGU
   AT91PS_USART us = (AT91PS_USART) AT91C_BASE_DBGU;
#endif
#ifdef CONFIG_USART0
   AT91PS_USART us = AT91C_BASE_US0;
#endif
#ifdef CONFIG_USART1
   AT91PS_USART us = AT91C_BASE_US1;
#endif
   AT91PS_PIO pio = AT91C_BASE_PIOA;

   /*shutdown the console to avoid strange chars during reset */
   us->US_CR = (AT91C_US_RSTRX | AT91C_US_RSTTX);

#ifdef CONFIG_AT91RM9200DK
   /* Clear PA19 to trigger the hard reset */
   pio->PIO_CODR = 0x00080000;
   pio->PIO_OER  = 0x00080000;
   pio->PIO_PER  = 0x00080000;
#endif
#ifdef CONFIG_CMC_PU2
/* this is the way Linux does it */
#define AT91C_ST_RSTEN (0x1 << 16)
#define AT91C_ST_EXTEN (0x1 << 17)
#define AT91C_ST_WDRST (0x1 <<  0)
/* watchdog mode register */
#define ST_WDMR *((unsigned long *)0xfffffd08)
/* system clock control register */
#define ST_CR *((unsigned long *)0xfffffd00)
	ST_WDMR = AT91C_ST_RSTEN | AT91C_ST_EXTEN | 1 ;
	ST_CR = AT91C_ST_WDRST;
   /* Never reached */
#endif
#endif
   return 0;
}

void icache_enable(void)
{
    ulong reg;
    reg = read_p15_c1();
    cp_delay();
    write_p15_c1(reg | C1_IDC);
}

void icache_disable(void)
{
    ulong reg;
    reg = read_p15_c1();
    cp_delay();
    write_p15_c1(reg & ~C1_IDC);
}

int icache_status(void)
{
    return (read_p15_c1() & C1_IDC) != 0;
    return 0;
}

void dcache_enable(void)
{
    ulong reg;
    reg = read_p15_c1();
    cp_delay();
    write_p15_c1(reg | C1_IDC);
}

void dcache_disable(void)
{
    ulong reg;
    reg = read_p15_c1();
    cp_delay();
    write_p15_c1(reg & ~C1_IDC);
}

int dcache_status(void)
{
    return (read_p15_c1() & C1_IDC) != 0;
    return 0;
}
