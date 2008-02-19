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
#include <devices.h>
#include <environment.h>
#include <i2c.h>
#include <malloc.h>
#include <net.h>
#include <version.h>

#include <asm/cplb.h>
#include <asm/mach-common/bits/mpu.h>

#ifdef CONFIG_CMD_NAND
#include <nand.h>	/* cannot even include nand.h if it isnt configured */
#endif

#if defined(CONFIG_POST)
#include <post.h>
int post_flag;
#endif

DECLARE_GLOBAL_DATA_PTR;

const char version_string[] = U_BOOT_VERSION " (" __DATE__ " - " __TIME__ ")";

__attribute__((always_inline))
static inline void serial_early_puts(const char *s)
{
#ifdef CONFIG_DEBUG_EARLY_SERIAL
	serial_puts("Early: ");
	serial_puts(s);
#endif
}

/* Get the input voltage */
static u_long get_vco(void)
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

/* Get the Core clock */
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

static void *mem_malloc_start, *mem_malloc_end, *mem_malloc_brk;

static void mem_malloc_init(void)
{
	mem_malloc_start = (void *)CFG_MALLOC_BASE;
	mem_malloc_end = (void *)(CFG_MALLOC_BASE + CFG_MALLOC_LEN);
	mem_malloc_brk = mem_malloc_start;
	memset(mem_malloc_start, 0, mem_malloc_end - mem_malloc_start);
}

void *sbrk(ptrdiff_t increment)
{
	void *old = mem_malloc_brk;
	void *new = old + increment;

	if (new < mem_malloc_start || new > mem_malloc_end)
		return NULL;

	mem_malloc_brk = new;

	return old;
}

static int display_banner(void)
{
	printf("\n\n%s\n\n", version_string);
	printf("CPU:   ADSP " MK_STR(CONFIG_BFIN_CPU) " (Detected Rev: 0.%d)\n", bfin_revid());
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
	printf(" gd: %x\n", gd);
	printf(" |-flags: %x\n", gd->flags);
	printf(" |-board_type: %x\n", gd->board_type);
	printf(" |-baudrate: %i\n", gd->baudrate);
	printf(" |-have_console: %x\n", gd->have_console);
	printf(" |-ram_size: %x\n", gd->ram_size);
	printf(" |-reloc_off: %x\n", gd->reloc_off);
	printf(" |-env_addr: %x\n", gd->env_addr);
	printf(" |-env_valid: %x\n", gd->env_valid);
	printf(" |-jt(%x): %x\n", gd->jt, *(gd->jt));
	printf(" \\-bd: %x\n", gd->bd);
	printf("   |-bi_baudrate: %x\n", bd->bi_baudrate);
	printf("   |-bi_ip_addr: %x\n", bd->bi_ip_addr);
	printf("   |-bi_enetaddr: %x %x %x %x %x %x\n",
	       bd->bi_enetaddr[0], bd->bi_enetaddr[1],
	       bd->bi_enetaddr[2], bd->bi_enetaddr[3],
	       bd->bi_enetaddr[4], bd->bi_enetaddr[5]);
	printf("   |-bi_boot_params: %x\n", bd->bi_boot_params);
	printf("   |-bi_memstart: %x\n", bd->bi_memstart);
	printf("   |-bi_memsize: %x\n", bd->bi_memsize);
	printf("   |-bi_flashstart: %x\n", bd->bi_flashstart);
	printf("   |-bi_flashsize: %x\n", bd->bi_flashsize);
	printf("   \\-bi_flashoffset: %x\n", bd->bi_flashoffset);
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

	icplb_add(CFG_MONITOR_BASE & CPLB_PAGE_MASK, SDRAM_IKERNEL);
	dcplb_add(CFG_MONITOR_BASE & CPLB_PAGE_MASK, SDRAM_DKERNEL);
	++i;

	/* If the monitor crosses a 4 meg boundary, we'll need
	 * to lock two entries for it.
	 */
	if ((CFG_MONITOR_BASE & CPLB_PAGE_MASK) != ((CFG_MONITOR_BASE + CFG_MONITOR_LEN) & CPLB_PAGE_MASK)) {
		icplb_add((CFG_MONITOR_BASE + CFG_MONITOR_LEN) & CPLB_PAGE_MASK, SDRAM_IKERNEL);
		dcplb_add((CFG_MONITOR_BASE + CFG_MONITOR_LEN) & CPLB_PAGE_MASK, SDRAM_DKERNEL);
		++i;
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

	while (i < 16 && extern_memory < (CFG_MONITOR_BASE & CPLB_PAGE_MASK)) {
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
extern int rtc_init(void);
extern int timer_init(void);

void board_init_f(ulong bootflag)
{
	ulong addr;
	bd_t *bd;

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

	serial_early_puts("Init global data\n");
	gd = (gd_t *) (CFG_GBL_DATA_ADDR);
	memset((void *)gd, 0, sizeof(gd_t));

	/* Board data initialization */
	addr = (CFG_GBL_DATA_ADDR + sizeof(gd_t));

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
#if defined(CONFIG_RTC_BFIN) && defined(CONFIG_CMD_DATE)
	rtc_init();
#endif
	timer_init();

	printf("Clock: VCO: %lu MHz, Core: %lu MHz, System: %lu MHz\n",
	       get_vco() / 1000000, get_cclk() / 1000000, get_sclk() / 1000000);

	printf("RAM:   ");
	print_size(initdram(0), "\n");
#if defined(CONFIG_POST)
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
	extern void malloc_bin_reloc(void);
	char *s;
	bd_t *bd;
	gd = id;
	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */
	bd = gd->bd;

#if defined(CONFIG_POST)
	post_output_backlog();
	post_reloc();
#endif

#if	!defined(CFG_NO_FLASH)
	/* There are some other pointer constants we must deal with */
	/* configure available FLASH banks */
	extern flash_info_t flash_info[];
	ulong size = flash_init();
	puts("Flash: ");
	print_size(size, "\n");
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

#ifdef CONFIG_CMD_NET
	/* board MAC address */
	s = getenv("ethaddr");
	if (s == NULL) {
# ifndef CONFIG_ETHADDR
#  if 0
		if (!board_get_enetaddr(bd->bi_enetaddr)) {
			char nid[20];
			sprintf(nid, "%02X:%02X:%02X:%02X:%02X:%02X",
				bd->bi_enetaddr[0], bd->bi_enetaddr[1],
				bd->bi_enetaddr[2], bd->bi_enetaddr[3],
				bd->bi_enetaddr[4], bd->bi_enetaddr[5]);
			setenv("ethaddr", nid);
		}
#  endif
# endif
	} else {
		int i;
		char *e;
		for (i = 0; i < 6; ++i) {
			bd->bi_enetaddr[i] = simple_strtoul(s, &e, 16);
			s = (*e) ? e + 1 : e;
		}
	}

	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr("ipaddr");
#endif

	/* Initialize devices */
	devices_init();
	jumptable_init();

	/* Initialize the console (after the relocation and devices init) */
	console_init_r();

	/* Initialize from environment */
	if ((s = getenv("loadaddr")) != NULL)
		load_addr = simple_strtoul(s, NULL, 16);
#ifdef CONFIG_CMD_NET
	if ((s = getenv("bootfile")) != NULL)
		copy_filename(BootFile, s, sizeof(BootFile));
#endif

#ifdef CONFIG_CMD_NAND
	puts("NAND:  ");
	nand_init();		/* go init the NAND */
#endif

#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r();
#endif

#ifdef CONFIG_CMD_NET
	printf("Net:   ");
	eth_initialize(gd->bd);
	if (getenv("ethaddr"))
		printf("MAC:   %02X:%02X:%02X:%02X:%02X:%02X\n",
			bd->bi_enetaddr[0], bd->bi_enetaddr[1], bd->bi_enetaddr[2],
			bd->bi_enetaddr[3], bd->bi_enetaddr[4], bd->bi_enetaddr[5]);
#endif

#if defined(CONFIG_SOFT_I2C) || defined(CONFIG_HARD_I2C)
	init_func_i2c();
#endif

	display_global_data();

#if defined(CONFIG_POST)
	if (post_flag)
		post_run(NULL, POST_RAM | post_bootmode_get(0));
#endif

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;)
		main_loop();
}

void hang(void)
{
	puts("### ERROR ### Please RESET the board ###\n");
	while (1)
		/* If a JTAG emulator is hooked up, we'll automatically trigger
		 * a breakpoint in it.  If one isn't, this is just a NOP.
		 */
		asm("emuexcpt;");
}
