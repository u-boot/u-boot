/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <watchdog.h>
#include <asm/io.h>
#include <serial.h>
#include <linux/compiler.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>

#define US1_TDRE	(1 << 7)
#define US1_RDRF	(1 << 5)
#define US1_OR		(1 << 3)
#define UC2_TE		(1 << 3)
#define UC2_RE		(1 << 2)
#define CFIFO_TXFLUSH	(1 << 7)
#define CFIFO_RXFLUSH	(1 << 6)
#define SFIFO_RXOF	(1 << 2)
#define SFIFO_RXUF	(1 << 0)

#define STAT_LBKDIF	(1 << 31)
#define STAT_RXEDGIF	(1 << 30)
#define STAT_TDRE	(1 << 23)
#define STAT_RDRF	(1 << 21)
#define STAT_IDLE	(1 << 20)
#define STAT_OR		(1 << 19)
#define STAT_NF		(1 << 18)
#define STAT_FE		(1 << 17)
#define STAT_PF		(1 << 16)
#define STAT_MA1F	(1 << 15)
#define STAT_MA2F	(1 << 14)
#define STAT_FLAGS	(STAT_LBKDIF | STAT_RXEDGIF | STAT_IDLE | STAT_OR | \
			 STAT_NF | STAT_FE | STAT_PF | STAT_MA1F | STAT_MA2F)

#define CTRL_TE		(1 << 19)
#define CTRL_RE		(1 << 18)

#define FIFO_TXFE		0x80
#define FIFO_RXFE		0x40

#define WATER_TXWATER_OFF	1
#define WATER_RXWATER_OFF	16

DECLARE_GLOBAL_DATA_PTR;

struct lpuart_serial_platdata {
	struct lpuart_fsl *reg;
};

#ifndef CONFIG_LPUART_32B_REG
static void _lpuart_serial_setbrg(struct lpuart_fsl *base, int baudrate)
{
	u32 clk = mxc_get_clock(MXC_UART_CLK);
	u16 sbr;

	sbr = (u16)(clk / (16 * baudrate));

	/* place adjustment later - n/32 BRFA */
	__raw_writeb(sbr >> 8, &base->ubdh);
	__raw_writeb(sbr & 0xff, &base->ubdl);
}

static int _lpuart_serial_getc(struct lpuart_fsl *base)
{
	while (!(__raw_readb(&base->us1) & (US1_RDRF | US1_OR)))
		WATCHDOG_RESET();

	barrier();

	return __raw_readb(&base->ud);
}

static void _lpuart_serial_putc(struct lpuart_fsl *base, const char c)
{
	while (!(__raw_readb(&base->us1) & US1_TDRE))
		WATCHDOG_RESET();

	__raw_writeb(c, &base->ud);
}

/* Test whether a character is in the RX buffer */
static int _lpuart_serial_tstc(struct lpuart_fsl *base)
{
	if (__raw_readb(&base->urcfifo) == 0)
		return 0;

	return 1;
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static int _lpuart_serial_init(struct lpuart_fsl *base)
{
	u8 ctrl;

	ctrl = __raw_readb(&base->uc2);
	ctrl &= ~UC2_RE;
	ctrl &= ~UC2_TE;
	__raw_writeb(ctrl, &base->uc2);

	__raw_writeb(0, &base->umodem);
	__raw_writeb(0, &base->uc1);

	/* Disable FIFO and flush buffer */
	__raw_writeb(0x0, &base->upfifo);
	__raw_writeb(0x0, &base->utwfifo);
	__raw_writeb(0x1, &base->urwfifo);
	__raw_writeb(CFIFO_TXFLUSH | CFIFO_RXFLUSH, &base->ucfifo);

	/* provide data bits, parity, stop bit, etc */
	_lpuart_serial_setbrg(base, gd->baudrate);

	__raw_writeb(UC2_RE | UC2_TE, &base->uc2);

	return 0;
}

static int lpuart_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;

	_lpuart_serial_setbrg(reg, baudrate);

	return 0;
}

static int lpuart_serial_getc(struct udevice *dev)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;

	return _lpuart_serial_getc(reg);
}

static int lpuart_serial_putc(struct udevice *dev, const char c)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;

	_lpuart_serial_putc(reg, c);

	return 0;
}

static int lpuart_serial_pending(struct udevice *dev, bool input)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;

	if (input)
		return _lpuart_serial_tstc(reg);
	else
		return __raw_readb(&reg->us1) & US1_TDRE ? 0 : 1;
}

static int lpuart_serial_probe(struct udevice *dev)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;

	return _lpuart_serial_init(reg);
}
#else

u32 __weak get_lpuart_clk(void)
{
	return CONFIG_SYS_CLK_FREQ;
}

static void _lpuart32_serial_setbrg(struct lpuart_fsl *base, int baudrate)
{
	u32 clk = get_lpuart_clk();
	u32 sbr;

	sbr = (clk / (16 * baudrate));

	/* place adjustment later - n/32 BRFA */
	out_be32(&base->baud, sbr);
}

static int _lpuart32_serial_getc(struct lpuart_fsl *base)
{
	u32 stat;

	while (((stat = in_be32(&base->stat)) & STAT_RDRF) == 0) {
		out_be32(&base->stat, STAT_FLAGS);
		WATCHDOG_RESET();
	}

	return in_be32(&base->data) & 0x3ff;
}

static void _lpuart32_serial_putc(struct lpuart_fsl *base, const char c)
{
	while (!(in_be32(&base->stat) & STAT_TDRE))
		WATCHDOG_RESET();

	out_be32(&base->data, c);
}

/* Test whether a character is in the RX buffer */
static int _lpuart32_serial_tstc(struct lpuart_fsl *base)
{
	if ((in_be32(&base->water) >> 24) == 0)
		return 0;

	return 1;
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static int _lpuart32_serial_init(struct lpuart_fsl *base)
{
	u8 ctrl;

	ctrl = in_be32(&base->ctrl);
	ctrl &= ~CTRL_RE;
	ctrl &= ~CTRL_TE;
	out_be32(&base->ctrl, ctrl);

	out_be32(&base->modir, 0);
	out_be32(&base->fifo, ~(FIFO_TXFE | FIFO_RXFE));

	out_be32(&base->match, 0);

	/* provide data bits, parity, stop bit, etc */
	_lpuart32_serial_setbrg(base, gd->baudrate);

	out_be32(&base->ctrl, CTRL_RE | CTRL_TE);

	return 0;
}

static int lpuart32_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;

	_lpuart32_serial_setbrg(reg, baudrate);

	return 0;
}

static int lpuart32_serial_getc(struct udevice *dev)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;

	return _lpuart32_serial_getc(reg);
}

static int lpuart32_serial_putc(struct udevice *dev, const char c)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;

	_lpuart32_serial_putc(reg, c);

	return 0;
}

static int lpuart32_serial_pending(struct udevice *dev, bool input)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;

	if (input)
		return _lpuart32_serial_tstc(reg);
	else
		return in_be32(&reg->stat) & STAT_TDRE ? 0 : 1;
}

static int lpuart32_serial_probe(struct udevice *dev)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;

	return _lpuart32_serial_init(reg);
}
#endif /* CONFIG_LPUART_32B_REG */

static int lpuart_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	fdt_addr_t addr;

	addr = dev_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->reg = (struct lpuart_fsl *)addr;

	return 0;
}

#ifndef CONFIG_LPUART_32B_REG
static const struct dm_serial_ops lpuart_serial_ops = {
	.putc = lpuart_serial_putc,
	.pending = lpuart_serial_pending,
	.getc = lpuart_serial_getc,
	.setbrg = lpuart_serial_setbrg,
};

static const struct udevice_id lpuart_serial_ids[] = {
	{ .compatible = "fsl,vf610-lpuart" },
	{ }
};

U_BOOT_DRIVER(serial_lpuart) = {
	.name	= "serial_lpuart",
	.id	= UCLASS_SERIAL,
	.of_match = lpuart_serial_ids,
	.ofdata_to_platdata = lpuart_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct lpuart_serial_platdata),
	.probe = lpuart_serial_probe,
	.ops	= &lpuart_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
#else /* CONFIG_LPUART_32B_REG */
static const struct dm_serial_ops lpuart32_serial_ops = {
	.putc = lpuart32_serial_putc,
	.pending = lpuart32_serial_pending,
	.getc = lpuart32_serial_getc,
	.setbrg = lpuart32_serial_setbrg,
};

static const struct udevice_id lpuart32_serial_ids[] = {
	{ .compatible = "fsl,ls1021a-lpuart" },
	{ }
};

U_BOOT_DRIVER(serial_lpuart32) = {
	.name	= "serial_lpuart32",
	.id	= UCLASS_SERIAL,
	.of_match = lpuart32_serial_ids,
	.ofdata_to_platdata = lpuart_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct lpuart_serial_platdata),
	.probe = lpuart32_serial_probe,
	.ops	= &lpuart32_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
#endif /* CONFIG_LPUART_32B_REG */
