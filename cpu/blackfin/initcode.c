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
static inline void serial_init(void)
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

	if (BFIN_DEBUG_EARLY_SERIAL) {
		int ucen = *pUART_GCTL & UCEN;
		serial_early_init();

		/* If the UART is off, that means we need to program
		 * the baud rate ourselves initially.
		 */
		if (ucen != UCEN)
			serial_early_set_baud(CONFIG_BAUDRATE);
	}
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


/* Max SCLK can be 133MHz ... dividing that by (2*4) gives
 * us a freq of 16MHz for SPI which should generally be
 * slow enough for the slow reads the bootrom uses.
 */
#if !defined(CONFIG_SPI_FLASH_SLOW_READ) && \
    ((defined(__ADSPBF52x__) && __SILICON_REVISION__ >= 2) || \
     (defined(__ADSPBF54x__) && __SILICON_REVISION__ >= 1))
# define BOOTROM_SUPPORTS_SPI_FAST_READ 1
#else
# define BOOTROM_SUPPORTS_SPI_FAST_READ 0
#endif
#ifndef CONFIG_SPI_BAUD_INITBLOCK
# define CONFIG_SPI_BAUD_INITBLOCK (BOOTROM_SUPPORTS_SPI_FAST_READ ? 2 : 4)
#endif
#ifdef SPI0_BAUD
# define bfin_write_SPI_BAUD bfin_write_SPI0_BAUD
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
# define CONFIG_PLL_CTL_VAL (SPORT_HYST | (CONFIG_VCO_MULT << 9) | CONFIG_CLKIN_HALF)
#endif

#ifndef CONFIG_EBIU_RSTCTL_VAL
# define CONFIG_EBIU_RSTCTL_VAL 0 /* only MDDRENABLE is useful */
#endif
#if ((CONFIG_EBIU_RSTCTL_VAL & 0xFFFFFFC4) != 0)
# error invalid EBIU_RSTCTL value: must not set reserved bits
#endif

#ifndef CONFIG_EBIU_MBSCTL_VAL
# define CONFIG_EBIU_MBSCTL_VAL 0
#endif

#if defined(CONFIG_EBIU_DDRQUE_VAL) && ((CONFIG_EBIU_DDRQUE_VAL & 0xFFFF8000) != 0)
# error invalid EBIU_DDRQUE value: must not set reserved bits
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
# elif defined(__ADSPBF538__) || defined(__ADSPBF539__)	/* TBD; use default */
#  undef CONFIG_VR_CTL_VLEV
#  define CONFIG_VR_CTL_VLEV VLEV_125
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

/* some parts do not have an on-chip voltage regulator */
#if defined(__ADSPBF51x__)
# define CONFIG_HAS_VR 0
# undef CONFIG_VR_CTL_VAL
# define CONFIG_VR_CTL_VAL 0
#else
# define CONFIG_HAS_VR 1
#endif

#ifndef EBIU_RSTCTL
/* Blackfin with SDRAM */
#ifndef CONFIG_EBIU_SDBCTL_VAL
# if CONFIG_MEM_SIZE == 16
#  define CONFIG_EBSZ_VAL EBSZ_16
# elif CONFIG_MEM_SIZE == 32
#  define CONFIG_EBSZ_VAL EBSZ_32
# elif CONFIG_MEM_SIZE == 64
#  define CONFIG_EBSZ_VAL EBSZ_64
# elif CONFIG_MEM_SIZE == 128
#  define CONFIG_EBSZ_VAL EBSZ_128
# elif CONFIG_MEM_SIZE == 256
#  define CONFIG_EBSZ_VAL EBSZ_256
# elif CONFIG_MEM_SIZE == 512
#  define CONFIG_EBSZ_VAL EBSZ_512
# else
#  error You need to define CONFIG_EBIU_SDBCTL_VAL or CONFIG_MEM_SIZE
# endif
# if CONFIG_MEM_ADD_WDTH == 8
#  define CONFIG_EBCAW_VAL EBCAW_8
# elif CONFIG_MEM_ADD_WDTH == 9
#  define CONFIG_EBCAW_VAL EBCAW_9
# elif CONFIG_MEM_ADD_WDTH == 10
#  define CONFIG_EBCAW_VAL EBCAW_10
# elif CONFIG_MEM_ADD_WDTH == 11
#  define CONFIG_EBCAW_VAL EBCAW_11
# else
#  error You need to define CONFIG_EBIU_SDBCTL_VAL or CONFIG_MEM_ADD_WDTH
# endif
# define CONFIG_EBIU_SDBCTL_VAL (CONFIG_EBCAW_VAL | CONFIG_EBSZ_VAL | EBE)
#endif
#endif

BOOTROM_CALLED_FUNC_ATTR
void initcode(ADI_BOOT_DATA *bootstruct)
{
	/* Save the clock pieces that are used in baud rate calculation */
	unsigned int sdivB, divB, vcoB;
	serial_init();
	if (BFIN_DEBUG_EARLY_SERIAL || CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_UART) {
		sdivB = bfin_read_PLL_DIV() & 0xf;
		vcoB = (bfin_read_PLL_CTL() >> 9) & 0x3f;
		divB = serial_early_get_div();
	}

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
	if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_SPI_MASTER) {
		if (BOOTROM_SUPPORTS_SPI_FAST_READ && CONFIG_SPI_BAUD_INITBLOCK < 4)
			bootstruct->dFlags |= BFLAG_FASTREAD;
		bfin_write_SPI_BAUD(CONFIG_SPI_BAUD_INITBLOCK);
	}

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

	/* With newer bootroms, we use the helper function to set up
	 * the memory controller.  Older bootroms lacks such helpers
	 * so we do it ourselves.
	 */
#define BOOTROM_CAPS_SYSCONTROL 0
	if (BOOTROM_CAPS_SYSCONTROL) {
		serial_putc('S');

		ADI_SYSCTRL_VALUES memory_settings;
		uint32_t actions = SYSCTRL_WRITE | SYSCTRL_PLLCTL | SYSCTRL_PLLDIV | SYSCTRL_LOCKCNT;
		if (CONFIG_HAS_VR) {
			actions |= SYSCTRL_VRCTL;
			if (CONFIG_VR_CTL_VAL & FREQ_MASK)
				actions |= SYSCTRL_INTVOLTAGE;
			else
				actions |= SYSCTRL_EXTVOLTAGE;
			memory_settings.uwVrCtl = CONFIG_VR_CTL_VAL;
		} else
			actions |= SYSCTRL_EXTVOLTAGE;
		memory_settings.uwPllCtl = CONFIG_PLL_CTL_VAL;
		memory_settings.uwPllDiv = CONFIG_PLL_DIV_VAL;
		memory_settings.uwPllLockCnt = CONFIG_PLL_LOCKCNT_VAL;
#if ANOMALY_05000432
		bfin_write_SIC_IWR1(0);
#endif
		bfrom_SysControl(actions, &memory_settings, NULL);
#if ANOMALY_05000432
		bfin_write_SIC_IWR1(-1);
#endif
	} else {
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
	}

	/* Since we've changed the SCLK above, we may need to update
	 * the UART divisors (UART baud rates are based on SCLK).
	 * Do the division by hand as there are no native instructions
	 * for dividing which means we'd generate a libgcc reference.
	 */
	if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_UART) {
		unsigned int sdivR, vcoR;
		sdivR = bfin_read_PLL_DIV() & 0xf;
		vcoR = (bfin_read_PLL_CTL() >> 9) & 0x3f;
		int dividend = sdivB * divB * vcoR;
		int divisor = vcoB * sdivR;
		unsigned int quotient;
		for (quotient = 0; dividend > 0; ++quotient)
			dividend -= divisor;
		serial_early_put_div(quotient - ANOMALY_05000230);
	}

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

	/* tell the bootrom where our entry point is */
	if (CONFIG_BFIN_BOOT_MODE != BFIN_BOOT_BYPASS)
		bfin_write_EVT1(CONFIG_SYS_MONITOR_BASE);

	serial_putc('>');
	serial_putc('\n');

	serial_deinit();
}
