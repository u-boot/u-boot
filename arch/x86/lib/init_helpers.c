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

int copy_gd_to_ram_f_r(void)
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

int init_ip_address_r(void)
{
	/* IP Address */
	bd_data.bi_ip_addr = getenv_IPaddr("ipaddr");

	return 0;
}

#ifdef CONFIG_STATUS_LED
int status_led_set_r(void)
{
	status_led_set(STATUS_LED_BOOT, STATUS_LED_BLINKING);

	return 0;
}
#endif

int set_bootfile_r(void)
{
	char *s;

	s = getenv("bootfile");

	if (s != NULL)
		copy_filename(BootFile, s, sizeof(BootFile));

	return 0;
}

int set_load_addr_r(void)
{
	/* Initialize from environment */
	load_addr = getenv_ulong("loadaddr", 16, load_addr);

	return 0;
}
