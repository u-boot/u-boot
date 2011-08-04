/*
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
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
#include <version.h>
#include <malloc.h>
#include <net.h>
#include <ide.h>
#include <serial.h>
#include <asm/u-boot-x86.h>
#include <elf.h>

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

/*
 * Pointer to initial global data area
 *
 * Here we initialize it.
 */
#undef	XTRN_DECLARE_GLOBAL_DATA_PTR
#define XTRN_DECLARE_GLOBAL_DATA_PTR	/* empty = allocate here */
DECLARE_GLOBAL_DATA_PTR = (gd_t *) (CONFIG_SYS_INIT_GD_ADDR);


/* Exports from the Linker Script */
extern ulong __text_start;
extern ulong __data_end;
extern ulong __rel_dyn_start;
extern ulong __rel_dyn_end;
extern ulong __bss_start;
extern ulong __bss_end;

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

static int calculate_relocation_address(void);
static int copy_uboot_to_ram(void);
static int clear_bss(void);
static int do_elf_reloc_fixups(void);

init_fnc_t *init_sequence_f[] = {
	cpu_init_f,
	board_early_init_f,
	env_init,
	init_baudrate,
	serial_init,
	console_init_f,
	dram_init_f,
	calculate_relocation_address,
	copy_uboot_to_ram,
	clear_bss,
	do_elf_reloc_fixups,

	NULL,
};

init_fnc_t *init_sequence_r[] = {
	cpu_init_r,		/* basic cpu dependent setup */
	board_early_init_r,	/* basic board dependent setup */
	dram_init,		/* configure available RAM banks */
	interrupt_init,		/* set up exceptions */
	timer_init,
	display_banner,
	display_dram_config,

	NULL,
};

gd_t *gd;

static int calculate_relocation_address(void)
{
	void *text_start = &__text_start;
	void *bss_end = &__bss_end;
	void *dest_addr;
	ulong rel_offset;

	/* Calculate destination RAM Address and relocation offset */
	dest_addr = (void *)gd->ram_size;
	dest_addr -= CONFIG_SYS_STACK_SIZE;
	dest_addr -= (bss_end - text_start);
	rel_offset = dest_addr - text_start;

	gd->start_addr_sp = gd->ram_size;
	gd->relocaddr = (ulong)dest_addr;
	gd->reloc_off = rel_offset;

	return 0;
}

static int copy_uboot_to_ram(void)
{
	ulong *dst_addr = (ulong *)gd->relocaddr;
	ulong *src_addr = (ulong *)&__text_start;
	ulong *end_addr = (ulong *)&__data_end;

	while (src_addr < end_addr)
		*dst_addr++ = *src_addr++;

	return 0;
}

static int clear_bss(void)
{
	void *bss_start = &__bss_start;
	void *bss_end = &__bss_end;

	ulong *dst_addr = (ulong *)(bss_start + gd->reloc_off);
	ulong *end_addr = (ulong *)(bss_end + gd->reloc_off);;

	while (dst_addr < end_addr)
		*dst_addr++ = 0x00000000;

	return 0;
}

static int do_elf_reloc_fixups(void)
{
	Elf32_Rel *re_src = (Elf32_Rel *)(&__rel_dyn_start);
	Elf32_Rel *re_end = (Elf32_Rel *)(&__rel_dyn_end);

	do {
		if (re_src->r_offset >= CONFIG_SYS_TEXT_BASE)
			if (*(Elf32_Addr *)(re_src->r_offset + gd->reloc_off) >= CONFIG_SYS_TEXT_BASE)
				*(Elf32_Addr *)(re_src->r_offset + gd->reloc_off) += gd->reloc_off;
	} while (re_src++ < re_end);

	return 0;
}

/* Load U-Boot into RAM, initialize BSS, perform relocation adjustments */
void board_init_f(ulong boot_flags)
{
	init_fnc_t **init_fnc_ptr;

	gd->flags = boot_flags;

	for (init_fnc_ptr = init_sequence_f; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0)
			hang();
	}

	gd->flags |= GD_FLG_RELOC;

	/* Enter the relocated U-Boot! */
	relocate_code(gd->start_addr_sp, gd, gd->relocaddr);

	/* NOTREACHED - relocate_code() does not return */
	while(1);
}

void board_init_r(gd_t *id, ulong dest_addr)
{
	char *s;
	ulong size;
	static bd_t bd_data;
	static gd_t gd_data;
	init_fnc_t **init_fnc_ptr;

	show_boot_progress(0x21);

	/* Global data pointer is now writable */
	gd = &gd_data;
	memcpy(gd, id, sizeof(gd_t));

	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");

	gd->bd = &bd_data;
	memset (gd->bd, 0, sizeof (bd_t));
	show_boot_progress(0x22);

	gd->baudrate =  CONFIG_BAUDRATE;

	mem_malloc_init((((ulong)dest_addr - CONFIG_SYS_MALLOC_LEN)+3)&~3,
			CONFIG_SYS_MALLOC_LEN);

	for (init_fnc_ptr = init_sequence_r; *init_fnc_ptr; ++init_fnc_ptr) {
		if ((*init_fnc_ptr)() != 0)
			hang ();
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
