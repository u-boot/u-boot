/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __COMMON_H_
#define __COMMON_H_	1

#ifndef __ASSEMBLY__		/* put C only stuff in this section */

typedef unsigned char		uchar;
typedef volatile unsigned long	vu_long;
typedef volatile unsigned short vu_short;
typedef volatile unsigned char	vu_char;

/* Allow sharing constants with type modifiers between C and assembly. */
#define _AC(X, Y)       (X##Y)

#include <config.h>
#include <errno.h>
#include <time.h>
#include <asm-offsets.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/stringify.h>
#include <asm/ptrace.h>
#include <stdarg.h>
#include <linux/kernel.h>

#include <part.h>
#include <flash.h>
#include <image.h>

/* Bring in printf format macros if inttypes.h is included */
#define __STDC_FORMAT_MACROS

#ifdef __LP64__
#define CONFIG_SYS_SUPPORT_64BIT_DATA
#endif

#ifdef DEBUG
#define _DEBUG	1
#else
#define _DEBUG	0
#endif

#ifdef CONFIG_SPL_BUILD
#define _SPL_BUILD	1
#else
#define _SPL_BUILD	0
#endif

/* Define this at the top of a file to add a prefix to debug messages */
#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

/*
 * Output a debug text when condition "cond" is met. The "cond" should be
 * computed by a preprocessor in the best case, allowing for the best
 * optimization.
 */
#define debug_cond(cond, fmt, args...)			\
	do {						\
		if (cond)				\
			printf(pr_fmt(fmt), ##args);	\
	} while (0)

/* Show a message if DEBUG is defined in a file */
#define debug(fmt, args...)			\
	debug_cond(_DEBUG, fmt, ##args)

/* Show a message if not in SPL */
#define warn_non_spl(fmt, args...)			\
	debug_cond(!_SPL_BUILD, fmt, ##args)

/*
 * An assertion is run-time check done in debug mode only. If DEBUG is not
 * defined then it is skipped. If DEBUG is defined and the assertion fails,
 * then it calls panic*( which may or may not reset/halt U-Boot (see
 * CONFIG_PANIC_HANG), It is hoped that all failing assertions are found
 * before release, and after release it is hoped that they don't matter. But
 * in any case these failing assertions cannot be fixed with a reset (which
 * may just do the same assertion again).
 */
void __assert_fail(const char *assertion, const char *file, unsigned line,
		   const char *function);
#define assert(x) \
	({ if (!(x) && _DEBUG) \
		__assert_fail(#x, __FILE__, __LINE__, __func__); })

#define error(fmt, args...) do {					\
		printf("ERROR: " pr_fmt(fmt) "\nat %s:%d/%s()\n",	\
			##args, __FILE__, __LINE__, __func__);		\
} while (0)

#ifndef BUG
#define BUG() do { \
	printf("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__); \
	panic("BUG!"); \
} while (0)
#define BUG_ON(condition) do { if (unlikely((condition)!=0)) BUG(); } while(0)
#endif /* BUG */

typedef void (interrupt_handler_t)(void *);

#include <asm/u-boot.h> /* boot information for Linux kernel */
#include <asm/global_data.h>	/* global data used for startup functions */

#if defined(CONFIG_ENV_IS_EMBEDDED)
#define TOTAL_MALLOC_LEN	CONFIG_SYS_MALLOC_LEN
#elif ( ((CONFIG_ENV_ADDR+CONFIG_ENV_SIZE) < CONFIG_SYS_MONITOR_BASE) || \
	(CONFIG_ENV_ADDR >= (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)) ) || \
      defined(CONFIG_ENV_IS_IN_NVRAM)
#define	TOTAL_MALLOC_LEN	(CONFIG_SYS_MALLOC_LEN + CONFIG_ENV_SIZE)
#else
#define	TOTAL_MALLOC_LEN	CONFIG_SYS_MALLOC_LEN
#endif

/*
 * Function Prototypes
 */
int dram_init(void);

/**
 * dram_init_banksize() - Set up DRAM bank sizes
 *
 * This can be implemented by boards to set up the DRAM bank information in
 * gd->bd->bi_dram(). It is called just before relocation, after dram_init()
 * is called.
 *
 * If this is not provided, a default implementation will try to set up a
 * single bank. It will do this if CONFIG_NR_DRAM_BANKS and
 * CONFIG_SYS_SDRAM_BASE are set. The bank will have a start address of
 * CONFIG_SYS_SDRAM_BASE and the size will be determined by a call to
 * get_effective_memsize().
 *
 * @return 0 if OK, -ve on error
 */
int dram_init_banksize(void);

void	hang		(void) __attribute__ ((noreturn));

int	timer_init(void);
int	cpu_init(void);

#include <display_options.h>

/* common/main.c */
void	main_loop	(void);
int run_command(const char *cmd, int flag);
int run_command_repeatable(const char *cmd, int flag);

/**
 * Run a list of commands separated by ; or even \0
 *
 * Note that if 'len' is not -1, then the command does not need to be nul
 * terminated, Memory will be allocated for the command in that case.
 *
 * @param cmd	List of commands to run, each separated bu semicolon
 * @param len	Length of commands excluding terminator if known (-1 if not)
 * @param flag	Execution flags (CMD_FLAG_...)
 * @return 0 on success, or != 0 on error.
 */
int run_command_list(const char *cmd, int len, int flag);

/* arch/$(ARCH)/lib/board.c */
void board_init_f(ulong);
void board_init_r(gd_t *, ulong) __attribute__ ((noreturn));

/**
 * ulong board_init_f_alloc_reserve - allocate reserved area
 *
 * This function is called by each architecture very early in the start-up
 * code to allow the C runtime to reserve space on the stack for writable
 * 'globals' such as GD and the malloc arena.
 *
 * @top:	top of the reserve area, growing down.
 * @return:	bottom of reserved area
 */
ulong board_init_f_alloc_reserve(ulong top);

/**
 * board_init_f_init_reserve - initialize the reserved area(s)
 *
 * This function is called once the C runtime has allocated the reserved
 * area on the stack. It must initialize the GD at the base of that area.
 *
 * @base:	top from which reservation was done
 */
void board_init_f_init_reserve(ulong base);

/**
 * arch_setup_gd() - Set up the global_data pointer
 *
 * This pointer is special in some architectures and cannot easily be assigned
 * to. For example on x86 it is implemented by adding a specific record to its
 * Global Descriptor Table! So we we provide a function to carry out this task.
 * For most architectures this can simply be:
 *
 *    gd = gd_ptr;
 *
 * @gd_ptr:	Pointer to global data
 */
void arch_setup_gd(gd_t *gd_ptr);

int checkboard(void);
int show_board_info(void);
int checkflash(void);
int checkdram(void);
int last_stage_init(void);
extern ulong monitor_flash_len;
int mac_read_from_eeprom(void);
extern u8 __dtb_dt_begin[];	/* embedded device tree blob */
int set_cpu_clk_info(void);
int mdm_init(void);
int print_cpuinfo(void);
int update_flash_size(int flash_size);
int arch_early_init_r(void);

/*
 * setup_board_extra() - Fill in extra details in the bd_t structure
 *
 * @return 0 if OK, -ve on error
 */
int setup_board_extra(void);

/**
 * arch_fsp_init() - perform firmware support package init
 *
 * Where U-Boot relies on binary blobs to handle part of the system init, this
 * function can be used to set up the blobs. This is used on some Intel
 * platforms.
 */
int arch_fsp_init(void);

/**
 * arch_cpu_init_dm() - init CPU after driver model is available
 *
 * This is called immediately after driver model is available before
 * relocation. This is similar to arch_cpu_init() but is able to reference
 * devices
 *
 * @return 0 if OK, -ve on error
 */
int arch_cpu_init_dm(void);

/**
 * Reserve all necessary stacks
 *
 * This is used in generic board init sequence in common/board_f.c. Each
 * architecture could provide this function to tailor the required stacks.
 *
 * On entry gd->start_addr_sp is pointing to the suggested top of the stack.
 * The callee ensures gd->start_add_sp is 16-byte aligned, so architectures
 * require only this can leave it untouched.
 *
 * On exit gd->start_addr_sp and gd->irq_sp should be set to the respective
 * positions of the stack. The stack pointer(s) will be set to this later.
 * gd->irq_sp is only required, if the architecture needs it.
 *
 * @return 0 if no error
 */
__weak int arch_reserve_stacks(void);

/**
 * Show the DRAM size in a board-specific way
 *
 * This is used by boards to display DRAM information in their own way.
 *
 * @param size	Size of DRAM (which should be displayed along with other info)
 */
void board_show_dram(phys_size_t size);

/**
 * arch_fixup_fdt() - Write arch-specific information to fdt
 *
 * Defined in arch/$(ARCH)/lib/bootm-fdt.c
 *
 * @blob:	FDT blob to write to
 * @return 0 if ok, or -ve FDT_ERR_... on failure
 */
int arch_fixup_fdt(void *blob);

/* common/flash.c */
void flash_perror (int);

/* common/cmd_source.c */
int	source (ulong addr, const char *fit_uname);

extern ulong load_addr;		/* Default Load Address */
extern ulong save_addr;		/* Default Save Address */
extern ulong save_size;		/* Default Save Size */

/* common/cmd_net.c */
int do_tftpb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

/* common/cmd_fat.c */
int do_fat_fsload(cmd_tbl_t *, int, int, char * const []);

/* common/cmd_ext2.c */
int do_ext2load(cmd_tbl_t *, int, int, char * const []);

/* common/cmd_nvedit.c */
int	env_init     (void);
void	env_relocate (void);
int	envmatch     (uchar *, int);

/* Avoid unfortunate conflict with libc's getenv() */
#ifdef CONFIG_SANDBOX
#define getenv uboot_getenv
#endif
char	*getenv	     (const char *);
int	getenv_f     (const char *name, char *buf, unsigned len);
ulong getenv_ulong(const char *name, int base, ulong default_val);

/**
 * getenv_hex() - Return an environment variable as a hex value
 *
 * Decode an environment as a hex number (it may or may not have a 0x
 * prefix). If the environment variable cannot be found, or does not start
 * with hex digits, the default value is returned.
 *
 * @varname:		Variable to decode
 * @default_val:	Value to return on error
 */
ulong getenv_hex(const char *varname, ulong default_val);

/*
 * Read an environment variable as a boolean
 * Return -1 if variable does not exist (default to true)
 */
int getenv_yesno(const char *var);
int	saveenv	     (void);
int	setenv	     (const char *, const char *);
int setenv_ulong(const char *varname, ulong value);
int setenv_hex(const char *varname, ulong value);
/**
 * setenv_addr - Set an environment variable to an address in hex
 *
 * @varname:	Environment variable to set
 * @addr:	Value to set it to
 * @return 0 if ok, 1 on error
 */
static inline int setenv_addr(const char *varname, const void *addr)
{
	return setenv_hex(varname, (ulong)addr);
}

#ifdef CONFIG_AUTO_COMPLETE
int env_complete(char *var, int maxv, char *cmdv[], int maxsz, char *buf);
#endif
int get_env_id (void);

void	pci_init      (void);
void	pci_init_board(void);

int	misc_init_f   (void);
int	misc_init_r   (void);

/* common/exports.c */
void	jumptable_init(void);

/* common/kallsysm.c */
const char *symbol_lookup(unsigned long addr, unsigned long *caddr);

/* common/memsize.c */
long	get_ram_size  (long *, long);
phys_size_t get_effective_memsize(void);

/* $(BOARD)/$(BOARD).c */
void	reset_phy     (void);
void	fdc_hw_init   (void);

/* $(BOARD)/eeprom.c */
#ifdef CONFIG_CMD_EEPROM
void eeprom_init  (int bus);
int  eeprom_read  (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt);
int  eeprom_write (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt);
#else
/*
 * Some EEPROM code is depecated because it used the legacy I2C interface. Add
 * some macros here so we don't have to touch every one of those uses
 */
#define eeprom_init(bus)
#define eeprom_read(dev_addr, offset, buffer, cnt) ((void)-ENOSYS)
#define eeprom_write(dev_addr, offset, buffer, cnt) ((void)-ENOSYS)
#endif

/*
 * Set this up regardless of board
 * type, to prevent errors.
 */
#if defined(CONFIG_SPI) || !defined(CONFIG_SYS_I2C_EEPROM_ADDR)
# define CONFIG_SYS_DEF_EEPROM_ADDR 0
#else
#if !defined(CONFIG_ENV_EEPROM_IS_ON_I2C)
# define CONFIG_SYS_DEF_EEPROM_ADDR CONFIG_SYS_I2C_EEPROM_ADDR
#endif
#endif /* CONFIG_SPI || !defined(CONFIG_SYS_I2C_EEPROM_ADDR) */

#if defined(CONFIG_SPI)
extern void spi_init_f (void);
extern void spi_init_r (void);
extern ssize_t spi_read	 (uchar *, int, uchar *, int);
extern ssize_t spi_write (uchar *, int, uchar *, int);
#endif

/* $(BOARD)/$(BOARD).c */
int board_early_init_f (void);
int board_fix_fdt (void *rw_fdt_blob); /* manipulate the U-Boot fdt before its relocation */
int board_late_init (void);
int board_postclk_init (void); /* after clocks/timebase, before env/serial */
int board_early_init_r (void);
void board_poweroff (void);

#if defined(CONFIG_SYS_DRAM_TEST)
int testdram(void);
#endif /* CONFIG_SYS_DRAM_TEST */

/* $(CPU)/start.S */
int	icache_status (void);
void	icache_enable (void);
void	icache_disable(void);
int	dcache_status (void);
void	dcache_enable (void);
void	dcache_disable(void);
void	mmu_disable(void);
#if defined(CONFIG_ARM)
void	relocate_code(ulong);
#else
void	relocate_code(ulong, gd_t *, ulong) __attribute__ ((noreturn));
#endif
ulong	get_endaddr   (void);
void	trap_init     (ulong);

/* $(CPU)/cpu.c */
static inline int cpumask_next(int cpu, unsigned int mask)
{
	for (cpu++; !((1 << cpu) & mask); cpu++)
		;

	return cpu;
}

#define for_each_cpu(iter, cpu, num_cpus, mask) \
	for (iter = 0, cpu = cpumask_next(-1, mask); \
		iter < num_cpus; \
		iter++, cpu = cpumask_next(cpu, mask)) \

int	cpu_numcores  (void);
int	cpu_num_dspcores(void);
u32	cpu_mask      (void);
u32	cpu_dsp_mask(void);
int	is_core_valid (unsigned int);

/**
 * arch_cpu_init() - basic cpu-dependent setup for an architecture
 *
 * This is called after early malloc is available. It should handle any
 * CPU- or SoC- specific init needed to continue the init sequence. See
 * board_f.c for where it is called. If this is not provided, a default
 * version (which does nothing) will be used.
 */
int arch_cpu_init(void);

int	checkcpu      (void);
int	checkicache   (void);
int	checkdcache   (void);
void	upmconfig     (unsigned int, unsigned int *, unsigned int);
ulong	get_tbclk     (void);
void	reset_misc    (void);
void	reset_cpu     (ulong addr);
void ft_cpu_setup(void *blob, bd_t *bd);
void ft_pci_setup(void *blob, bd_t *bd);

void smp_set_core_boot_addr(unsigned long addr, int corenr);
void smp_kick_all_cpus(void);

/* $(CPU)/serial.c */
int	serial_init   (void);
void	serial_setbrg (void);
void	serial_putc   (const char);
void	serial_putc_raw(const char);
void	serial_puts   (const char *);
int	serial_getc   (void);
int	serial_tstc   (void);

/* $(CPU)/speed.c */
int	get_clocks (void);
ulong	get_bus_freq  (ulong);
int get_serial_clock(void);

int	cpu_init_r    (void);

/* $(CPU)/interrupts.c */
int	interrupt_init	   (void);
void	timer_interrupt	   (struct pt_regs *);
void	external_interrupt (struct pt_regs *);
void	irq_install_handler(int, interrupt_handler_t *, void *);
void	irq_free_handler   (int);
void	reset_timer	   (void);

/* Return value of monotonic microsecond timer */
unsigned long timer_get_us(void);

void	enable_interrupts  (void);
int	disable_interrupts (void);

/* $(CPU)/.../commproc.c */
int	dpram_init (void);
uint	dpram_base(void);
uint	dpram_base_align(uint align);
uint	dpram_alloc(uint size);
uint	dpram_alloc_align(uint size,uint align);
void	bootcount_store (ulong);
ulong	bootcount_load (void);
#define BOOTCOUNT_MAGIC		0xB001C041

/* $(CPU)/.../<eth> */
void mii_init (void);

/* $(CPU)/.../lcd.c */
ulong	lcd_setmem (ulong);

/* $(CPU)/.../video.c */
ulong	video_setmem (ulong);

/* arch/$(ARCH)/lib/cache.c */
void	enable_caches(void);
void	flush_cache   (unsigned long, unsigned long);
void	flush_dcache_all(void);
void	flush_dcache_range(unsigned long start, unsigned long stop);
void	invalidate_dcache_range(unsigned long start, unsigned long stop);
void	invalidate_dcache_all(void);
void	invalidate_icache_all(void);

enum {
	/* Disable caches (else flush caches but leave them active) */
	CBL_DISABLE_CACHES		= 1 << 0,
	CBL_SHOW_BOOTSTAGE_REPORT	= 1 << 1,

	CBL_ALL				= 3,
};

/**
 * Clean up ready for linux
 *
 * @param flags		Flags to control what is done
 */
int cleanup_before_linux_select(int flags);

/* arch/$(ARCH)/lib/ticks.S */
uint64_t get_ticks(void);
void	wait_ticks    (unsigned long);

/* arch/$(ARCH)/lib/time.c */
ulong	usec2ticks    (unsigned long usec);
ulong	ticks2usec    (unsigned long ticks);

/* lib/gunzip.c */
int gunzip(void *, int, unsigned char *, unsigned long *);
int zunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp,
						int stoponerr, int offset);

/**
 * gzwrite progress indicators: defined weak to allow board-specific
 * overrides:
 *
 *	gzwrite_progress_init called on startup
 *	gzwrite_progress called during decompress/write loop
 *	gzwrite_progress_finish called at end of loop to
 *		indicate success (retcode=0) or failure
 */
void gzwrite_progress_init(u64 expected_size);

void gzwrite_progress(int iteration,
		     u64 bytes_written,
		     u64 total_bytes);

void gzwrite_progress_finish(int retcode,
			     u64 totalwritten,
			     u64 totalsize,
			     u32 expected_crc,
			     u32 calculated_crc);

/**
 * decompress and write gzipped image from memory to block device
 *
 * @param	src		compressed image address
 * @param	len		compressed image length in bytes
 * @param	dev		block device descriptor
 * @param	szwritebuf	bytes per write (pad to erase size)
 * @param	startoffs	offset in bytes of first write
 * @param	szexpected	expected uncompressed length
 *				may be zero to use gzip trailer
 *				for files under 4GiB
 */
int gzwrite(unsigned char *src, int len,
	    struct blk_desc *dev,
	    unsigned long szwritebuf,
	    u64 startoffs,
	    u64 szexpected);

/* lib/lz4_wrapper.c */
int ulz4fn(const void *src, size_t srcn, void *dst, size_t *dstn);

/* lib/qsort.c */
void qsort(void *base, size_t nmemb, size_t size,
	   int(*compar)(const void *, const void *));
int strcmp_compar(const void *, const void *);

/* lib/uuid.c */
#include <uuid.h>

/* lib/vsprintf.c */
#include <vsprintf.h>

/* lib/strmhz.c */
char *	strmhz(char *buf, unsigned long hz);

/* lib/crc32.c */
#include <u-boot/crc.h>

/* lib/rand.c */
#define RAND_MAX -1U
void srand(unsigned int seed);
unsigned int rand(void);
unsigned int rand_r(unsigned int *seedp);

/*
 * STDIO based functions (can always be used)
 */
/* serial stuff */
int	serial_printf (const char *fmt, ...)
		__attribute__ ((format (__printf__, 1, 2)));
/* stdin */
int	getc(void);
int	tstc(void);

/* stdout */
#if !defined(CONFIG_SPL_BUILD) || \
	(defined(CONFIG_TPL_BUILD) && defined(CONFIG_TPL_SERIAL_SUPPORT)) || \
	(defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD) && \
		defined(CONFIG_SPL_SERIAL_SUPPORT))
void	putc(const char c);
void	puts(const char *s);
int	printf(const char *fmt, ...)
		__attribute__ ((format (__printf__, 1, 2)));
int	vprintf(const char *fmt, va_list args);
#else
#define	putc(...) do { } while (0)
#define puts(...) do { } while (0)
#define printf(...) do { } while (0)
#define vprintf(...) do { } while (0)
#endif

/* stderr */
#define eputc(c)		fputc(stderr, c)
#define eputs(s)		fputs(stderr, s)
#define eprintf(fmt,args...)	fprintf(stderr,fmt ,##args)

/*
 * FILE based functions (can only be used AFTER relocation!)
 */
#define stdin		0
#define stdout		1
#define stderr		2
#define MAX_FILES	3

int	fprintf(int file, const char *fmt, ...)
		__attribute__ ((format (__printf__, 2, 3)));
void	fputs(int file, const char *s);
void	fputc(int file, const char c);
int	ftstc(int file);
int	fgetc(int file);

/* lib/gzip.c */
int gzip(void *dst, unsigned long *lenp,
		unsigned char *src, unsigned long srclen);
int zzip(void *dst, unsigned long *lenp, unsigned char *src,
		unsigned long srclen, int stoponerr,
		int (*func)(unsigned long, unsigned long));

/* lib/net_utils.c */
#include <net.h>
static inline struct in_addr getenv_ip(char *var)
{
	return string_to_ip(getenv(var));
}

int	pcmcia_init (void);

#ifdef CONFIG_LED_STATUS
# include <status_led.h>
#endif

#include <bootstage.h>

#ifdef CONFIG_SHOW_ACTIVITY
void show_activity(int arg);
#endif

/* Multicore arch functions */
#ifdef CONFIG_MP
int cpu_status(int nr);
int cpu_reset(int nr);
int cpu_disable(int nr);
int cpu_release(int nr, int argc, char * const argv[]);
#endif

#else	/* __ASSEMBLY__ */

/* Drop a C type modifier (like in 3UL) for constants used in assembly. */
#define _AC(X, Y)       X

#endif	/* __ASSEMBLY__ */

/* Put only stuff here that the assembler can digest */

/* Declare an unsigned long constant digestable both by C and an assembler. */
#define UL(x)           _AC(x, UL)

#ifdef CONFIG_POST
#define CONFIG_HAS_POST
#ifndef CONFIG_POST_ALT_LIST
#define CONFIG_POST_STD_LIST
#endif
#endif

#ifdef CONFIG_INIT_CRITICAL
#error CONFIG_INIT_CRITICAL is deprecated!
#error Read section CONFIG_SKIP_LOWLEVEL_INIT in README.
#endif

#define ROUND(a,b)		(((a) + (b) - 1) & ~((b) - 1))

/*
 * check_member() - Check the offset of a structure member
 *
 * @structure:	Name of structure (e.g. global_data)
 * @member:	Name of member (e.g. baudrate)
 * @offset:	Expected offset in bytes
 */
#define check_member(structure, member, offset) _Static_assert( \
	offsetof(struct structure, member) == offset, \
	"`struct " #structure "` offset for `" #member "` is not " #offset)

/* Avoid using CONFIG_EFI_STUB directly as we may boot from other loaders */
#ifdef CONFIG_EFI_STUB
#define ll_boot_init()	false
#else
#define ll_boot_init()	true
#endif

/* Pull in stuff for the build system */
#ifdef DO_DEPS_ONLY
# include <environment.h>
#endif

#endif	/* __COMMON_H_ */
