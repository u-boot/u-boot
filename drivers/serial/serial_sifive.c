// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Anup Patel <anup@brainfault.org>
 */

#include <clk.h>
#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_TXFIFO_FULL	0x80000000
#define UART_RXFIFO_EMPTY	0x80000000
#define UART_RXFIFO_DATA	0x000000ff
#define UART_TXCTRL_TXEN	0x1
#define UART_RXCTRL_RXEN	0x1

struct uart_sifive {
	u32 txfifo;
	u32 rxfifo;
	u32 txctrl;
	u32 rxctrl;
	u32 ie;
	u32 ip;
	u32 div;
};

struct sifive_uart_platdata {
	unsigned int clock;
	int saved_input_char;
	struct uart_sifive *regs;
};

/* Set up the baud rate in gd struct */
static void _sifive_serial_setbrg(struct uart_sifive *regs,
				  unsigned long clock, unsigned long baud)
{
	writel((u32)((clock / baud) - 1), &regs->div);
}

static void _sifive_serial_init(struct uart_sifive *regs)
{
	writel(UART_TXCTRL_TXEN, &regs->txctrl);
	writel(UART_RXCTRL_RXEN, &regs->rxctrl);
	writel(0, &regs->ie);
}

static int _sifive_serial_putc(struct uart_sifive *regs, const char c)
{
	if (readl(&regs->txfifo) & UART_TXFIFO_FULL)
		return -EAGAIN;

	writel(c, &regs->txfifo);

	return 0;
}

static int _sifive_serial_getc(struct uart_sifive *regs)
{
	int ch = readl(&regs->rxfifo);

	if (ch & UART_RXFIFO_EMPTY)
		return -EAGAIN;
	ch &= UART_RXFIFO_DATA;

	return (!ch) ? -EAGAIN : ch;
}

static int sifive_serial_setbrg(struct udevice *dev, int baudrate)
{
	int err;
	struct clk clk;
	struct sifive_uart_platdata *platdata = dev_get_platdata(dev);

	err = clk_get_by_index(dev, 0, &clk);
	if (!err) {
		err = clk_get_rate(&clk);
		if (!IS_ERR_VALUE(err))
			platdata->clock = err;
	} else if (err != -ENOENT && err != -ENODEV && err != -ENOSYS) {
		debug("SiFive UART failed to get clock\n");
		return err;
	}

	if (!platdata->clock)
		platdata->clock = dev_read_u32_default(dev, "clock-frequency", 0);
	if (!platdata->clock) {
		debug("SiFive UART clock not defined\n");
		return -EINVAL;
	}

	_sifive_serial_setbrg(platdata->regs, platdata->clock, baudrate);

	return 0;
}

static int sifive_serial_probe(struct udevice *dev)
{
	struct sifive_uart_platdata *platdata = dev_get_platdata(dev);

	/* No need to reinitialize the UART after relocation */
	if (gd->flags & GD_FLG_RELOC)
		return 0;

	platdata->saved_input_char = 0;
	_sifive_serial_init(platdata->regs);

	return 0;
}

static int sifive_serial_getc(struct udevice *dev)
{
	int c;
	struct sifive_uart_platdata *platdata = dev_get_platdata(dev);
	struct uart_sifive *regs = platdata->regs;

	if (platdata->saved_input_char > 0) {
		c = platdata->saved_input_char;
		platdata->saved_input_char = 0;
		return c;
	}

	while ((c = _sifive_serial_getc(regs)) == -EAGAIN) ;

	return c;
}

static int sifive_serial_putc(struct udevice *dev, const char ch)
{
	int rc;
	struct sifive_uart_platdata *platdata = dev_get_platdata(dev);

	while ((rc = _sifive_serial_putc(platdata->regs, ch)) == -EAGAIN) ;

	return rc;
}

static int sifive_serial_pending(struct udevice *dev, bool input)
{
	struct sifive_uart_platdata *platdata = dev_get_platdata(dev);
	struct uart_sifive *regs = platdata->regs;

	if (input) {
		if (platdata->saved_input_char > 0)
			return 1;
		platdata->saved_input_char = _sifive_serial_getc(regs);
		return (platdata->saved_input_char > 0) ? 1 : 0;
	} else {
		return !!(readl(&regs->txfifo) & UART_TXFIFO_FULL);
	}
}

static int sifive_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct sifive_uart_platdata *platdata = dev_get_platdata(dev);

	platdata->regs = (struct uart_sifive *)dev_read_addr(dev);
	if (IS_ERR(platdata->regs))
		return PTR_ERR(platdata->regs);

	return 0;
}

static const struct dm_serial_ops sifive_serial_ops = {
	.putc = sifive_serial_putc,
	.getc = sifive_serial_getc,
	.pending = sifive_serial_pending,
	.setbrg = sifive_serial_setbrg,
};

static const struct udevice_id sifive_serial_ids[] = {
	{ .compatible = "sifive,uart0" },
	{ }
};

U_BOOT_DRIVER(serial_sifive) = {
	.name	= "serial_sifive",
	.id	= UCLASS_SERIAL,
	.of_match = sifive_serial_ids,
	.ofdata_to_platdata = sifive_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct sifive_uart_platdata),
	.probe = sifive_serial_probe,
	.ops	= &sifive_serial_ops,
};

#ifdef CONFIG_DEBUG_UART_SIFIVE
static inline void _debug_uart_init(void)
{
	struct uart_sifive *regs =
			(struct uart_sifive *)CONFIG_DEBUG_UART_BASE;

	_sifive_serial_setbrg(regs, CONFIG_DEBUG_UART_CLOCK,
			      CONFIG_BAUDRATE);
	_sifive_serial_init(regs);
}

static inline void _debug_uart_putc(int ch)
{
	struct uart_sifive *regs =
			(struct uart_sifive *)CONFIG_DEBUG_UART_BASE;

	while (_sifive_serial_putc(regs, ch) == -EAGAIN)
		WATCHDOG_RESET();
}

DEBUG_UART_FUNCS

#endif
