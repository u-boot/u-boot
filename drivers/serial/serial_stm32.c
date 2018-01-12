/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/io.h>
#include <serial.h>
#include <asm/arch/stm32.h>
#include "serial_stm32.h"

DECLARE_GLOBAL_DATA_PTR;

static int stm32_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);
	bool stm32f4 = plat->uart_info->stm32f4;
	fdt_addr_t base = plat->base;
	u32 int_div, mantissa, fraction, oversampling;

	int_div = DIV_ROUND_CLOSEST(plat->clock_rate, baudrate);

	if (int_div < 16) {
		oversampling = 8;
		setbits_le32(base + CR1_OFFSET(stm32f4), USART_CR1_OVER8);
	} else {
		oversampling = 16;
		clrbits_le32(base + CR1_OFFSET(stm32f4), USART_CR1_OVER8);
	}

	mantissa = (int_div / oversampling) << USART_BRR_M_SHIFT;
	fraction = int_div % oversampling;

	writel(mantissa | fraction, base + BRR_OFFSET(stm32f4));

	return 0;
}

static int stm32_serial_getc(struct udevice *dev)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);
	bool stm32f4 = plat->uart_info->stm32f4;
	fdt_addr_t base = plat->base;

	if ((readl(base + ISR_OFFSET(stm32f4)) & USART_SR_FLAG_RXNE) == 0)
		return -EAGAIN;

	return readl(base + RDR_OFFSET(stm32f4));
}

static int stm32_serial_putc(struct udevice *dev, const char c)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);
	bool stm32f4 = plat->uart_info->stm32f4;
	fdt_addr_t base = plat->base;

	if ((readl(base + ISR_OFFSET(stm32f4)) & USART_SR_FLAG_TXE) == 0)
		return -EAGAIN;

	writel(c, base + TDR_OFFSET(stm32f4));

	return 0;
}

static int stm32_serial_pending(struct udevice *dev, bool input)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);
	bool stm32f4 = plat->uart_info->stm32f4;
	fdt_addr_t base = plat->base;

	if (input)
		return readl(base + ISR_OFFSET(stm32f4)) &
			USART_SR_FLAG_RXNE ? 1 : 0;
	else
		return readl(base + ISR_OFFSET(stm32f4)) &
			USART_SR_FLAG_TXE ? 0 : 1;
}

static int stm32_serial_probe(struct udevice *dev)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);
	struct clk clk;
	fdt_addr_t base = plat->base;
	int ret;
	bool stm32f4;
	u8 uart_enable_bit;

	plat->uart_info = (struct stm32_uart_info *)dev_get_driver_data(dev);
	stm32f4 = plat->uart_info->stm32f4;
	uart_enable_bit = plat->uart_info->uart_enable_bit;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	plat->clock_rate = clk_get_rate(&clk);
	if (plat->clock_rate < 0) {
		clk_disable(&clk);
		return plat->clock_rate;
	};

	/* Disable uart-> disable overrun-> enable uart */
	clrbits_le32(base + CR1_OFFSET(stm32f4), USART_CR1_RE | USART_CR1_TE |
		     BIT(uart_enable_bit));
	if (plat->uart_info->has_overrun_disable)
		setbits_le32(base + CR3_OFFSET(stm32f4), USART_CR3_OVRDIS);
	if (plat->uart_info->has_fifo)
		setbits_le32(base + CR1_OFFSET(stm32f4), USART_CR1_FIFOEN);
	setbits_le32(base + CR1_OFFSET(stm32f4), USART_CR1_RE | USART_CR1_TE |
		     BIT(uart_enable_bit));

	return 0;
}

static const struct udevice_id stm32_serial_id[] = {
	{ .compatible = "st,stm32-uart", .data = (ulong)&stm32f4_info},
	{ .compatible = "st,stm32f7-uart", .data = (ulong)&stm32f7_info},
	{ .compatible = "st,stm32h7-uart", .data = (ulong)&stm32h7_info},
	{}
};

static int stm32_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct stm32x7_serial_platdata *plat = dev_get_platdata(dev);

	plat->base = devfdt_get_addr(dev);
	if (plat->base == FDT_ADDR_T_NONE)
		return -EINVAL;

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
	.of_match = of_match_ptr(stm32_serial_id),
	.ofdata_to_platdata = of_match_ptr(stm32_serial_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct stm32x7_serial_platdata),
	.ops = &stm32_serial_ops,
	.probe = stm32_serial_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
