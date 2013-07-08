/* Initializes CPU and basic hardware such as memory
 * controllers, IRQ controller and system timer 0.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/asi.h>
#include <asm/leon.h>

#include <config.h>

DECLARE_GLOBAL_DATA_PTR;

/* reset CPU (jump to 0, without reset) */
void start(void);

struct {
	gd_t gd_area;
	bd_t bd;
} global_data;

/*
 * Breath some life into the CPU...
 *
 * Set up the memory map,
 * initialize a bunch of registers.
 *
 * Run from FLASH/PROM:
 *  - until memory controller is set up, only registers available
 *  - no global variables available for writing
 *  - constants available
 */

void cpu_init_f(void)
{
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;

	/* initialize the IRQMP */
	leon2->Interrupt_Force = 0;
	leon2->Interrupt_Pending = 0;
	leon2->Interrupt_Clear = 0xfffe;	/* clear all old pending interrupts */
	leon2->Interrupt_Mask = 0xfffe0000;	/* mask all IRQs */

	/* cache */

       /* I/O port setup */
#ifdef LEON2_IO_PORT_DIR
       leon2->PIO_Direction = LEON2_IO_PORT_DIR;
#endif
#ifdef LEON2_IO_PORT_DATA
       leon2->PIO_Data = LEON2_IO_PORT_DATA;
#endif
#ifdef LEON2_IO_PORT_INT
       leon2->PIO_Interrupt = LEON2_IO_PORT_INT;
#else
       leon2->PIO_Interrupt = 0;
#endif
}

void cpu_init_f2(void)
{

}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r(void)
{
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;

	/* initialize prescaler common to all timers to 1MHz */
	leon2->Scaler_Counter = leon2->Scaler_Reload =
	    (((CONFIG_SYS_CLK_FREQ / 1000) + 500) / 1000) - 1;

	return (0);
}

/* Uses Timer 0 to get accurate
 * pauses. Max 2 raised to 32 ticks
 *
 */
void cpu_wait_ticks(unsigned long ticks)
{
	unsigned long start = get_timer(0);
	while (get_timer(start) < ticks) ;
}

/* initiate and setup timer0 interrupt to 1MHz
 * Return irq number for timer int or a negative number for
 * dealing with self
 */
int timer_interrupt_init_cpu(void)
{
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;

	/* 1ms ticks */
	leon2->Timer_Counter_1 = 0;
	leon2->Timer_Reload_1 = 999;	/* (((1000000 / 100) - 1)) */
	leon2->Timer_Control_1 =
	    (LEON2_TIMER_CTRL_EN | LEON2_TIMER_CTRL_RS | LEON2_TIMER_CTRL_LD);

	return LEON2_TIMER1_IRQNO;
}

/*
 * This function is intended for SHORT delays only.
 */
unsigned long cpu_usec2ticks(unsigned long usec)
{
	/* timer set to 1kHz ==> 1 clk tick = 1 msec */
	if (usec < 1000)
		return 1;
	return (usec / 1000);
}

unsigned long cpu_ticks2usec(unsigned long ticks)
{
	/* 1tick = 1usec */
	return ticks * 1000;
}
