/*
 * (C) Copyright 2003
 * Josef Baumgartner <josef.baumgartner@telex.de>
 *
 * (C) Copyright 2000-2002
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

#ifdef	CONFIG_M5272
#include <asm/immap_5272.h>
#endif

#if defined(CONFIG_CMD_IDE)
#include <ide.h>
#endif
#if defined(CONFIG_CMD_SCSI)
#include <scsi.h>
#endif
#if defined(CONFIG_CMD_KGDB)
#include <kgdb.h>
#endif
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif
#include <net.h>
#if defined(CONFIG_CMD_BEDBUG)
#include <cmd_bedbug.h>
#endif
#ifdef CFG_ALLOC_DPRAM
#include <commproc.h>
#endif
#include <version.h>

#if defined(CONFIG_HARD_I2C) || \
    defined(CONFIG_SOFT_I2C)
#include <i2c.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static char *failed = "*** failed ***\n";

#ifdef	CONFIG_PCU_E
extern flash_info_t flash_info[];
#endif

#include <environment.h>

#if ( ((CFG_ENV_ADDR+CFG_ENV_SIZE) < CFG_MONITOR_BASE) || \
      (CFG_ENV_ADDR >= (CFG_MONITOR_BASE + CFG_MONITOR_LEN)) ) || \
    defined(CFG_ENV_IS_IN_NVRAM)
#define	TOTAL_MALLOC_LEN	(CFG_MALLOC_LEN + CFG_ENV_SIZE)
#else
#define	TOTAL_MALLOC_LEN	CFG_MALLOC_LEN
#endif

extern ulong __init_end;
extern ulong _end;

extern	void timer_init(void);

#if defined(CONFIG_WATCHDOG)
# define INIT_FUNC_WATCHDOG_INIT	watchdog_init,
# define WATCHDOG_DISABLE		watchdog_disable

extern int watchdog_init(void);
extern int watchdog_disable(void);
#else
# define INIT_FUNC_WATCHDOG_INIT	/* undef */
# define WATCHDOG_DISABLE		/* undef */
#endif /* CONFIG_WATCHDOG */

ulong monitor_flash_len;

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
static	ulong	mem_malloc_start = 0;
static	ulong	mem_malloc_end	 = 0;
static	ulong	mem_malloc_brk	 = 0;

/************************************************************************
 * Utilities								*
 ************************************************************************
 */

/*
 * The Malloc area is immediately below the monitor copy in DRAM
 */
static void mem_malloc_init (void)
{
	ulong dest_addr = CFG_MONITOR_BASE + gd->reloc_off;

	mem_malloc_end = dest_addr;
	mem_malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;

	memset ((void *) mem_malloc_start,
		0,
		mem_malloc_end - mem_malloc_start);
}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) ||
	    (new > mem_malloc_end) ) {
		return (NULL);
	}
	mem_malloc_brk = new;
	return ((void *)old);
}

char *strmhz(char *buf, long hz)
{
    long l, n;
    long m;

    n = hz / 1000000L;

    l = sprintf (buf, "%ld", n);

    m = (hz % 1000000L) / 1000L;

    if (m != 0)
	sprintf (buf+l, ".%03ld", m);

    return (buf);
}

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

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * but let's get it working (again) first...
 */

static int init_baudrate (void)
{
	uchar tmp[64];	/* long enough for environment variables */
	int i = getenv_r ("baudrate", tmp, sizeof (tmp));

	gd->baudrate = (i > 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;
	return (0);
}

/***********************************************************************/

static int init_func_ram (void)
{
	int board_type = 0;	/* use dummy arg */
	puts ("DRAM:  ");

	if ((gd->ram_size = initdram (board_type)) > 0) {
		print_size (gd->ram_size, "\n");
		return (0);
	}
	puts (failed);
	return (1);
}

/***********************************************************************/

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
static int init_func_i2c (void)
{
	puts ("I2C:   ");
	i2c_init (CFG_I2C_SPEED, CFG_I2C_SLAVE);
	puts ("ready\n");
	return (0);
}
#endif

/***********************************************************************/

/************************************************************************
 * Initialization sequence						*
 ************************************************************************
 */

init_fnc_t *init_sequence[] = {
	get_clocks,
	env_init,
	init_baudrate,
	serial_init,
	console_init_f,
	display_options,
	checkcpu,
	checkboard,
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	init_func_i2c,
#endif
	init_func_ram,
#if defined(CFG_DRAM_TEST)
	testdram,
#endif /* CFG_DRAM_TEST */
	INIT_FUNC_WATCHDOG_INIT
	NULL,			/* Terminate this list */
};


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

void
board_init_f (ulong bootflag)
{
	bd_t *bd;
	ulong len, addr, addr_sp;
	ulong *paddr;
	gd_t *id;
	init_fnc_t **init_fnc_ptr;
#ifdef CONFIG_PRAM
	int i;
	ulong reg;
	uchar tmp[64];		/* long enough for environment variables */
#endif

	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *) (CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET);
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	/* Clear initial global data */
	memset ((void *) gd, 0, sizeof (gd_t));

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}

	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM.
	 *
	 * Reserve memory at end of RAM for (top down in that order):
	 *	- protected RAM
	 *	- LCD framebuffer
	 *	- monitor code
	 *	- board info struct
	 */
	len = (ulong)&_end - CFG_MONITOR_BASE;

	addr = CFG_SDRAM_BASE + gd->ram_size;

#ifdef CONFIG_LOGBUFFER
	/* reserve kernel log buffer */
	addr -= (LOGBUFF_RESERVE);
	debug ("Reserving %dk for kernel logbuffer at %08lx\n", LOGBUFF_LEN, addr);
#endif

#ifdef CONFIG_PRAM
	/*
	 * reserve protected RAM
	 */
	i = getenv_r ("pram", tmp, sizeof (tmp));
	reg = (i > 0) ? simple_strtoul (tmp, NULL, 10) : CONFIG_PRAM;
	addr -= (reg << 10);		/* size is in kB */
	debug ("Reserving %ldk for protected RAM at %08lx\n", reg, addr);
#endif /* CONFIG_PRAM */

	/*
	 * reserve memory for U-Boot code, data & bss
	 * round down to next 4 kB limit
	 */
	addr -= len;
	addr &= ~(4096 - 1);

	debug ("Reserving %ldk for U-Boot at: %08lx\n", len >> 10, addr);

	/*
	 * reserve memory for malloc() arena
	 */
	addr_sp = addr - TOTAL_MALLOC_LEN;
	debug ("Reserving %dk for malloc() at: %08lx\n",
			TOTAL_MALLOC_LEN >> 10, addr_sp);

	/*
	 * (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	addr_sp -= sizeof (bd_t);
	bd = (bd_t *) addr_sp;
	gd->bd = bd;
	debug ("Reserving %d Bytes for Board Info at: %08lx\n",
			sizeof (bd_t), addr_sp);
	addr_sp -= sizeof (gd_t);
	id = (gd_t *) addr_sp;
	debug ("Reserving %d Bytes for Global Data at: %08lx\n",
			sizeof (gd_t), addr_sp);

 	/* Reserve memory for boot params. */
	addr_sp -= CFG_BOOTPARAMS_LEN;
	bd->bi_boot_params = addr_sp;
	debug ("Reserving %dk for boot parameters at: %08lx\n",
			CFG_BOOTPARAMS_LEN >> 10, addr_sp);

	/*
	 * Finally, we set up a new (bigger) stack.
	 *
	 * Leave some safety gap for SP, force alignment on 16 byte boundary
	 * Clear initial stack frame
	 */
	addr_sp -= 16;
	addr_sp &= ~0xF;

	paddr = (ulong *)addr_sp;
	*paddr-- = 0;
	*paddr-- = 0;
	addr_sp = (ulong)paddr;

	debug ("Stack Pointer at: %08lx\n", addr_sp);

	/*
	 * Save local variables to board info struct
	 */
	bd->bi_memstart  = CFG_SDRAM_BASE;	/* start of  DRAM memory      */
	bd->bi_memsize   = gd->ram_size;	/* size  of  DRAM memory in bytes */
#ifdef CFG_INIT_RAM_ADDR
	bd->bi_sramstart = CFG_INIT_RAM_ADDR;	/* start of  SRAM memory	*/
	bd->bi_sramsize  = CFG_INIT_RAM_END;	/* size  of  SRAM memory	*/
#endif
	bd->bi_mbar_base = CFG_MBAR;		/* base of internal registers */

	bd->bi_bootflags = bootflag;		/* boot / reboot flag (for LynxOS)    */

	WATCHDOG_RESET ();
	bd->bi_intfreq = gd->cpu_clk;	/* Internal Freq, in Hz */
	bd->bi_busfreq = gd->bus_clk;	/* Bus Freq,      in Hz */
	bd->bi_baudrate = gd->baudrate;	/* Console Baudrate     */

#ifdef CFG_EXTBDINFO
	strncpy (bd->bi_s_version, "1.2", sizeof (bd->bi_s_version));
	strncpy (bd->bi_r_version, U_BOOT_VERSION, sizeof (bd->bi_r_version));
#endif

	WATCHDOG_RESET ();

#ifdef CONFIG_POST
	post_bootmode_init();
	post_run (NULL, POST_ROM | post_bootmode_get(0));
#endif

	WATCHDOG_RESET();

	memcpy (id, (void *)gd, sizeof (gd_t));

	debug ("Start relocate of code from %08x to %08lx\n", CFG_MONITOR_BASE, addr);
	relocate_code (addr_sp, id, addr);

	/* NOTREACHED - jump_to_ram() does not return */
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
void board_init_r (gd_t *id, ulong dest_addr)
{
	cmd_tbl_t *cmdtp;
	char *s, *e;
	bd_t *bd;
	int i;
	extern void malloc_bin_reloc (void);

#ifndef CFG_ENV_IS_NOWHERE
	extern char * env_name_spec;
#endif
#ifndef CFG_NO_FLASH
	ulong flash_size;
#endif
	gd = id;		/* initialize RAM version of global data */
	bd = gd->bd;

	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */

#ifdef CONFIG_SERIAL_MULTI
	serial_initialize();
#endif

	debug ("Now running in RAM - U-Boot at: %08lx\n", dest_addr);

	WATCHDOG_RESET ();

	gd->reloc_off =  dest_addr - CFG_MONITOR_BASE;

	monitor_flash_len = (ulong)&__init_end - dest_addr;

	/*
	 * We have to relocate the command table manually
	 */
	for (cmdtp = &__u_boot_cmd_start; cmdtp !=  &__u_boot_cmd_end; cmdtp++) {
		ulong addr;
		addr = (ulong) (cmdtp->cmd) + gd->reloc_off;
#if 0
		printf ("Command \"%s\": 0x%08lx => 0x%08lx\n",
				cmdtp->name, (ulong) (cmdtp->cmd), addr);
#endif
		cmdtp->cmd =
			(int (*)(struct cmd_tbl_s *, int, int, char *[]))addr;

		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (char *)addr;

		if (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef	CFG_LONGHELP
		if (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif
	}
	/* there are some other pointer constants we must deal with */
#ifndef CFG_ENV_IS_NOWHERE
	env_name_spec += gd->reloc_off;
#endif

	WATCHDOG_RESET ();

#ifdef CONFIG_LOGBUFFER
	logbuff_init_ptrs ();
#endif
#ifdef CONFIG_POST
	post_output_backlog ();
	post_reloc ();
#endif
	WATCHDOG_RESET();

#if 0
	/* instruction cache enabled in cpu_init_f() for faster relocation */
	icache_enable ();	/* it's time to enable the instruction cache */
#endif

	/*
	 * Setup trap handlers
	 */
	trap_init (CFG_SDRAM_BASE);

#if !defined(CFG_NO_FLASH)
	puts ("FLASH: ");

	if ((flash_size = flash_init ()) > 0) {
# ifdef CFG_FLASH_CHECKSUM
		print_size (flash_size, "");
		/*
		 * Compute and print flash CRC if flashchecksum is set to 'y'
		 *
		 * NOTE: Maybe we should add some WATCHDOG_RESET()? XXX
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
# else	/* !CFG_FLASH_CHECKSUM */
		print_size (flash_size, "\n");
# endif /* CFG_FLASH_CHECKSUM */
	} else {
		puts (failed);
		hang ();
	}

	bd->bi_flashstart = CFG_FLASH_BASE;	/* update start of FLASH memory    */
	bd->bi_flashsize = flash_size;	/* size of FLASH memory (final value) */
	bd->bi_flashoffset = 0;
#else	/* CFG_NO_FLASH */
	bd->bi_flashsize = 0;
	bd->bi_flashstart = 0;
	bd->bi_flashoffset = 0;
#endif /* !CFG_NO_FLASH */

	WATCHDOG_RESET ();

	/* initialize higher level parts of CPU like time base and timers */
	cpu_init_r ();

	WATCHDOG_RESET ();

	/* initialize malloc() area */
	mem_malloc_init ();
	malloc_bin_reloc ();

#ifdef CONFIG_SPI
# if !defined(CFG_ENV_IS_IN_EEPROM)
	spi_init_f ();
# endif
	spi_init_r ();
#endif

	/* relocate environment function pointers etc. */
	env_relocate ();

	/*
	 * Fill in missing fields of bd_info.
	 * We do this here, where we have "normal" access to the
	 * environment; we used to do this still running from ROM,
	 * where had to use getenv_r(), which can be pretty slow when
	 * the environment is in EEPROM.
	 */
	s = getenv ("ethaddr");
	for (i = 0; i < 6; ++i) {
		bd->bi_enetaddr[i] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
#ifdef CONFIG_HAS_ETH1
	/* handle the 2nd ethernet address */

	s = getenv ("eth1addr");
	for (i = 0; i < 6; ++i) {
		bd->bi_enet1addr[i] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
#endif
#ifdef CONFIG_HAS_ETH2
	/* handle the 3rd ethernet address */

	s = getenv ("eth2addr");
	for (i = 0; i < 6; ++i) {
		bd->bi_enet2addr[i] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
#endif

#ifdef CONFIG_HAS_ETH3
	/* handle 4th ethernet address */
	s = getenv("eth3addr");
	for (i = 0; i < 6; ++i) {
		bd->bi_enet3addr[i] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
#endif

	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr ("ipaddr");

	WATCHDOG_RESET ();

#if defined(CONFIG_PCI)
	/*
	 * Do pci configuration
	 */
	pci_init ();
#endif

	/** leave this here (after malloc(), environment and PCI are working) **/
	/* Initialize devices */
	devices_init ();

	/* Initialize the jump table for applications */
	jumptable_init ();

	/* Initialize the console (after the relocation and devices init) */
	console_init_r ();

#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
#endif

#if defined(CONFIG_CMD_KGDB)
	WATCHDOG_RESET ();
	puts ("KGDB:  ");
	kgdb_init ();
#endif

	debug ("U-Boot relocated to %08lx\n", dest_addr);

	/*
	 * Enable Interrupts
	 */
	interrupt_init ();

	/* Must happen after interrupts are initialized since
	 * an irq handler gets installed
	 */
	timer_init();

#ifdef CONFIG_SERIAL_SOFTWARE_FIFO
	serial_buffered_init();
#endif

#ifdef CONFIG_STATUS_LED
	status_led_set (STATUS_LED_BOOT, STATUS_LED_BLINKING);
#endif

	udelay (20);

	set_timer (0);

	/* Insert function pointers now that we have relocated the code */

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if defined(CONFIG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif

	WATCHDOG_RESET ();

#if defined(CONFIG_CMD_DOC)
	WATCHDOG_RESET ();
	puts ("DOC:   ");
	doc_init ();
#endif

#if defined(CONFIG_CMD_NAND)
	WATCHDOG_RESET ();
	puts ("NAND:  ");
	nand_init();		/* go init the NAND */
#endif

#if defined(CONFIG_CMD_NET)
	WATCHDOG_RESET();
#if defined(FEC_ENET)
	eth_init(bd);
#endif
#if defined(CONFIG_NET_MULTI)
	puts ("Net:   ");
        eth_initialize (bd);
#endif
#endif

#ifdef CONFIG_POST
	post_run (NULL, POST_RAM | post_bootmode_get(0));
#endif

#if defined(CONFIG_CMD_PCMCIA) \
    && !defined(CONFIG_CMD_IDE)
	WATCHDOG_RESET ();
	puts ("PCMCIA:");
	pcmcia_init ();
#endif

#if defined(CONFIG_CMD_IDE)
	WATCHDOG_RESET ();
	puts ("IDE:   ");
	ide_init ();
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

#if defined(CONFIG_CMD_BEDBUG)
	WATCHDOG_RESET ();
	bedbug_init ();
#endif

#if defined(CONFIG_PRAM) || defined(CONFIG_LOGBUFFER)
	/*
	 * Export available size of memory for Linux,
	 * taking into account the protected RAM at top of memory
	 */
	{
		ulong pram;
		uchar memsz[32];
#ifdef CONFIG_PRAM
		char *s;

		if ((s = getenv ("pram")) != NULL) {
			pram = simple_strtoul (s, NULL, 10);
		} else {
			pram = CONFIG_PRAM;
		}
#else
		pram=0;
#endif
#ifdef CONFIG_LOGBUFFER
		/* Also take the logbuffer into account (pram is in kB) */
		pram += (LOGBUFF_LEN+LOGBUFF_OVERHEAD)/1024;
#endif
		sprintf (memsz, "%ldk", (bd->bi_memsize / 1024) - pram);
		setenv ("mem", memsz);
	}
#endif

#ifdef CONFIG_MODEM_SUPPORT
 {
	 extern int do_mdm_init;
	 do_mdm_init = gd->do_mdm_init;
 }
#endif

#ifdef CONFIG_WATCHDOG
	/* disable watchdog if environment is set */
	if ((s = getenv ("watchdog")) != NULL) {
		if (strncmp (s, "off", 3) == 0) {
			WATCHDOG_DISABLE ();
		}
	}
#endif /* CONFIG_WATCHDOG*/


	/* Initialization complete - start the monitor */

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		WATCHDOG_RESET ();
		main_loop ();
	}

	/* NOTREACHED - no way out of command loop except booting */
}


void hang(void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
