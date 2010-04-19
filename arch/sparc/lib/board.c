/* SPARC Board initialization
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
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
#include <stdio_dev.h>
#include <config.h>
#if defined(CONFIG_CMD_IDE)
#include <ide.h>
#endif
#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif
#include <net.h>
#include <serial.h>
#include <version.h>
#if defined(CONFIG_POST)
#include <post.h>
#endif
#ifdef CONFIG_PS2KBD
#include <keyboard.h>
#endif
#ifdef CONFIG_CMD_AMBAPP
#include <ambapp.h>
#endif

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* Debug options
#define DEBUG_INIT_SEQUENCE
#define DEBUG_MEM_LAYOUT
#define DEBUG_COMMANDS
*/

extern void timer_interrupt_init(void);
extern void malloc_bin_reloc(void);
extern int do_ambapp_print(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);
extern int prom_init(void);

#if defined(CONFIG__CMD_DOC)
void doc_init(void);
#endif

#if !defined(CONFIG_SYS_NO_FLASH)
static char *failed = "*** failed ***\n";
#endif

#include <environment.h>

ulong monitor_flash_len;

/************************************************************************
 * Init Utilities							*
 ************************************************************************
 * Some of this code should be moved into the core functions,
 * but let's get it working (again) first...
 */

static int init_baudrate(void)
{
	char tmp[64];		/* long enough for environment variables */
	int i = getenv_r("baudrate", tmp, sizeof(tmp));

	gd->baudrate = (i > 0)
	    ? (int)simple_strtoul(tmp, NULL, 10)
	    : CONFIG_BAUDRATE;
	return (0);
}

/***********************************************************************/

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

#define WATCHDOG_RESET(x)

/************************************************************************
 * Initialization sequence						*
 ************************************************************************
 */

init_fnc_t *init_sequence[] = {

#if defined(CONFIG_BOARD_EARLY_INIT_F)
	board_early_init_f,
#endif
	serial_init,

	init_timebase,

#if defined(CONFIG_CMD_AMBAPP)
	ambapp_init_reloc,
#endif

	env_init,

	init_baudrate,

	console_init_f,
	display_options,

	checkcpu,
	checkboard,
#if defined(CONFIG_MISC_INIT_F)
	misc_init_f,
#endif

#ifdef CONFIG_POST
	post_init_f,
#endif

	NULL,			/* Terminate this list,
				 * beware: this list will be relocated
				 * which means that NULL will become
				 * NULL+RELOC_OFFSET. We simply make
				 * NULL be -RELOC_OFFSET instead.
				 */
};

/************************************************************************
 *
 * This is the SPARC board initialization routine, running from RAM.
 *
 ************************************************************************
 */
#ifdef DEBUG_INIT_SEQUENCE
char *str_init_seq = "INIT_SEQ 00\n";
char *str_init_seq_done = "\n\rInit sequence done...\r\n\r\n";
#endif

void board_init_f(ulong bootflag)
{
	cmd_tbl_t *cmdtp;
	bd_t *bd;
	unsigned char *s;
	init_fnc_t **init_fnc_ptr;
	int j;
	int i;
	char *e;

#ifndef CONFIG_SYS_NO_FLASH
	ulong flash_size;
#endif

	gd = (gd_t *) (CONFIG_SYS_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset((void *)gd, 0, sizeof(gd_t));

	gd->bd = (bd_t *) (gd + 1);	/* At end of global data */
	gd->baudrate = CONFIG_BAUDRATE;
	gd->cpu_clk = CONFIG_SYS_CLK_FREQ;

	bd = gd->bd;
	bd->bi_memstart = CONFIG_SYS_RAM_BASE;
	bd->bi_memsize = CONFIG_SYS_RAM_SIZE;
	bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
#if	defined(CONFIG_SYS_SRAM_BASE) && defined(CONFIG_SYS_SRAM_SIZE)
	bd->bi_sramstart = CONFIG_SYS_SRAM_BASE;
	bd->bi_sramsize = CONFIG_SYS_SRAM_SIZE;
#endif
	bd->bi_baudrate = CONFIG_BAUDRATE;
	bd->bi_bootflags = bootflag;	/* boot / reboot flag (for LynxOS)    */

	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */
	gd->reloc_off = CONFIG_SYS_RELOC_MONITOR_BASE - CONFIG_SYS_MONITOR_BASE;

	for (init_fnc_ptr = init_sequence, j = 0; *init_fnc_ptr;
	     ++init_fnc_ptr, j++) {
#ifdef DEBUG_INIT_SEQUENCE
		if (j > 9)
			str_init_seq[9] = '0' + (j / 10);
		str_init_seq[10] = '0' + (j - (j / 10) * 10);
		serial_puts(str_init_seq);
#endif
		if ((*init_fnc_ptr + gd->reloc_off) () != 0) {
			hang();
		}
	}
#ifdef DEBUG_INIT_SEQUENCE
	serial_puts(str_init_seq_done);
#endif

	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM.
	 *
	 * Reserve memory at end of RAM for (top down in that order):
	 *  - kernel log buffer
	 *  - protected RAM
	 *  - LCD framebuffer
	 *  - monitor code
	 *  - board info struct
	 */
#ifdef DEBUG_MEM_LAYOUT
	printf("CONFIG_SYS_MONITOR_BASE:       0x%lx\n", CONFIG_SYS_MONITOR_BASE);
	printf("CONFIG_ENV_ADDR:           0x%lx\n", CONFIG_ENV_ADDR);
	printf("CONFIG_SYS_RELOC_MONITOR_BASE: 0x%lx (%d)\n", CONFIG_SYS_RELOC_MONITOR_BASE,
	       CONFIG_SYS_MONITOR_LEN);
	printf("CONFIG_SYS_MALLOC_BASE:        0x%lx (%d)\n", CONFIG_SYS_MALLOC_BASE,
	       CONFIG_SYS_MALLOC_LEN);
	printf("CONFIG_SYS_INIT_SP_OFFSET:     0x%lx (%d)\n", CONFIG_SYS_INIT_SP_OFFSET,
	       CONFIG_SYS_STACK_SIZE);
	printf("CONFIG_SYS_PROM_OFFSET:        0x%lx (%d)\n", CONFIG_SYS_PROM_OFFSET,
	       CONFIG_SYS_PROM_SIZE);
	printf("CONFIG_SYS_GBL_DATA_OFFSET:    0x%lx (%d)\n", CONFIG_SYS_GBL_DATA_OFFSET,
	       CONFIG_SYS_GBL_DATA_SIZE);
#endif

#ifdef CONFIG_POST
	post_bootmode_init();
	post_run(NULL, POST_ROM | post_bootmode_get(0));
#endif

	/*
	 * We have to relocate the command table manually
	 */
	for (cmdtp = &__u_boot_cmd_start; cmdtp != &__u_boot_cmd_end; cmdtp++) {
		ulong addr;
		addr = (ulong) (cmdtp->cmd) + gd->reloc_off;
#if DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong) (cmdtp->cmd), addr);
#endif
		cmdtp->cmd =
		    (int (*)(struct cmd_tbl_s *, int, int, char *[]))addr;

		addr = (ulong) (cmdtp->name) + gd->reloc_off;
		cmdtp->name = (char *)addr;

		if (cmdtp->usage) {
			addr = (ulong) (cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		if (cmdtp->help) {
			addr = (ulong) (cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif
	}

#if defined(CONFIG_CMD_AMBAPP) && defined(CONFIG_SYS_AMBAPP_PRINT_ON_STARTUP)
	puts("AMBA:\n");
	do_ambapp_print(NULL, 0, 0, NULL);
#endif

	/* initialize higher level parts of CPU like time base and timers */
	cpu_init_r();

	/* start timer */
	timer_interrupt_init();

	/*
	 * Enable Interrupts before any calls to udelay,
	 * the flash driver may use udelay resulting in
	 * a hang if not timer0 IRQ is enabled.
	 */
	interrupt_init();

	/* The Malloc area is immediately below the monitor copy in RAM */
	mem_malloc_init(CONFIG_SYS_MALLOC_BASE,
			CONFIG_SYS_MALLOC_END - CONFIG_SYS_MALLOC_BASE);
	malloc_bin_reloc();

#if !defined(CONFIG_SYS_NO_FLASH)
	puts("FLASH: ");

	if ((flash_size = flash_init()) > 0) {
# ifdef CONFIG_SYS_FLASH_CHECKSUM
		print_size(flash_size, "");
		/*
		 * Compute and print flash CRC if flashchecksum is set to 'y'
		 *
		 * NOTE: Maybe we should add some WATCHDOG_RESET()? XXX
		 */
		s = getenv("flashchecksum");
		if (s && (*s == 'y')) {
			printf("  CRC: %08lX",
			       crc32(0, (const unsigned char *)CONFIG_SYS_FLASH_BASE,
				     flash_size)
			    );
		}
		putc('\n');
# else				/* !CONFIG_SYS_FLASH_CHECKSUM */
		print_size(flash_size, "\n");
# endif				/* CONFIG_SYS_FLASH_CHECKSUM */
	} else {
		puts(failed);
		hang();
	}

	bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;	/* update start of FLASH memory    */
	bd->bi_flashsize = flash_size;	/* size of FLASH memory (final value) */
#if CONFIG_SYS_MONITOR_BASE == CONFIG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for startup monitor  */
#else
	bd->bi_flashoffset = 0;
#endif
#else				/* CONFIG_SYS_NO_FLASH */
	bd->bi_flashsize = 0;
	bd->bi_flashstart = 0;
	bd->bi_flashoffset = 0;
#endif				/* !CONFIG_SYS_NO_FLASH */

#ifdef CONFIG_SPI
# if !defined(CONFIG_ENV_IS_IN_EEPROM)
	spi_init_f();
# endif
	spi_init_r();
#endif

	/* relocate environment function pointers etc. */
	env_relocate();

#if defined(CONFIG_BOARD_LATE_INIT)
	board_late_init();
#endif

#ifdef CONFIG_ID_EEPROM
	mac_read_from_eeprom();
#endif

	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr("ipaddr");
#if defined(CONFIG_PCI)
	/*
	 * Do pci configuration
	 */
	pci_init();
#endif

	/* Initialize stdio devices */
	stdio_init();

	/* Initialize the jump table for applications */
	jumptable_init();

	/* Initialize the console (after the relocation and devices init) */
	console_init_r();

#ifdef CONFIG_SERIAL_SOFTWARE_FIFO
	serial_buffered_init();
#endif

#ifdef CONFIG_STATUS_LED
	status_led_set(STATUS_LED_BOOT, STATUS_LED_BLINKING);
#endif

	udelay(20);

	set_timer(0);

	/* Initialize from environment */
	if ((s = getenv("loadaddr")) != NULL) {
		load_addr = simple_strtoul(s, NULL, 16);
	}
#if defined(CONFIG_CMD_NET)
	if ((s = getenv("bootfile")) != NULL) {
		copy_filename(BootFile, s, sizeof(BootFile));
	}
#endif /* CONFIG_CMD_NET */

	WATCHDOG_RESET();

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
	eth_initialize(bd);
#endif

#if defined(CONFIG_CMD_NET) && defined(CONFIG_RESET_PHY_R)
	WATCHDOG_RESET();
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif

#ifdef CONFIG_POST
	post_run(NULL, POST_RAM | post_bootmode_get(0));
#endif

#if defined(CONFIG_CMD_IDE)
	WATCHDOG_RESET();
	puts("IDE:   ");
	ide_init();
#endif /* CONFIG_CMD_IDE */

#ifdef CONFIG_LAST_STAGE_INIT
	WATCHDOG_RESET();
	/*
	 * Some parts can be only initialized if all others (like
	 * Interrupts) are up and running (i.e. the PC-style ISA
	 * keyboard).
	 */
	last_stage_init();
#endif

#ifdef CONFIG_PS2KBD
	puts("PS/2:  ");
	kbd_init();
#endif
	prom_init();

	/* main_loop */
	for (;;) {
		WATCHDOG_RESET();
		main_loop();
	}

}

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
#ifdef CONFIG_SHOW_BOOT_PROGRESS
	show_boot_progress(-30);
#endif
	for (;;) ;
}

/************************************************************************/
