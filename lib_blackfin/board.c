/*
 * U-boot - board.c First C file to be called contains init routines
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <stdio_dev.h>
#include <environment.h>
#include <malloc.h>
#include <mmc.h>
#include <net.h>
#include <timestamp.h>
#include <status_led.h>
#include <version.h>

#include <asm/cplb.h>
#include <asm/mach-common/bits/mpu.h>
#include <kgdb.h>

#ifdef CONFIG_CMD_NAND
#include <nand.h>	/* cannot even include nand.h if it isnt configured */
#endif

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

#if defined(CONFIG_POST)
#include <post.h>
int post_flag;
#endif

DECLARE_GLOBAL_DATA_PTR;

const char version_string[] = U_BOOT_VERSION " ("U_BOOT_DATE" - "U_BOOT_TIME")";

__attribute__((always_inline))
static inline void serial_early_puts(const char *s)
{
#ifdef CONFIG_DEBUG_EARLY_SERIAL
	serial_puts("Early: ");
	serial_puts(s);
#endif
}

static int display_banner(void)
{
	printf("\n\n%s\n\n", version_string);
	printf("CPU:   ADSP " MK_STR(CONFIG_BFIN_CPU) " "
		"(Detected Rev: 0.%d) "
		"(%s boot)\n",
		bfin_revid(),
		get_bfin_boot_mode(CONFIG_BFIN_BOOT_MODE));
	return 0;
}

static int init_baudrate(void)
{
	char baudrate[15];
	int i = getenv_r("baudrate", baudrate, sizeof(baudrate));
	gd->bd->bi_baudrate = gd->baudrate = (i > 0)
	    ? simple_strtoul(baudrate, NULL, 10)
	    : CONFIG_BAUDRATE;
	return 0;
}

static void display_global_data(void)
{
#ifdef CONFIG_DEBUG_EARLY_SERIAL
	bd_t *bd;
	bd = gd->bd;
	printf(" gd: %p\n", gd);
	printf(" |-flags: %lx\n", gd->flags);
	printf(" |-board_type: %lx\n", gd->board_type);
	printf(" |-baudrate: %lu\n", gd->baudrate);
	printf(" |-have_console: %lx\n", gd->have_console);
	printf(" |-ram_size: %lx\n", gd->ram_size);
	printf(" |-reloc_off: %lx\n", gd->reloc_off);
	printf(" |-env_addr: %lx\n", gd->env_addr);
	printf(" |-env_valid: %lx\n", gd->env_valid);
	printf(" |-jt(%p): %p\n", gd->jt, *(gd->jt));
	printf(" \\-bd: %p\n", gd->bd);
	printf("   |-bi_baudrate: %x\n", bd->bi_baudrate);
	printf("   |-bi_ip_addr: %lx\n", bd->bi_ip_addr);
	printf("   |-bi_boot_params: %lx\n", bd->bi_boot_params);
	printf("   |-bi_memstart: %lx\n", bd->bi_memstart);
	printf("   |-bi_memsize: %lx\n", bd->bi_memsize);
	printf("   |-bi_flashstart: %lx\n", bd->bi_flashstart);
	printf("   |-bi_flashsize: %lx\n", bd->bi_flashsize);
	printf("   \\-bi_flashoffset: %lx\n", bd->bi_flashoffset);
#endif
}

#define CPLB_PAGE_SIZE (4 * 1024 * 1024)
#define CPLB_PAGE_MASK (~(CPLB_PAGE_SIZE - 1))
void init_cplbtables(void)
{
	volatile uint32_t *ICPLB_ADDR, *ICPLB_DATA;
	volatile uint32_t *DCPLB_ADDR, *DCPLB_DATA;
	uint32_t extern_memory;
	size_t i;

	void icplb_add(uint32_t addr, uint32_t data)
	{
		*(ICPLB_ADDR + i) = addr;
		*(ICPLB_DATA + i) = data;
	}
	void dcplb_add(uint32_t addr, uint32_t data)
	{
		*(DCPLB_ADDR + i) = addr;
		*(DCPLB_DATA + i) = data;
	}

	/* populate a few common entries ... we'll let
	 * the memory map and cplb exception handler do
	 * the rest of the work.
	 */
	i = 0;
	ICPLB_ADDR = (uint32_t *)ICPLB_ADDR0;
	ICPLB_DATA = (uint32_t *)ICPLB_DATA0;
	DCPLB_ADDR = (uint32_t *)DCPLB_ADDR0;
	DCPLB_DATA = (uint32_t *)DCPLB_DATA0;

	icplb_add(0xFFA00000, L1_IMEMORY);
	dcplb_add(0xFF800000, L1_DMEMORY);
	++i;

	if (CONFIG_MEM_SIZE) {
		uint32_t mbase = CONFIG_SYS_MONITOR_BASE;
		uint32_t mend  = mbase + CONFIG_SYS_MONITOR_LEN;
		mbase &= CPLB_PAGE_MASK;
		mend &= CPLB_PAGE_MASK;

		icplb_add(mbase, SDRAM_IKERNEL);
		dcplb_add(mbase, SDRAM_DKERNEL);
		++i;

		/*
		 * If the monitor crosses a 4 meg boundary, we'll need
		 * to lock two entries for it.  We assume it doesn't
		 * cross two 4 meg boundaries ...
		 */
		if (mbase != mend) {
			icplb_add(mend, SDRAM_IKERNEL);
			dcplb_add(mend, SDRAM_DKERNEL);
			++i;
		}
	}

	icplb_add(0x20000000, SDRAM_INON_CHBL);
	dcplb_add(0x20000000, SDRAM_EBIU);
	++i;

	/* Add entries for the rest of external RAM up to the bootrom */
	extern_memory = 0;

#ifdef CONFIG_DEBUG_NULL_PTR
	icplb_add(extern_memory, (SDRAM_IKERNEL & ~PAGE_SIZE_MASK) | PAGE_SIZE_1KB);
	dcplb_add(extern_memory, (SDRAM_DKERNEL & ~PAGE_SIZE_MASK) | PAGE_SIZE_1KB);
	++i;
	icplb_add(extern_memory, SDRAM_IKERNEL);
	dcplb_add(extern_memory, SDRAM_DKERNEL);
	extern_memory += CPLB_PAGE_SIZE;
	++i;
#endif

	while (i < 16 && extern_memory < (CONFIG_SYS_MONITOR_BASE & CPLB_PAGE_MASK)) {
		icplb_add(extern_memory, SDRAM_IGENERIC);
		dcplb_add(extern_memory, SDRAM_DGENERIC);
		extern_memory += CPLB_PAGE_SIZE;
		++i;
	}
	while (i < 16) {
		icplb_add(0, 0);
		dcplb_add(0, 0);
		++i;
	}
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

extern int exception_init(void);
extern int irq_init(void);
extern int timer_init(void);

void board_init_f(ulong bootflag)
{
	ulong addr;
	bd_t *bd;
	char buf[32];

#ifdef CONFIG_BOARD_EARLY_INIT_F
	serial_early_puts("Board early init flash\n");
	board_early_init_f();
#endif

	serial_early_puts("Init CPLB tables\n");
	init_cplbtables();

	serial_early_puts("Exceptions setup\n");
	exception_init();

#ifndef CONFIG_ICACHE_OFF
	serial_early_puts("Turn on ICACHE\n");
	icache_enable();
#endif
#ifndef CONFIG_DCACHE_OFF
	serial_early_puts("Turn on DCACHE\n");
	dcache_enable();
#endif

#ifdef DEBUG
	if (CONFIG_SYS_GBL_DATA_SIZE < sizeof(*gd))
		hang();
#endif
	serial_early_puts("Init global data\n");
	gd = (gd_t *) (CONFIG_SYS_GBL_DATA_ADDR);
	memset((void *)gd, 0, CONFIG_SYS_GBL_DATA_SIZE);

	/* Board data initialization */
	addr = (CONFIG_SYS_GBL_DATA_ADDR + sizeof(gd_t));

	/* Align to 4 byte boundary */
	addr &= ~(4 - 1);
	bd = (bd_t *) addr;
	gd->bd = bd;
	memset((void *)bd, 0, sizeof(bd_t));

	bd->bi_r_version = version_string;
	bd->bi_cpu = MK_STR(CONFIG_BFIN_CPU);
	bd->bi_board_name = BFIN_BOARD_NAME;
	bd->bi_vco = get_vco();
	bd->bi_cclk = get_cclk();
	bd->bi_sclk = get_sclk();
	bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	bd->bi_memsize = CONFIG_SYS_MAX_RAM_SIZE;

	/* Initialize */
	serial_early_puts("IRQ init\n");
	irq_init();
	serial_early_puts("Environment init\n");
	env_init();
	serial_early_puts("Baudrate init\n");
	init_baudrate();
	serial_early_puts("Serial init\n");
	serial_init();
	serial_early_puts("Console init flash\n");
	console_init_f();
	serial_early_puts("End of early debugging\n");
	display_banner();

	checkboard();
	timer_init();

	printf("Clock: VCO: %s MHz, ", strmhz(buf, get_vco()));
	printf("Core: %s MHz, ", strmhz(buf, get_cclk()));
	printf("System: %s MHz\n", strmhz(buf, get_sclk()));

	printf("RAM:   ");
	print_size(bd->bi_memsize, "\n");
#if defined(CONFIG_POST)
	post_init_f();
	post_bootmode_init();
	post_run(NULL, POST_ROM | post_bootmode_get(0));
#endif

	board_init_r((gd_t *) gd, 0x20000010);
}

static void board_net_init_r(bd_t *bd)
{
#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif
#ifdef CONFIG_CMD_NET
	char *s;

	if ((s = getenv("bootfile")) != NULL)
		copy_filename(BootFile, s, sizeof(BootFile));

	bd->bi_ip_addr = getenv_IPaddr("ipaddr");

	printf("Net:   ");
	eth_initialize(gd->bd);
#endif
}

void board_init_r(gd_t * id, ulong dest_addr)
{
	char *s;
	bd_t *bd;
	gd = id;
	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */
	bd = gd->bd;

#if defined(CONFIG_POST)
	post_output_backlog();
	post_reloc();
#endif

	/* initialize malloc() area */
	mem_malloc_init(CONFIG_SYS_MALLOC_BASE, CONFIG_SYS_MALLOC_LEN);

#if	!defined(CONFIG_SYS_NO_FLASH)
	/* Initialize the flash and protect u-boot by default */
	extern flash_info_t flash_info[];
	puts("Flash: ");
	ulong size = flash_init();
	print_size(size, "\n");
	flash_protect(FLAG_PROTECT_SET, CONFIG_SYS_FLASH_BASE,
		CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN - 1,
		&flash_info[0]);
	bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
	bd->bi_flashsize = size;
	bd->bi_flashoffset = 0;
#else
	bd->bi_flashstart = 0;
	bd->bi_flashsize = 0;
	bd->bi_flashoffset = 0;
#endif

#ifdef CONFIG_CMD_NAND
	puts("NAND:  ");
	nand_init();		/* go init the NAND */
#endif

#ifdef CONFIG_GENERIC_MMC
	puts("MMC:  ");
	mmc_initialize(bd);
#endif

	/* relocate environment function pointers etc. */
	env_relocate();

	/* Initialize stdio devices */
	stdio_init();
	jumptable_init();

	/* Initialize the console (after the relocation and devices init) */
	console_init_r();

#ifdef CONFIG_CMD_KGDB
	puts("KGDB:  ");
	kgdb_init();
#endif

#ifdef CONFIG_STATUS_LED
	status_led_set(STATUS_LED_BOOT, STATUS_LED_BLINKING);
	status_led_set(STATUS_LED_CRASH, STATUS_LED_OFF);
#endif

	/* Initialize from environment */
	if ((s = getenv("loadaddr")) != NULL)
		load_addr = simple_strtoul(s, NULL, 16);

#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif

	board_net_init_r(bd);

	display_global_data();

#if defined(CONFIG_POST)
	if (post_flag)
		post_run(NULL, POST_RAM | post_bootmode_get(0));
#endif

	if (bfin_os_log_check()) {
		puts("\nLog buffer from operating system:\n");
		bfin_os_log_dump();
		puts("\n");
	}

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;)
		main_loop();
}

void hang(void)
{
#ifdef CONFIG_STATUS_LED
	status_led_set(STATUS_LED_BOOT, STATUS_LED_OFF);
	status_led_set(STATUS_LED_CRASH, STATUS_LED_BLINKING);
#endif
	puts("### ERROR ### Please RESET the board ###\n");
	while (1)
		/* If a JTAG emulator is hooked up, we'll automatically trigger
		 * a breakpoint in it.  If one isn't, this is just a NOP.
		 */
		asm("emuexcpt;");
}
