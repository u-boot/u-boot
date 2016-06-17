/*
 * (C) Copyright 2016
 * Vikas Manocha, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <serial.h>
#include <dm/platform_data/serial_stm32x7.h>
#include "serial_stm32x7.h"

DECLARE_GLOBAL_DATA_PTR;

static int stm32_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct stm32x7_serial_platdata *plat = dev->platdata;
	struct stm32_usart *const usart = plat->base;
	writel(plat->clock/baudrate, &usart->brr);

	return 0;
}

static int stm32_serial_getc(struct udevice *dev)
{
	struct stm32x7_serial_platdata *plat = dev->platdata;
	struct stm32_usart *const usart = plat->base;

	if ((readl(&usart->sr) & USART_SR_FLAG_RXNE) == 0)
		return -EAGAIN;

	return readl(&usart->rd_dr);
}

static int stm32_serial_putc(struct udevice *dev, const char c)
{
	struct stm32x7_serial_platdata *plat = dev->platdata;
	struct stm32_usart *const usart = plat->base;

	if ((readl(&usart->sr) & USART_SR_FLAG_TXE) == 0)
		return -EAGAIN;

	writel(c, &usart->tx_dr);

	return 0;
}

static int stm32_serial_pending(struct udevice *dev, bool input)
{
	struct stm32x7_serial_platdata *plat = dev->platdata;
	struct stm32_usart *const usart = plat->base;

	if (input)
		return readl(&usart->sr) & USART_SR_FLAG_RXNE ? 1 : 0;
	else
		return readl(&usart->sr) & USART_SR_FLAG_TXE ? 0 : 1;
}

static int stm32_serial_probe(struct udevice *dev)
{
	struct stm32x7_serial_platdata *plat = dev->platdata;
	struct stm32_usart *const usart = plat->base;
	setbits_le32(&usart->cr1, USART_CR1_RE | USART_CR1_TE | USART_CR1_UE);

	return 0;
}

static const struct dm_serial_ops stm32_serial_ops = {
	.putc = stm32_serial_putc,
	.pending = stm32_serial_pending,
	.getc = stm32_serial_getc,
	.setbrg = stm32_serial_setbrg,
};

U_BOOT_DRIVER(serial_stm32) = {
	.name = "serial_stm32x7",
	.id = UCLASS_SERIAL,
	.ops = &stm32_serial_ops,
	.probe = stm32_serial_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
