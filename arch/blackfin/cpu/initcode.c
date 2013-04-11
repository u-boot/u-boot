/*
 * initcode.c - Initialize the processor.  This is usually entails things
 * like external memory, voltage regulators, etc...  Note that this file
 * cannot make any function calls as it may be executed all by itself by
 * the Blackfin's bootrom in LDR format.
 *
 * Copyright (c) 2004-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#define BFIN_IN_INITCODE

#include <config.h>
#include <asm/blackfin.h>
#include <asm/mach-common/bits/watchdog.h>
#include <asm/mach-common/bits/bootrom.h>
#include <asm/mach-common/bits/core.h>
#include <asm/serial.h>

#define BUG() while (1) asm volatile("emuexcpt;");

#ifndef __ADSPBF60x__
#include <asm/mach-common/bits/ebiu.h>
#include <asm/mach-common/bits/pll.h>
#else /* __ADSPBF60x__ */
#include <asm/mach-common/bits/cgu.h>

#define CONFIG_BFIN_GET_DCLK_M \
	((CONFIG_CLKIN_HZ*CONFIG_VCO_MULT)/(CONFIG_DCLK_DIV*1000000))

#ifndef CONFIG_DMC_DDRCFG
#if ((CONFIG_BFIN_GET_DCLK_M != 125) && \
	(CONFIG_BFIN_GET_DCLK_M != 133) && \
	(CONFIG_BFIN_GET_DCLK_M != 150) && \
	(CONFIG_BFIN_GET_DCLK_M != 166) && \
	(CONFIG_BFIN_GET_DCLK_M != 200) && \
	(CONFIG_BFIN_GET_DCLK_M != 225) && \
	(CONFIG_BFIN_GET_DCLK_M != 250))
#error "DDR2 CLK must be in (125, 133, 150, 166, 200, 225, 250)MHz"
#endif
#endif

/* DMC control bits */
#define SRREQ			0x8

/* DMC status bits */
#define IDLE                    0x1
#define MEMINITDONE             0x4
#define SRACK                   0x8
#define PDACK                   0x10
#define DPDACK                  0x20
#define DLLCALDONE              0x2000
#define PENDREF                 0xF0000
#define PHYRDPHASE              0xF00000
#define PHYRDPHASE_OFFSET       20

/* DMC DLL control bits */
#define DLLCALRDCNT             0xFF
#define DATACYC_OFFSET          8

struct ddr_config {
	u32 ddr_clk;
	u32 dmc_ddrctl;
	u32 dmc_ddrcfg;
	u32 dmc_ddrtr0;
	u32 dmc_ddrtr1;
	u32 dmc_ddrtr2;
	u32 dmc_ddrmr;
	u32 dmc_ddrmr1;
};

static struct ddr_config ddr_config_table[] = {
	[0] = {
		.ddr_clk    = 125,	/* 125MHz */
		.dmc_ddrctl = 0x00000904,
		.dmc_ddrcfg = 0x00000422,
		.dmc_ddrtr0 = 0x20705212,
		.dmc_ddrtr1 = 0x201003CF,
		.dmc_ddrtr2 = 0x00320107,
		.dmc_ddrmr  = 0x00000422,
		.dmc_ddrmr1 = 0x4,
	},
	[1] = {
		.ddr_clk    = 133,	/* 133MHz */
		.dmc_ddrctl = 0x00000904,
		.dmc_ddrcfg = 0x00000422,
		.dmc_ddrtr0 = 0x20806313,
		.dmc_ddrtr1 = 0x2013040D,
		.dmc_ddrtr2 = 0x00320108,
		.dmc_ddrmr  = 0x00000632,
		.dmc_ddrmr1 = 0x4,
	},
	[2] = {
		.ddr_clk    = 150,	/* 150MHz */
		.dmc_ddrctl = 0x00000904,
		.dmc_ddrcfg = 0x00000422,
		.dmc_ddrtr0 = 0x20A07323,
		.dmc_ddrtr1 = 0x20160492,
		.dmc_ddrtr2 = 0x00320209,
		.dmc_ddrmr  = 0x00000632,
		.dmc_ddrmr1 = 0x4,
	},
	[3] = {
		.ddr_clk    = 166,	/* 166MHz */
		.dmc_ddrctl = 0x00000904,
		.dmc_ddrcfg = 0x00000422,
		.dmc_ddrtr0 = 0x20A07323,
		.dmc_ddrtr1 = 0x2016050E,
		.dmc_ddrtr2 = 0x00320209,
		.dmc_ddrmr  = 0x00000632,
		.dmc_ddrmr1 = 0x4,
	},
	[4] = {
		.ddr_clk    = 200,	/* 200MHz */
		.dmc_ddrctl = 0x00000904,
		.dmc_ddrcfg = 0x00000422,
		.dmc_ddrtr0 = 0x20a07323,
		.dmc_ddrtr1 = 0x2016050f,
		.dmc_ddrtr2 = 0x00320509,
		.dmc_ddrmr  = 0x00000632,
		.dmc_ddrmr1 = 0x4,
	},
	[5] = {
		.ddr_clk    = 225,	/* 225MHz */
		.dmc_ddrctl = 0x00000904,
		.dmc_ddrcfg = 0x00000422,
		.dmc_ddrtr0 = 0x20E0A424,
		.dmc_ddrtr1 = 0x302006DB,
		.dmc_ddrtr2 = 0x0032020D,
		.dmc_ddrmr  = 0x00000842,
		.dmc_ddrmr1 = 0x4,
	},
	[6] = {
		.ddr_clk    = 250,	/* 250MHz */
		.dmc_ddrctl = 0x00000904,
		.dmc_ddrcfg = 0x00000422,
		.dmc_ddrtr0 = 0x20E0A424,
		.dmc_ddrtr1 = 0x3020079E,
		.dmc_ddrtr2 = 0x0032050D,
		.dmc_ddrmr  = 0x00000842,
		.dmc_ddrmr1 = 0x4,
	},
};
#endif /* __ADSPBF60x__ */

__attribute__((always_inline))
static inline void serial_init(void)
{
	uint32_t uart_base = UART_BASE;

#if defined(__ADSPBF54x__) || defined(__ADSPBF60x__)
# ifdef BFIN_BOOT_UART_USE_RTS
#  define BFIN_UART_USE_RTS 1
# else
#  define BFIN_UART_USE_RTS 0
# endif
	if (BFIN_UART_USE_RTS && CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_UART) {
		size_t i;

		/* force RTS rather than relying on auto RTS */
#if BFIN_UART_HW_VER < 4
		bfin_write16(&pUART->mcr, bfin_read16(&pUART->mcr) | FCPOL);
#else
		bfin_write32(&pUART->control, bfin_read32(&pUART->control) |
				FCPOL);
#endif

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

#if CONFIG_BFIN_BOOT_MODE != BFIN_BOOT_BYPASS
	if (BFIN_DEBUG_EARLY_SERIAL) {
		serial_early_init(uart_base);
		serial_early_set_baud(uart_base, CONFIG_BAUDRATE);
	}
#endif
}

__attribute__((always_inline))
static inline void serial_deinit(void)
{
#if defined(__ADSPBF54x__) || defined(__ADSPBF60x__)
	uint32_t uart_base = UART_BASE;

	if (BFIN_UART_USE_RTS && CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_UART) {
		/* clear forced RTS rather than relying on auto RTS */
#if BFIN_UART_HW_VER < 4
		bfin_write16(&pUART->mcr, bfin_read16(&pUART->mcr) & ~FCPOL);
#else
		bfin_write32(&pUART->control, bfin_read32(&pUART->control) &
				~FCPOL);
#endif
	}
#endif
}

__attribute__((always_inline))
static inline void serial_putc(char c)
{
	uint32_t uart_base = UART_BASE;

	if (!BFIN_DEBUG_EARLY_SERIAL)
		return;

	if (c == '\n')
		serial_putc('\r');

	bfin_write(&pUART->thr, c);

	while (!(_lsr_read(pUART) & TEMT))
		continue;
}

#include "initcode.h"

__attribute__((always_inline)) static inline void
program_nmi_handler(void)
{
	u32 tmp1, tmp2;

	/* Older bootroms don't create a dummy NMI handler,
	 * so make one ourselves ASAP in case it fires.
	 */
	if (CONFIG_BFIN_BOOT_MODE != BFIN_BOOT_BYPASS && !ANOMALY_05000219)
		return;

	asm volatile (
		"%0 = RETS;" /* Save current RETS */
		"CALL 1f;"   /* Figure out current PC */
		"RTN;"       /* The simple NMI handler */
		"1:"
		"%1 = RETS;" /* Load addr of NMI handler */
		"RETS = %0;" /* Restore RETS */
		"[%2] = %1;" /* Write NMI handler */
		: "=d"(tmp1), "=d"(tmp2)
		: "ab"(EVT2)
	);
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

#ifdef __ADSPBF60x__

#ifndef CONFIG_CGU_CTL_VAL
# define CONFIG_CGU_CTL_VAL ((CONFIG_VCO_MULT << 8) | CONFIG_CLKIN_HALF)
#endif

#ifndef CONFIG_CGU_DIV_VAL
# define CONFIG_CGU_DIV_VAL \
	((CONFIG_CCLK_DIV   << CSEL_P)   | \
	 (CONFIG_SCLK0_DIV  << S0SEL_P)  | \
	 (CONFIG_SCLK_DIV << SYSSEL_P) | \
	 (CONFIG_SCLK1_DIV  << S1SEL_P)  | \
	 (CONFIG_DCLK_DIV   << DSEL_P)   | \
	 (CONFIG_OCLK_DIV   << OSEL_P))
#endif

#else /* __ADSPBF60x__ */

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

#if CONFIG_MEM_SIZE
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
#endif

/* Conflicting Column Address Widths Causes SDRAM Errors:
 * EB2CAW and EB3CAW must be the same
 */
#if ANOMALY_05000362
# if ((CONFIG_EBIU_SDBCTL_VAL & 0x30000000) >> 8) != (CONFIG_EBIU_SDBCTL_VAL & 0x00300000)
#  error "Anomaly 05000362: EB2CAW and EB3CAW must be the same"
# endif
#endif

#endif /*  __ADSPBF60x__ */

__attribute__((always_inline)) static inline void
program_early_devices(ADI_BOOT_DATA *bs, uint *sdivB, uint *divB, uint *vcoB)
{
	serial_putc('a');

	/* Save the clock pieces that are used in baud rate calculation */
	if (BFIN_DEBUG_EARLY_SERIAL || CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_UART) {
		serial_putc('b');
#ifdef __ADSPBF60x__
		*sdivB = bfin_read_CGU_DIV();
		*sdivB = ((*sdivB >> 8) & 0x1f) * ((*sdivB >> 5) & 0x7);
		*vcoB = (bfin_read_CGU_CTL() >> 8) & 0x7f;
#else
		*sdivB = bfin_read_PLL_DIV() & 0xf;
		*vcoB = (bfin_read_PLL_CTL() >> 9) & 0x3f;
#endif
		*divB = serial_early_get_div();
		serial_putc('c');
	}

	serial_putc('d');

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
		serial_putc('e');
#ifdef __ADSPBF60x__
		/* Reset system event controller */
		bfin_write_SEC_GCTL(0x2);
		bfin_write_SEC_CCTL(0x2);
		SSYNC();

		/* Enable fault event input and system reset action in fault
		 * controller. Route watchdog timeout event to fault interface.
		 */
		bfin_write_SEC_FCTL(0xc1);
		/* Enable watchdog interrupt source */
		bfin_write_SEC_SCTL(2, bfin_read_SEC_SCTL(2) | 0x6);
		SSYNC();

		/* Enable system event controller */
		bfin_write_SEC_GCTL(0x1);
		bfin_write_SEC_CCTL(0x1);
		SSYNC();
#endif
		bfin_write_WDOG_CTL(WDDIS);
		SSYNC();
		bfin_write_WDOG_CNT(MSEC_TO_SCLK(CONFIG_HW_WATCHDOG_TIMEOUT_INITCODE));
#if CONFIG_BFIN_BOOT_MODE != BFIN_BOOT_UART
		bfin_write_WDOG_CTL(WDEN);
#endif
		serial_putc('f');
	}
#endif

	serial_putc('g');

	/* Blackfin bootroms use the SPI slow read opcode instead of the SPI
	 * fast read, so we need to slow down the SPI clock a lot more during
	 * boot.  Once we switch over to u-boot's SPI flash driver, we'll
	 * increase the speed appropriately.
	 */
#ifdef SPI_BAUD
	if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_SPI_MASTER) {
		serial_putc('h');
		if (BOOTROM_SUPPORTS_SPI_FAST_READ && CONFIG_SPI_BAUD_INITBLOCK < 4)
			bs->dFlags |= BFLAG_FASTREAD;
		bfin_write_SPI_BAUD(CONFIG_SPI_BAUD_INITBLOCK);
		serial_putc('i');
	}
#endif

	serial_putc('j');
}

__attribute__((always_inline)) static inline bool
maybe_self_refresh(ADI_BOOT_DATA *bs)
{
	serial_putc('a');

	if (!CONFIG_MEM_SIZE)
		return false;

#ifdef __ADSPBF60x__
	/* resume from hibernate, return false let ddr initialize */
	if ((bfin_read32(DPM0_STAT) & 0xF0) == 0x50) {
		serial_putc('b');
		return false;
	}

#else /* __ADSPBF60x__ */

	/* If external memory is enabled, put it into self refresh first. */
#if defined(EBIU_RSTCTL)
	if (bfin_read_EBIU_RSTCTL() & DDR_SRESET) {
		serial_putc('b');
		bfin_write_EBIU_RSTCTL(bfin_read_EBIU_RSTCTL() | SRREQ);
		return true;
	}
#elif defined(EBIU_SDGCTL)
	if (bfin_read_EBIU_SDBCTL() & EBE) {
		serial_putc('b');
		bfin_write_EBIU_SDGCTL(bfin_read_EBIU_SDGCTL() | SRFS);
		return true;
	}
#endif

#endif /* __ADSPBF60x__ */
	serial_putc('c');

	return false;
}

__attribute__((always_inline)) static inline u16
program_clocks(ADI_BOOT_DATA *bs, bool put_into_srfs)
{
	u16 vr_ctl;

	serial_putc('a');

#ifdef __ADSPBF60x__
	if (bfin_read_DMC0_STAT() & MEMINITDONE) {
		bfin_write_DMC0_CTL(bfin_read_DMC0_CTL() | SRREQ);
		SSYNC();
		while (!(bfin_read_DMC0_STAT() & SRACK))
			continue;
	}

	/* Don't set the same value of MSEL and DF to CGU_CTL */
	if ((bfin_read_CGU_CTL() & (MSEL_MASK | DF_MASK))
			!= CONFIG_CGU_CTL_VAL) {
		bfin_write_CGU_DIV(CONFIG_CGU_DIV_VAL);
		bfin_write_CGU_CTL(CONFIG_CGU_CTL_VAL);
		while ((bfin_read_CGU_STAT() & (CLKSALGN | PLLBP)) ||
				!(bfin_read_CGU_STAT() & PLLLK))
			continue;
	}

	bfin_write_CGU_DIV(CONFIG_CGU_DIV_VAL | UPDT);
	while (bfin_read_CGU_STAT() & CLKSALGN)
		continue;

	if (bfin_read_DMC0_STAT() & MEMINITDONE) {
		bfin_write_DMC0_CTL(bfin_read_DMC0_CTL() & ~SRREQ);
		SSYNC();
		while (bfin_read_DMC0_STAT() & SRACK)
			continue;
	}

#else /* __ADSPBF60x__ */

	vr_ctl = bfin_read_VR_CTL();

	serial_putc('b');

	/* If we're entering self refresh, make sure it has happened. */
	if (put_into_srfs)
#if defined(EBIU_RSTCTL)
		while (!(bfin_read_EBIU_RSTCTL() & SRACK))
			continue;
#elif defined(EBIU_SDGCTL)
		while (!(bfin_read_EBIU_SDSTAT() & SDSRA))
			continue;
#else
		;
#endif

	serial_putc('c');

	/* With newer bootroms, we use the helper function to set up
	 * the memory controller.  Older bootroms lacks such helpers
	 * so we do it ourselves.
	 */
	if (!ANOMALY_05000386) {
		serial_putc('d');

		/* Always programming PLL_LOCKCNT avoids Anomaly 05000430 */
		ADI_SYSCTRL_VALUES memory_settings;
		uint32_t actions = SYSCTRL_WRITE | SYSCTRL_PLLCTL | SYSCTRL_LOCKCNT;
		if (!ANOMALY_05000440)
			actions |= SYSCTRL_PLLDIV;
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
		serial_putc('e');
		bfrom_SysControl(actions, &memory_settings, NULL);
		serial_putc('f');
		if (ANOMALY_05000440)
			bfin_write_PLL_DIV(CONFIG_PLL_DIV_VAL);
#if ANOMALY_05000432
		bfin_write_SIC_IWR1(-1);
#endif
#if ANOMALY_05000171
		bfin_write_SICA_IWR0(-1);
		bfin_write_SICA_IWR1(-1);
#endif
		serial_putc('g');
	} else {
		serial_putc('h');

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
#elif defined(SIC_IWR)
		bfin_write_SIC_IWR(1);
#endif

		serial_putc('i');

		/* Always programming PLL_LOCKCNT avoids Anomaly 05000430 */
		bfin_write_PLL_LOCKCNT(CONFIG_PLL_LOCKCNT_VAL);

		serial_putc('j');

		/* Only reprogram when needed to avoid triggering unnecessary
		 * PLL relock sequences.
		 */
		if (vr_ctl != CONFIG_VR_CTL_VAL) {
			serial_putc('?');
			bfin_write_VR_CTL(CONFIG_VR_CTL_VAL);
			asm("idle;");
			serial_putc('!');
		}

		serial_putc('k');

		bfin_write_PLL_DIV(CONFIG_PLL_DIV_VAL);

		serial_putc('l');

		/* Only reprogram when needed to avoid triggering unnecessary
		 * PLL relock sequences.
		 */
		if (ANOMALY_05000242 || bfin_read_PLL_CTL() != CONFIG_PLL_CTL_VAL) {
			serial_putc('?');
			bfin_write_PLL_CTL(CONFIG_PLL_CTL_VAL);
			asm("idle;");
			serial_putc('!');
		}

		serial_putc('m');

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
#elif defined(SIC_IWR)
		bfin_write_SIC_IWR(-1);
#endif

		serial_putc('n');
	}

#endif /* __ADSPBF60x__ */

	serial_putc('o');

	return vr_ctl;
}

__attribute__((always_inline)) static inline void
update_serial_clocks(ADI_BOOT_DATA *bs, uint sdivB, uint divB, uint vcoB)
{
	/* Since we've changed the SCLK above, we may need to update
	 * the UART divisors (UART baud rates are based on SCLK).
	 * Do the division by hand as there are no native instructions
	 * for dividing which means we'd generate a libgcc reference.
	 */
	unsigned int sdivR, vcoR;
	unsigned int dividend;
	unsigned int divisor;
	unsigned int quotient;

	serial_putc('a');

#ifdef __ADSPBF60x__
	sdivR = bfin_read_CGU_DIV();
	sdivR = ((sdivR >> 8) & 0x1f) * ((sdivR >> 5) & 0x7);
	vcoR = (bfin_read_CGU_CTL() >> 8) & 0x7f;
#else
	sdivR = bfin_read_PLL_DIV() & 0xf;
	vcoR = (bfin_read_PLL_CTL() >> 9) & 0x3f;
#endif

	dividend = sdivB * divB * vcoR;
	divisor = vcoB * sdivR;
	quotient = early_division(dividend, divisor);
	serial_early_put_div(quotient - ANOMALY_05000230);
	serial_putc('c');
}

__attribute__((always_inline)) static inline void
program_memory_controller(ADI_BOOT_DATA *bs, bool put_into_srfs)
{
	serial_putc('a');

	if (!CONFIG_MEM_SIZE)
		return;

	serial_putc('b');

#ifdef __ADSPBF60x__
	int dlldatacycle;
	int dll_ctl;
	int i = 0;

	if (CONFIG_BFIN_GET_DCLK_M ==  125)
		i = 0;
	else if (CONFIG_BFIN_GET_DCLK_M ==  133)
		i = 1;
	else if (CONFIG_BFIN_GET_DCLK_M ==  150)
		i = 2;
	else if (CONFIG_BFIN_GET_DCLK_M ==  166)
		i = 3;
	else if (CONFIG_BFIN_GET_DCLK_M ==  200)
		i = 4;
	else if (CONFIG_BFIN_GET_DCLK_M ==  225)
		i = 5;
	else if (CONFIG_BFIN_GET_DCLK_M ==  250)
		i = 6;

#if 0
	for (i = 0; i < ARRAY_SIZE(ddr_config_table); i++)
		if (CONFIG_BFIN_GET_DCLK_M == ddr_config_table[i].ddr_clk)
			break;
#endif

#ifndef CONFIG_DMC_DDRCFG
	bfin_write_DMC0_CFG(ddr_config_table[i].dmc_ddrcfg);
#else
	bfin_write_DMC0_CFG(CONFIG_DMC_DDRCFG);
#endif
#ifndef CONFIG_DMC_DDRTR0
	bfin_write_DMC0_TR0(ddr_config_table[i].dmc_ddrtr0);
#else
	bfin_write_DMC0_TR0(CONFIG_DMC_DDRTR0);
#endif
#ifndef CONFIG_DMC_DDRTR1
	bfin_write_DMC0_TR1(ddr_config_table[i].dmc_ddrtr1);
#else
	bfin_write_DMC0_TR1(CONFIG_DMC_DDRTR1);
#endif
#ifndef CONFIG_DMC_DDRTR2
	bfin_write_DMC0_TR2(ddr_config_table[i].dmc_ddrtr2);
#else
	bfin_write_DMC0_TR2(CONFIG_DMC_DDRTR2);
#endif
#ifndef CONFIG_DMC_DDRMR
	bfin_write_DMC0_MR(ddr_config_table[i].dmc_ddrmr);
#else
	bfin_write_DMC0_MR(CONFIG_DMC_DDRMR);
#endif
#ifndef CONFIG_DMC_DDREMR1
	bfin_write_DMC0_EMR1(ddr_config_table[i].dmc_ddrmr1);
#else
	bfin_write_DMC0_EMR1(CONFIG_DMC_DDREMR1);
#endif
#ifndef CONFIG_DMC_DDRCTL
	bfin_write_DMC0_CTL(ddr_config_table[i].dmc_ddrctl);
#else
	bfin_write_DMC0_CTL(CONFIG_DMC_DDRCTL);
#endif

	SSYNC();
	while (!(bfin_read_DMC0_STAT() & MEMINITDONE))
		continue;

	dlldatacycle = (bfin_read_DMC0_STAT() & PHYRDPHASE) >>
			PHYRDPHASE_OFFSET;
	dll_ctl = bfin_read_DMC0_DLLCTL();
	dll_ctl &= 0x0ff;
	bfin_write_DMC0_DLLCTL(dll_ctl | (dlldatacycle << DATACYC_OFFSET));

	SSYNC();
	while (!(bfin_read_DMC0_STAT() & DLLCALDONE))
		continue;
	serial_putc('!');

#else /* __ADSPBF60x__ */

	/* Program the external memory controller before we come out of
	 * self-refresh.  This only works with our SDRAM controller.
	 */
#ifdef EBIU_SDGCTL
# ifdef CONFIG_EBIU_SDRRC_VAL
	bfin_write_EBIU_SDRRC(CONFIG_EBIU_SDRRC_VAL);
# endif
# ifdef CONFIG_EBIU_SDBCTL_VAL
	bfin_write_EBIU_SDBCTL(CONFIG_EBIU_SDBCTL_VAL);
# endif
# ifdef CONFIG_EBIU_SDGCTL_VAL
	bfin_write_EBIU_SDGCTL(CONFIG_EBIU_SDGCTL_VAL);
# endif
#endif

	serial_putc('c');

	/* Now that we've reprogrammed, take things out of self refresh. */
	if (put_into_srfs)
#if defined(EBIU_RSTCTL)
		bfin_write_EBIU_RSTCTL(bfin_read_EBIU_RSTCTL() & ~(SRREQ));
#elif defined(EBIU_SDGCTL)
		bfin_write_EBIU_SDGCTL(bfin_read_EBIU_SDGCTL() & ~(SRFS));
#endif

	serial_putc('d');

	/* Our DDR controller sucks and cannot be programmed while in
	 * self-refresh.  So we have to pull it out before programming.
	 */
#ifdef EBIU_RSTCTL
# ifdef CONFIG_EBIU_RSTCTL_VAL
	bfin_write_EBIU_RSTCTL(bfin_read_EBIU_RSTCTL() | 0x1 /*DDRSRESET*/ | CONFIG_EBIU_RSTCTL_VAL);
# endif
# ifdef CONFIG_EBIU_DDRCTL0_VAL
	bfin_write_EBIU_DDRCTL0(CONFIG_EBIU_DDRCTL0_VAL);
# endif
# ifdef CONFIG_EBIU_DDRCTL1_VAL
	bfin_write_EBIU_DDRCTL1(CONFIG_EBIU_DDRCTL1_VAL);
# endif
# ifdef CONFIG_EBIU_DDRCTL2_VAL
	bfin_write_EBIU_DDRCTL2(CONFIG_EBIU_DDRCTL2_VAL);
# endif
# ifdef CONFIG_EBIU_DDRCTL3_VAL
	/* default is disable, so don't need to force this */
	bfin_write_EBIU_DDRCTL3(CONFIG_EBIU_DDRCTL3_VAL);
# endif
# ifdef CONFIG_EBIU_DDRQUE_VAL
	bfin_write_EBIU_DDRQUE(bfin_read_EBIU_DDRQUE() | CONFIG_EBIU_DDRQUE_VAL);
# endif
#endif

#endif /* __ADSPBF60x__ */
	serial_putc('e');
}

__attribute__((always_inline)) static inline void
check_hibernation(ADI_BOOT_DATA *bs, u16 vr_ctl, bool put_into_srfs)
{
	serial_putc('a');

	if (!CONFIG_MEM_SIZE)
		return;

	serial_putc('b');
#ifdef __ADSPBF60x__
	if (bfin_read32(DPM0_RESTORE0) != 0) {
		uint32_t reg = bfin_read_DMC0_CTL();
		reg &= ~0x8;
		bfin_write_DMC0_CTL(reg);

		while ((bfin_read_DMC0_STAT() & 0x8))
			continue;
		while (!(bfin_read_DMC0_STAT() & 0x1))
			continue;

		serial_putc('z');
		uint32_t *hibernate_magic = bfin_read32(DPM0_RESTORE4);
		SSYNC(); /* make sure memory controller is done */
		if (hibernate_magic[0] == 0xDEADBEEF) {
			serial_putc('c');
			SSYNC();
			bfin_write_EVT15(hibernate_magic[1]);
			bfin_write_IMASK(EVT_IVG15);
			__asm__ __volatile__ (
				/* load reti early to avoid anomaly 281 */
				"reti = %2;"
				/* clear hibernate magic */
				"[%0] = %1;"
				/* load stack pointer */
				"SP = [%0 + 8];"
				/* lower ourselves from reset ivg to ivg15 */
				"raise 15;"
				"nop;nop;nop;"
				"rti;"
				:
				: "p"(hibernate_magic),
				"d"(0x2000 /* jump.s 0 */),
				"d"(0xffa00000)
			);
		}


	}
#else
	/* Are we coming out of hibernate (suspend to memory) ?
	 * The memory layout is:
	 * 0x0: hibernate magic for anomaly 307 (0xDEADBEEF)
	 * 0x4: return address
	 * 0x8: stack pointer
	 *
	 * SCKELOW is unreliable on older parts (anomaly 307)
	 */
	if (ANOMALY_05000307 || vr_ctl & 0x8000) {
		uint32_t *hibernate_magic = 0;

		SSYNC();
		if (hibernate_magic[0] == 0xDEADBEEF) {
			serial_putc('c');
			bfin_write_EVT15(hibernate_magic[1]);
			bfin_write_IMASK(EVT_IVG15);
			__asm__ __volatile__ (
				/* load reti early to avoid anomaly 281 */
				"reti = %0;"
				/* clear hibernate magic */
				"[%0] = %1;"
				/* load stack pointer */
				"SP = [%0 + 8];"
				/* lower ourselves from reset ivg to ivg15 */
				"raise 15;"
				"rti;"
				:
				: "p"(hibernate_magic), "d"(0x2000 /* jump.s 0 */)
			);
		}
		serial_putc('d');
	}
#endif

	serial_putc('e');
}

BOOTROM_CALLED_FUNC_ATTR
void initcode(ADI_BOOT_DATA *bs)
{
	ADI_BOOT_DATA bootstruct_scratch;

	/* Setup NMI handler before anything else */
	program_nmi_handler();

	serial_init();

	serial_putc('A');

	/* If the bootstruct is NULL, then it's because we're loading
	 * dynamically and not via LDR (bootrom).  So set the struct to
	 * some scratch space.
	 */
	if (!bs)
		bs = &bootstruct_scratch;

	serial_putc('B');
	bool put_into_srfs = maybe_self_refresh(bs);

	serial_putc('C');
	uint sdivB, divB, vcoB;
	program_early_devices(bs, &sdivB, &divB, &vcoB);

	serial_putc('D');
	u16 vr_ctl = program_clocks(bs, put_into_srfs);

	serial_putc('E');
	update_serial_clocks(bs, sdivB, divB, vcoB);

	serial_putc('F');
	program_memory_controller(bs, put_into_srfs);

	serial_putc('G');
	check_hibernation(bs, vr_ctl, put_into_srfs);

	serial_putc('H');
	program_async_controller(bs);

#ifdef CONFIG_BFIN_BOOTROM_USES_EVT1
	serial_putc('I');
	/* Tell the bootrom where our entry point is so that it knows
	 * where to jump to when finishing processing the LDR.  This
	 * allows us to avoid small jump blocks in the LDR, and also
	 * works around anomaly 05000389 (init address in external
	 * memory causes bootrom to trigger external addressing IVHW).
	 */
	if (CONFIG_BFIN_BOOT_MODE != BFIN_BOOT_BYPASS)
		bfin_write_EVT1(CONFIG_SYS_MONITOR_BASE);
#endif

	serial_putc('>');
	serial_putc('\n');

	serial_deinit();
}
