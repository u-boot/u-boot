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
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

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

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
		"subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	/* arch number of Integrator Board */
	gd->bd->bi_arch_number = MACH_TYPE_INTEGRATOR;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x00000100;

	gd->flags = 0;

#ifdef CONFIG_CM_REMAP
extern void cm_remap(void);
	cm_remap();	/* remaps writeable memory to 0x00000000 */
#endif

	icache_enable ();

	flash__init ();
	return 0;
}


int misc_init_r (void)
{
#ifdef CONFIG_PCI
	pci_init();
#endif
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

/* The Integrator/AP timer1 is clocked at 24MHz
 * can be divided by 16 or 256
 * and is a 16-bit counter
 */
/* U-Boot expects a 32 bit timer running at CONFIG_SYS_HZ*/
static ulong timestamp;		/* U-Boot ticks since startup	      */
static ulong total_count = 0;	/* Total timer count		      */
static ulong lastdec;		/* Timer reading at last call	      */
static ulong div_clock	 = 256; /* Divisor applied to the timer clock */
static ulong div_timer	 = 1;	/* Divisor to convert timer reading
				 * change to U-Boot ticks
				 */
/* CONFIG_SYS_HZ = CONFIG_SYS_HZ_CLOCK/(div_clock * div_timer) */

#define TIMER_LOAD_VAL 0x0000FFFFL
#define READ_TIMER ((*(volatile ulong *)(CONFIG_SYS_TIMERBASE+4)) & 0x0000FFFFL)

/* all function return values in U-Boot ticks i.e. (1/CONFIG_SYS_HZ) sec
 *  - unless otherwise stated
 */

/* starts a counter
 * - the Integrator/AP timer issues an interrupt
 *   each time it reaches zero
 */
int timer_init (void)
{
	/* Load timer with initial value */
	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 0) = TIMER_LOAD_VAL;
	/* Set timer to be
	 *	enabled		  1
	 *	free-running	  0
	 *	XX		 00
	 *	divider 256	 10
	 *	XX		 00
	 */
	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8) = 0x00000088;
	total_count = 0;
	/* init the timestamp and lastdec value */
	reset_timer_masked();

	div_timer  = CONFIG_SYS_HZ_CLOCK / CONFIG_SYS_HZ;
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
	timestamp = ticks;
	total_count = ticks * div_timer;
	reset_timer_masked();
}

/* delay x useconds */
void udelay (unsigned long usec)
{
	ulong tmo, tmp;

	/* Convert to U-Boot ticks */
	tmo  = usec * CONFIG_SYS_HZ;
	tmo /= (1000000L);

	tmp  = get_timer_masked();	/* get current timestamp */
	tmo += tmp;			/* wake up timestamp	 */

	while (get_timer_masked () < tmo) { /* loop till event */
		/*NOP*/;
	}
}

void reset_timer_masked (void)
{
	/* reset time */
	lastdec	  = READ_TIMER; /* capture current decrementer value   */
	timestamp = 0;		/* start "advancing" time stamp from 0 */
}

/* converts the timer reading to U-Boot ticks	       */
/* the timestamp is the number of ticks since reset    */
/* This routine does not detect wraps unless called regularly
   ASSUMES a call at least every 16 seconds to detect every reload */
ulong get_timer_masked (void)
{
	ulong now = READ_TIMER;		/* current count */

	if (now > lastdec) {
		/* Must have wrapped */
		total_count += lastdec + TIMER_LOAD_VAL + 1 - now;
	} else {
		total_count += lastdec - now;
	}
	lastdec	  = now;
	timestamp = total_count/div_timer;

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
	return get_timer(0);
}

/*
 * Return the timebase clock frequency
 * i.e. how often the timer decrements
 */
ulong get_tbclk (void)
{
	return CONFIG_SYS_HZ_CLOCK/div_clock;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}
