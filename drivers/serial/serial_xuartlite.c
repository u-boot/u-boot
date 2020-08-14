// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 - 2015 Michal Simek <monstr@monstr.eu>
 * Clean driver and add xilinx constant from header file
 *
 * (C) Copyright 2004 Atmark Techno, Inc.
 * Yasushi SHOJI <yashi@atmark-techno.com>
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>

#define SR_TX_FIFO_FULL		BIT(3) /* transmit FIFO full */
#define SR_TX_FIFO_EMPTY	BIT(2) /* transmit FIFO empty */
#define SR_RX_FIFO_VALID_DATA	BIT(0) /* data in receive FIFO */
#define SR_RX_FIFO_FULL		BIT(1) /* receive FIFO full */

#define ULITE_CONTROL_RST_TX	0x01
#define ULITE_CONTROL_RST_RX	0x02

struct uartlite {
	unsigned int rx_fifo;
	unsigned int tx_fifo;
	unsigned int status;
	unsigned int control;
};

struct uartlite_platdata {
	struct uartlite *regs;
	const struct uartlite_reg_ops *reg_ops;
};

struct uartlite_reg_ops {
	u32 (*in)(void __iomem *addr);
	void (*out)(u32 val, void __iomem *addr);
};

static u32 uartlite_inle32(void __iomem *addr)
{
	return in_le32(addr);
}

static void uartlite_outle32(u32 val, void __iomem *addr)
{
	out_le32(addr, val);
}

static const struct uartlite_reg_ops uartlite_le = {
	.in = uartlite_inle32,
	.out = uartlite_outle32,
};

static u32 uartlite_inbe32(void __iomem *addr)
{
	return in_be32(addr);
}

static void uartlite_outbe32(u32 val, void __iomem *addr)
{
	out_be32(addr, val);
}

static const struct uartlite_reg_ops uartlite_be = {
	.in = uartlite_inbe32,
	.out = uartlite_outbe32,
};

static u32 uart_in32(void __iomem *addr, struct uartlite_platdata *plat)
{
	return plat->reg_ops->in(addr);
}

static void uart_out32(void __iomem *addr, u32 val,
		       struct uartlite_platdata *plat)
{
	plat->reg_ops->out(val, addr);
}

static int uartlite_serial_putc(struct udevice *dev, const char ch)
{
	struct uartlite_platdata *plat = dev_get_platdata(dev);
	struct uartlite *regs = plat->regs;

	if (uart_in32(&regs->status, plat) & SR_TX_FIFO_FULL)
		return -EAGAIN;

	uart_out32(&regs->tx_fifo, ch & 0xff, plat);

	return 0;
}

static int uartlite_serial_getc(struct udevice *dev)
{
	struct uartlite_platdata *plat = dev_get_platdata(dev);
	struct uartlite *regs = plat->regs;

	if (!(uart_in32(&regs->status, plat) & SR_RX_FIFO_VALID_DATA))
		return -EAGAIN;

	return uart_in32(&regs->rx_fifo, plat) & 0xff;
}

static int uartlite_serial_pending(struct udevice *dev, bool input)
{
	struct uartlite_platdata *plat = dev_get_platdata(dev);
	struct uartlite *regs = plat->regs;

	if (input)
		return uart_in32(&regs->status, plat) & SR_RX_FIFO_VALID_DATA;

	return !(uart_in32(&regs->status, plat) & SR_TX_FIFO_EMPTY);
}

static int uartlite_serial_probe(struct udevice *dev)
{
	struct uartlite_platdata *plat = dev_get_platdata(dev);
	struct uartlite *regs = plat->regs;
	int ret;

	plat->reg_ops = &uartlite_be;
	ret = uart_in32(&regs->control, plat);
	uart_out32(&regs->control, 0, plat);
	uart_out32(&regs->control,
		   ULITE_CONTROL_RST_RX | ULITE_CONTROL_RST_TX, plat);
	ret = uart_in32(&regs->status, plat);
	/* Endianness detection */
	if ((ret & SR_TX_FIFO_EMPTY) != SR_TX_FIFO_EMPTY)
		plat->reg_ops = &uartlite_le;

	return 0;
}

static int uartlite_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct uartlite_platdata *plat = dev_get_platdata(dev);

	plat->regs = (struct uartlite *)devfdt_get_addr(dev);

	return 0;
}

static const struct dm_serial_ops uartlite_serial_ops = {
	.putc = uartlite_serial_putc,
	.pending = uartlite_serial_pending,
	.getc = uartlite_serial_getc,
};

static const struct udevice_id uartlite_serial_ids[] = {
	{ .compatible = "xlnx,opb-uartlite-1.00.b", },
	{ .compatible = "xlnx,xps-uartlite-1.00.a" },
	{ }
};

U_BOOT_DRIVER(serial_uartlite) = {
	.name	= "serial_uartlite",
	.id	= UCLASS_SERIAL,
	.of_match = uartlite_serial_ids,
	.ofdata_to_platdata = uartlite_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct uartlite_platdata),
	.probe = uartlite_serial_probe,
	.ops	= &uartlite_serial_ops,
};

#ifdef CONFIG_DEBUG_UART_UARTLITE

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	struct uartlite *regs = (struct uartlite *)CONFIG_DEBUG_UART_BASE;

	out_be32(&regs->control, 0);
	out_be32(&regs->control, ULITE_CONTROL_RST_RX | ULITE_CONTROL_RST_TX);
	in_be32(&regs->control);
}

static inline void _debug_uart_putc(int ch)
{
	struct uartlite *regs = (struct uartlite *)CONFIG_DEBUG_UART_BASE;

	while (in_be32(&regs->status) & SR_TX_FIFO_FULL)
		;

	out_be32(&regs->tx_fifo, ch & 0xff);
}

DEBUG_UART_FUNCS
#endif
