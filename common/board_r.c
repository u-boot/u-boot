/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
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
#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>
#endif
#include <environment.h>
#include <fdtdec.h>
#include <initcall.h>
#include <logbuff.h>
#include <malloc.h>
#include <mmc.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <serial.h>
#include <stdio_dev.h>
#include <asm/sections.h>

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;


static int initr_reloc(void)
{
	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");

	return 0;
}

#ifdef CONFIG_ARM
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
static int initr_caches(void)
{
	/* Enable caches */
	enable_caches();
	return 0;
}
#endif

static int initr_reloc_global_data(void)
{
#ifdef CONFIG_SYS_SYM_OFFSETS
	monitor_flash_len = _end_ofs;
#else
	monitor_flash_len = (ulong)&__init_end - gd->dest_addr;
#endif
}

static int initr_serial(void)
{
	serial_initialize();
	return 0;
}

#ifdef CONFIG_LOGBUFFER
unsigned long logbuffer_base(void)
{
	return gd->ram_top - LOGBUFF_LEN;
}

static int initr_logbuffer(void)
{
	logbuff_init_ptrs();
	return 0;
}
#endif

#ifdef CONFIG_POST
static int initr_post_backlog(void)
{
	post_output_backlog();
	return 0;
}
#endif

static int initr_malloc(void)
{
	ulong malloc_start;

	/* The malloc area is immediately below the monitor copy in DRAM */
	malloc_start = gd->dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_init(malloc_start, TOTAL_MALLOC_LEN);
	return 0;
}

__weak int power_init_board(void)
{
	return 0;
}

static int initr_announce(void)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->dest_addr);
	return 0;
}

#if !defined(CONFIG_SYS_NO_FLASH)
static int initr_flash(void)
{
	ulong flash_size;

	puts("Flash: ");

	flash_size = flash_init();
	if (flash_size <= 0) {
		puts("*** failed ***\n");
		return -1;
	}
	print_size(flash_size, "");
#ifdef CONFIG_SYS_FLASH_CHECKSUM
	/*
	* Compute and print flash CRC if flashchecksum is set to 'y'
	*
	* NOTE: Maybe we should add some WATCHDOG_RESET()? XXX
	*/
	if (getenv_yesno("flashchecksum") == 1) {
		printf("  CRC: %08X", crc32(0,
			(const unsigned char *) CONFIG_SYS_FLASH_BASE,
			flash_size));
	}
#endif /* CONFIG_SYS_FLASH_CHECKSUM */
	putc('\n');

	return 0;
}
#endif

#ifdef CONFIG_CMD_NAND
/* go init the NAND */
int initr_nand(void)
{
	puts("NAND:  ");
	nand_init();
	return 0;
}
#endif

#if defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
int initr_onenand(void)
{
	puts("NAND:  ");
	onenand_init();
	return 0;
}
#endif

#ifdef CONFIG_GENERIC_MMC
int initr_mmc(void)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	return 0;
}
#endif

#ifdef CONFIG_HAS_DATAFLASH
int initr_dataflash(void)
{
	AT91F_DataflashInit();
	dataflash_print_info();
	return 0;
}
#endif

/*
 * Tell if it's OK to load the environment early in boot.
 *
 * If CONFIG_OF_CONFIG is defined, we'll check with the FDT to see
 * if this is OK (defaulting to saying it's OK).
 *
 * NOTE: Loading the environment early can be a bad idea if security is
 *       important, since no verification is done on the environment.
 *
 * @return 0 if environment should not be loaded, !=0 if it is ok to load
 */
static int should_load_env(void)
{
#ifdef CONFIG_OF_CONTROL
	return fdtdec_get_config_int(gd->fdt_blob, "load-environment", 1);
#elif defined CONFIG_DELAY_ENVIRONMENT
	return 0;
#else
	return 1;
#endif
}

static int initr_env(void)
{
	/* initialize environment */
	if (should_load_env())
		env_relocate();
	else
		set_default_env(NULL);

	/* Initialize from environment */
	load_addr = getenv_ulong("loadaddr", 16, load_addr);
	return 0;
}

static int initr_jumptable(void)
{
	jumptable_init();
	return 0;
}

#if defined(CONFIG_API)
static int initr_api(void)
{
	/* Initialize API */
	api_init();
	return 0;
}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO_LATE
static int show_model_r(void)
{
	/* Put this here so it appears on the LCD, now it is ready */
# ifdef CONFIG_OF_CONTROL
	const char *model;

	model = (char *)fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	printf("Model: %s\n", model ? model : "<unknown>");
# else
	checkboard();
# endif
}
#endif

/* enable exceptions */
static int initr_enable_interrupts(void)
{
	enable_interrupts();
	return 0;
}

#ifdef CONFIG_CMD_NET
static int initr_ethaddr(void)
{

	return 0;
}
#endif /* CONFIG_CMD_NET */

#ifdef CONFIG_BITBANGMII
static int initr_bbmii(void)
{
	bb_miiphy_init();
	return 0;
}
#endif

#ifdef CONFIG_CMD_NET
static int initr_net(void)
{
	puts("Net:   ");
	eth_initialize(gd->bd);
#if defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
	return 0;
}
#endif

#ifdef CONFIG_POST
static int initr_post(void)
{
	post_run(NULL, POST_RAM | post_bootmode_get(0));
	return 0;
}
#endif

#if defined(CONFIG_PRAM) || defined(CONFIG_LOGBUFFER)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
int initr_mem(void)
{
	ulong pram = 0;
	char memsz[32];

# ifdef CONFIG_PRAM
	pram = getenv_ulong("pram", 10, CONFIG_PRAM);
# endif
# if defined(CONFIG_LOGBUFFER) && !defined(CONFIG_ALT_LB_ADDR)
	/* Also take the logbuffer into account (pram is in kB) */
	pram += (LOGBUFF_LEN + LOGBUFF_OVERHEAD) / 1024;
# endif
	sprintf(memsz, "%ldk", (gd->ram_size / 1024) - pram);
	setenv("mem", memsz);
}
#endif

static int run_main_loop(void)
{
	/* main_loop() can return to retry autoboot, if so just run it again */
	for (;;)
		main_loop();
	return 0;
}

/*
 * Over time we hope to remove these functions with code fragments and
 * stub funtcions, and instead call the relevant function directly.
 *
 * We also hope to remove most of the driver-related init and do it if/when
 * the driver is later used.
 */
init_fnc_t init_sequence_r[] = {
	initr_reloc,
#ifdef CONFIG_ARM
	initr_caches,
	board_init,	/* Setup chipselects */
#endif
	initr_reloc_global_data,
	initr_serial,
	initr_announce,
#ifdef CONFIG_LOGBUFFER
	initr_logbuffer,
#endif
#ifdef CONFIG_POST
	initr_post_backlog,
#endif
	initr_malloc,
#ifdef CONFIG_ARCH_EARLY_INIT_R
	arch_early_init_r,
#endif
	power_init_board,
#ifndef CONFIG_SYS_NO_FLASH
	initr_flash,
#endif
#ifdef CONFIG_CMD_NAND
	initr_nand,
#endif
#ifdef CONFIG_CMD_ONENAND
	initr_onenand,
#endif
#ifdef CONFIG_GENERIC_MMC
	initr_mmc,
#endif
#ifdef CONFIG_HAS_DATAFLASH
	initr_dataflash,
#endif
	initr_env,
	stdio_init,
	initr_jumptable,
#ifdef CONFIG_API
	initr_api,
#endif
	console_init_r,		/* fully init console as a device */
#ifdef CONFIG_DISPLAY_BOARDINFO_LATE
	show_model_r,
#endif
#ifdef CONFIG_ARCH_MISC_INIT
	arch_misc_init,		/* miscellaneous arch-dependent init */
#endif
#ifdef CONFIG_MISC_INIT_R
	misc_init_r,		/* miscellaneous platform-dependent init */
#endif
	interrupt_init,
	initr_enable_interrupts,
#ifdef CONFIG_CMD_NET
	initr_ethaddr,
#endif
#ifdef CONFIG_BOARD_LATE_INIT
	board_late_init,
#endif
#ifdef CONFIG_BITBANGMII
	initr_bbmii,
#endif
#ifdef CONFIG_CMD_NET
	initr_net,
#endif
#ifdef CONFIG_POST
	initr_post,
#endif
	run_main_loop,
};

void board_init_r(gd_t *new_gd, ulong dest_addr)
{
	gd = new_gd;
	if (initcall_run_list(init_sequence_r))
		hang();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
