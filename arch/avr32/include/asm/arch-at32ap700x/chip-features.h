/*
 * Copyright (C) 2007 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_AVR32_ARCH_CHIP_FEATURES_H__
#define __ASM_AVR32_ARCH_CHIP_FEATURES_H__

/* Currently, all the AP700x chips have these */
#define AT32AP700x_CHIP_HAS_USART
#define AT32AP700x_CHIP_HAS_MMCI
#define AT32AP700x_CHIP_HAS_SPI

/* Only AP7000 has ethernet interface */
#ifdef CONFIG_AT32AP7000
#define AT32AP700x_CHIP_HAS_MACB
#endif

/* AP7000 and AP7002 have LCD controller, but AP7001 does not */
#if defined(CONFIG_AT32AP7000) || defined(CONFIG_AT32AP7002)
#define AT32AP700x_CHIP_HAS_LCDC
#endif

#endif /* __ASM_AVR32_ARCH_CHIP_FEATURES_H__ */
