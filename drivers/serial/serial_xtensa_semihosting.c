// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <dm.h>
#include <malloc.h>
#include <serial.h>

#include <asm/platform/simcall.h>

/**
 * struct simc_serial_priv - Semihosting serial private data
 * @counter: Counter used to fake pending every other call
 */
struct simc_serial_priv {
	unsigned int counter;
};

static int simc_serial_getc(struct udevice *dev)
{
	char ch = 0;

	simc_read(0, &ch, sizeof(ch));

	return ch;
}

static int simc_serial_putc(struct udevice *dev, const char ch)
{
	char str[2] = {0};

	str[0] = ch;
	simc_write(1, str, 1);

	return 0;
}

static int simc_serial_pending(struct udevice *dev, bool input)
{
	struct simc_serial_priv *priv = dev_get_priv(dev);

	if (input) {
		int res = simc_poll(0);
		return res < 0 ? priv->counter++ & 1 : res;
	}

	return false;
}

static ssize_t smh_serial_puts(struct udevice *dev, const char *s, size_t len)
{
	int ret;

	ret = simc_write(1, s, len);

	return ret;
}

static const struct dm_serial_ops simc_serial_ops = {
	.putc = simc_serial_putc,
	.puts = smh_serial_puts,
	.getc = simc_serial_getc,
	.pending = simc_serial_pending,
};

U_BOOT_DRIVER(simc_serial) = {
	.name	= "serial_xtensa_semihosting",
	.id	= UCLASS_SERIAL,
	.priv_auto = sizeof(struct simc_serial_priv),
	.ops	= &simc_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

U_BOOT_DRVINFO(simc_serial) = {
	.name = "serial_xtensa_semihosting",
};

#if CONFIG_IS_ENABLED(DEBUG_UART_XTENSA_SEMIHOSTING)
#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
}

static inline void _debug_uart_putc(int c)
{
	simc_serial_putc(NULL, c);
}

DEBUG_UART_FUNCS
#endif
