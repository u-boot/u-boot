/*
 * (C) Copyright 2016
 * Vikas Manocha, <vikas.manocha@st.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SERIAL_STM32x7_H
#define __SERIAL_STM32x7_H

/* Information about a serial port */
struct stm32x7_serial_platdata {
	struct stm32_usart *base;  /* address of registers in physical memory */
	unsigned int clock;
};

#endif /* __SERIAL_STM32x7_H */
