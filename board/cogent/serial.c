/*
 * Simple serial driver for Cogent motherboard serial ports
 * for use during boot
 */

#include <common.h>
#include <board/cogent/serial.h>

DECLARE_GLOBAL_DATA_PTR;

#if (CMA_MB_CAPS & CMA_MB_CAP_SERPAR)

#if (defined(CONFIG_8xx) && defined(CONFIG_8xx_CONS_NONE)) || \
     (defined(CONFIG_8260) && defined(CONFIG_CONS_NONE))

#if CONFIG_CONS_INDEX == 1
#define CMA_MB_SERIAL_BASE	CMA_MB_SERIALA_BASE
#elif CONFIG_CONS_INDEX == 2
#define CMA_MB_SERIAL_BASE	CMA_MB_SERIALB_BASE
#elif CONFIG_CONS_INDEX == 3 && (CMA_MB_CAPS & CMA_MB_CAP_SER2)
#define CMA_MB_SERIAL_BASE	CMA_MB_SER2A_BASE
#elif CONFIG_CONS_INDEX == 4 && (CMA_MB_CAPS & CMA_MB_CAP_SER2)
#define CMA_MB_SERIAL_BASE	CMA_MB_SER2B_BASE
#else
#error CONFIG_CONS_INDEX must be configured for Cogent motherboard serial
#endif

int serial_init (void)
{
	cma_mb_serial *mbsp = (cma_mb_serial *) CMA_MB_SERIAL_BASE;

	cma_mb_reg_write (&mbsp->ser_ier, 0x00);	/* turn off interrupts */
	serial_setbrg ();
	cma_mb_reg_write (&mbsp->ser_lcr, 0x03);	/* 8 data, 1 stop, no parity */
	cma_mb_reg_write (&mbsp->ser_mcr, 0x03);	/* RTS/DTR */
	cma_mb_reg_write (&mbsp->ser_fcr, 0x07);	/* Clear & enable FIFOs */

	return (0);
}

void serial_setbrg (void)
{
	cma_mb_serial *mbsp = (cma_mb_serial *) CMA_MB_SERIAL_BASE;
	unsigned int divisor;
	unsigned char lcr;

	if ((divisor = br_to_div (gd->baudrate)) == 0)
		divisor = DEFDIV;

	lcr = cma_mb_reg_read (&mbsp->ser_lcr);
	cma_mb_reg_write (&mbsp->ser_lcr, lcr | 0x80);	/* Access baud rate(set DLAB) */
	cma_mb_reg_write (&mbsp->ser_brl, divisor & 0xff);
	cma_mb_reg_write (&mbsp->ser_brh, (divisor >> 8) & 0xff);
	cma_mb_reg_write (&mbsp->ser_lcr, lcr);	/* unset DLAB */
}

void serial_putc (const char c)
{
	cma_mb_serial *mbsp = (cma_mb_serial *) CMA_MB_SERIAL_BASE;

	if (c == '\n')
		serial_putc ('\r');

	while ((cma_mb_reg_read (&mbsp->ser_lsr) & LSR_THRE) == 0);

	cma_mb_reg_write (&mbsp->ser_thr, c);
}

void serial_puts (const char *s)
{
	while (*s != '\0')
		serial_putc (*s++);
}

int serial_getc (void)
{
	cma_mb_serial *mbsp = (cma_mb_serial *) CMA_MB_SERIAL_BASE;

	while ((cma_mb_reg_read (&mbsp->ser_lsr) & LSR_DR) == 0);

	return ((int) cma_mb_reg_read (&mbsp->ser_rhr) & 0x7f);
}

int serial_tstc (void)
{
	cma_mb_serial *mbsp = (cma_mb_serial *) CMA_MB_SERIAL_BASE;

	return ((cma_mb_reg_read (&mbsp->ser_lsr) & LSR_DR) != 0);
}

#endif /* CONS_NONE */

#if defined(CONFIG_CMD_KGDB) && \
    defined(CONFIG_KGDB_NONE)

#if CONFIG_KGDB_INDEX == CONFIG_CONS_INDEX
#error Console and kgdb are on the same serial port - this is not supported
#endif

#if CONFIG_KGDB_INDEX == 1
#define CMA_MB_KGDB_SER_BASE	CMA_MB_SERIALA_BASE
#elif CONFIG_KGDB_INDEX == 2
#define CMA_MB_KGDB_SER_BASE	CMA_MB_SERIALB_BASE
#elif CONFIG_KGDB_INDEX == 3 && (CMA_MB_CAPS & CMA_MB_CAP_SER2)
#define CMA_MB_KGDB_SER_BASE	CMA_MB_SER2A_BASE
#elif CONFIG_KGDB_INDEX == 4 && (CMA_MB_CAPS & CMA_MB_CAP_SER2)
#define CMA_MB_KGDB_SER_BASE	CMA_MB_SER2B_BASE
#else
#error CONFIG_KGDB_INDEX must be configured for Cogent motherboard serial
#endif

void kgdb_serial_init (void)
{
	cma_mb_serial *mbsp = (cma_mb_serial *) CMA_MB_KGDB_SER_BASE;
	unsigned int divisor;

	if ((divisor = br_to_div (CONFIG_KGDB_BAUDRATE)) == 0)
		divisor = DEFDIV;

	cma_mb_reg_write (&mbsp->ser_ier, 0x00);	/* turn off interrupts */
	cma_mb_reg_write (&mbsp->ser_lcr, 0x80);	/* Access baud rate(set DLAB) */
	cma_mb_reg_write (&mbsp->ser_brl, divisor & 0xff);
	cma_mb_reg_write (&mbsp->ser_brh, (divisor >> 8) & 0xff);
	cma_mb_reg_write (&mbsp->ser_lcr, 0x03);	/* 8 data, 1 stop, no parity */
	cma_mb_reg_write (&mbsp->ser_mcr, 0x03);	/* RTS/DTR */
	cma_mb_reg_write (&mbsp->ser_fcr, 0x07);	/* Clear & enable FIFOs */

	printf ("[on cma10x serial port B] ");
}

void putDebugChar (int c)
{
	cma_mb_serial *mbsp = (cma_mb_serial *) CMA_MB_KGDB_SER_BASE;

	while ((cma_mb_reg_read (&mbsp->ser_lsr) & LSR_THRE) == 0);

	cma_mb_reg_write (&mbsp->ser_thr, c & 0xff);
}

void putDebugStr (const char *str)
{
	while (*str != '\0') {
		if (*str == '\n')
			putDebugChar ('\r');
		putDebugChar (*str++);
	}
}

int getDebugChar (void)
{
	cma_mb_serial *mbsp = (cma_mb_serial *) CMA_MB_KGDB_SER_BASE;

	while ((cma_mb_reg_read (&mbsp->ser_lsr) & LSR_DR) == 0);

	return ((int) cma_mb_reg_read (&mbsp->ser_rhr) & 0x7f);
}

void kgdb_interruptible (int yes)
{
	cma_mb_serial *mbsp = (cma_mb_serial *) CMA_MB_KGDB_SER_BASE;

	if (yes == 1) {
		printf ("kgdb: turning serial ints on\n");
		cma_mb_reg_write (&mbsp->ser_ier, 0xf);
	} else {
		printf ("kgdb: turning serial ints off\n");
		cma_mb_reg_write (&mbsp->ser_ier, 0x0);
	}
}

#endif /* KGDB && KGDB_NONE */

#endif /* CAPS & SERPAR */
