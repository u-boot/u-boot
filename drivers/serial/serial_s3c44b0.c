/*
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
 *
 * (C) Copyright 2002-2004
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
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
 *
 */

#include <common.h>
#include <asm/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

/* flush serial input queue. returns 0 on success or negative error
 * number otherwise
 */
static int serial_flush_input(void)
{
	volatile u32 tmp;

	/* keep on reading as long as the receiver is not empty */
	while(UTRSTAT0&0x01) {
		tmp = REGB(URXH0);
	}

	return 0;
}


/* flush output queue. returns 0 on success or negative error number
 * otherwise
 */
static int serial_flush_output(void)
{
	/* wait until the transmitter is no longer busy */
	while(!(UTRSTAT0 & 0x02)) {
	}

	return 0;
}


static void s3c44b0_serial_setbrg(void)
{
	u32 divisor = 0;

	/* get correct divisor */
	switch(gd->baudrate) {

	case 1200:
#if CONFIG_S3C44B0_CLOCK_SPEED==66
		divisor = 3124;
#elif CONFIG_S3C44B0_CLOCK_SPEED==75
		divisor = 3905;
#else
# error CONFIG_S3C44B0_CLOCK_SPEED undefined
#endif
		break;

	case 9600:
#if CONFIG_S3C44B0_CLOCK_SPEED==66
		divisor = 390;
#elif CONFIG_S3C44B0_CLOCK_SPEED==75
		divisor = 487;
#else
# error CONFIG_S3C44B0_CLOCK_SPEED undefined
#endif
		break;

	case 19200:
#if CONFIG_S3C44B0_CLOCK_SPEED==66
		divisor = 194;
#elif CONFIG_S3C44B0_CLOCK_SPEED==75
		divisor = 243;
#else
# error CONFIG_S3C44B0_CLOCK_SPEED undefined
#endif
		break;

	case 38400:
#if CONFIG_S3C44B0_CLOCK_SPEED==66
		divisor = 97;
#elif CONFIG_S3C44B0_CLOCK_SPEED==75
		divisor = 121;
#else
# error CONFIG_S3C44B0_CLOCK_SPEED undefined
#endif	/* break; */

	case 57600:
#if CONFIG_S3C44B0_CLOCK_SPEED==66
		divisor = 64;
#elif CONFIG_S3C44B0_CLOCK_SPEED==75
		divisor = 80;
#else
# error CONFIG_S3C44B0_CLOCK_SPEED undefined
#endif	/* break; */

	case 115200:
#if CONFIG_S3C44B0_CLOCK_SPEED==66
		divisor = 32;
#elif CONFIG_S3C44B0_CLOCK_SPEED==75
		divisor = 40;
#else
# error CONFIG_S3C44B0_CLOCK_SPEED undefined
#endif	/* break; */
	}

	serial_flush_output();
	serial_flush_input();
	UFCON0 = 0x0;
	ULCON0 = 0x03;
	UCON0 = 0x05;
	UBRDIV0 = divisor;

	UFCON1 = 0x0;
	ULCON1 = 0x03;
	UCON1 = 0x05;
	UBRDIV1 = divisor;

	for(divisor=0; divisor<100; divisor++) {
		/* NOP */
	}
}


/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
static int s3c44b0_serial_init(void)
{
	serial_setbrg ();

	return (0);
}


/*
 * Output a single byte to the serial port.
 */
static void s3c44b0_serial_putc(const char c)
{
	/* wait for room in the transmit FIFO */
	while(!(UTRSTAT0 & 0x02));

	UTXH0 = (unsigned char)c;

	/*
		to be polite with serial console add a line feed
		to the carriage return character
	*/
	if (c=='\n')
		serial_putc('\r');
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
static int s3c44b0_serial_tstc(void)
{
	return (UTRSTAT0 & 0x01);
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
static int s3c44b0_serial_getc(void)
{
	int rv;

	for(;;) {
		rv = s3c44b0_serial_tstc();

		if(rv > 0)
			return URXH0;
	}
}

static struct serial_device s3c44b0_serial_drv = {
	.name	= "s3c44b0_serial",
	.start	= s3c44b0_serial_init,
	.stop	= NULL,
	.setbrg	= s3c44b0_serial_setbrg,
	.putc	= s3c44b0_serial_putc,
	.puts	= default_serial_puts,
	.getc	= s3c44b0_serial_getc,
	.tstc	= s3c44b0_serial_tstc,
};

void s3c44b0_serial_initialize(void)
{
	serial_register(&s3c44b0_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &s3c44b0_serial_drv;
}
