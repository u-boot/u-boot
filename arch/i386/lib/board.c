/*
 * (C) Copyright 2002
 * Daniel Engstr�m, Omicron Ceti AB, daniel@omicron.se
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
#include <stdio_dev.h>
#include <timestamp.h>
#include <version.h>
#include <malloc.h>
#include <net.h>
#include <ide.h>
#include <serial.h>
#include <asm/u-boot-i386.h>
#include <elf.h>

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* Exports from the Linker Script */
extern ulong __text_start;
extern ulong __data_end;
extern ulong __rel_dyn_start;
extern ulong __rel_dyn_end;
extern ulong __bss_start;
extern ulong __bss_end;

const char version_string[] =
	U_BOOT_VERSION" (" U_BOOT_DATE " - " U_BOOT_TIME ")";

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
	int i = getenv_f("baudrate", tmp, 64);

	gd->baudrate = (i != 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;

	return (0);
}

static int display_banner (void)
{

	printf ("\n\n%s\n\n", version_string);
/*
	printf ("U-Boot code: %08lX -> %08lX  data: %08lX -> %08lX\n"
		"        BSS: %08lX -> %08lX stack: %08lX -> %08lX\n",
		i386boot_start, i386boot_romdata_start-1,
		i386boot_romdata_dest, i386boot_romdata_dest+i386boot_romdata_size-1,
		i386boot_bss_start, i386boot_bss_start+i386boot_bss_size-1,
		i386boot_bss_start+i386boot_bss_size,
		i386boot_bss_start+i386boot_bss_size+CONFIG_SYS_STACK_SIZE-1);

*/

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
	cpu_init_r,		/* basic cpu dependent setup */
	board_early_init_r,	/* basic board dependent setup */
	dram_init,		/* configure available RAM banks */
	interrupt_init,		/* set up exceptions */
	timer_init,
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	display_banner,
	display_dram_config,

	NULL,
};

gd_t *gd;

/*
 * Load U-Boot into RAM, initialize BSS, perform relocation adjustments
 */
void board_init_f (ulong gdp)
{
	void *text_start = &__text_start;
	void *data_end = &__data_end;
	void *rel_dyn_start = &__rel_dyn_start;
	void *rel_dyn_end = &__rel_dyn_end;
	void *bss_start = &__bss_start;
	void *bss_end = &__bss_end;

	ulong *dst_addr;
	ulong *src_addr;
	ulong *end_addr;

	void *dest_addr;
	ulong rel_offset;
	Elf32_Rel *re_src;
	Elf32_Rel *re_end;

	/* Calculate destination RAM Address and relocation offset */
	dest_addr  = (void *)gdp - (bss_end - text_start);
	rel_offset = text_start - dest_addr;

	/* Perform low-level initialization only when cold booted */
	if (((gd_t *)gdp)->flags & GD_FLG_COLD_BOOT) {
		/* First stage CPU initialization */
		if (cpu_init_f() != 0)
			hang();

		/* First stage Board initialization */
		if (board_early_init_f() != 0)
			hang();
	}

	/* Copy U-Boot into RAM */
	dst_addr = (ulong *)dest_addr;
	src_addr = (ulong *)(text_start + ((gd_t *)gdp)->load_off);
	end_addr = (ulong *)(data_end  + ((gd_t *)gdp)->load_off);

	while (src_addr < end_addr)
		*dst_addr++ = *src_addr++;

	/* Clear BSS */
	dst_addr = (ulong *)(bss_start - rel_offset);
	end_addr = (ulong *)(bss_end - rel_offset);

	while (dst_addr < end_addr)
		*dst_addr++ = 0x00000000;

	/* Perform relocation adjustments */
	re_src = (Elf32_Rel *)(rel_dyn_start + ((gd_t *)gdp)->load_off);
	re_end = (Elf32_Rel *)(rel_dyn_end + ((gd_t *)gdp)->load_off);

	do {
		if (re_src->r_offset >= CONFIG_SYS_TEXT_BASE)
			if (*(Elf32_Addr *)(re_src->r_offset - rel_offset) >= CONFIG_SYS_TEXT_BASE)
				*(Elf32_Addr *)(re_src->r_offset - rel_offset) -= rel_offset;
	} while (re_src++ < re_end);

	((gd_t *)gdp)->reloc_off = rel_offset;
	((gd_t *)gdp)->flags |= GD_FLG_RELOC;

	/* Enter the relocated U-Boot! */
	(board_init_r - rel_offset)((gd_t *)gdp, (ulong)dest_addr);

	/* NOTREACHED - board_init_f() does not return */
	while(1);
}

void board_init_r(gd_t *id, ulong dest_addr)
{
	char *s;
	int i;
	ulong size;
	static bd_t bd_data;
	init_fnc_t **init_fnc_ptr;

	show_boot_progress(0x21);

	gd = id;
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	gd->bd = &bd_data;
	memset (gd->bd, 0, sizeof (bd_t));
	show_boot_progress(0x22);

	gd->baudrate =  CONFIG_BAUDRATE;

	mem_malloc_init((((ulong)dest_addr - CONFIG_SYS_MALLOC_LEN)+3)&~3,
			CONFIG_SYS_MALLOC_LEN);

	for (init_fnc_ptr = init_sequence, i=0; *init_fnc_ptr; ++init_fnc_ptr, i++) {
		show_boot_progress(0xa130|i);

		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}
	show_boot_progress(0x23);

#ifdef CONFIG_SERIAL_MULTI
	serial_initialize();
#endif
	/* configure available FLASH banks */
	size = flash_init();
	display_flash_config(size);
	show_boot_progress(0x24);

	show_boot_progress(0x25);

	/* initialize environment */
	env_relocate ();
	show_boot_progress(0x26);


#ifdef CONFIG_CMD_NET
	/* IP Address */
	bd_data.bi_ip_addr = getenv_IPaddr ("ipaddr");
#endif

#if defined(CONFIG_PCI)
	/*
	 * Do pci configuration
	 */
	pci_init();
#endif

	show_boot_progress(0x27);


	stdio_init ();

	jumptable_init ();

	/* Initialize the console (after the relocation and devices init) */
	console_init_r();

#ifdef CONFIG_MISC_INIT_R
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif

#if defined(CONFIG_CMD_PCMCIA) && !defined(CONFIG_CMD_IDE)
	WATCHDOG_RESET();
	puts ("PCMCIA:");
	pcmcia_init();
#endif

#if defined(CONFIG_CMD_KGDB)
	WATCHDOG_RESET();
	puts("KGDB:  ");
	kgdb_init();
#endif

	/* enable exceptions */
	enable_interrupts();
	show_boot_progress(0x28);

#ifdef CONFIG_STATUS_LED
	status_led_set (STATUS_LED_BOOT, STATUS_LED_BLINKING);
#endif

	udelay(20);

	set_timer (0);

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if defined(CONFIG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif

	WATCHDOG_RESET();

#if defined(CONFIG_CMD_IDE)
	WATCHDOG_RESET();
	puts("IDE:   ");
	ide_init();
#endif

#if defined(CONFIG_CMD_SCSI)
	WATCHDOG_RESET();
	puts("SCSI:  ");
	scsi_init();
#endif

#if defined(CONFIG_CMD_DOC)
	WATCHDOG_RESET();
	puts("DOC:   ");
	doc_init();
#endif

#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif
#if defined(CONFIG_CMD_NET)
#if defined(CONFIG_NET_MULTI)
	WATCHDOG_RESET();
	puts("Net:   ");
#endif
	eth_initialize(gd->bd);
#endif

#if ( defined(CONFIG_CMD_NET)) && (0)
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

unsigned long do_go_exec (ulong (*entry)(int, char * const []), int argc, char * const argv[])
{
	unsigned long ret = 0;
	char **argv_tmp;

	/*
	 * x86 does not use a dedicated register to pass the pointer to
	 * the global_data, so it is instead passed as argv[-1]. By using
	 * argv[-1], the called 'Application' can use the contents of
	 * argv natively. However, to safely use argv[-1] a new copy of
	 * argv is needed with the extra element
	 */
	argv_tmp = malloc(sizeof(char *) * (argc + 1));

	if (argv_tmp) {
		argv_tmp[0] = (char *)gd;

		memcpy(&argv_tmp[1], argv, (size_t)(sizeof(char *) * argc));

		ret = (entry) (argc, &argv_tmp[1]);
		free(argv_tmp);
	}

	return ret;
}

void setup_pcat_compatibility(void)
	__attribute__((weak, alias("__setup_pcat_compatibility")));

void __setup_pcat_compatibility(void)
{
}
