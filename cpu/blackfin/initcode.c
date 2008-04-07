/*
 * initcode.c - Initialize the processor.  This is usually entails things
 * like external memory, voltage regulators, etc...  Note that this file
 * cannot make any function calls as it may be executed all by itself by
 * the Blackfin's bootrom in LDR format.
 *
 * Copyright (c) 2004-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <config.h>
#include <asm/blackfin.h>
#include <asm/mach-common/bits/bootrom.h>
#include <asm/mach-common/bits/ebiu.h>
#include <asm/mach-common/bits/pll.h>
#include <asm/mach-common/bits/uart.h>

#define BFIN_IN_INITCODE
#include "serial.h"

__attribute__((always_inline))
static inline uint32_t serial_init(void)
{
#ifdef __ADSPBF54x__
# ifdef BFIN_BOOT_UART_USE_RTS
#  define BFIN_UART_USE_RTS 1
# else
#  define BFIN_UART_USE_RTS 0
# endif
	if (BFIN_UART_USE_RTS && CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_UART) {
		size_t i;

		/* force RTS rather than relying on auto RTS */
		bfin_write_UART1_MCR(bfin_read_UART1_MCR() | FCPOL);

		/* Wait for the line to clear up.  We cannot rely on UART
		 * registers as none of them reflect the status of the RSR.
		 * Instead, we'll sleep for ~10 bit times at 9600 baud.
		 * We can precalc things here by assuming boot values for
		 * PLL rather than loading registers and calculating.
		 *	baud    = SCLK / (16 ^ (1 - EDBO) * Divisor)
		 *	EDB0    = 0
		 *	Divisor = (SCLK / baud) / 16
		 *	SCLK    = baud * 16 * Divisor
		 *	SCLK    = (0x14 * CONFIG_CLKIN_HZ) / 5
		 *	CCLK    = (16 * Divisor * 5) * (9600 / 10)
		 * In reality, this will probably be just about 1 second delay,
		 * so assuming 9600 baud is OK (both as a very low and too high
		 * speed as this will buffer things enough).
		 */
#define _NUMBITS (10)                                   /* how many bits to delay */
#define _LOWBAUD (9600)                                 /* low baud rate */
#define _SCLK    ((0x14 * CONFIG_CLKIN_HZ) / 5)         /* SCLK based on PLL */
#define _DIVISOR ((_SCLK / _LOWBAUD) / 16)              /* UART DLL/DLH */
#define _NUMINS  (3)                                    /* how many instructions in loop */
#define _CCLK    (((16 * _DIVISOR * 5) * (_LOWBAUD / _NUMBITS)) / _NUMINS)
		i = _CCLK;
		while (i--)
			asm volatile("" : : : "memory");
	}
#endif

	uint32_t old_baud = serial_early_get_baud();

	if (BFIN_DEBUG_EARLY_SERIAL) {
		serial_early_init();

		/* If the UART is off, that means we need to program
		 * the baud rate ourselves initially.
		 */
		if (!old_baud) {
			old_baud = CONFIG_BAUDRATE;
			serial_early_set_baud(CONFIG_BAUDRATE);
		}
	}

	return old_baud;
}

__attribute__((always_inline))
static inline void serial_deinit(void)
{
#ifdef __ADSPBF54x__
	if (BFIN_UART_USE_RTS && CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_UART) {
		/* clear forced RTS rather than relying on auto RTS */
		bfin_write_UART1_MCR(bfin_read_UART1_MCR() & ~FCPOL);
	}
#endif
}

/* We need to reset the baud rate when we have early debug turned on
 * or when we are booting over the UART.
 * XXX: we should fix this to calc the old baud and restore it rather
 *      than hardcoding it via CONFIG_LDR_LOAD_BAUD ... but we have
 *      to figure out how to avoid the division in the baud calc ...
 */
__attribute__((always_inline))
static inline void serial_reset_baud(uint32_t baud)
{
	if (!BFIN_DEBUG_EARLY_SERIAL && CONFIG_BFIN_BOOT_MODE != BFIN_BOOT_UART)
		return;

#ifndef CONFIG_LDR_LOAD_BAUD
# define CONFIG_LDR_LOAD_BAUD 115200
#endif

	if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_BYPASS)
		serial_early_set_baud(baud);
	else if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_UART)
		serial_early_set_baud(CONFIG_LDR_LOAD_BAUD);
	else
		serial_early_set_baud(CONFIG_BAUDRATE);
}

__attribute__((always_inline))
static inline void serial_putc(char c)
{
	if (!BFIN_DEBUG_EARLY_SERIAL)
		return;

	if (c == '\n')
		*pUART_THR = '\r';

	*pUART_THR = c;

	while (!(*pUART_LSR & TEMT))
		continue;
}


/* Max SCLK can be 133MHz ... dividing that by 4 gives
 * us a freq of 33MHz for SPI which should generally be
 * slow enough for the slow reads the bootrom uses.
 */
#ifndef CONFIG_SPI_BAUD_INITBLOCK
# define CONFIG_SPI_BAUD_INITBLOCK 4
#endif

/* PLL_DIV defines */
#ifndef CONFIG_PLL_DIV_VAL
# if (CONFIG_CCLK_DIV == 1)
#  define CONFIG_CCLK_ACT_DIV CCLK_DIV1
# elif (CONFIG_CCLK_DIV == 2)
#  define CONFIG_CCLK_ACT_DIV CCLK_DIV2
# elif (CONFIG_CCLK_DIV == 4)
#  define CONFIG_CCLK_ACT_DIV CCLK_DIV4
# elif (CONFIG_CCLK_DIV == 8)
#  define CONFIG_CCLK_ACT_DIV CCLK_DIV8
# else
#  define CONFIG_CCLK_ACT_DIV CONFIG_CCLK_DIV_not_defined_properly
# endif
# define CONFIG_PLL_DIV_VAL (CONFIG_CCLK_ACT_DIV | CONFIG_SCLK_DIV)
#endif

#ifndef CONFIG_PLL_LOCKCNT_VAL
# define CONFIG_PLL_LOCKCNT_VAL 0x0300
#endif

#ifndef CONFIG_PLL_CTL_VAL
# define CONFIG_PLL_CTL_VAL (SPORT_HYST | (CONFIG_VCO_MULT << 9))
#endif

#ifndef CONFIG_EBIU_RSTCTL_VAL
# define CONFIG_EBIU_RSTCTL_VAL 0 /* only MDDRENABLE is useful */
#endif

#ifndef CONFIG_EBIU_MBSCTL_VAL
# define CONFIG_EBIU_MBSCTL_VAL 0
#endif

/* Make sure our voltage value is sane so we don't blow up! */
#ifndef CONFIG_VR_CTL_VAL
# define BFIN_CCLK ((CONFIG_CLKIN_HZ * CONFIG_VCO_MULT) / CONFIG_CCLK_DIV)
# if defined(__ADSPBF533__) || defined(__ADSPBF532__) || defined(__ADSPBF531__)
#  define CCLK_VLEV_120	400000000
#  define CCLK_VLEV_125	533000000
# elif defined(__ADSPBF537__) || defined(__ADSPBF536__) || defined(__ADSPBF534__)
#  define CCLK_VLEV_120	401000000
#  define CCLK_VLEV_125	401000000
# elif defined(__ADSPBF561__)
#  define CCLK_VLEV_120	300000000
#  define CCLK_VLEV_125	501000000
# endif
# if BFIN_CCLK < CCLK_VLEV_120
#  define CONFIG_VR_CTL_VLEV VLEV_120
# elif BFIN_CCLK < CCLK_VLEV_125
#  define CONFIG_VR_CTL_VLEV VLEV_125
# else
#  define CONFIG_VR_CTL_VLEV VLEV_130
# endif
# if defined(__ADSPBF52x__)	/* TBD; use default */
#  undef CONFIG_VR_CTL_VLEV
#  define CONFIG_VR_CTL_VLEV VLEV_110
# elif defined(__ADSPBF54x__)	/* TBD; use default */
#  undef CONFIG_VR_CTL_VLEV
#  define CONFIG_VR_CTL_VLEV VLEV_120
# endif

# ifdef CONFIG_BFIN_MAC
#  define CONFIG_VR_CTL_CLKBUF CLKBUFOE
# else
#  define CONFIG_VR_CTL_CLKBUF 0
# endif

# if defined(__ADSPBF52x__)
#  define CONFIG_VR_CTL_FREQ FREQ_1000
# else
#  define CONFIG_VR_CTL_FREQ (GAIN_20 | FREQ_1000)
# endif

# define CONFIG_VR_CTL_VAL (CONFIG_VR_CTL_CLKBUF | CONFIG_VR_CTL_VLEV | CONFIG_VR_CTL_FREQ)
#endif

__attribute__((saveall))
void initcode(ADI_BOOT_DATA *bootstruct)
{
	uint32_t old_baud = serial_init();

#ifdef CONFIG_HW_WATCHDOG
# ifndef CONFIG_HW_WATCHDOG_TIMEOUT_INITCODE
#  define CONFIG_HW_WATCHDOG_TIMEOUT_INITCODE 20000
# endif
	/* Program the watchdog with an initial timeout of ~20 seconds.
	 * Hopefully that should be long enough to load the u-boot LDR
	 * (from wherever) and then the common u-boot code can take over.
	 * In bypass mode, the start.S would have already set a much lower
	 * timeout, so don't clobber that.
	 */
	if (CONFIG_BFIN_BOOT_MODE != BFIN_BOOT_BYPASS) {
		bfin_write_WDOG_CNT(MSEC_TO_SCLK(CONFIG_HW_WATCHDOG_TIMEOUT_INITCODE));
		bfin_write_WDOG_CTL(0);
	}
#endif

	serial_putc('S');

	/* Blackfin bootroms use the SPI slow read opcode instead of the SPI
	 * fast read, so we need to slow down the SPI clock a lot more during
	 * boot.  Once we switch over to u-boot's SPI flash driver, we'll
	 * increase the speed appropriately.
	 */
	if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_SPI_MASTER)
#ifdef SPI0_BAUD
		bfin_write_SPI0_BAUD(CONFIG_SPI_BAUD_INITBLOCK);
#else
		bfin_write_SPI_BAUD(CONFIG_SPI_BAUD_INITBLOCK);
#endif

	serial_putc('B');

	/* Disable all peripheral wakeups except for the PLL event. */
#ifdef SIC_IWR0
	bfin_write_SIC_IWR0(1);
	bfin_write_SIC_IWR1(0);
# ifdef SIC_IWR2
	bfin_write_SIC_IWR2(0);
# endif
#elif defined(SICA_IWR0)
	bfin_write_SICA_IWR0(1);
	bfin_write_SICA_IWR1(0);
#else
	bfin_write_SIC_IWR(1);
#endif

	serial_putc('L');

	bfin_write_PLL_LOCKCNT(CONFIG_PLL_LOCKCNT_VAL);

	serial_putc('A');

	/* Only reprogram when needed to avoid triggering unnecessary
	 * PLL relock sequences.
	 */
	if (bfin_read_VR_CTL() != CONFIG_VR_CTL_VAL) {
		serial_putc('!');
		bfin_write_VR_CTL(CONFIG_VR_CTL_VAL);
		asm("idle;");
	}

	serial_putc('C');

	bfin_write_PLL_DIV(CONFIG_PLL_DIV_VAL);

	serial_putc('K');

	/* Only reprogram when needed to avoid triggering unnecessary
	 * PLL relock sequences.
	 */
	if (bfin_read_PLL_CTL() != CONFIG_PLL_CTL_VAL) {
		serial_putc('!');
		bfin_write_PLL_CTL(CONFIG_PLL_CTL_VAL);
		asm("idle;");
	}

	/* Since we've changed the SCLK above, we may need to update
	 * the UART divisors (UART baud rates are based on SCLK).
	 */
	serial_reset_baud(old_baud);

	serial_putc('F');

	/* Program the async banks controller. */
	bfin_write_EBIU_AMBCTL0(CONFIG_EBIU_AMBCTL0_VAL);
	bfin_write_EBIU_AMBCTL1(CONFIG_EBIU_AMBCTL1_VAL);
	bfin_write_EBIU_AMGCTL(CONFIG_EBIU_AMGCTL_VAL);

#ifdef EBIU_MODE
	/* Not all parts have these additional MMRs. */
	bfin_write_EBIU_MBSCTL(CONFIG_EBIU_MBSCTL_VAL);
	bfin_write_EBIU_MODE(CONFIG_EBIU_MODE_VAL);
	bfin_write_EBIU_FCTL(CONFIG_EBIU_FCTL_VAL);
#endif

	serial_putc('I');

	/* Program the external memory controller. */
#ifdef EBIU_RSTCTL
	bfin_write_EBIU_RSTCTL(bfin_read_EBIU_RSTCTL() | 0x1 /*DDRSRESET*/ | CONFIG_EBIU_RSTCTL_VAL);
	bfin_write_EBIU_DDRCTL0(CONFIG_EBIU_DDRCTL0_VAL);
	bfin_write_EBIU_DDRCTL1(CONFIG_EBIU_DDRCTL1_VAL);
	bfin_write_EBIU_DDRCTL2(CONFIG_EBIU_DDRCTL2_VAL);
# ifdef CONFIG_EBIU_DDRCTL3_VAL
	/* default is disable, so don't need to force this */
	bfin_write_EBIU_DDRCTL3(CONFIG_EBIU_DDRCTL3_VAL);
# endif
#else
	bfin_write_EBIU_SDRRC(CONFIG_EBIU_SDRRC_VAL);
	bfin_write_EBIU_SDBCTL(CONFIG_EBIU_SDBCTL_VAL);
	bfin_write_EBIU_SDGCTL(CONFIG_EBIU_SDGCTL_VAL);
#endif

	serial_putc('N');

	/* Restore all peripheral wakeups. */
#ifdef SIC_IWR0
	bfin_write_SIC_IWR0(-1);
	bfin_write_SIC_IWR1(-1);
# ifdef SIC_IWR2
	bfin_write_SIC_IWR2(-1);
# endif
#elif defined(SICA_IWR0)
	bfin_write_SICA_IWR0(-1);
	bfin_write_SICA_IWR1(-1);
#else
	bfin_write_SIC_IWR(-1);
#endif

	serial_putc('>');
	serial_putc('\n');

	serial_deinit();
}
