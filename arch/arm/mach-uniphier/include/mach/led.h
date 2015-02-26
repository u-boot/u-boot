/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_LED_H
#define ARCH_LED_H

#include <config.h>

#define LED_CHAR_0	0x7e
#define LED_CHAR_1	0x0c
#define LED_CHAR_2	0xb6
#define LED_CHAR_3	0x9e
#define LED_CHAR_4	0xcc
#define LED_CHAR_5	0xda
#define LED_CHAR_6	0xfa
#define LED_CHAR_7	0x4e
#define LED_CHAR_8	0xfe
#define LED_CHAR_9	0xde

#define LED_CHAR_A	0xee
#define LED_CHAR_B	0xf8
#define LED_CHAR_C	0x72
#define LED_CHAR_D	0xbc
#define LED_CHAR_E	0xf2
#define LED_CHAR_F	0xe2
#define LED_CHAR_G	0x7a
#define LED_CHAR_H	0xe8
#define LED_CHAR_I	0x08
#define LED_CHAR_J	0x3c
#define LED_CHAR_K	0xea
#define LED_CHAR_L	0x70
#define LED_CHAR_M	0x6e
#define LED_CHAR_N	0xa8
#define LED_CHAR_O	0xb8
#define LED_CHAR_P	0xe6
#define LED_CHAR_Q	0xce
#define LED_CHAR_R	0xa0
#define LED_CHAR_S	0xc8
#define LED_CHAR_T	0x8c
#define LED_CHAR_U	0x7c
#define LED_CHAR_V	0x54
#define LED_CHAR_W	0xfc
#define LED_CHAR_X	0xec
#define LED_CHAR_Y	0xdc
#define LED_CHAR_Z	0xa4

#define LED_CHAR_SPACE	0x00
#define LED_CHAR_DOT	0x01

#define LED_CHAR_	(LED_CHAR_SPACE)

/** Macro to translate 4 characters into integer to display led */
#define LED_C2I(C0, C1, C2, C3)			\
	(~(					\
		(LED_CHAR_##C0 << 24) |		\
		(LED_CHAR_##C1 << 16) |		\
		(LED_CHAR_##C2 <<  8) |		\
		(LED_CHAR_##C3)			\
	))

#if defined(CONFIG_SUPPORT_CARD_LED_BASE)

#define LED_ADDR    CONFIG_SUPPORT_CARD_LED_BASE

#ifdef __ASSEMBLY__

#define led_write(C0, C1, C2, C3)  raw_led_write LED_C2I(C0, C1, C2, C3)
.macro raw_led_write data
	ldr r0, =\data
	ldr r1, =LED_ADDR
	str r0, [r1]
.endm

#else /* __ASSEMBLY__ */

#include <asm/io.h>

#define led_write(C0, C1, C2, C3)		\
do {						\
	raw_led_write(LED_C2I(C0, C1, C2, C3));	\
} while (0)

static inline void raw_led_write(u32 data)
{
	writel(data, LED_ADDR);
}

#endif /* __ASSEMBLY__ */

#else /* CONFIG_SUPPORT_CARD_LED_BASE */

#define led_write(C0, C1, C2, C3)
#define raw_led_write(x)

#endif /* CONFIG_SUPPORT_CARD_LED_BASE */

#endif /* ARCH_LED_H */
