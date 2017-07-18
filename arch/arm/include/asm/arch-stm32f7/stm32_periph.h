/*
 * (C) Copyright 2016
 * Vikas Manocha, ST Micoelectronics, vikas.manocha@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARM_ARCH_PERIPH_H
#define __ASM_ARM_ARCH_PERIPH_H

/*
 * Peripherals required for pinmux configuration. List will
 * grow with support for more devices getting added.
 * Numbering based on interrupt table.
 *
 */
enum periph_id {
	PERIPH_ID_USART1 = 37,

	PERIPH_ID_QUADSPI = 92,
};

enum periph_clock {
	SYSCFG_CLOCK_CFG,
	TIMER2_CLOCK_CFG,
	STMMAC_CLOCK_CFG,
};

#endif /* __ASM_ARM_ARCH_PERIPH_H */
