/*
 * U-boot - board.c First C file to be called contains init routines
 *
 * Copyright (c) 2005 blackfin.uclinux.org
 *
 * (C) Copyright 2000-2004
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
#include <command.h>
#include <malloc.h>
#include <devices.h>
#include <version.h>
#include <net.h>
#include <environment.h>
#include "blackfin_board.h"
#include "../drivers/smc91111.h"

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[];


static void mem_malloc_init(void)
{
	mem_malloc_start = CFG_MALLOC_BASE;
	mem_malloc_end = (CFG_MALLOC_BASE + CFG_MALLOC_LEN);
	mem_malloc_brk = mem_malloc_start;
	memset((void *) mem_malloc_start, 0,
	mem_malloc_end - mem_malloc_start);
}

void *sbrk(ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (NULL);
	}
	mem_malloc_brk = new;

	return ((void *) old);
}

static int display_banner(void)
{
	sprintf(version_string, VERSION_STRING_FORMAT, VERSION_STRING);
	printf("%s\n", version_string);
	return (0);
}

static void display_flash_config(ulong size)
{
	puts("FLASH:  ");
	print_size(size, "\n");
	return;
}

static int init_baudrate(void)
{
	uchar tmp[64];
	int i = getenv_r("baudrate", tmp, sizeof(tmp));
	gd->bd->bi_baudrate = gd->baudrate = (i > 0)
		? (int) simple_strtoul(tmp, NULL, 10)
		: CONFIG_BAUDRATE;
	return (0);
}

#ifdef DEBUG
static void display_global_data(void)
{
	bd_t *bd;
	bd = gd->bd;
	printf("--flags:%x\n", gd->flags);
	printf("--board_type:%x\n", gd->board_type);
	printf("--baudrate:%x\n", gd->baudrate);
	printf("--have_console:%x\n", gd->have_console);
	printf("--ram_size:%x\n", gd->ram_size);
	printf("--reloc_off:%x\n", gd->reloc_off);
	printf("--env_addr:%x\n", gd->env_addr);
	printf("--env_valid:%x\n", gd->env_valid);
	printf("--bd:%x %x\n", gd->bd, bd);
	printf("---bi_baudrate:%x\n", bd->bi_baudrate);
	printf("---bi_ip_addr:%x\n", bd->bi_ip_addr);
	printf("---bi_enetaddr:%x %x %x %x %x %x\n",
				bd->bi_enetaddr[0],
				bd->bi_enetaddr[1],
				bd->bi_enetaddr[2],
				bd->bi_enetaddr[3],
				bd->bi_enetaddr[4],
				bd->bi_enetaddr[5]);
	printf("---bi_arch_number:%x\n", bd->bi_arch_number);
	printf("---bi_boot_params:%x\n", bd->bi_boot_params);
	printf("---bi_memstart:%x\n", bd->bi_memstart);
	printf("---bi_memsize:%x\n", bd->bi_memsize);
	printf("---bi_flashstart:%x\n", bd->bi_flashstart);
	printf("---bi_flashsize:%x\n", bd->bi_flashsize);
	printf("---bi_flashoffset:%x\n", bd->bi_flashoffset);
	printf("--jt:%x *:%x\n", gd->jt, *(gd->jt));
}
#endif

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

void board_init_f(ulong bootflag)
{
	ulong addr;
	bd_t *bd;

	gd = (gd_t *) (CFG_GBL_DATA_ADDR);
	memset((void *) gd, 0, sizeof(gd_t));

	/* Board data initialization */
	addr = (CFG_GBL_DATA_ADDR + sizeof(gd_t));

	/* Align to 4 byte boundary */
	addr &= ~(4 - 1);
	bd = (bd_t*)addr;
	gd->bd = bd;
	memset((void *) bd, 0, sizeof(bd_t));

	/* Initialize */
	init_IRQ();
	env_init();		/* initialize environment */
	init_baudrate();	/* initialze baudrate settings */
	serial_init();		/* serial communications setup */
	console_init_f();
	display_banner();	/* say that we are here */
	checkboard();
#if defined(CONFIG_RTC_BF533) && (CONFIG_COMMANDS & CFG_CMD_DATE)
	rtc_init();
#endif
	timer_init();
	printf("Clock: VCO: %lu MHz, Core: %lu MHz, System: %lu MHz\n", \
	CONFIG_VCO_HZ/1000000, CONFIG_CCLK_HZ/1000000, CONFIG_SCLK_HZ/1000000);
	printf("SDRAM: ");
	print_size(initdram(0), "\n");
	board_init_r((gd_t *) gd, 0x20000010);
}

void board_init_r(gd_t * id, ulong dest_addr)
{
	ulong size;
	extern void malloc_bin_reloc(void);
	char *s, *e;
	bd_t *bd;
	int i;
	gd = id;
	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */
	bd = gd->bd;

#if	CONFIG_STAMP
	/* There are some other pointer constants we must deal with */
	/* configure available FLASH banks */
	size = flash_init();
	display_flash_config(size);
	flash_protect(FLAG_PROTECT_SET, CFG_FLASH_BASE, CFG_FLASH_BASE + 0x1ffff, &flash_info[0]);
	bd->bi_flashstart = CFG_FLASH_BASE;
	bd->bi_flashsize = size;
	bd->bi_flashoffset = 0;
#else
	bd->bi_flashstart = 0;
	bd->bi_flashsize = 0;
	bd->bi_flashoffset = 0;
#endif
	/* initialize malloc() area */
	mem_malloc_init();
	malloc_bin_reloc();

	/* relocate environment function pointers etc. */
	env_relocate();

	/* board MAC address */
	s = getenv("ethaddr");
	for (i = 0; i < 6; ++i) {
		bd->bi_enetaddr[i] = s ? simple_strtoul(s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}

	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr("ipaddr");

	/* Initialize devices */
	devices_init();
	jumptable_init();

	/* Initialize the console (after the relocation and devices init) */
	console_init_r();

	/* Initialize from environment */
	if ((s = getenv("loadaddr")) != NULL) {
		load_addr = simple_strtoul(s, NULL, 16);
	}
#if (CONFIG_COMMANDS & CFG_CMD_NET)
	if ((s = getenv("bootfile")) != NULL) {
		copy_filename(BootFile, s, sizeof(BootFile));
	}
#endif
#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif

#ifdef CONFIG_DRIVER_SMC91111
#ifdef SHARED_RESOURCES
	/* Switch to Ethernet */
	swap_to(ETHERNET);
#endif
	if  ( (SMC_inw(BANK_SELECT) & UPPER_BYTE_MASK) != SMC_IDENT ) {
		printf("ERROR: Can't find SMC91111 at address %x\n", SMC_BASE_ADDRESS);
	} else {
		printf("Net:   SMC91111 at 0x%08X\n", SMC_BASE_ADDRESS);
	}

#ifdef SHARED_RESOURCES
	swap_to(FLASH);
#endif
#endif
#ifdef CONFIG_SOFT_I2C
	init_func_i2c();
#endif

#ifdef DEBUG
	display_global_data(void);
#endif

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		main_loop();
	}
}

#ifdef CONFIG_SOFT_I2C
static int init_func_i2c (void)
{
	puts ("I2C:   ");
	i2c_init (CFG_I2C_SPEED, CFG_I2C_SLAVE);
	puts ("ready\n");
	return (0);
}
#endif

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;);
}
