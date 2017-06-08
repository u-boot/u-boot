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


#define USART_CR1_OVER8			(1 << 15)
#define USART_CR1_TE			(1 << 3)
#define USART_CR1_RE			(1 << 2)
#define USART_CR1_UE			(1 << 0)

#define USART_CR3_OVRDIS		(1 << 12)

#define USART_SR_FLAG_RXNE		(1 << 5)
#define USART_SR_FLAG_TXE		(1 << 7)

#define USART_BRR_F_MASK		0xFF
#define USART_BRR_M_SHIFT		4
#define USART_BRR_M_MASK		0xFFF0

#endif
