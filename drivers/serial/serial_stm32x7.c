/*
 * (C) Copyright 2016
 * Vikas Manocha, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/io.h>
#include <serial.h>
#include <asm/arch/stm32.h>
#include <dm/platform_data/serial_stm32x7.h>
#include "serial_stm32x7.h"

DECLARE_GLOBAL_DATA_PTR;

static int stm32_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct stm32x7_serial_platdata *plat = dev->platdata;
	struct stm32_usart *const usart = plat->base;
	u32  clock, int_div, mantissa, fraction, oversampling;

	if (((u32)usart & STM32_BUS_MASK) == APB1_PERIPH_BASE)
		clock = clock_get(CLOCK_APB1);
	else if (((u32)usart & STM32_BUS_MASK) == APB2_PERIPH_BASE)
		clock = clock_get(CLOCK_APB2);
	else
		return -EINVAL;

	int_div = DIV_ROUND_CLOSEST(clock, baudrate);

	if (int_div < 16) {
		oversampling = 8;
		setbits_le32(&usart->cr1, USART_CR1_OVER8);
	} else {
		oversampling = 16;
		clrbits_le32(&usart->cr1, USART_CR1_OVER8);
	}

	mantissa = (int_div / oversampling) << USART_BRR_M_SHIFT;
	fraction = int_div % oversampling;

	writel(mantissa | fraction, &usart->brr);

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

#ifdef CONFIG_CLK
	int ret;
	struct clk clk;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}
#endif

	/* Disable usart-> disable overrun-> enable usart */
	clrbits_le32(&usart->cr1, USART_CR1_RE | USART_CR1_TE | USART_CR1_UE);
	setbits_le32(&usart->cr3, USART_CR3_OVRDIS);
	setbits_le32(&usart->cr1, USART_CR1_RE | USART_CR1_TE | USART_CR1_UE);

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id stm32_serial_id[] = {
	{.compatible = "st,stm32f7-usart"},
	{.compatible = "st,stm32f7-uart"},
	{}
};

static int stm32_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = (struct stm32_usart *)addr;

	return 0;
}
#endif

static const struct dm_serial_ops stm32_serial_ops = {
	.putc = stm32_serial_putc,
	.pending = stm32_serial_pending,
	.getc = stm32_serial_getc,
	.setbrg = stm32_serial_setbrg,
};

U_BOOT_DRIVER(serial_stm32) = {
	.name = "serial_stm32x7",
	.id = UCLASS_SERIAL,
	.of_match = of_match_ptr(stm32_serial_id),
	.ofdata_to_platdata = of_match_ptr(stm32_serial_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct stm32x7_serial_platdata),
	.ops = &stm32_serial_ops,
	.probe = stm32_serial_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
