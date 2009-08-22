/*
 * (C) Copyright 2007 Michal Simek
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Michal  SIMEK <monstr@monstr.eu>
 * Yasushi SHOJI <yashi@atmark-techno.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
#include <timestamp.h>
#include <version.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

const char version_string[] = U_BOOT_VERSION " ("U_BOOT_DATE" - "U_BOOT_TIME")";

#ifdef CONFIG_SYS_GPIO_0
extern int gpio_init (void);
#endif
#ifdef CONFIG_SYS_INTC_0
extern int interrupts_init (void);
#endif
#if defined(CONFIG_CMD_NET)
extern int eth_init (bd_t * bis);
extern int getenv_IPaddr (char *);
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
typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
	env_init,
	serial_init,
#ifdef CONFIG_SYS_GPIO_0
	gpio_init,
#endif
#ifdef CONFIG_SYS_INTC_0
	interrupts_init,
#endif
	NULL,
};

void board_init (void)
{
	bd_t *bd;
	init_fnc_t **init_fnc_ptr;
	gd = (gd_t *) CONFIG_SYS_GBL_DATA_OFFSET;
#if defined(CONFIG_CMD_FLASH)
	ulong flash_size = 0;
#endif
	asm ("nop");	/* FIXME gd is not initialize - wait */
	memset ((void *)gd, 0, CONFIG_SYS_GBL_DATA_SIZE);
	gd->bd = (bd_t *) (gd + 1);	/* At end of global data */
	gd->baudrate = CONFIG_BAUDRATE;
	bd = gd->bd;
	bd->bi_baudrate = CONFIG_BAUDRATE;
	bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;
	gd->flags |= GD_FLG_RELOC;      /* tell others: relocation done */

	/*
	 * The Malloc area is immediately below the monitor copy in DRAM
	 * aka CONFIG_SYS_MONITOR_BASE - Note there is no need for reloc_off
	 * as our monitory code is run from SDRAM
	 */
	mem_malloc_init (CONFIG_SYS_MALLOC_BASE, CONFIG_SYS_MALLOC_LEN);

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		WATCHDOG_RESET ();
		if ((*init_fnc_ptr) () != 0) {
			hang ();
		}
	}

	puts ("SDRAM :\n");
	printf ("\t\tIcache:%s\n", icache_status() ? "OK" : "FAIL");
	printf ("\t\tDcache:%s\n", dcache_status() ? "OK" : "FAIL");
	printf ("\tU-Boot Start:0x%08x\n", TEXT_BASE);

#if defined(CONFIG_CMD_FLASH)
	puts ("FLASH: ");
	bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
	if (0 < (flash_size = flash_init ())) {
		bd->bi_flashsize = flash_size;
		bd->bi_flashoffset = CONFIG_SYS_FLASH_BASE + flash_size;
# ifdef CONFIG_SYS_FLASH_CHECKSUM
		print_size (flash_size, "");
		/*
		 * Compute and print flash CRC if flashchecksum is set to 'y'
		 *
		 * NOTE: Maybe we should add some WATCHDOG_RESET()? XXX
		 */
		s = getenv ("flashchecksum");
		if (s && (*s == 'y')) {
			printf ("  CRC: %08X",
				crc32 (0, (const unsigned char *) CONFIG_SYS_FLASH_BASE, flash_size)
			);
		}
		putc ('\n');
# else	/* !CONFIG_SYS_FLASH_CHECKSUM */
		print_size (flash_size, "\n");
# endif /* CONFIG_SYS_FLASH_CHECKSUM */
	} else {
		puts ("Flash init FAILED");
		bd->bi_flashstart = 0;
		bd->bi_flashsize = 0;
		bd->bi_flashoffset = 0;
	}
#endif

#if defined(CONFIG_CMD_NET)
	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr ("ipaddr");
	eth_init (bd);
#endif

	/* relocate environment function pointers etc. */
	env_relocate ();

	/* main_loop */
	for (;;) {
		WATCHDOG_RESET ();
		main_loop ();
	}
}

void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;) ;
}
