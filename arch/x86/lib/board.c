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
#include <asm/processor.h>

#include <asm/init_helpers.h>
#include <asm/init_wrappers.h>

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
static int copy_gd_to_ram(void);

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

init_fnc_t *init_sequence_r[] = {
	init_bd_struct_r,
	mem_malloc_init_r,
	cpu_init_r,
	board_early_init_r,
	dram_init,
	interrupt_init,
	timer_init,
	display_banner,
	display_dram_config,
#ifdef CONFIG_SERIAL_MULTI
	serial_initialize_r,
#endif
#ifndef CONFIG_SYS_NO_FLASH
	flash_init_r,
#endif
	env_relocate_r,
#ifdef CONFIG_CMD_NET
	init_ip_address_r,
#endif
#ifdef CONFIG_PCI
	pci_init_r,
#endif
	stdio_init,
	jumptable_init_r,
	console_init_r,
#ifdef CONFIG_MISC_INIT_R
	misc_init_r,
#endif
#if defined(CONFIG_CMD_PCMCIA) && !defined(CONFIG_CMD_IDE)
	pci_init_r,
#endif
#if defined(CONFIG_CMD_KGDB)
	kgdb_init_r,
#endif
	enable_interrupts_r,
#ifdef CONFIG_STATUS_LED
	status_led_set_r,
#endif
	set_load_addr_r,
#if defined(CONFIG_CMD_NET)
	set_bootfile_r,
#endif
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

static int calculate_relocation_address(void)
{
	ulong text_start = (ulong)&__text_start;
	ulong bss_end = (ulong)&__bss_end;
	ulong dest_addr;

	/*
	 * NOTE: All destination address are rounded down to 16-byte
	 *       boundary to satisfy various worst-case alignment
	 *       requirements
	 */

	/* Global Data is at top of available memory */
	dest_addr = gd->ram_size;
	dest_addr -= GENERATED_GBL_DATA_SIZE;
	dest_addr &= ~15;
	gd->new_gd_addr = dest_addr;

	/* GDT is below Global Data */
	dest_addr -= X86_GDT_SIZE;
	dest_addr &= ~15;
	gd->gdt_addr = dest_addr;

	/* Stack is below GDT */
	gd->start_addr_sp = dest_addr;

	/* U-Boot is below the stack */
	dest_addr -= CONFIG_SYS_STACK_SIZE;
	dest_addr -= (bss_end - text_start);
	dest_addr &= ~15;
	gd->relocaddr = dest_addr;
	gd->reloc_off = (dest_addr - text_start);

	return 0;
}

/* Perform all steps necessary to get RAM initialised ready for relocation */
void board_init_f(ulong boot_flags)
{
	gd->flags = boot_flags;

	do_init_loop(init_sequence_f);

	/*
	 * SDRAM is now initialised, U-Boot has been copied into SDRAM,
	 * the BSS has been cleared etc. The final stack can now be setup
	 * in SDRAM. Code execution will continue (momentarily) in Flash,
	 * but with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	board_init_f_r_trampoline(gd->start_addr_sp);

	/* NOTREACHED - board_init_f_r_trampoline() does not return */
	while (1)
		;
}

void board_init_f_r(void)
{
	if (copy_gd_to_ram() != 0)
		hang();

	if (init_cache() != 0)
		hang();

	relocate_code(0, gd, 0);

	/* NOTREACHED - relocate_code() does not return */
	while (1)
		;
}

static int copy_gd_to_ram(void)
{
	gd_t *ram_gd;

	/*
	 * Global data is still in temporary memory (the CPU cache).
	 * calculate_relocation_address() has set gd->new_gd_addr to
	 * where the global data lives in RAM but getting it there
	 * safely is a bit tricky due to the 'F-Segment Hack' that
	 * we need to use for x86
	 */
	ram_gd = (gd_t *)gd->new_gd_addr;
	memcpy((void *)ram_gd, gd, sizeof(gd_t));

	/*
	 * Reload the Global Descriptor Table so FS points to the
	 * in-RAM copy of Global Data (calculate_relocation_address()
	 * has already calculated the in-RAM location of the GDT)
	 */
	ram_gd->gd_addr = (ulong)ram_gd;
	init_gd(ram_gd, (u64 *)gd->gdt_addr);

	return 0;
}

void board_init_r(gd_t *id, ulong dest_addr)
{
	gd->flags |= GD_FLG_RELOC;

	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("" : : : "memory");

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
