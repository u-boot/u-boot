/*
 * (C) Copyright 2000-2002
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

#ifndef __COMMON_H_
#define __COMMON_H_	1

#undef	_LINUX_CONFIG_H
#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

typedef unsigned char		uchar;
typedef volatile unsigned long	vu_long;
typedef volatile unsigned short	vu_short;
typedef volatile unsigned char	vu_char;

#include <config.h>
#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/string.h>
#include <asm/ptrace.h>
#include <stdarg.h>
#if defined(CONFIG_PCI) && defined(CONFIG_440)
#include <pci.h>
#endif
#ifdef	CONFIG_8xx
#include <asm/8xx_immap.h>
#elif defined(CONFIG_8260)
#include <asm/immap_8260.h>
#endif
#ifdef	CONFIG_4xx
#include <ppc4xx.h>
#endif
#ifdef CONFIG_HYMOD
#include <asm/hymod.h>
#endif
#ifdef CONFIG_ARM
#define asmlinkage	/* nothing */
#endif

#include <part.h>
#include <flash.h>
#include <image.h>

#ifdef	DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif	/* DEBUG */

typedef	void (interrupt_handler_t)(void *);

#include <asm/u-boot.h>	/* boot information for Linux kernel */
#include <asm/global_data.h>	/* global data used for startup functions */

/* enable common handling for all TQM8xxL boards */
#if defined(CONFIG_TQM823L) || defined(CONFIG_TQM850L) || \
    defined(CONFIG_TQM855L) || defined(CONFIG_TQM860L)
# ifndef CONFIG_TQM8xxL
#  define CONFIG_TQM8xxL
# endif
#endif


/*
 * General Purpose Utilities
 */
#define min(X, Y)				\
	({ typeof (X) __x = (X), __y = (Y);	\
		(__x < __y) ? __x : __y; })

#define max(X, Y)				\
	({ typeof (X) __x = (X), __y = (Y);	\
		(__x > __y) ? __x : __y; })


/*
 * Function Prototypes
 */

#if CONFIG_SERIAL_SOFTWARE_FIFO
void	serial_buffered_init (void);
void	serial_buffered_putc (const char);
void	serial_buffered_puts (const char *);
int	serial_buffered_getc (void);
int	serial_buffered_tstc (void);
#endif /* CONFIG_SERIAL_SOFTWARE_FIFO */

void	hang	      (void) __attribute__ ((noreturn));

/* */
long int initdram (int);
int	display_options	(void);
void	print_size (ulong, const char *);

/* common/main.c */
void	main_loop	(void);
int	run_command	(const char *cmd, int flag);
int	readline	(const char *const prompt);
void	reset_cmd_timeout(void);

/* common/board.c */
void	board_init_f  (ulong);
void	board_init_r  (gd_t *, ulong);
int	checkboard    (void);
int	checkflash    (void);
int	checkdram     (void);
char *	strmhz(char *buf, long hz);
int	last_stage_init(void);

/* common/flash.c */
void flash_perror (int);

/* common/cmd_bootm.c */
void print_image_hdr (image_header_t *hdr);

extern ulong load_addr;		/* Default Load Address	*/

/* common/cmd_nvedit.c */
int	env_init     (void);
void	env_relocate (void);
char 	*getenv      (uchar *);
int	getenv_r     (uchar *name, uchar *buf, unsigned len);
int	saveenv      (void);
#ifdef CONFIG_PPC		/* ARM version to be fixed! */
void inline setenv   (char *, char *);
#else
void    setenv       (char *, char *);
#endif /* CONFIG_PPC */
#ifdef CONFIG_ARM
# include <asm/u-boot-arm.h>	/* ARM version to be fixed! */
#endif /* CONFIG_ARM */
#ifdef CONFIG_I386		/* x86 version to be fixed! */
# include <asm/u-boot-i386.h>  
#endif /* CONFIG_I386 */

void    pci_init      (void);
void    pci_init_board(void);
void    pciinfo       (int, int);

#if defined(CONFIG_PCI) && defined(CONFIG_440)
#   if defined(CFG_PCI_PRE_INIT)
    int    pci_pre_init        (struct pci_controller * );
#   endif
#   if defined(CFG_PCI_TARGET_INIT)
	void    pci_target_init      (struct pci_controller *);
#   endif
#   if defined(CFG_PCI_MASTER_INIT)
	void    pci_master_init      (struct pci_controller *);
#   endif
    int     is_pci_host         (struct pci_controller *);
#endif

int	misc_init_f   (void);
int	misc_init_r   (void);

/* $(BOARD)/$(BOARD).c */
void	reset_phy     (void);
void    fdc_hw_init   (void);

/* $(BOARD)/eeprom.c */
void eeprom_init  (void);
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
#if defined(CONFIG_SPI) || !defined(CFG_I2C_EEPROM_ADDR)
# define CFG_DEF_EEPROM_ADDR 0
#else
# define CFG_DEF_EEPROM_ADDR CFG_I2C_EEPROM_ADDR
#endif /* CONFIG_SPI || !defined(CFG_I2C_EEPROM_ADDR) */

#if defined(CONFIG_PCU_E) || defined(CONFIG_CCM)
extern void spi_init_f (void);
extern void spi_init_r (void);
extern ssize_t spi_read  (uchar *, int, uchar *, int);
extern ssize_t spi_write (uchar *, int, uchar *, int);
#endif

#ifdef CONFIG_RPXCLASSIC
void rpxclassic_init (void);
#endif

#ifdef CONFIG_MBX
/* $(BOARD)/mbx8xx.c */
void	mbx_init (void);
void	board_serial_init (void);
void	board_ether_init (void);
#endif

#if defined(CONFIG_RPXCLASSIC) || defined(CONFIG_MBX) || defined(CONFIG_IAD210)
void	board_get_enetaddr (uchar *addr);
#endif

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

void	load_sernum_ethaddr (void);

/* $(BOARD)/$(BOARD).c */
int board_pre_init (void);
int board_postclk_init (void); /* after clocks/timebase, before env/serial */
void board_poweroff (void);

#if defined(CFG_DRAM_TEST)
int testdram(void);
#endif /* CFG_DRAM_TEST */

/* $(CPU)/start.S */
#ifdef	CONFIG_8xx
uint	get_immr      (uint);
#endif
uint	get_pvr	      (void);
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
void	relocate_code (ulong, gd_t *, ulong);
ulong	get_endaddr   (void);
void	trap_init     (ulong);
#if defined (CONFIG_4xx)	|| \
    defined (CONFIG_74xx_7xx)	|| \
    defined (CONFIG_74x)	|| \
    defined (CONFIG_75x)	|| \
    defined (CONFIG_74xx)
unsigned char   in8(unsigned int);
void            out8(unsigned int, unsigned char);
unsigned short  in16(unsigned int);
unsigned short  in16r(unsigned int);
void            out16(unsigned int, unsigned short value);
void            out16r(unsigned int, unsigned short value);
unsigned long   in32(unsigned int);
unsigned long   in32r(unsigned int);
void            out32(unsigned int, unsigned long value);
void            out32r(unsigned int, unsigned long value);
void            ppcDcbf(unsigned long value);
void            ppcDcbi(unsigned long value);
void            ppcSync(void);
#endif

/* $(CPU)/cpu.c */
int	checkcpu      (void);
int	checkicache   (void);
int	checkdcache   (void);
void	upmconfig     (unsigned int, unsigned int *, unsigned int);
ulong	get_tbclk     (void);

/* $(CPU)/serial.c */
int	serial_init   (void);
void	serial_setbrg (void);
void	serial_putc   (const char);
void	serial_puts   (const char *);
void	serial_addr   (unsigned int);
int	serial_getc   (void);
int	serial_tstc   (void);

/* $(CPU)/speed.c */
int	get_clocks (void);
#if defined(CONFIG_8260)
int	prt_8260_clks (void);
#endif
#ifdef CONFIG_4xx
ulong	get_OPB_freq (void);
ulong	get_PCI_freq (void);
#endif
#if defined(CONFIG_S3C2400) || defined(CONFIG_S3C2410)
ulong	get_FCLK (void);
ulong	get_HCLK (void);
ulong	get_PCLK (void);
ulong	get_UCLK (void);
#endif
ulong	get_bus_freq  (ulong);

#if defined(CONFIG_4xx) || defined(CONFIG_IOP480)
#  if defined(CONFIG_440)
    typedef PPC440_SYS_INFO sys_info_t;
#  else
    typedef PPC405_SYS_INFO sys_info_t;
#  endif
void    get_sys_info  ( sys_info_t * );
#endif

/* $(CPU)/cpu_init.c */
#if defined(CONFIG_8xx) || defined(CONFIG_8260)
void	cpu_init_f    (volatile immap_t *immr);
#endif
#ifdef	CONFIG_4xx
void	cpu_init_f    (void);
#endif
int	cpu_init_r    (void);
#if defined(CONFIG_8260)
int	prt_8260_rsr  (void);
#endif

/* $(CPU)/interrupts.c */
int	interrupt_init     (void);
void	timer_interrupt    (struct pt_regs *);
void	external_interrupt (struct pt_regs *);
void	irq_install_handler(int, interrupt_handler_t *, void *);
void	irq_free_handler   (int);
void	reset_timer	   (void);
ulong	get_timer	   (ulong base);
void	set_timer	   (ulong t);
void	enable_interrupts  (void);
int	disable_interrupts (void);

/* $(CPU)/.../commproc.c */
int	dpram_init (void);
uint	dpram_base(void);
uint	dpram_base_align(uint align);
uint	dpram_alloc(uint size);
uint	dpram_alloc_align(uint size,uint align);
void	post_word_store (ulong);
ulong	post_word_load (void);

/* $(CPU)/.../<eth> */
void mii_init (void);

/* $(CPU)/.../lcd.c */
ulong	lcd_setmem (ulong);

/* $(CPU)/.../vfd.c */
ulong	vfd_setmem (ulong);

/* $(CPU)/.../video.c */
ulong	video_setmem (ulong);

/* ppc/cache.c */
void	flush_cache   (unsigned long, unsigned long);

/* ppc/ticks.S */
unsigned long long get_ticks(void);
void	wait_ticks    (unsigned long);

/* ppc/time.c */
void	udelay	      (unsigned long);
ulong	usec2ticks    (unsigned long usec);
ulong	ticks2usec    (unsigned long ticks);
int	init_timebase (void);

/* ppc/vsprintf.c */
ulong	simple_strtoul(const char *cp,char **endp,unsigned int base);
long	simple_strtol(const char *cp,char **endp,unsigned int base);
void	panic(const char *fmt, ...);
int	sprintf(char * buf, const char *fmt, ...);
int 	vsprintf(char *buf, const char *fmt, va_list args);

/* ppc/crc32.c */
ulong crc32 (ulong, const unsigned char *, uint);
ulong crc32_no_comp (ulong, const unsigned char *, uint);

/* common/console.c */
extern void **syscall_tbl;

int	console_init_f(void);	/* Before relocation; uses the serial  stuff	*/
int	console_init_r(void);	/* After  relocation; uses the console stuff	*/
int	console_assign (int file, char *devname);	/* Assign the console	*/
int	ctrlc (void);
int	had_ctrlc (void);	/* have we had a Control-C since last clear? */
void	clear_ctrlc (void);	/* clear the Control-C condition */
int	disable_ctrlc (int);	/* 1 to disable, 0 to enable Control-C detect */

/*
 * STDIO based functions (can always be used)
 */

/* serial stuff */
void	serial_printf (const char *fmt, ...);

/* stdin */
int	getc(void);
int	tstc(void);

/* stdout */
void	putc(const char c);
void	puts(const char *s);
void	printf(const char *fmt, ...);

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

void	fprintf(int file, const char *fmt, ...);
void	fputs(int file, const char *s);
void	fputc(int file, const char c);
int	ftstc(int file);
int	fgetc(int file);

int	pcmcia_init (void);

#ifdef CONFIG_SHOW_BOOT_PROGRESS
void	show_boot_progress (int status);
#endif

#endif	/* __COMMON_H_ */
