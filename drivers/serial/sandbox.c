/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * This provide a test serial port. It provides an emulated serial port where
 * a test program and read out the serial output and inject serial input for
 * U-Boot.
 */

#include <common.h>
#include <lcd.h>
#include <os.h>
#include <serial.h>
#include <linux/compiler.h>
#include <asm/state.h>

/*
 *
 *   serial_buf: A buffer that holds keyboard characters for the
 *		 Sandbox U-boot.
 *
 * invariants:
 *   serial_buf_write		 == serial_buf_read -> empty buffer
 *   (serial_buf_write + 1) % 16 == serial_buf_read -> full buffer
 */
static char serial_buf[16];
static unsigned int serial_buf_write;
static unsigned int serial_buf_read;

static int sandbox_serial_init(void)
{
	struct sandbox_state *state = state_get_current();

	if (state->term_raw != STATE_TERM_COOKED)
		os_tty_raw(0, state->term_raw == STATE_TERM_RAW_WITH_SIGS);
	return 0;
}

static void sandbox_serial_setbrg(void)
{
}

static void sandbox_serial_putc(const char ch)
{
	os_write(1, &ch, 1);
}

static void sandbox_serial_puts(const char *str)
{
	os_write(1, str, strlen(str));
}

static unsigned int increment_buffer_index(unsigned int index)
{
	return (index + 1) % ARRAY_SIZE(serial_buf);
}

static int sandbox_serial_tstc(void)
{
	const unsigned int next_index =
		increment_buffer_index(serial_buf_write);
	ssize_t count;

	os_usleep(100);
#ifdef CONFIG_LCD
	lcd_sync();
#endif
	if (next_index == serial_buf_read)
		return 1;	/* buffer full */

	count = os_read_no_block(0, &serial_buf[serial_buf_write], 1);
	if (count == 1)
		serial_buf_write = next_index;
	return serial_buf_write != serial_buf_read;
}

static int sandbox_serial_getc(void)
{
	int result;

	while (!sandbox_serial_tstc())
		;	/* buffer empty */

	result = serial_buf[serial_buf_read];
	serial_buf_read = increment_buffer_index(serial_buf_read);
	return result;
}

static struct serial_device sandbox_serial_drv = {
	.name	= "sandbox_serial",
	.start	= sandbox_serial_init,
	.stop	= NULL,
	.setbrg	= sandbox_serial_setbrg,
	.putc	= sandbox_serial_putc,
	.puts	= sandbox_serial_puts,
	.getc	= sandbox_serial_getc,
	.tstc	= sandbox_serial_tstc,
};

void sandbox_serial_initialize(void)
{
	serial_register(&sandbox_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &sandbox_serial_drv;
}
