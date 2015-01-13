/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/serial_reg.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <dm/device.h>
#include <dm/platform_data/serial-uniphier.h>
#include <serial.h>
#include <fdtdec.h>

#define UART_REG(x)					\
	u8 x;						\
	u8 postpad_##x[3];

/*
 * Note: Register map is slightly different from that of 16550.
 */
struct uniphier_serial {
	UART_REG(rbr);		/* 0x00 */
	UART_REG(ier);		/* 0x04 */
	UART_REG(iir);		/* 0x08 */
	UART_REG(fcr);		/* 0x0c */
	u8 mcr;			/* 0x10 */
	u8 lcr;
	u16 __postpad;
	UART_REG(lsr);		/* 0x14 */
	UART_REG(msr);		/* 0x18 */
	u32 __none1;
	u32 __none2;
	u16 dlr;
	u16 __postpad2;
};

#define thr rbr

struct uniphier_serial_private_data {
	struct uniphier_serial __iomem *membase;
};

#define uniphier_serial_port(dev)	\
	((struct uniphier_serial_private_data *)dev_get_priv(dev))->membase

static int uniphier_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct uniphier_serial_platform_data *plat = dev_get_platdata(dev);
	struct uniphier_serial __iomem *port = uniphier_serial_port(dev);
	const unsigned int mode_x_div = 16;
	unsigned int divisor;

	writeb(UART_LCR_WLEN8, &port->lcr);

	divisor = DIV_ROUND_CLOSEST(plat->uartclk, mode_x_div * baudrate);

	writew(divisor, &port->dlr);

	return 0;
}

static int uniphier_serial_getc(struct udevice *dev)
{
	struct uniphier_serial __iomem *port = uniphier_serial_port(dev);

	if (!(readb(&port->lsr) & UART_LSR_DR))
		return -EAGAIN;

	return readb(&port->rbr);
}

static int uniphier_serial_putc(struct udevice *dev, const char c)
{
	struct uniphier_serial __iomem *port = uniphier_serial_port(dev);

	if (!(readb(&port->lsr) & UART_LSR_THRE))
		return -EAGAIN;

	writeb(c, &port->thr);

	return 0;
}

static int uniphier_serial_pending(struct udevice *dev, bool input)
{
	struct uniphier_serial __iomem *port = uniphier_serial_port(dev);

	if (input)
		return readb(&port->lsr) & UART_LSR_DR;
	else
		return !(readb(&port->lsr) & UART_LSR_THRE);
}

static int uniphier_serial_probe(struct udevice *dev)
{
	struct uniphier_serial_private_data *priv = dev_get_priv(dev);
	struct uniphier_serial_platform_data *plat = dev_get_platdata(dev);

	priv->membase = map_sysmem(plat->base, sizeof(struct uniphier_serial));

	if (!priv->membase)
		return -ENOMEM;

	return 0;
}

static int uniphier_serial_remove(struct udevice *dev)
{
	unmap_sysmem(uniphier_serial_port(dev));

	return 0;
}

#ifdef CONFIG_OF_CONTROL
static const struct udevice_id uniphier_uart_of_match[] = {
	{ .compatible = "panasonic,uniphier-uart" },
	{},
};

static int uniphier_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct uniphier_serial_platform_data *plat = dev_get_platdata(dev);
	DECLARE_GLOBAL_DATA_PTR;

	plat->base = fdtdec_get_addr(gd->fdt_blob, dev->of_offset, "reg");
	plat->uartclk = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
				       "clock-frequency", 0);

	return 0;
}
#endif

static const struct dm_serial_ops uniphier_serial_ops = {
	.setbrg = uniphier_serial_setbrg,
	.getc = uniphier_serial_getc,
	.putc = uniphier_serial_putc,
	.pending = uniphier_serial_pending,
};

U_BOOT_DRIVER(uniphier_serial) = {
	.name = DRIVER_NAME,
	.id = UCLASS_SERIAL,
	.of_match = of_match_ptr(uniphier_uart_of_match),
	.ofdata_to_platdata = of_match_ptr(uniphier_serial_ofdata_to_platdata),
	.probe = uniphier_serial_probe,
	.remove = uniphier_serial_remove,
	.priv_auto_alloc_size = sizeof(struct uniphier_serial_private_data),
	.platdata_auto_alloc_size =
				sizeof(struct uniphier_serial_platform_data),
	.ops = &uniphier_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
