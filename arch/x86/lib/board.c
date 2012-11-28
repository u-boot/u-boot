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
#include <stdio_dev.h>
#include <asm/u-boot-x86.h>
#include <asm/relocate.h>
#include <asm/processor.h>

#include <asm/init_helpers.h>
#include <asm/init_wrappers.h>

/*
 * Breath some life into the board...
 *
 * Getting the board up and running is a three-stage process:
 *  1) Execute from Flash, SDRAM Uninitialised
 *     At this point, there is a limited amount of non-SDRAM memory
 *     (typically the CPU cache, but can also be SRAM or even a buffer of
 *     of some peripheral). This limited memory is used to hold:
 *      - The initial copy of the Global Data Structure
 *      - A temporary stack
 *      - A temporary x86 Global Descriptor Table
 *      - The pre-console buffer (if enabled)
 *
 *     The following is performed during this phase of execution:
 *      - Core low-level CPU initialisation
 *      - Console initialisation
 *      - SDRAM initialisation
 *
 *  2) Execute from Flash, SDRAM Initialised
 *     At this point we copy Global Data from the initial non-SDRAM
 *     memory and set up the permanent stack in SDRAM. The CPU cache is no
 *     longer being used as temporary memory, so we can now fully enable
 *     it.
 *
 *     The following is performed during this phase of execution:
 *      - Create final stack in SDRAM
 *      - Copy Global Data from temporary memory to SDRAM
 *      - Enabling of CPU cache(s),
 *      - Copying of U-Boot code and data from Flash to RAM
 *      - Clearing of the BSS
 *      - ELF relocation adjustments
 *
 *  3) Execute from SDRAM
 *     The following is performed during this phase of execution:
 *      - All remaining initialisation
 */

/*
 * The requirements for any new initalization function is simple: it is
 * a function with no parameters which returns an integer return code,
 * where 0 means "continue" and != 0 means "fatal error, hang the system"
 */
typedef int (init_fnc_t) (void);

/*
 * init_sequence_f is the list of init functions which are run when U-Boot
 * is executing from Flash with a limited 'C' environment. The following
 * limitations must be considered when implementing an '_f' function:
 *  - 'static' variables are read-only
 *  - Global Data (gd->xxx) is read/write
 *  - Stack space is limited
 *
 * The '_f' sequence must, as a minimum, initialise SDRAM. It _should_
 * also initialise the console (to provide early debug output)
 */
init_fnc_t *init_sequence_f[] = {
	cpu_init_f,
	board_early_init_f,
	env_init,
	init_baudrate_f,
	serial_init,
	console_init_f,
	dram_init_f,
	calculate_relocation_address,

	NULL,
};

/*
 * init_sequence_f_r is the list of init functions which are run when
 * U-Boot is executing from Flash with a semi-limited 'C' environment.
 * The following limitations must be considered when implementing an
 * '_f_r' function:
 *  - 'static' variables are read-only
 *  - Global Data (gd->xxx) is read/write
 *
 * The '_f_r' sequence must, as a minimum, copy U-Boot to RAM (if
 * supported).  It _should_, if possible, copy global data to RAM and
 * initialise the CPU caches (to speed up the relocation process)
 */
init_fnc_t *init_sequence_f_r[] = {
	init_cache_f_r,
	copy_uboot_to_ram,
	clear_bss,
	do_elf_reloc_fixups,

	NULL,
};

/*
 * init_sequence_r is the list of init functions which are run when U-Boot
 * is executing from RAM with a full 'C' environment. There are no longer
 * any limitations which must be considered when implementing an '_r'
 * function, (i.e.'static' variables are read/write)
 *
 * If not already done, the '_r' sequence must copy global data to RAM and
 * (should) initialise the CPU caches.
 */
init_fnc_t *init_sequence_r[] = {
	set_reloc_flag_r,
	init_bd_struct_r,
	mem_malloc_init_r,
	cpu_init_r,
	board_early_init_r,
	dram_init,
	interrupt_init,
	timer_init,
	display_banner,
	display_dram_config,
	serial_initialize_r,
#ifndef CONFIG_SYS_NO_FLASH
	flash_init_r,
#endif
	env_relocate_r,
#ifdef CONFIG_PCI
	pci_init_r,
#endif
	stdio_init,
	jumptable_init_r,
	console_init_r,
#ifdef CONFIG_MISC_INIT_R
	misc_init_r,
#endif
#if defined(CONFIG_CMD_KGDB)
	kgdb_init_r,
#endif
	enable_interrupts_r,
#ifdef CONFIG_STATUS_LED
	status_led_set_r,
#endif
	set_load_addr_r,
#if defined(CONFIG_CMD_IDE)
	ide_init_r,
#endif
#if defined(CONFIG_CMD_SCSI)
	scsi_init_r,
#endif
#if defined(CONFIG_CMD_DOC)
	doc_init_r,
#endif
#ifdef CONFIG_BITBANGMII
	bb_miiphy_init_r,
#endif
#if defined(CONFIG_CMD_NET)
	eth_initialize_r,
#ifdef CONFIG_RESET_PHY_R
	reset_phy_r,
#endif
#endif
#ifdef CONFIG_LAST_STAGE_INIT
	last_stage_init,
#endif
	NULL,
};

static void do_init_loop(init_fnc_t **init_fnc_ptr)
{
	for (; *init_fnc_ptr; ++init_fnc_ptr) {
		WATCHDOG_RESET();
		if ((*init_fnc_ptr)() != 0)
			hang();
	}
}

void board_init_f(ulong boot_flags)
{
	gd->flags = boot_flags;

	do_init_loop(init_sequence_f);

	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	board_init_f_r_trampoline(gd->start_addr_sp);

	/* NOTREACHED - board_init_f_r_trampoline() does not return */
	while (1)
		;
}

void board_init_f_r(void)
{
	do_init_loop(init_sequence_f_r);

	/*
	 * U-Boot has been copied into SDRAM, the BSS has been cleared etc.
	 * Transfer execution from Flash to RAM by calculating the address
	 * of the in-RAM copy of board_init_r() and calling it
	 */
	(board_init_r + gd->reloc_off)(gd, gd->relocaddr);

	/* NOTREACHED - board_init_r() does not return */
	while (1)
		;
}

void board_init_r(gd_t *id, ulong dest_addr)
{
	do_init_loop(init_sequence_r);

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;)
		main_loop();

	/* NOTREACHED - no way out of command loop except booting */
}

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;)
		;
}
