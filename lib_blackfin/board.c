/*
 * U-boot - board.c First C file to be called contains init routines
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <devices.h>
#include <version.h>
#include <net.h>
#include <environment.h>
#include <i2c.h>
#include "blackfin_board.h"
#include <asm/cplb.h>
#include "../drivers/net/smc91111.h"

#if defined(CONFIG_BF537)&&defined(CONFIG_POST)
#include <post.h>
int post_flag;
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifndef CFG_NO_FLASH
extern flash_info_t flash_info[];
#endif

static inline u_long get_vco(void)
{
	u_long msel;
	u_long vco;

	msel = (*pPLL_CTL >> 9) & 0x3F;
	if (0 == msel)
		msel = 64;

	vco = CONFIG_CLKIN_HZ;
	vco >>= (1 & *pPLL_CTL);	/* DF bit */
	vco = msel * vco;
	return vco;
}

/*Get the Core clock*/
u_long get_cclk(void)
{
	u_long csel, ssel;
	if (*pPLL_STAT & 0x1)
		return CONFIG_CLKIN_HZ;

	ssel = *pPLL_DIV;
	csel = ((ssel >> 4) & 0x03);
	ssel &= 0xf;
	if (ssel && ssel < (1 << csel))	/* SCLK > CCLK */
		return get_vco() / ssel;
	return get_vco() >> csel;
}

/* Get the System clock */
u_long get_sclk(void)
{
	u_long ssel;

	if (*pPLL_STAT & 0x1)
		return CONFIG_CLKIN_HZ;

	ssel = (*pPLL_DIV & 0xf);

	return get_vco() / ssel;
}

static void mem_malloc_init(void)
{
	mem_malloc_start = CFG_MALLOC_BASE;
	mem_malloc_end = (CFG_MALLOC_BASE + CFG_MALLOC_LEN);
	mem_malloc_brk = mem_malloc_start;
	memset((void *)mem_malloc_start, 0, mem_malloc_end - mem_malloc_start);
}

void *sbrk(ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (NULL);
	}
	mem_malloc_brk = new;

	return ((void *)old);
}

static int display_banner(void)
{
	sprintf(version_string, VERSION_STRING_FORMAT, VERSION_STRING);
	printf("%s\n", version_string);
	printf("CPU:   ADSP " MK_STR(CONFIG_BFIN_CPU) " (Detected Rev: 0.%d)\n", bfin_revid());
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
	char tmp[64];
	int i = getenv_r("baudrate", tmp, sizeof(tmp));
	gd->bd->bi_baudrate = gd->baudrate = (i > 0)
	    ? (int)simple_strtoul(tmp, NULL, 10)
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
	       bd->bi_enetaddr[3], bd->bi_enetaddr[4], bd->bi_enetaddr[5]);
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

/* we cover everything with 4 meg pages, and need an extra for L1 */
unsigned int icplb_table[page_descriptor_table_size][2];
unsigned int dcplb_table[page_descriptor_table_size][2];

void init_cplbtables(void)
{
	int i, j;

	j = 0;
	icplb_table[j][0] = 0xFFA00000;
	icplb_table[j][1] = L1_IMEMORY;
	j++;

	for (i = 0; i < CONFIG_MEM_SIZE / 4; i++) {
		icplb_table[j][0] = (i * 4 * 1024 * 1024);
		if (i * 4 * 1024 * 1024 <= CFG_MONITOR_BASE
		    && (i + 1) * 4 * 1024 * 1024 >= CFG_MONITOR_BASE) {
			icplb_table[j][1] = SDRAM_IKERNEL;
		} else {
			icplb_table[j][1] = SDRAM_IGENERIC;
		}
		j++;
	}
#if defined(CONFIG_BF561)
	/* MAC space */
	icplb_table[j][0] = 0x2C000000;
	icplb_table[j][1] = SDRAM_INON_CHBL;
	j++;
	/* Async Memory space */
	for (i = 0; i < 3; i++) {
		icplb_table[j][0] = 0x20000000 + i * 4 * 1024 * 1024;
		icplb_table[j][1] = SDRAM_INON_CHBL;
		j++;
	}
#else
	icplb_table[j][0] = 0x20000000;
	icplb_table[j][1] = SDRAM_INON_CHBL;
#endif
	j = 0;
	dcplb_table[j][0] = 0xFF800000;
	dcplb_table[j][1] = L1_DMEMORY;
	j++;

	for (i = 0; i < CONFIG_MEM_SIZE / 4; i++) {
		dcplb_table[j][0] = (i * 4 * 1024 * 1024);
		if (i * 4 * 1024 * 1024 <= CFG_MONITOR_BASE
		    && (i + 1) * 4 * 1024 * 1024 >= CFG_MONITOR_BASE) {
			dcplb_table[j][1] = SDRAM_DKERNEL;
		} else {
			dcplb_table[j][1] = SDRAM_DGENERIC;
		}
		j++;
	}

#if defined(CONFIG_BF561)
	/* MAC space */
	dcplb_table[j][0] = 0x2C000000;
	dcplb_table[j][1] = SDRAM_EBIU;
	j++;

	/* Flash space */
	for (i = 0; i < 3; i++) {
		dcplb_table[j][0] = 0x20000000 + i * 4 * 1024 * 1024;
		dcplb_table[j][1] = SDRAM_EBIU;
		j++;
	}
#else
	dcplb_table[j][0] = 0x20000000;
	dcplb_table[j][1] = SDRAM_EBIU;
#endif
}

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
	int i;

	init_cplbtables();

	gd = (gd_t *) (CFG_GBL_DATA_ADDR);
	memset((void *)gd, 0, sizeof(gd_t));

	/* Board data initialization */
	addr = (CFG_GBL_DATA_ADDR + sizeof(gd_t));

	/* Align to 4 byte boundary */
	addr &= ~(4 - 1);
	bd = (bd_t *) addr;
	gd->bd = bd;
	memset((void *)bd, 0, sizeof(bd_t));

	/* Initialize */
	init_IRQ();
	env_init();		/* initialize environment */
	init_baudrate();	/* initialze baudrate settings */
	serial_init();		/* serial communications setup */
	console_init_f();
#ifdef CONFIG_ICACHE_ON
	icache_enable();
#endif
#ifdef CONFIG_DCACHE_ON
	dcache_enable();
#endif
	display_banner();	/* say that we are here */

	for (i = 0; i < page_descriptor_table_size; i++) {
		debug
		    ("data (%02i)= 0x%08x : 0x%08x    intr = 0x%08x : 0x%08x\n",
		     i, dcplb_table[i][0], dcplb_table[i][1], icplb_table[i][0],
		     icplb_table[i][1]);
	}

	checkboard();
#if defined(CONFIG_RTC_BF533) && defined(CONFIG_CMD_DATE)
	rtc_init();
#endif
	timer_init();
	printf("Clock: VCO: %lu MHz, Core: %lu MHz, System: %lu MHz\n",
	       get_vco() / 1000000, get_cclk() / 1000000, get_sclk() / 1000000);
	printf("SDRAM: ");
	print_size(initdram(0), "\n");
#if defined(CONFIG_BF537)&&defined(CONFIG_POST)
	post_init_f();
	post_bootmode_init();
	post_run(NULL, POST_ROM | post_bootmode_get(0));
#endif
	board_init_r((gd_t *) gd, 0x20000010);
}

#if defined(CONFIG_SOFT_I2C) || defined(CONFIG_HARD_I2C)
static int init_func_i2c(void)
{
	puts("I2C:   ");
	i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);
	puts("ready\n");
	return (0);
}
#endif

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

#if    defined(CONFIG_BF537) && defined(CONFIG_POST)
	post_output_backlog();
	post_reloc();
#endif

#if	(CONFIG_STAMP || CONFIG_BF537 || CONFIG_EZKIT561) && !defined(CFG_NO_FLASH)
	/* There are some other pointer constants we must deal with */
	/* configure available FLASH banks */
	size = flash_init();
	display_flash_config(size);
	flash_protect(FLAG_PROTECT_SET, CFG_FLASH_BASE,
		      CFG_FLASH_BASE + 0x1ffff, &flash_info[0]);
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

#ifdef CONFIG_SPI
# if ! defined(CFG_ENV_IS_IN_EEPROM)
	spi_init_f();
# endif
	spi_init_r();
#endif

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
#if defined(CONFIG_CMD_NET)
	if ((s = getenv("bootfile")) != NULL) {
		copy_filename(BootFile, s, sizeof(BootFile));
	}
#endif

#if defined(CONFIG_CMD_NAND)
	puts("NAND:  ");
	nand_init();		/* go init the NAND */
#endif

#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif

#ifdef CONFIG_CMD_NET
	printf("Net:    ");
	eth_initialize(bd);
#endif

#ifdef CONFIG_DRIVER_SMC91111
#ifdef SHARED_RESOURCES
	/* Switch to Ethernet */
	swap_to(ETHERNET);
#endif
	if ((SMC_inw(BANK_SELECT) & UPPER_BYTE_MASK) != SMC_IDENT) {
		printf("ERROR: Can't find SMC91111 at address %x\n",
		       SMC_BASE_ADDRESS);
	} else {
		printf("Net:   SMC91111 at 0x%08X\n", SMC_BASE_ADDRESS);
	}

#ifdef SHARED_RESOURCES
	swap_to(FLASH);
#endif
#endif
#if defined(CONFIG_SOFT_I2C) || defined(CONFIG_HARD_I2C)
	init_func_i2c();
#endif

#ifdef DEBUG
	display_global_data();
#endif

#if defined(CONFIG_BF537) && defined(CONFIG_POST)
	if (post_flag)
		post_run(NULL, POST_RAM | post_bootmode_get(0));
#endif

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;) {
		main_loop();
	}
}

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	for (;;) ;
}
