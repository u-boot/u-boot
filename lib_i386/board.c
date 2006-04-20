/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <watchdog.h>
#include <command.h>
#include <devices.h>
#include <version.h>
#include <malloc.h>
#include <net.h>
#include <ide.h>
#include <asm/u-boot-i386.h>

DECLARE_GLOBAL_DATA_PTR;

extern long _i386boot_start;
extern long _i386boot_end;
extern long _i386boot_romdata_start;
extern long _i386boot_romdata_dest;
extern long _i386boot_romdata_size;
extern long _i386boot_bss_start;
extern long _i386boot_bss_size;

extern long _i386boot_realmode;
extern long _i386boot_realmode_size;
extern long _i386boot_bios;
extern long _i386boot_bios_size;

/* The symbols defined by the linker script becomes pointers
 * which is somewhat inconveient ... */
ulong i386boot_start         = (ulong)&_i386boot_start;         /* code start (in flash) defined in start.S */
ulong i386boot_end           = (ulong)&_i386boot_end;	        /* code end (in flash) */
ulong i386boot_romdata_start = (ulong)&_i386boot_romdata_start; /* datasegment in flash (also code+rodata end) */
ulong i386boot_romdata_dest  = (ulong)&_i386boot_romdata_dest;  /* data location segment in ram */
ulong i386boot_romdata_size  = (ulong)&_i386boot_romdata_size;  /* size of data segment */
ulong i386boot_bss_start     = (ulong)&_i386boot_bss_start;     /* bss start */
ulong i386boot_bss_size      = (ulong)&_i386boot_bss_size;      /* bss size */

ulong i386boot_realmode      = (ulong)&_i386boot_realmode;      /* start of realmode entry code */
ulong i386boot_realmode_size = (ulong)&_i386boot_realmode_size; /* size of realmode entry code */
ulong i386boot_bios          = (ulong)&_i386boot_bios;          /* start of BIOS emulation code */
ulong i386boot_bios_size     = (ulong)&_i386boot_bios_size;     /* size of BIOS emulation code */


const char version_string[] =
	U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ")";


/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
static ulong mem_malloc_start = 0;
static ulong mem_malloc_end = 0;
static ulong mem_malloc_brk = 0;

static int mem_malloc_init(void)
{
	/* start malloc area right after the stack */
	mem_malloc_start = i386boot_bss_start +
		i386boot_bss_size + CFG_STACK_SIZE;
	mem_malloc_start = (mem_malloc_start+3)&~3;

	/* Use all available RAM for malloc() */
	mem_malloc_end = gd->ram_size;

	mem_malloc_brk = mem_malloc_start;

	return 0;
}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (NULL);
	}
	mem_malloc_brk = new;

	return ((void *) old);
}

char *strmhz (char *buf, long hz)
{
	long l, n;
	long m;

	n = hz / 1000000L;
	l = sprintf (buf, "%ld", n);
	m = (hz % 1000000L) / 1000L;
	if (m != 0)
		sprintf (buf + l, ".%03ld", m);
	return (buf);
}

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * or dropped completely,
 * but let's get it working (again) first...
 */
static int init_baudrate (void)
{
	char tmp[64];	/* long enough for environment variables */
	int i = getenv_r("baudrate", tmp, 64);

	gd->baudrate = (i != 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;

	return (0);
}

static int display_banner (void)
{

	printf ("\n\n%s\n\n", version_string);
	printf ("U-Boot code: %08lX -> %08lX  data: %08lX -> %08lX\n"
		"        BSS: %08lX -> %08lX stack: %08lX -> %08lX\n",
		i386boot_start, i386boot_romdata_start-1,
		i386boot_romdata_dest, i386boot_romdata_dest+i386boot_romdata_size-1,
		i386boot_bss_start, i386boot_bss_start+i386boot_bss_size-1,
		i386boot_bss_start+i386boot_bss_size,
		i386boot_bss_start+i386boot_bss_size+CFG_STACK_SIZE-1);


	return (0);
}

/*
 * WARNING: this code looks "cleaner" than the PowerPC version, but
 * has the disadvantage that you either get nothing, or everything.
 * On PowerPC, you might see "DRAM: " before the system hangs - which
 * gives a simple yet clear indication which part of the
 * initialization if failing.
 */
static int display_dram_config (void)
{
	int i;

	puts ("DRAM Configuration:\n");

	for (i=0; i<CONFIG_NR_DRAM_BANKS; i++) {
		printf ("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size (gd->bd->bi_dram[i].size, "\n");
	}

	return (0);
}

static void display_flash_config (ulong size)
{
	puts ("Flash: ");
	print_size (size, "\n");
}


/*
 * Breath some life into the board...
 *
 * Initialize an SMC for serial comms, and carry out some hardware
 * tests.
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependend #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
	cpu_init,		/* basic cpu dependent setup */
	board_init,		/* basic board dependent setup */
	dram_init,		/* configure available RAM banks */
	mem_malloc_init,        /* dependant on dram_init */
	interrupt_init,		/* set up exceptions */
	timer_init,
	serial_init,
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	display_banner,
	display_dram_config,

	NULL,
};

gd_t *global_data;

void start_i386boot (void)
{
	char *s;
	int i;
	ulong size;
	static gd_t gd_data;
	static bd_t bd_data;
	init_fnc_t **init_fnc_ptr;

	show_boot_progress(0x21);

	gd = global_data = &gd_data;
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	memset (gd, 0, sizeof (gd_t));
	gd->bd = &bd_data;
	memset (gd->bd, 0, sizeof (bd_t));
	show_boot_progress(0x22);

	gd->baudrate =  CONFIG_BAUDRATE;

	for (init_fnc_ptr = init_sequence, i=0; *init_fnc_ptr; ++init_fnc_ptr, i++) {
		show_boot_progress(0xa130|i);

		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}
	show_boot_progress(0x23);

	/* configure available FLASH banks */
	size = flash_init();
	display_flash_config(size);
	show_boot_progress(0x24);

	show_boot_progress(0x25);

	/* initialize environment */
	env_relocate ();
	show_boot_progress(0x26);


	/* IP Address */
	bd_data.bi_ip_addr = getenv_IPaddr ("ipaddr");

	/* MAC Address */
	{
		int i;
		ulong reg;
		char *s, *e;
		uchar tmp[64];

		i = getenv_r ("ethaddr", tmp, sizeof (tmp));
		s = (i > 0) ? tmp : NULL;

		for (reg = 0; reg < 6; ++reg) {
			bd_data.bi_enetaddr[reg] = s ? simple_strtoul (s, &e, 16) : 0;
			if (s)
				s = (*e) ? e + 1 : e;
		}
	}

#if defined(CONFIG_PCI)
	/*
	 * Do pci configuration
	 */
	pci_init();
#endif

	show_boot_progress(0x27);


	devices_init ();

	jumptable_init ();

	/* Initialize the console (after the relocation and devices init) */
	console_init_r();

#ifdef CONFIG_MISC_INIT_R
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif

#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA) && !(CONFIG_COMMANDS & CFG_CMD_IDE)
	WATCHDOG_RESET();
	puts ("PCMCIA:");
	pcmcia_init();
#endif

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
	WATCHDOG_RESET();
	puts("KGDB:  ");
	kgdb_init();
#endif

	/* enable exceptions */
	enable_interrupts();
	show_boot_progress(0x28);

	/* Must happen after interrupts are initialized since
	 * an irq handler gets installed
	 */
#ifdef CONFIG_SERIAL_SOFTWARE_FIFO
	serial_buffered_init();
#endif

#ifdef CONFIG_STATUS_LED
	status_led_set (STATUS_LED_BOOT, STATUS_LED_BLINKING);
#endif

	udelay(20);

	set_timer (0);

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if (CONFIG_COMMANDS & CFG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif /* CFG_CMD_NET */

	WATCHDOG_RESET();

#if (CONFIG_COMMANDS & CFG_CMD_IDE)
	WATCHDOG_RESET();
	puts("IDE:   ");
	ide_init();
#endif /* CFG_CMD_IDE */

#if (CONFIG_COMMANDS & CFG_CMD_SCSI)
	WATCHDOG_RESET();
	puts("SCSI:  ");
	scsi_init();
#endif

#if (CONFIG_COMMANDS & CFG_CMD_DOC)
	WATCHDOG_RESET();
	puts("DOC:   ");
	doc_init();
#endif

#if (CONFIG_COMMANDS & CFG_CMD_NET)
#if defined(CONFIG_NET_MULTI)
	WATCHDOG_RESET();
	puts("Net:   ");
#endif
	eth_initialize(gd->bd);
#endif

#if (CONFIG_COMMANDS & CFG_CMD_NET) && (0)
	WATCHDOG_RESET();
# ifdef DEBUG
	puts ("Reset Ethernet PHY\n");
# endif
	reset_phy();
#endif

#ifdef CONFIG_LAST_STAGE_INIT
	WATCHDOG_RESET();
	/*
	 * Some parts can be only initialized if all others (like
	 * Interrupts) are up and running (i.e. the PC-style ISA
	 * keyboard).
	 */
	last_stage_init();
#endif


#ifdef CONFIG_POST
	post_run (NULL, POST_RAM | post_bootmode_get(0));
#endif


	show_boot_progress(0x29);

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		main_loop();
	}

	/* NOTREACHED - no way out of command loop except booting */
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
