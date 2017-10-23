/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SERIAL_STM32_X7_
#define _SERIAL_STM32_X7_

#define CR1_OFFSET(x)	(x ? 0x0c : 0x00)
#define CR3_OFFSET(x)	(x ? 0x14 : 0x08)
#define BRR_OFFSET(x)	(x ? 0x08 : 0x0c)
#define ISR_OFFSET(x)	(x ? 0x00 : 0x1c)
/*
 * STM32F4 has one Data Register (DR) for received or transmitted
 * data, so map Receive Data Register (RDR) and Transmit Data
 * Register (TDR) at the same offset
 */
#define RDR_OFFSET(x)	(x ? 0x04 : 0x24)
#define TDR_OFFSET(x)	(x ? 0x04 : 0x28)

struct stm32_uart_info {
	u8 uart_enable_bit;	/* UART_CR1_UE */
	bool stm32f4;		/* true for STM32F4, false otherwise */
	bool has_overrun_disable;
	bool has_fifo;
};

struct stm32_uart_info stm32f4_info = {
	.stm32f4 = true,
	.uart_enable_bit = 13,
	.has_overrun_disable = false,
	.has_fifo = false,
};

struct stm32_uart_info stm32f7_info = {
	.uart_enable_bit = 0,
	.stm32f4 = false,
	.has_overrun_disable = true,
	.has_fifo = false,
};

struct stm32_uart_info stm32h7_info = {
	.uart_enable_bit = 0,
	.stm32f4 = false,
	.has_overrun_disable = true,
	.has_fifo = true,
};

/* Information about a serial port */
struct stm32x7_serial_platdata {
	fdt_addr_t base;  /* address of registers in physical memory */
	struct stm32_uart_info *uart_info;
	unsigned long int clock_rate;
};

#define USART_CR1_FIFOEN		BIT(29)
#define USART_CR1_OVER8			BIT(15)
#define USART_CR1_TE			BIT(3)
#define USART_CR1_RE			BIT(2)

#define USART_CR3_OVRDIS		BIT(12)

#define USART_SR_FLAG_RXNE		BIT(5)
#define USART_SR_FLAG_TXE		BIT(7)

#define USART_BRR_F_MASK		GENMASK(7, 0)
#define USART_BRR_M_SHIFT		4
#define USART_BRR_M_MASK		GENMASK(15, 4)

#endif
