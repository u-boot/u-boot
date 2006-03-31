#include <common.h>
#include <ns16550.h>
#include "short_types.h"
#include "memio.h"
#include "articiaS.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CFG_NS16550
static uint32 ComPort1;

uint16 SerialEcho = 1;


#define RECEIVER_HOLDING 0
#define TRANSMITTER_HOLDING 0
#define INTERRUPT_ENABLE 1
#define INTERRUPT_STATUS 2
#define FIFO_CONTROL 2
#define LINE_CONTROL 3
#define MODEM_CONTROL 4
#define LINE_STATUS 5
#define MODEM_STATUS 6
#define SCRATCH_PAD 7

#define DIVISOR_LATCH_LSB 0
#define DIVISOR_LATCH_MSB 1
#define PRESCALER_DIVISION 5

#define COM_WRITE_BYTE(reg, byte) out_byte((ComPort1+reg), byte)
#define COM_READ_BYTE(reg) in_byte((ComPort1+reg))

static int serial_init_done = 0;

void serial_init (void)
{
#if 0
	uint32 clock_divisor = 115200 / baudrate;
	uint8 cfg;
	uint8 a;
	uint16 devfn = 7 << 3;

	if (serial_init_done)
		return;

	/*  Enter configuration mode */
	cfg = pci_read_cfg_byte (0, devfn, 0x85);
	pci_write_cfg_byte (0, devfn, 0x85, cfg | 0x02);

	/* Set serial port COM1 as 3F8 */
	out_byte (0x3F0, 0xE7);
	out_byte (0x3f1, 0xfe);

	/* Set serial port COM2 as 2F8 */
	out_byte (0x3f0, 0xe8);
	out_byte (0x3f1, 0xeb);

	/* Enable */
	out_byte (0x3f0, 0xe2);
	a = in_byte (0x3f1);
	a |= 0xc;
	out_byte (0x3f0, 0xe2);
	out_byte (0x3f1, a);

	/*  Reset the configuration mode */
	pci_write_cfg_byte (0, devfn, 0x85, cfg);
#endif

	ComPort1 = 0x3F8;

	/*  Disable interrupts */
	COM_WRITE_BYTE (INTERRUPT_ENABLE, 0x00);

	/*  Set baud rate */
	/* COM_WRITE_BYTE(LINE_CONTROL, 0x83); */
	/* COM_WRITE_BYTE(DIVISOR_LATCH_LSB, (uint8)(clock_divisor & 0xFF)); */
	/* COM_WRITE_BYTE(DIVISOR_LATCH_MSB, (uint8)(clock_divisor >> 8)); */
	/* __asm("eieio"); */

	/*  Set 8-N-1 */
	COM_WRITE_BYTE (LINE_CONTROL, 0x03);
	__asm ("eieio");

	/*  Disable FIFO */
	COM_WRITE_BYTE (MODEM_CONTROL, 0x03);
	COM_WRITE_BYTE (FIFO_CONTROL, 0x07);

	__asm ("eieio");
	serial_init_done = 1;
}

extern int console_changed;

void serial_putc (const char sendme)
{
	if (sendme == '\n') {
		while ((in_byte (0x3FD) & 0x40) == 0);
		out_byte (0x3f8, 0x0D);
	}

	while ((in_byte (0x3FD) & 0x40) == 0);
	out_byte (0x3f8, sendme);
}

int serial_getc (void)
{
#if 0
	uint8 c;

	for (;;) {
		uint8 x = in_byte (0x3FD);

		if (x & 0x01)
			break;

		if (x & 0x0C)
			out_byte (0x3fd, 0x0c);
	}

	c = in_byte (0x3F8);

	return c;
#else
	while ((in_byte (0x3FD) & 0x01) == 0) {
		if (console_changed != 0) {
			printf ("Console changed\n");
			console_changed = 0;
			return 0;
		}
	}
	return in_byte (0x3F8);
#endif
}

int serial_tstc (void)
{
	return (in_byte (0x03FD) & 0x01) != 0;
}

void serial_debug_putc (int c)
{
	serial_puts ("DBG");
	serial_putc (c);
	serial_putc (0x0d);
	serial_putc (0x0A);
}

#else

const NS16550_t Com0 = (NS16550_t) CFG_NS16550_COM1;
const NS16550_t Com1 = (NS16550_t) CFG_NS16550_COM2;

int serial_init (void)
{
	uint32 clock_divisor = 115200 / gd->baudrate;

	NS16550_init (Com0, clock_divisor);
	/* NS16550_reinit(Com1, clock_divisor); */
	/* serial_puts("COM1: 3F8h initalized"); */

	return (0);
}

#if 0
void serial_putc (const char c)
{
	NS16550_putc (Com0, c);
	if (c == '\n')
		NS16550_putc (Com0, 0x0D);
}

int serial_getc (void)
{
	return (int) NS16550_getc (Com0);
}

int serial_tstc (void)
{
	return NS16550_tstc (Com0);
}
#else
void serial_putc (const char sendme)
{
	if (sendme == '\n') {
		while ((in_byte (0x3FD) & 0x40) == 0);
		out_byte (0x3f8, 0x0D);
	}

	while ((in_byte (0x3FD) & 0x40) == 0);
	out_byte (0x3f8, sendme);
}


extern int console_changed;

int serial_getc (void)
{
#if 0
	uint8 c;

	for (;;) {
		uint8 x = in_byte (0x3FD);

		if (x & 0x01)
			break;

		if (x & 0x0C)
			out_byte (0x3fd, 0x0c);
	}

	c = in_byte (0x3F8);

	return c;
#else
	while ((in_byte (0x3FD) & 0x01) == 0) {
		if (console_changed != 0) {
			console_changed = 0;
			return 0;
		}
	}

	return in_byte (0x3F8);
#endif
}

int serial_tstc (void)
{
	return (in_byte (0x03FD) & 0x01) != 0;
}
#endif

#endif

void serial_puts (const char *string)
{
	while (*string)
		serial_putc (*string++);
}

void serial_setbrg (void)
{
	uint32 clock_divisor = 115200 / gd->baudrate;

	NS16550_init (Com0, clock_divisor);
}
