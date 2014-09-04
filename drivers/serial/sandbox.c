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
#include <dm.h>
#include <fdtdec.h>
#include <lcd.h>
#include <os.h>
#include <serial.h>
#include <linux/compiler.h>
#include <asm/state.h>

DECLARE_GLOBAL_DATA_PTR;

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

static int sandbox_serial_probe(struct udevice *dev)
{
	struct sandbox_state *state = state_get_current();

	if (state->term_raw != STATE_TERM_COOKED)
		os_tty_raw(0, state->term_raw == STATE_TERM_RAW_WITH_SIGS);

	return 0;
}

static int sandbox_serial_putc(struct udevice *dev, const char ch)
{
	os_write(1, &ch, 1);

	return 0;
}

static unsigned int increment_buffer_index(unsigned int index)
{
	return (index + 1) % ARRAY_SIZE(serial_buf);
}

static int sandbox_serial_pending(struct udevice *dev, bool input)
{
	const unsigned int next_index =
		increment_buffer_index(serial_buf_write);
	ssize_t count;

	if (!input)
		return 0;

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

static int sandbox_serial_getc(struct udevice *dev)
{
	int result;

	if (!sandbox_serial_pending(dev, true))
		return -EAGAIN;	/* buffer empty */

	result = serial_buf[serial_buf_read];
	serial_buf_read = increment_buffer_index(serial_buf_read);
	return result;
}

static const struct dm_serial_ops sandbox_serial_ops = {
	.putc = sandbox_serial_putc,
	.pending = sandbox_serial_pending,
	.getc = sandbox_serial_getc,
};

static const struct udevice_id sandbox_serial_ids[] = {
	{ .compatible = "sandbox,serial" },
	{ }
};

U_BOOT_DRIVER(serial_sandbox) = {
	.name	= "serial_sandbox",
	.id	= UCLASS_SERIAL,
	.of_match = sandbox_serial_ids,
	.probe = sandbox_serial_probe,
	.ops	= &sandbox_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DEVICE(serial_sandbox_non_fdt) = {
	.name = "serial_sandbox",
};
