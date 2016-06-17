/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <serial.h>
#include <asm/arch/stm32.h>
#include <dm/platform_data/serial_stm32.h>

struct stm32_usart {
	u32 sr;
	u32 dr;
	u32 brr;
	u32 cr1;
	u32 cr2;
	u32 cr3;
	u32 gtpr;
};

#define USART_CR1_RE			(1 << 2)
#define USART_CR1_TE			(1 << 3)
#define USART_CR1_UE			(1 << 13)

#define USART_SR_FLAG_RXNE	(1 << 5)
#define USART_SR_FLAG_TXE		(1 << 7)

#define USART_BRR_F_MASK		0xF
#define USART_BRR_M_SHIFT	4
#define USART_BRR_M_MASK	0xFFF0

DECLARE_GLOBAL_DATA_PTR;

static int stm32_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct stm32_serial_platdata *plat = dev->platdata;
	struct stm32_usart *const usart = plat->base;
	u32  clock, int_div, frac_div, tmp;

	if (((u32)usart & STM32_BUS_MASK) == STM32_APB1PERIPH_BASE)
		clock = clock_get(CLOCK_APB1);
	else if (((u32)usart & STM32_BUS_MASK) == STM32_APB2PERIPH_BASE)
		clock = clock_get(CLOCK_APB2);
	else
		return -EINVAL;

	int_div = (25 * clock) / (4 * baudrate);
	tmp = ((int_div / 100) << USART_BRR_M_SHIFT) & USART_BRR_M_MASK;
	frac_div = int_div - (100 * (tmp >> USART_BRR_M_SHIFT));
	tmp |= (((frac_div * 16) + 50) / 100) & USART_BRR_F_MASK;
	writel(tmp, &usart->brr);

	return 0;
}

static int stm32_serial_getc(struct udevice *dev)
{
	struct stm32_serial_platdata *plat = dev->platdata;
	struct stm32_usart *const usart = plat->base;

	if ((readl(&usart->sr) & USART_SR_FLAG_RXNE) == 0)
		return -EAGAIN;

	return readl(&usart->dr);
}

static int stm32_serial_putc(struct udevice *dev, const char c)
{
	struct stm32_serial_platdata *plat = dev->platdata;
	struct stm32_usart *const usart = plat->base;

	if ((readl(&usart->sr) & USART_SR_FLAG_TXE) == 0)
		return -EAGAIN;

	writel(c, &usart->dr);

	return 0;
}

static int stm32_serial_pending(struct udevice *dev, bool input)
{
	struct stm32_serial_platdata *plat = dev->platdata;
	struct stm32_usart *const usart = plat->base;

	if (input)
		return readl(&usart->sr) & USART_SR_FLAG_RXNE ? 1 : 0;
	else
		return readl(&usart->sr) & USART_SR_FLAG_TXE ? 0 : 1;
}

static int stm32_serial_probe(struct udevice *dev)
{
	struct stm32_serial_platdata *plat = dev->platdata;
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
	.name = "serial_stm32",
	.id = UCLASS_SERIAL,
	.ops = &stm32_serial_ops,
	.probe = stm32_serial_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
