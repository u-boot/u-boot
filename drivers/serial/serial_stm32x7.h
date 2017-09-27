/*
 * (C) Copyright 2016
 * Vikas Manocha, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SERIAL_STM32_X7_
#define _SERIAL_STM32_X7_

struct stm32_usart {
	u32 cr1;
	u32 cr2;
	u32 cr3;
	u32 brr;
	u32 gtpr;
	u32 rtor;
	u32 rqr;
	u32 sr;
	u32 icr;
	u32 rd_dr;
	u32 tx_dr;
};

/* Information about a serial port */
struct stm32x7_serial_platdata {
	struct stm32_usart *base;  /* address of registers in physical memory */
	unsigned long int clock_rate;
};

#define USART_CR1_OVER8			BIT(15)
#define USART_CR1_TE			BIT(3)
#define USART_CR1_RE			BIT(2)
#define USART_CR1_UE			BIT(0)

#define USART_CR3_OVRDIS		BIT(12)

#define USART_SR_FLAG_RXNE		BIT(5)
#define USART_SR_FLAG_TXE		BIT(7)

#define USART_BRR_F_MASK		GENMASK(7, 0)
#define USART_BRR_M_SHIFT		4
#define USART_BRR_M_MASK		GENMASK(15, 4)

#endif
