/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
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

void flash__init (void);
void ether__init (void);
void peripheral_power_enable (void);

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	/* arch number of Integrator Board */
	gd->bd->bi_arch_number = MACH_TYPE_CINTEGRATOR;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	gd->flags = 0;

#ifdef CONFIG_CM_REMAP
extern void cm_remap(void);
	cm_remap();	/* remaps writeable memory to 0x00000000 */
#endif

	icache_enable ();

	flash__init ();
	ether__init ();
	return 0;
}


int misc_init_r (void)
{
	setenv("verify", "n");
	return (0);
}

/******************************
 Routine:
 Description:
******************************/
void flash__init (void)
{
}
/*************************************************************
 Routine:ether__init
 Description: take the Ethernet controller out of reset and wait
	      for the EEPROM load to complete.
*************************************************************/
void ether__init (void)
{
}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size	 = PHYS_SDRAM_1_SIZE;

#ifdef CONFIG_CM_SPD_DETECT
    {
extern void dram_query(void);
	unsigned long cm_reg_sdram;
	unsigned long sdram_shift;

	dram_query();	/* Assembler accesses to CM registers */
			/* Queries the SPD values	      */

	/* Obtain the SDRAM size from the CM SDRAM register */

	cm_reg_sdram = *(volatile ulong *)(CM_BASE + OS_SDRAM);
	/*   Register	      SDRAM size
	 *
	 *   0xXXXXXXbbb000bb	 16 MB
	 *   0xXXXXXXbbb001bb	 32 MB
	 *   0xXXXXXXbbb010bb	 64 MB
	 *   0xXXXXXXbbb011bb	128 MB
	 *   0xXXXXXXbbb100bb	256 MB
	 *
	 */
	sdram_shift		 = ((cm_reg_sdram & 0x0000001C)/4)%4;
	gd->bd->bi_dram[0].size	 = 0x01000000 << sdram_shift;

    }
#endif /* CM_SPD_DETECT */

	return 0;
}

/* The Integrator/CP timer1 is clocked at 1MHz
 * can be divided by 16 or 256
 * and can be set up as a 32-bit timer
 */
/* U-Boot expects a 32 bit timer, running at CFG_HZ */
/* Keep total timer count to avoid losing decrements < div_timer */
static unsigned long long total_count = 0;
static unsigned long long lastdec;	 /* Timer reading at last call	   */
static unsigned long long div_clock = 1; /* Divisor applied to timer clock */
static unsigned long long div_timer = 1; /* Divisor to convert timer reading
					  * change to U-Boot ticks
					  */
/* CFG_HZ = CFG_HZ_CLOCK/(div_clock * div_timer) */
static ulong timestamp;		/* U-Boot ticks since startup	      */

#define TIMER_LOAD_VAL ((ulong)0xFFFFFFFF)
#define READ_TIMER (*(volatile ulong *)(CFG_TIMERBASE+4))

/* all function return values in U-Boot ticks i.e. (1/CFG_HZ) sec
 *  - unless otherwise stated
 */

/* starts up a counter
 * - the Integrator/CP timer can be set up to issue an interrupt */
int interrupt_init (void)
{
	/* Load timer with initial value */
	*(volatile ulong *)(CFG_TIMERBASE + 0) = TIMER_LOAD_VAL;
	/* Set timer to be
	 *	enabled		  1
	 *	periodic	  1
	 *	no interrupts	  0
	 *	X		  0
	 *	divider 1	 00 == less rounding error
	 *	32 bit		  1
	 *	wrapping	  0
	 */
	*(volatile ulong *)(CFG_TIMERBASE + 8) = 0x000000C2;
	/* init the timestamp */
	total_count = 0ULL;
	reset_timer_masked();

	div_timer  = (unsigned long long)(CFG_HZ_CLOCK / CFG_HZ);
	div_timer /= div_clock;

	return (0);
}

/*
 * timer without interrupts
 */
void reset_timer (void)
{
	reset_timer_masked ();
}

ulong get_timer (ulong base_ticks)
{
	return get_timer_masked () - base_ticks;
}

void set_timer (ulong ticks)
{
	timestamp   = ticks;
	total_count = (unsigned long long)ticks * div_timer;
}

/* delay usec useconds */
void udelay (unsigned long usec)
{
	ulong tmo, tmp;

	/* Convert to U-Boot ticks */
	tmo  = usec * CFG_HZ;
	tmo /= (1000000L);

	tmp  = get_timer_masked();	/* get current timestamp */
	tmo += tmp;			/* form target timestamp */

	while (get_timer_masked () < tmo) {/* loop till event */
		/*NOP*/;
	}
}

void reset_timer_masked (void)
{
	/* capure current decrementer value    */
	lastdec	  = (unsigned long long)READ_TIMER;
	/* start "advancing" time stamp from 0 */
	timestamp = 0L;
}

/* converts the timer reading to U-Boot ticks	       */
/* the timestamp is the number of ticks since reset    */
ulong get_timer_masked (void)
{
	/* get current count */
	unsigned long long now = (unsigned long long)READ_TIMER;

	if(now > lastdec) {
		/* Must have wrapped */
		total_count += lastdec + TIMER_LOAD_VAL + 1 - now;
	} else {
		total_count += lastdec - now;
	}
	lastdec	  = now;
	timestamp = (ulong)(total_count/div_timer);

	return timestamp;
}

/* waits specified delay value and resets timestamp */
void udelay_masked (unsigned long usec)
{
	udelay(usec);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return (unsigned long long)get_timer(0);
}

/*
 * Return the timebase clock frequency
 * i.e. how often the timer decrements
 */
ulong get_tbclk (void)
{
	return (ulong)(((unsigned long long)CFG_HZ_CLOCK)/div_clock);
}
