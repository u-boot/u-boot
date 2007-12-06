/*
 * SuperH SCIF device driver.
 * Copyright (c) 2007 Nobuhiro Iwamatsu
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
#include <asm/processor.h>

#ifdef CFG_SCIF_CONSOLE

#if defined (CONFIG_CONS_SCIF0)
#define SCIF_BASE	SCIF0_BASE
#elif defined (CONFIG_CONS_SCIF1)
#define SCIF_BASE	SCIF1_BASE
#else
#error "Default SCIF doesn't set....."
#endif

#define SCSMR 	(vu_short *)(SCIF_BASE + 0x0)
#define SCBRR 	(vu_char  *)(SCIF_BASE + 0x4) 
#define SCSCR 	(vu_short *)(SCIF_BASE + 0x8)
#define SCFTDR 	(vu_char  *)(SCIF_BASE + 0xC)
#define SCFSR 	(vu_short *)(SCIF_BASE + 0x10)
#define SCFRDR 	(vu_char  *)(SCIF_BASE + 0x14)
#define SCFCR 	(vu_short *)(SCIF_BASE + 0x18)
#define SCFDR 	(vu_short *)(SCIF_BASE + 0x1C)
#if defined(CONFIG_SH4A)
#define SCRFDR	(vu_short *)(SCIF_BASE + 0x20)
#define SCSPTR	(vu_short *)(SCIF_BASE + 0x24)
#define SCLSR   (vu_short *)(SCIF_BASE + 0x28)
#define SCRER	(vu_short *)(SCIF_BASE + 0x2C)
#elif defined (CONFIG_SH4)
#define SCSPTR 	(vu_short *)(SCIF_BASE + 0x20)
#define SCLSR 	(vu_short *)(SCIF_BASE + 0x24)
#elif defined (CONFIG_SH3)
#define SCLSR 	(vu_short *)(SCIF_BASE + 0x24)
#endif

#define SCR_RE 		(1 << 4)
#define SCR_TE 		(1 << 5) 
#define FCR_RFRST	(1 << 1) /* RFCL */
#define FCR_TFRST	(1 << 2) /* TFCL */
#define FSR_DR   	(1 << 0)
#define FSR_RDF  	(1 << 1)
#define FSR_FER  	(1 << 3)
#define FSR_BRK  	(1 << 4)
#define FSR_FER  	(1 << 3)
#define FSR_TEND 	(1 << 6)
#define FSR_ER   	(1 << 7)

/*----------------------------------------------------------------------*/

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	int divisor = gd->baudrate * 32;

	*SCBRR = (CONFIG_SYS_CLK_FREQ + (divisor / 2)) / 
						(gd->baudrate * 32) - 1;
}

int serial_init (void)
{
	*SCSCR = (SCR_RE | SCR_TE);
	*SCSMR = 0 ;
	*SCSMR = 0;
	*SCFCR = (FCR_RFRST | FCR_TFRST);
	*SCFCR;
	*SCFCR = 0;

	serial_setbrg();
	return 0;
}

static int serial_tx_fifo_level (void)
{
	return (*SCFDR >> 8) & 0x1F;
}

static int serial_rx_fifo_level (void)
{
	return (*SCFDR >> 0) & 0x1F;
}

void serial_raw_putc (const char c)
{
	unsigned int fsr_bits_to_clear;

	while (1) {
		if (*SCFSR & FSR_TEND) {		/* Tx fifo is empty */
			fsr_bits_to_clear = FSR_TEND;
			break;
		}
	}

	*SCFTDR = c;
	if (fsr_bits_to_clear != 0)
		*SCFSR &= ~fsr_bits_to_clear;
}

void serial_putc (const char c)
{
	if (c == '\n')
		serial_raw_putc ('\r');
	serial_raw_putc (c);
}

void serial_puts (const char *s)
{
	char c;
	while ((c = *s++) != 0)
		serial_putc (c);
}

int serial_tstc (void)
{
	return serial_rx_fifo_level() ? 1 : 0;
}

#define FSR_ERR_CLEAR   0x0063
#define RDRF_CLEAR      0x00fc
#define LSR_ORER        1
void handle_error( void ){

	(void)*SCFSR ;
	*SCFSR = FSR_ERR_CLEAR ;
	(void)*SCLSR ;
	*SCLSR = 0x00 ;
}

int serial_getc_check( void ){
	unsigned short status;

	status = *SCFSR ;

	if (status & (FSR_FER | FSR_FER | FSR_ER | FSR_BRK))
		handle_error();
	if( *SCLSR & LSR_ORER )
		handle_error();
	return (status & ( FSR_DR | FSR_RDF ));
}

int serial_getc (void)
{
	unsigned short status ;
	char ch;
	while(!serial_getc_check());

	ch = *SCFRDR;
	status =  *SCFSR ;

	*SCFSR = RDRF_CLEAR ;

	if (status & (FSR_FER | FSR_FER | FSR_ER | FSR_BRK))
		handle_error();

	if( *SCLSR & LSR_ORER )
		handle_error();

	return ch ;
}

#endif	/* CFG_SCIF_CONSOLE */
