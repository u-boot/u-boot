/*
 * SuperH SCIF device driver.
 * Copyright (c) 2007,2008 Nobuhiro Iwamatsu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>

#if defined(CONFIG_CONS_SCIF0)
# define SCIF_BASE	SCIF0_BASE
#elif defined(CONFIG_CONS_SCIF1)
# define SCIF_BASE	SCIF1_BASE
#elif defined(CONFIG_CONS_SCIF2)
# define SCIF_BASE	SCIF2_BASE
#elif defined(CONFIG_CONS_SCIF3)
# define SCIF_BASE	SCIF3_BASE
#elif defined(CONFIG_CONS_SCIF4)
# define SCIF_BASE	SCIF4_BASE
#elif defined(CONFIG_CONS_SCIF5)
# define SCIF_BASE	SCIF5_BASE
#else
# error "Default SCIF doesn't set....."
#endif

/* Base register */
#define SCSMR	(vu_short *)(SCIF_BASE + 0x0)
#define SCBRR	(vu_char  *)(SCIF_BASE + 0x4)
#define SCSCR	(vu_short *)(SCIF_BASE + 0x8)
#define SCFCR	(vu_short *)(SCIF_BASE + 0x18)
#define SCFDR	(vu_short *)(SCIF_BASE + 0x1C)
#if defined(CONFIG_CPU_SH7720) || \
	(defined(CONFIG_CPU_SH7723) && defined(CONFIG_SCIF_A))
# define SCFSR	(vu_short *)(SCIF_BASE + 0x14)	/* SCSSR */
# define SCFTDR	(vu_char  *)(SCIF_BASE + 0x20)
# define SCFRDR	(vu_char  *)(SCIF_BASE + 0x24)
#else
# define SCFTDR (vu_char  *)(SCIF_BASE + 0xC)
# define SCFSR	(vu_short *)(SCIF_BASE + 0x10)
# define SCFRDR (vu_char  *)(SCIF_BASE + 0x14)
#endif

#if	defined(CONFIG_CPU_SH7780) || \
	defined(CONFIG_CPU_SH7785)
# define SCRFDR	(vu_short *)(SCIF_BASE + 0x20)
# define SCSPTR	(vu_short *)(SCIF_BASE + 0x24)
# define SCLSR	(vu_short *)(SCIF_BASE + 0x28)
# define SCRER	(vu_short *)(SCIF_BASE + 0x2C)
# define LSR_ORER	1
# define FIFOLEVEL_MASK	0xFF
#elif defined(CONFIG_CPU_SH7763)
# if defined(CONFIG_CONS_SCIF2)
# define SCSPTR	(vu_short *)(SCIF_BASE + 0x20)
# define SCLSR	(vu_short *)(SCIF_BASE + 0x24)
# define LSR_ORER	1
# define FIFOLEVEL_MASK	0x1F
# else
# define SCRFDR	(vu_short *)(SCIF_BASE + 0x20)
# define SCSPTR	(vu_short *)(SCIF_BASE + 0x24)
# define SCLSR	(vu_short *)(SCIF_BASE + 0x28)
# define SCRER	(vu_short *)(SCIF_BASE + 0x2C)
# define LSR_ORER	1
# define FIFOLEVEL_MASK	0xFF
# endif
#elif defined(CONFIG_CPU_SH7723)
# if defined(CONFIG_SCIF_A)
# define SCLSR	SCFSR
# define LSR_ORER	0x0200
# define FIFOLEVEL_MASK	0x3F
#else
# define SCLSR	(vu_short *)(SCIF_BASE + 0x24)
# define LSR_ORER	1
# define FIFOLEVEL_MASK	0x1F
#endif
#elif defined(CONFIG_CPU_SH7750) || \
	defined(CONFIG_CPU_SH7751) || \
	defined(CONFIG_CPU_SH7722) || \
	defined(CONFIG_CPU_SH7203)
# define SCSPTR	(vu_short *)(SCIF_BASE + 0x20)
# define SCLSR	(vu_short *)(SCIF_BASE + 0x24)
# define LSR_ORER	1
# define FIFOLEVEL_MASK	0x1F
#elif defined(CONFIG_CPU_SH7720)
# define SCLSR		SCFSR
# define LSR_ORER	0x0200
# define FIFOLEVEL_MASK	0x1F
#elif defined(CONFIG_CPU_SH7710) || \
	defined(CONFIG_CPU_SH7712)
# define SCLSR	SCFSR		/* SCSSR */
# define LSR_ORER	1
# define FIFOLEVEL_MASK	0x1F
#endif

/* SCBRR register value setting */
#if defined(CONFIG_CPU_SH7720)
# define SCBRR_VALUE(bps, clk) (((clk * 2) + 16 * bps) / (32 * bps) - 1)
#elif defined(CONFIG_CPU_SH7723) && defined(CONFIG_SCIF_A)
/* SH7723 SCIFA use bus clock. So clock *2 */
# define SCBRR_VALUE(bps, clk) (((clk * 2 * 2) + 16 * bps) / (32 * bps) - 1)
#else /* Generic SuperH */
# define SCBRR_VALUE(bps, clk) ((clk + 16 * bps) / (32 * bps) - 1)
#endif

#define SCR_RE		(1 << 4)
#define SCR_TE		(1 << 5)
#define FCR_RFRST	(1 << 1)	/* RFCL */
#define FCR_TFRST	(1 << 2)	/* TFCL */
#define FSR_DR		(1 << 0)
#define FSR_RDF		(1 << 1)
#define FSR_FER		(1 << 3)
#define FSR_BRK		(1 << 4)
#define FSR_FER		(1 << 3)
#define FSR_TEND	(1 << 6)
#define FSR_ER		(1 << 7)

/*----------------------------------------------------------------------*/

void serial_setbrg(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	writeb(SCBRR_VALUE(gd->baudrate, CONFIG_SYS_CLK_FREQ), SCBRR);
}

int serial_init(void)
{
	writew((SCR_RE | SCR_TE), SCSCR);
	writew(0, SCSMR);
	writew(0, SCSMR);
	writew((FCR_RFRST | FCR_TFRST), SCFCR);
	readw(SCFCR);
	writew(0, SCFCR);

	serial_setbrg();
	return 0;
}

static int serial_rx_fifo_level(void)
{
#if defined(SCRFDR)
	return (readw(SCRFDR) >> 0) & FIFOLEVEL_MASK;
#else
	return (readw(SCFDR) >> 0) & FIFOLEVEL_MASK;
#endif
}

void serial_raw_putc(const char c)
{
	unsigned int fsr_bits_to_clear;

	while (1) {
		if (readw(SCFSR) & FSR_TEND) { /* Tx fifo is empty */
			fsr_bits_to_clear = FSR_TEND;
			break;
		}
	}

	writeb(c, SCFTDR);
	if (fsr_bits_to_clear != 0)
		writew(readw(SCFSR) & ~fsr_bits_to_clear, SCFSR);
}

void serial_putc(const char c)
{
	if (c == '\n')
		serial_raw_putc('\r');
	serial_raw_putc(c);
}

void serial_puts(const char *s)
{
	char c;
	while ((c = *s++) != 0)
		serial_putc(c);
}

int serial_tstc(void)
{
	return serial_rx_fifo_level() ? 1 : 0;
}

#define FSR_ERR_CLEAR	0x0063
#define RDRF_CLEAR		0x00fc
void handle_error(void)
{
	readw(SCFSR);
	writew(FSR_ERR_CLEAR, SCFSR);
	readw(SCLSR);
	writew(0x00, SCLSR);
}

int serial_getc_check(void)
{
	unsigned short status;

	status = readw(SCFSR);

	if (status & (FSR_FER | FSR_ER | FSR_BRK))
		handle_error();
	if (readw(SCLSR) & LSR_ORER)
		handle_error();
	return status & (FSR_DR | FSR_RDF);
}

int serial_getc(void)
{
	unsigned short status;
	char ch;

	while (!serial_getc_check())
		;

	ch = readb(SCFRDR);
	status = readw(SCFSR);

	writew(RDRF_CLEAR, SCFSR);

	if (status & (FSR_FER | FSR_FER | FSR_ER | FSR_BRK))
		handle_error();

	if (readw(SCLSR) & LSR_ORER)
		handle_error();

	return ch;
}
