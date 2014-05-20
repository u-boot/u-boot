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

#include <config.h>
#include <asm-offsets.h>
#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/stringify.h>
#include <asm/ptrace.h>
#include <stdarg.h>
#if defined(CONFIG_PCI) && defined(CONFIG_4xx)
#include <pci.h>
#endif
#if defined(CONFIG_8xx)
#include <asm/8xx_immap.h>
#if defined(CONFIG_MPC852)	|| defined(CONFIG_MPC852T)	|| \
    defined(CONFIG_MPC859)	|| defined(CONFIG_MPC859T)	|| \
    defined(CONFIG_MPC859DSL)	|| \
    defined(CONFIG_MPC866)	|| defined(CONFIG_MPC866T)	|| \
    defined(CONFIG_MPC866P)
# define CONFIG_MPC866_FAMILY 1
#elif defined(CONFIG_MPC870) \
   || defined(CONFIG_MPC875) \
   || defined(CONFIG_MPC880) \
   || defined(CONFIG_MPC885)
# define CONFIG_MPC885_FAMILY   1
#endif
#if   defined(CONFIG_MPC860)	   \
   || defined(CONFIG_MPC860T)	   \
   || defined(CONFIG_MPC866_FAMILY) \
   || defined(CONFIG_MPC885_FAMILY)
# define CONFIG_MPC86x 1
#endif
#elif defined(CONFIG_5xx)
#include <asm/5xx_immap.h>
#elif defined(CONFIG_MPC5xxx)
#include <mpc5xxx.h>
#elif defined(CONFIG_MPC512X)
#include <asm/immap_512x.h>
#elif defined(CONFIG_MPC8260)
#if   defined(CONFIG_MPC8247) \
   || defined(CONFIG_MPC8248) \
   || defined(CONFIG_MPC8271) \
   || defined(CONFIG_MPC8272)
#define CONFIG_MPC8272_FAMILY	1
#endif
#include <asm/immap_8260.h>
#endif
#ifdef CONFIG_MPC86xx
#include <mpc86xx.h>
#include <asm/immap_86xx.h>
#endif
#ifdef CONFIG_MPC85xx
#include <mpc85xx.h>
#include <asm/immap_85xx.h>
#endif
#ifdef CONFIG_MPC83xx
#include <mpc83xx.h>
#include <asm/immap_83xx.h>
#endif
#ifdef	CONFIG_4xx
#include <asm/ppc4xx.h>
#endif
#ifdef CONFIG_HYMOD
#include <board/hymod/hymod.h>
#endif
#ifdef CONFIG_ARM
#define asmlinkage	/* nothing */
#endif
#ifdef CONFIG_BLACKFIN
#include <asm/blackfin.h>
#endif
#ifdef CONFIG_SOC_DA8XX
#include <asm/arch/hardware.h>
#endif

#include <part.h>
#include <flash.h>
#include <image.h>

#ifdef __LP64__
#define CONFIG_SYS_SUPPORT_64BIT_DATA
#endif

#ifdef DEBUG
#define _DEBUG	1
#else
#define _DEBUG	0
#endif

/*
 * Output a debug text when condition "cond" is met. The "cond" should be
 * computed by a preprocessor in the best case, allowing for the best
 * optimization.
 */
#define debug_cond(cond, fmt, args...)		\
	do {					\
		if (cond)			\
			printf(fmt, ##args);	\
	} while (0)

#define debug(fmt, args...)			\
	debug_cond(_DEBUG, fmt, ##args)

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
		printf("ERROR: " fmt "\nat %s:%d/%s()\n",		\
			##args, __FILE__, __LINE__, __func__);		\
} while (0)

#ifndef BUG
#define BUG() do { \
	printf("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __FUNCTION__); \
	panic("BUG!"); \
} while (0)
#define BUG_ON(condition) do { if (unlikely((condition)!=0)) BUG(); } while(0)
#endif /* BUG */

/* Force a compilation error if condition is true */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

typedef void (interrupt_handler_t)(void *);

#include <asm/u-boot.h> /* boot information for Linux kernel */
#include <asm/global_data.h>	/* global data used for startup functions */

/*
 * enable common handling for all TQM8xxL/M boards:
 * - CONFIG_TQM8xxM will be defined for all TQM8xxM boards
 * - CONFIG_TQM8xxL will be defined for all TQM8xxL _and_ TQM8xxM boards
 *                  and for the TQM885D board
 */
#if defined(CONFIG_TQM823M) || defined(CONFIG_TQM850M) || \
    defined(CONFIG_TQM855M) || defined(CONFIG_TQM860M) || \
    defined(CONFIG_TQM862M) || defined(CONFIG_TQM866M)
# ifndef CONFIG_TQM8xxM
#  define CONFIG_TQM8xxM
# endif
#endif
#if defined(CONFIG_TQM823L) || defined(CONFIG_TQM850L) || \
    defined(CONFIG_TQM855L) || defined(CONFIG_TQM860L) || \
    defined(CONFIG_TQM862L) || defined(CONFIG_TQM8xxM) || \
    defined(CONFIG_TQM885D)
# ifndef CONFIG_TQM8xxL
#  define CONFIG_TQM8xxL
# endif
#endif

/*
 * General Purpose Utilities
 */
#define min(X, Y)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		(__x < __y) ? __x : __y; })

#define max(X, Y)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		(__x > __y) ? __x : __y; })

#define MIN(x, y)  min(x, y)
#define MAX(x, y)  max(x, y)

#define min3(X, Y, Z)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		typeof(Z) __z = (Z);		\
		__x < __y ? (__x < __z ? __x : __z) :	\
		(__y < __z ? __y : __z); })

#define max3(X, Y, Z)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		typeof(Z) __z = (Z);		\
		__x > __y ? (__x > __z ? __x : __z) :	\
		(__y > __z ? __y : __z); })

#define MIN3(x, y, z)  min3(x, y, z)
#define MAX3(x, y, z)  max3(x, y, z)

/*
 * Return the absolute value of a number.
 *
 * This handles unsigned and signed longs, ints, shorts and chars.  For all
 * input types abs() returns a signed long.
 *
 * For 64-bit types, use abs64()
 */
#define abs(x) ({						\
		long ret;					\
		if (sizeof(x) == sizeof(long)) {		\
			long __x = (x);				\
			ret = (__x < 0) ? -__x : __x;		\
		} else {					\
			int __x = (x);				\
			ret = (__x < 0) ? -__x : __x;		\
		}						\
		ret;						\
	})

#define abs64(x) ({				\
		s64 __x = (x);			\
		(__x < 0) ? -__x : __x;		\
	})

#if defined(CONFIG_ENV_IS_EMBEDDED)
#define TOTAL_MALLOC_LEN	CONFIG_SYS_MALLOC_LEN
#elif ( ((CONFIG_ENV_ADDR+CONFIG_ENV_SIZE) < CONFIG_SYS_MONITOR_BASE) || \
	(CONFIG_ENV_ADDR >= (CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)) ) || \
      defined(CONFIG_ENV_IS_IN_NVRAM)
#define	TOTAL_MALLOC_LEN	(CONFIG_SYS_MALLOC_LEN + CONFIG_ENV_SIZE)
#else
#define	TOTAL_MALLOC_LEN	CONFIG_SYS_MALLOC_LEN
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

/*
 * Function Prototypes
 */

void	hang		(void) __attribute__ ((noreturn));

int	timer_init(void);
int	cpu_init(void);

/* */
phys_size_t initdram (int);
int	display_options (void);
void	print_size(unsigned long long, const char *);
int print_buffer(ulong addr, const void *data, uint width, uint count,
		 uint linelen);

/* common/main.c */
void	main_loop	(void);
int run_command(const char *cmd, int flag);

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
int	readline	(const char *const prompt);
int	readline_into_buffer(const char *const prompt, char *buffer,
			int timeout);
int	parse_line (char *, char *[]);
void	init_cmd_timeout(void);
void	reset_cmd_timeout(void);
extern char console_buffer[];

/* arch/$(ARCH)/lib/board.c */
void	board_init_f(ulong);
void	board_init_r  (gd_t *, ulong) __attribute__ ((noreturn));
int	checkboard    (void);
int	checkflash    (void);
int	checkdram     (void);
int	last_stage_init(void);
extern ulong monitor_flash_len;
int mac_read_from_eeprom(void);
extern u8 __dtb_dt_begin[];	/* embedded device tree blob */
int set_cpu_clk_info(void);
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void);
#else
static inline int print_cpuinfo(void)
{
	return 0;
}
#endif
int update_flash_size(int flash_size);
int arch_early_init_r(void);

/**
 * Show the DRAM size in a board-specific way
 *
 * This is used by boards to display DRAM information in their own way.
 *
 * @param size	Size of DRAM (which should be displayed along with other info)
 */
void board_show_dram(ulong size);

/**
 * arch_fixup_memory_node() - Write arch-specific memory information to fdt
 *
 * Defined in arch/$(ARCH)/lib/bootm.c
 *
 * @blob:	FDT blob to write to
 * @return 0 if ok, or -ve FDT_ERR_... on failure
 */
int arch_fixup_memory_node(void *blob);

/* common/flash.c */
void flash_perror (int);

/* common/cmd_source.c */
int	source (ulong addr, const char *fit_uname);

extern ulong load_addr;		/* Default Load Address */
extern ulong save_addr;		/* Default Save Address */
extern ulong save_size;		/* Default Save Size */

/* common/cmd_doc.c */
void	doc_probe(unsigned long physadr);

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

#ifdef CONFIG_ARM
# include <asm/mach-types.h>
# include <asm/setup.h>
# include <asm/u-boot-arm.h>	/* ARM version to be fixed! */
#endif /* CONFIG_ARM */
#ifdef CONFIG_X86		/* x86 version to be fixed! */
# include <asm/u-boot-x86.h>
#endif /* CONFIG_X86 */
#ifdef CONFIG_SANDBOX
# include <asm/u-boot-sandbox.h>	/* TODO(sjg) what needs to be fixed? */
#endif
#ifdef CONFIG_NDS32
# include <asm/mach-types.h>
# include <asm/setup.h>
# include <asm/u-boot-nds32.h>
#endif /* CONFIG_NDS32 */
#ifdef CONFIG_MIPS
# include <asm/u-boot-mips.h>
#endif /* CONFIG_MIPS */
#ifdef CONFIG_ARC
# include <asm/u-boot-arc.h>
#endif /* CONFIG_ARC */

#ifdef CONFIG_AUTO_COMPLETE
int env_complete(char *var, int maxv, char *cmdv[], int maxsz, char *buf);
#endif
int get_env_id (void);

void	pci_init      (void);
void	pci_init_board(void);
void	pciinfo	      (int, int);

#if defined(CONFIG_PCI) && defined(CONFIG_4xx)
    int	   pci_pre_init	       (struct pci_controller *);
    int	   is_pci_host	       (struct pci_controller *);
#endif

#if defined(CONFIG_PCI) && (defined(CONFIG_440) || defined(CONFIG_405EX))
#   if defined(CONFIG_SYS_PCI_TARGET_INIT)
	void	pci_target_init	     (struct pci_controller *);
#   endif
#   if defined(CONFIG_SYS_PCI_MASTER_INIT)
	void	pci_master_init	     (struct pci_controller *);
#   endif
#if defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT) || \
    defined(CONFIG_405EX)
   void pcie_setup_hoses(int busno);
#endif
#endif

int	misc_init_f   (void);
int	misc_init_r   (void);

/* common/exports.c */
void	jumptable_init(void);

/* common/kallsysm.c */
const char *symbol_lookup(unsigned long addr, unsigned long *caddr);

/* api/api.c */
void	api_init (void);

/* common/memsize.c */
long	get_ram_size  (long *, long);
phys_size_t get_effective_memsize(void);

/* $(BOARD)/$(BOARD).c */
void	reset_phy     (void);
void	fdc_hw_init   (void);

/* $(BOARD)/eeprom.c */
void eeprom_init  (void);
#ifndef CONFIG_SPI
int  eeprom_probe (unsigned dev_addr, unsigned offset);
#endif
int  eeprom_read  (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt);
int  eeprom_write (unsigned dev_addr, unsigned offset, uchar *buffer, unsigned cnt);
#ifdef CONFIG_LWMON
extern uchar pic_read  (uchar reg);
extern void  pic_write (uchar reg, uchar val);
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

void rpxlite_init (void);

#ifdef CONFIG_HERMES
/* $(BOARD)/hermes.c */
void hermes_start_lxt980 (int speed);
#endif

#ifdef CONFIG_EVB64260
void  evb64260_init(void);
void  debug_led(int, int);
void  display_mem_map(void);
void  perform_soft_reset(void);
#endif

/* $(BOARD)/$(BOARD).c */
int board_early_init_f (void);
int board_late_init (void);
int board_postclk_init (void); /* after clocks/timebase, before env/serial */
int board_early_init_r (void);
void board_poweroff (void);

#if defined(CONFIG_SYS_DRAM_TEST)
int testdram(void);
#endif /* CONFIG_SYS_DRAM_TEST */

/* $(CPU)/start.S */
#if defined(CONFIG_5xx) || \
    defined(CONFIG_8xx)
uint	get_immr      (uint);
#endif
uint	get_pir	      (void);
#if defined(CONFIG_MPC5xxx)
uint	get_svr       (void);
#endif
uint	get_pvr	      (void);
uint	get_svr	      (void);
uint	rd_ic_cst     (void);
void	wr_ic_cst     (uint);
void	wr_ic_adr     (uint);
uint	rd_dc_cst     (void);
void	wr_dc_cst     (uint);
void	wr_dc_adr     (uint);
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
#if defined (CONFIG_4xx)	|| \
    defined (CONFIG_MPC5xxx)	|| \
    defined (CONFIG_74xx_7xx)	|| \
    defined (CONFIG_74x)	|| \
    defined (CONFIG_75x)	|| \
    defined (CONFIG_74xx)	|| \
    defined (CONFIG_MPC85xx)	|| \
    defined (CONFIG_MPC86xx)	|| \
    defined (CONFIG_MPC83xx)
unsigned char	in8(unsigned int);
void		out8(unsigned int, unsigned char);
unsigned short	in16(unsigned int);
unsigned short	in16r(unsigned int);
void		out16(unsigned int, unsigned short value);
void		out16r(unsigned int, unsigned short value);
unsigned long	in32(unsigned int);
unsigned long	in32r(unsigned int);
void		out32(unsigned int, unsigned long value);
void		out32r(unsigned int, unsigned long value);
void		ppcDcbf(unsigned long value);
void		ppcDcbi(unsigned long value);
void		ppcSync(void);
void		ppcDcbz(unsigned long value);
#endif
#if defined (CONFIG_MICROBLAZE)
unsigned short	in16(unsigned int);
void		out16(unsigned int, unsigned short value);
#endif

#if defined (CONFIG_MPC83xx)
void		ppcDWload(unsigned int *addr, unsigned int *ret);
void		ppcDWstore(unsigned int *addr, unsigned int *value);
void disable_addr_trans(void);
void enable_addr_trans(void);
#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
void ddr_enable_ecc(unsigned int dram_size);
#endif
#endif

/*
 * Return the current value of a monotonically increasing microsecond timer.
 * Granularity may be larger than 1us if hardware does not support this.
 */
ulong timer_get_us(void);

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
u32	cpu_mask      (void);
int	is_core_valid (unsigned int);
int	probecpu      (void);
int	checkcpu      (void);
int	checkicache   (void);
int	checkdcache   (void);
void	upmconfig     (unsigned int, unsigned int *, unsigned int);
ulong	get_tbclk     (void);
void	reset_cpu     (ulong addr);
#if defined (CONFIG_OF_LIBFDT) && defined (CONFIG_OF_BOARD_SETUP)
void ft_cpu_setup(void *blob, bd_t *bd);
#ifdef CONFIG_PCI
void ft_pci_setup(void *blob, bd_t *bd);
#endif
#endif

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

void	_serial_setbrg (const int);
void	_serial_putc   (const char, const int);
void	_serial_putc_raw(const char, const int);
void	_serial_puts   (const char *, const int);
int	_serial_getc   (const int);
int	_serial_tstc   (const int);

/* $(CPU)/speed.c */
int	get_clocks (void);
int	get_clocks_866 (void);
int	sdram_adjust_866 (void);
int	adjust_sdram_tbs_8xx (void);
#if defined(CONFIG_MPC8260)
int	prt_8260_clks (void);
#elif defined(CONFIG_MPC5xxx)
int	prt_mpc5xxx_clks (void);
#endif
#ifdef CONFIG_4xx
ulong	get_OPB_freq (void);
ulong	get_PCI_freq (void);
#endif
#if defined(CONFIG_S3C24X0) || \
    defined(CONFIG_LH7A40X) || \
    defined(CONFIG_EP93XX)
ulong	get_FCLK (void);
ulong	get_HCLK (void);
ulong	get_PCLK (void);
ulong	get_UCLK (void);
#endif
#if defined(CONFIG_LH7A40X)
ulong	get_PLLCLK (void);
#endif
#if defined(CONFIG_IMX)
ulong get_systemPLLCLK(void);
ulong get_FCLK(void);
ulong get_HCLK(void);
ulong get_BCLK(void);
ulong get_PERCLK1(void);
ulong get_PERCLK2(void);
ulong get_PERCLK3(void);
#endif
ulong	get_bus_freq  (ulong);
int get_serial_clock(void);

#if defined(CONFIG_MPC83xx) || defined(CONFIG_MPC85xx)
ulong get_ddr_freq(ulong);
#endif
#if defined(CONFIG_MPC85xx)
typedef MPC85xx_SYS_INFO sys_info_t;
void	get_sys_info  ( sys_info_t * );
#  if defined(CONFIG_OF_LIBFDT)
	void ft_fixup_cpu(void *, u64);
	void ft_fixup_num_cores(void *);
#  endif
#endif
#if defined(CONFIG_MPC86xx)
typedef MPC86xx_SYS_INFO sys_info_t;
void   get_sys_info  ( sys_info_t * );
static inline ulong get_ddr_freq(ulong dummy)
{
	return get_bus_freq(dummy);
}
#endif

#if defined(CONFIG_4xx)
#  if defined(CONFIG_440)
#	if defined(CONFIG_440SPE)
	 unsigned long determine_sysper(void);
	 unsigned long determine_pci_clock_per(void);
#	endif
#  endif
typedef PPC4xx_SYS_INFO sys_info_t;
int	ppc440spe_revB(void);
void	get_sys_info  ( sys_info_t * );
#endif

/* $(CPU)/cpu_init.c */
#if defined(CONFIG_8xx) || defined(CONFIG_MPC8260)
void	cpu_init_f    (volatile immap_t *immr);
#endif
#if defined(CONFIG_4xx) || defined(CONFIG_MCF52x2) || defined(CONFIG_MPC86xx)
void	cpu_init_f    (void);
#endif
#ifdef CONFIG_MPC85xx
ulong cpu_init_f(void);
#endif

int	cpu_init_r    (void);
#if defined(CONFIG_MPC8260)
int	prt_8260_rsr  (void);
#elif defined(CONFIG_MPC83xx)
int	prt_83xx_rsr  (void);
#endif

/* $(CPU)/interrupts.c */
int	interrupt_init	   (void);
void	timer_interrupt	   (struct pt_regs *);
void	external_interrupt (struct pt_regs *);
void	irq_install_handler(int, interrupt_handler_t *, void *);
void	irq_free_handler   (int);
void	reset_timer	   (void);
ulong	get_timer	   (ulong base);

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

/* arch/$(ARCH)/lib/ticks.S */
unsigned long long get_ticks(void);
void	wait_ticks    (unsigned long);

/* arch/$(ARCH)/lib/time.c */
void	__udelay      (unsigned long);
ulong	usec2ticks    (unsigned long usec);
ulong	ticks2usec    (unsigned long ticks);
int	init_timebase (void);

/* lib/gunzip.c */
int gunzip(void *, int, unsigned char *, unsigned long *);
int zunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp,
						int stoponerr, int offset);

/* lib/qsort.c */
void qsort(void *base, size_t nmemb, size_t size,
	   int(*compar)(const void *, const void *));
int strcmp_compar(const void *, const void *);

/* lib/time.c */
void	udelay        (unsigned long);
void mdelay(unsigned long);

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

/* common/console.c */
int	console_init_f(void);	/* Before relocation; uses the serial  stuff	*/
int	console_init_r(void);	/* After  relocation; uses the console stuff	*/
int	console_assign(int file, const char *devname);	/* Assign the console	*/
int	ctrlc (void);
int	had_ctrlc (void);	/* have we had a Control-C since last clear? */
void	clear_ctrlc (void);	/* clear the Control-C condition */
int	disable_ctrlc (int);	/* 1 to disable, 0 to enable Control-C detect */

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
void	putc(const char c);
void	puts(const char *s);
int	printf(const char *fmt, ...)
		__attribute__ ((format (__printf__, 1, 2)));
int	vprintf(const char *fmt, va_list args);

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
static inline IPaddr_t getenv_IPaddr(char *var)
{
	return string_to_ip(getenv(var));
}

/*
 * CONSOLE multiplexing.
 */
#ifdef CONFIG_CONSOLE_MUX
#include <iomux.h>
#endif

int	pcmcia_init (void);

#ifdef CONFIG_STATUS_LED
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

/* Define a null map_sysmem() if the architecture doesn't use it */
# ifndef CONFIG_ARCH_MAP_SYSMEM
static inline void *map_sysmem(phys_addr_t paddr, unsigned long len)
{
	return (void *)(uintptr_t)paddr;
}

static inline void unmap_sysmem(const void *vaddr)
{
}

static inline phys_addr_t map_to_sysmem(const void *ptr)
{
	return (phys_addr_t)(uintptr_t)ptr;
}
# endif

#endif /* __ASSEMBLY__ */

#ifdef CONFIG_PPC
/*
 * Has to be included outside of the #ifndef __ASSEMBLY__ section.
 * Otherwise might lead to compilation errors in assembler files.
 */
#include <asm/cache.h>
#endif

/* Put only stuff here that the assembler can digest */

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

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define ROUND(a,b)		(((a) + (b) - 1) & ~((b) - 1))
#define DIV_ROUND(n,d)		(((n) + ((d)/2)) / (d))
#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))
#define roundup(x, y)		((((x) + ((y) - 1)) / (y)) * (y))

/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest integer. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 || (__x) > 0) ?	\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}							\
)

#define ALIGN(x,a)		__ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))

/*
 * ARCH_DMA_MINALIGN is defined in asm/cache.h for each architecture.  It
 * is used to align DMA buffers.
 */
#ifndef __ASSEMBLY__
#include <asm/cache.h>
#endif

/*
 * The ALLOC_CACHE_ALIGN_BUFFER macro is used to allocate a buffer on the
 * stack that meets the minimum architecture alignment requirements for DMA.
 * Such a buffer is useful for DMA operations where flushing and invalidating
 * the cache before and after a read and/or write operation is required for
 * correct operations.
 *
 * When called the macro creates an array on the stack that is sized such
 * that:
 *
 * 1) The beginning of the array can be advanced enough to be aligned.
 *
 * 2) The size of the aligned portion of the array is a multiple of the minimum
 *    architecture alignment required for DMA.
 *
 * 3) The aligned portion contains enough space for the original number of
 *    elements requested.
 *
 * The macro then creates a pointer to the aligned portion of this array and
 * assigns to the pointer the address of the first element in the aligned
 * portion of the array.
 *
 * Calling the macro as:
 *
 *     ALLOC_CACHE_ALIGN_BUFFER(uint32_t, buffer, 1024);
 *
 * Will result in something similar to saying:
 *
 *     uint32_t    buffer[1024];
 *
 * The following differences exist:
 *
 * 1) The resulting buffer is guaranteed to be aligned to the value of
 *    ARCH_DMA_MINALIGN.
 *
 * 2) The buffer variable created by the macro is a pointer to the specified
 *    type, and NOT an array of the specified type.  This can be very important
 *    if you want the address of the buffer, which you probably do, to pass it
 *    to the DMA hardware.  The value of &buffer is different in the two cases.
 *    In the macro case it will be the address of the pointer, not the address
 *    of the space reserved for the buffer.  However, in the second case it
 *    would be the address of the buffer.  So if you are replacing hard coded
 *    stack buffers with this macro you need to make sure you remove the & from
 *    the locations where you are taking the address of the buffer.
 *
 * Note that the size parameter is the number of array elements to allocate,
 * not the number of bytes.
 *
 * This macro can not be used outside of function scope, or for the creation
 * of a function scoped static buffer.  It can not be used to create a cache
 * line aligned global buffer.
 */
#define PAD_COUNT(s, pad) (((s) - 1) / (pad) + 1)
#define PAD_SIZE(s, pad) (PAD_COUNT(s, pad) * pad)
#define ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, pad)		\
	char __##name[ROUND(PAD_SIZE((size) * sizeof(type), pad), align)  \
		      + (align - 1)];					\
									\
	type *name = (type *) ALIGN((uintptr_t)__##name, align)
#define ALLOC_ALIGN_BUFFER(type, name, size, align)		\
	ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, 1)
#define ALLOC_CACHE_ALIGN_BUFFER_PAD(type, name, size, pad)		\
	ALLOC_ALIGN_BUFFER_PAD(type, name, size, ARCH_DMA_MINALIGN, pad)
#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size)			\
	ALLOC_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

/*
 * DEFINE_CACHE_ALIGN_BUFFER() is similar to ALLOC_CACHE_ALIGN_BUFFER, but it's
 * purpose is to allow allocating aligned buffers outside of function scope.
 * Usage of this macro shall be avoided or used with extreme care!
 */
#define DEFINE_ALIGN_BUFFER(type, name, size, align)			\
	static char __##name[roundup(size * sizeof(type), align)]	\
			__aligned(align);				\
									\
	static type *name = (type *)__##name
#define DEFINE_CACHE_ALIGN_BUFFER(type, name, size)			\
	DEFINE_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

/* Pull in stuff for the build system */
#ifdef DO_DEPS_ONLY
# include <environment.h>
#endif

#endif	/* __COMMON_H_ */
