/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <malloc.h>
#include <devices.h>
#include <syscall.h>
#if (CONFIG_COMMANDS & CFG_CMD_IDE)
#include <ide.h>
#endif
#if (CONFIG_COMMANDS & CFG_CMD_SCSI)
#include <scsi.h>
#endif
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#include <kgdb.h>
#endif
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif
#include <net.h>
#if (CONFIG_COMMANDS & CFG_CMD_BEDBUG)
#include <cmd_bedbug.h>
#endif
#ifdef CFG_ALLOC_DPRAM
#include <commproc.h>
#endif
#include <version.h>

static char *failed = "*** failed ***\n";

#ifdef	CONFIG_PCU_E
extern flash_info_t flash_info[];
#endif

#if defined(CFG_ENV_IS_IN_FLASH)
# ifndef  CFG_ENV_ADDR
#  define CFG_ENV_ADDR	(CFG_FLASH_BASE + CFG_ENV_OFFSET)
# endif
# ifndef  CFG_ENV_SIZE
#  define CFG_ENV_SIZE	CFG_ENV_SECT_SIZE
# endif
# if (CFG_ENV_ADDR >= CFG_MONITOR_BASE) && \
     (CFG_ENV_ADDR+CFG_ENV_SIZE) < (CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#  define ENV_IS_EMBEDDED
# endif
#endif /* CFG_ENV_IS_IN_FLASH */
#if ( ((CFG_ENV_ADDR+CFG_ENV_SIZE) < CFG_MONITOR_BASE) || \
      (CFG_ENV_ADDR >= (CFG_MONITOR_BASE + CFG_MONITOR_LEN)) ) || \
    defined(CFG_ENV_IS_IN_NVRAM)
#define	TOTAL_MALLOC_LEN	(CFG_MALLOC_LEN + CFG_ENV_SIZE)
#else
#define	TOTAL_MALLOC_LEN	CFG_MALLOC_LEN
#endif

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
static ulong mem_malloc_start = 0;
static ulong mem_malloc_end = 0;
static ulong mem_malloc_brk = 0;

/************************************************************************
 * Utilities								*
 ************************************************************************
 */

/*
 * The Malloc area is immediately below the monitor copy in DRAM
 */
static void mem_malloc_init (ulong dest_addr)
{
	mem_malloc_end = dest_addr;
	mem_malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;

	memset ((void *) mem_malloc_start, 0,
		mem_malloc_end - mem_malloc_start);
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

static void syscalls_init (int reloc_off)
{
	ulong *addr;

	addr = (ulong *) syscall_tbl;
	syscall_tbl[SYSCALL_MALLOC] = (void *) malloc;
	syscall_tbl[SYSCALL_FREE] = (void *) free;

	syscall_tbl[SYSCALL_INSTALL_HDLR] = (void *) irq_install_handler;
	syscall_tbl[SYSCALL_FREE_HDLR] = (void *) irq_free_handler;

	addr = (ulong *) 0xc00;	/* syscall ISR addr */

	/* patch ISR code */
	*addr++ |= (ulong) syscall_tbl >> 16;
	*addr++ |= (ulong) syscall_tbl & 0xFFFF;
	*addr++ |= NR_SYSCALLS >> 16;
	*addr++ |= NR_SYSCALLS & 0xFFFF;
}

/************************************************************************
 *
 * This is the first part of the initialization sequence that is
 * implemented in C, but still running from ROM.
 *
 * The main purpose is to provide a (serial) console interface as
 * soon as possible (so we can see any error messages), and to
 * initialize the RAM so that we can relocate the monitor code to
 * RAM.
 *
 * Be aware of the restrictions: global data is read-only, BSS is not
 * initialized, and stack space is limited to a few kB.
 *
 ************************************************************************
 */


gd_t *global_data;
static gd_t gdata;
static bd_t bdata;

void board_init_f (ulong bootflag)
{
	DECLARE_GLOBAL_DATA_PTR;

	bd_t *bd;
	ulong reg, len, addr, addr_sp, dram_size;
	int i, baudrate, board_type;
	char *s, *e;
	uchar tmp[64];		/* long enough for environment variables */

	/* Pointer to initial global data area */
	gd = global_data = &gdata;
	bd = gd->bd = &bdata;

	init_timebase ();
	env_init ();

	i = getenv_r ("baudrate", tmp, sizeof (tmp));
	baudrate =
		(i > 0) ? (int) simple_strtoul (tmp, NULL,
						10) : CONFIG_BAUDRATE;
	bd->bi_baudrate = baudrate;	/* Console Baudrate             */

	/* set up serial port */
	serial_init ();

	/* Initialize the console (before the relocation) */
	console_init_f ();

#ifdef DEBUG
	if (sizeof (init_data_t) > CFG_INIT_DATA_SIZE) {
		printf ("PANIC: sizeof(init_data_t)=%d > CFG_INIT_DATA_SIZE=%d\n", sizeof (init_data_t), CFG_INIT_DATA_SIZE);
		hang ();
	}
#endif /* DEBUG */

	/* now we can use standard printf/puts/getc/tstc functions */
	display_options ();

	puts ("CPU:   ");	/* Check CPU            */
	if (checkcpu () < 0) {
		puts (failed);
		hang ();
	}

	puts ("Board: ");	/* Check Board          */
	if ((board_type = checkboard ()) < 0) {
		puts (failed);
		hang ();
	}

	puts ("DRAM:  ");
	if ((dram_size = initdram (board_type)) > 0) {
		printf ("%2ld MB\n", dram_size >> 20);
	} else {
		puts (failed);
		hang ();
	}

#if defined(CFG_DRAM_TEST)
	if (testdram () != 0) {
		hang ();
	}
#endif /* CFG_DRAM_TEST */

	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM.
	 *
	 * Reserve memory at end of RAM for (top down in that order):
	 *  - protected RAM
	 *  - LCD framebuffer
	 *  - monitor code
	 *  - board info struct
	 */
	len = get_endaddr () - CFG_MONITOR_BASE;

	if (len > CFG_MONITOR_LEN) {
		printf ("*** u-boot size %ld > reserved memory (%d)\n",
			len, CFG_MONITOR_LEN);
		hang ();
	}

	if (CFG_MONITOR_LEN > len)
		len = CFG_MONITOR_LEN;

	addr = CFG_SDRAM_BASE + dram_size;
	addr -= len;

	/*
	 * Save local variables to board info struct
	 */
	bd->bi_memstart = CFG_SDRAM_BASE;	/* start of  DRAM memory              */
	bd->bi_memsize = dram_size;	/* size  of  DRAM memory in bytes     */
	bd->bi_bootflags = bootflag;	/* boot / reboot flag (for LynxOS)    */

	i = getenv_r ("ethaddr", tmp, sizeof (tmp));
	s = (i > 0) ? tmp : NULL;

	for (reg = 0; reg < 6; ++reg) {
		bd->bi_enetaddr[reg] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}

	bd->bi_intfreq = get_gclk_freq ();	/* Internal Freq, in Hz */
	bd->bi_busfreq = get_bus_freq (get_gclk_freq ());	/* Bus Freq,      in Hz */

#ifdef CFG_EXTBDINFO
	strncpy (bd->bi_s_version, "1.2", sizeof (bd->bi_s_version));
	strncpy (bd->bi_r_version, PPCBOOT_VERSION,
		 sizeof (bd->bi_r_version));

	bd->bi_procfreq = get_gclk_freq ();	/* Processor Speed, In Hz */
	bd->bi_plb_busfreq = bd->bi_busfreq;
#endif

	board_init_final (addr);

}


/************************************************************************
 *
 * This is the next part if the initialization sequence: we are now
 * running from RAM and have a "normal" C environment, i. e. global
 * data can be written, BSS has been cleared, the stack size in not
 * that critical any more, etc.
 *
 ************************************************************************
 */

void board_init_final (ulong dest_addr)
{
	DECLARE_GLOBAL_DATA_PTR;
	char *s;
	cmd_tbl_t *cmdtp;
	ulong flash_size;
	bd_t *bd;

	bd = gd->bd;
	/* icache_enable(); /XX* it's time to enable the instruction cache */

	/*
	 * Setup trap handlers
	 */
	trap_init (dest_addr);

	puts ("FLASH: ");

	if ((flash_size = flash_init ()) > 0) {
#ifdef CFG_FLASH_CHECKSUM
		if (flash_size >= (1 << 20)) {
			printf ("%2ld MB", flash_size >> 20);
		} else {
			printf ("%2ld kB", flash_size >> 10);
		}
		/*
		 * Compute and print flash CRC if flashchecksum is set to 'y'
		 *
		 * NOTE: Maybe we should add some WATCHDOG_RESET()?     XXX
		 */
		s = getenv ("flashchecksum");
		if (s && (*s == 'y')) {
			printf ("  CRC: %08lX",
				crc32 (0,
				       (const unsigned char *) CFG_FLASH_BASE,
				       flash_size)
				);
		}
		putc ('\n');
#else
		if (flash_size >= (1 << 20)) {
			printf ("%2ld MB\n", flash_size >> 20);
		} else {
			printf ("%2ld kB\n", flash_size >> 10);
		}
#endif /* CFG_FLASH_CHECKSUM */
	} else {
		puts (failed);
		hang ();
	}

	bd->bi_flashstart = CFG_FLASH_BASE;	/* update start of FLASH memory        */
	bd->bi_flashsize = flash_size;	/* size of FLASH memory (final value)     */
	bd->bi_flashoffset = 0x10000;	/* reserved area for startup monitor  */

	WATCHDOG_RESET ();

	/* initialize higher level parts of CPU like time base and timers */
	cpu_init_r ();

	WATCHDOG_RESET ();

	/* initialize malloc() area */
	mem_malloc_init (dest_addr);

#ifdef CONFIG_SPI
# if !defined(CFG_ENV_IS_IN_EEPROM)
	spi_init_f ();
# endif
	spi_init_r ();
#endif

	/* relocate environment function pointers etc. */
	env_relocate ();

	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

	WATCHDOG_RESET ();

	/* allocate syscalls table (console_init_r will fill it in */
	syscall_tbl = (void **) malloc (NR_SYSCALLS * sizeof (void *));

	/* Initialize the console (after the relocation and devices init) */


#if (CONFIG_COMMANDS & CFG_CMD_NET) && ( \
    defined(CONFIG_CCM)		|| \
    defined(CONFIG_EP8260)	|| \
    defined(CONFIG_IP860)	|| \
    defined(CONFIG_IVML24)	|| \
    defined(CONFIG_IVMS8)	|| \
    defined(CONFIG_LWMON)	|| \
    defined(CONFIG_MPC8260ADS)	|| \
    defined(CONFIG_PCU_E)	|| \
    defined(CONFIG_RPXSUPER)	|| \
    defined(CONFIG_SPD823TS)	)

	WATCHDOG_RESET ();
# ifdef DEBUG
	puts ("Reset Ethernet PHY\n");
# endif
	reset_phy ();
#endif


#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
	WATCHDOG_RESET ();
	puts ("KGDB:  ");
	kgdb_init ();
#endif

	/*
	 * Enable Interrupts
	 */
	interrupt_init ();
	udelay (20);
	set_timer (0);

	/* Insert function pointers now that we have relocated the code */

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if (CONFIG_COMMANDS & CFG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif /* CFG_CMD_NET */

	WATCHDOG_RESET ();

#if (CONFIG_COMMANDS & CFG_CMD_NET) && defined(CONFIG_NET_MULTI)
	WATCHDOG_RESET ();
	puts ("Net:   ");
	eth_initialize (bd);
#endif

#ifdef CONFIG_LAST_STAGE_INIT
	WATCHDOG_RESET ();
	/*
	 * Some parts can be only initialized if all others (like
	 * Interrupts) are up and running (i.e. the PC-style ISA
	 * keyboard).
	 */
	last_stage_init ();
#endif

#if (CONFIG_COMMANDS & CFG_CMD_BEDBUG)
	WATCHDOG_RESET ();
	bedbug_init ();
#endif

#ifdef CONFIG_PRAM
	/*
	 * Export available size of memory for Linux,
	 * taking into account the protected RAM at top of memory
	 */
	{
		ulong pram;
		char *s;
		uchar memsz[32];

		if ((s = getenv ("pram")) != NULL) {
			pram = simple_strtoul (s, NULL, 10);
		} else {
			pram = CONFIG_PRAM;
		}
		sprintf (memsz, "%ldk", (bd->bi_memsize / 1024) - pram);
		setenv ("mem", memsz);
	}
#endif

	/* Initialization complete - start the monitor */

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		WATCHDOG_RESET ();
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
