/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
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
#include <stdio_dev.h>
#include <version.h>
#include <malloc.h>
#include <net.h>
#include <ide.h>
#include <serial.h>
#include <status_led.h>
#include <asm/processor.h>
#include <asm/u-boot-x86.h>

#include <asm/init_helpers.h>

DECLARE_GLOBAL_DATA_PTR;

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * or dropped completely,
 * but let's get it working (again) first...
 */

int display_banner(void)
{
	printf("\n\n%s\n\n", version_string);

	return 0;
}

int display_dram_config(void)
{
	int i;

	puts("DRAM Configuration:\n");

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		printf("Bank #%d: %08lx ", i, gd->bd->bi_dram[i].start);
		print_size(gd->bd->bi_dram[i].size, "\n");
	}

	return 0;
}

int init_baudrate_f(void)
{
	gd->baudrate = getenv_ulong("baudrate", 10, CONFIG_BAUDRATE);
	return 0;
}

int calculate_relocation_address(void)
{
	ulong text_start = (ulong)&__text_start;
	ulong bss_end = (ulong)&__bss_end;
	ulong dest_addr;

	/*
	 * NOTE: All destination address are rounded down to 16-byte
	 *       boundary to satisfy various worst-case alignment
	 *       requirements
	 */

	/* Stack is at top of available memory */
	dest_addr = gd->ram_size;
	gd->start_addr_sp = dest_addr;

	/* U-Boot is below the stack */
	dest_addr -= CONFIG_SYS_STACK_SIZE;
	dest_addr -= (bss_end - text_start);
	dest_addr &= ~15;
	gd->relocaddr = dest_addr;
	gd->reloc_off = (dest_addr - text_start);

	return 0;
}

int init_cache_f_r(void)
{
	/* Initialise the CPU cache(s) */
	return init_cache();
}

int set_reloc_flag_r(void)
{
	gd->flags = GD_FLG_RELOC;

	return 0;
}

int mem_malloc_init_r(void)
{
	mem_malloc_init(((gd->relocaddr - CONFIG_SYS_MALLOC_LEN)+3)&~3,
			CONFIG_SYS_MALLOC_LEN);

	return 0;
}

bd_t bd_data;

int init_bd_struct_r(void)
{
	gd->bd = &bd_data;
	memset(gd->bd, 0, sizeof(bd_t));

	return 0;
}

#ifndef CONFIG_SYS_NO_FLASH
int flash_init_r(void)
{
	ulong size;

	puts("Flash: ");

	/* configure available FLASH banks */
	size = flash_init();

	print_size(size, "\n");

	return 0;
}
#endif

#ifdef CONFIG_STATUS_LED
int status_led_set_r(void)
{
	status_led_set(STATUS_LED_BOOT, STATUS_LED_BLINKING);

	return 0;
}
#endif

int set_load_addr_r(void)
{
	/* Initialize from environment */
	load_addr = getenv_ulong("loadaddr", 16, load_addr);

	return 0;
}
