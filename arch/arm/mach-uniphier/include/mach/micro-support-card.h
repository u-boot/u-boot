/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_BOARD_H
#define ARCH_BOARD_H

#if defined(CONFIG_MICRO_SUPPORT_CARD)
void support_card_reset(void);
void support_card_init(void);
void support_card_late_init(void);
int check_support_card(void);
void led_puts(const char *s);
#else
static inline void support_card_reset(void)
{
}

static inline void support_card_init(void)
{
}

static inline void support_card_late_init(void)
{
}

static inline int check_support_card(void)
{
	return 0;
}

static inline void led_puts(const char *s)
{
}
#endif

#endif /* ARCH_BOARD_H */
