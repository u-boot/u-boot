/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef ARCH_BOARD_H
#define ARCH_BOARD_H

#if defined(CONFIG_PFC_MICRO_SUPPORT_CARD) || \
	defined(CONFIG_DCC_MICRO_SUPPORT_CARD)
void support_card_reset(void);
void support_card_init(void);
void support_card_late_init(void);
int check_support_card(void);
#else
#define support_card_reset() do {} while (0)
#define support_card_init()  do {} while (0)
#define support_card_late_init()  do {} while (0)
static inline int check_support_card(void)
{
	return 0;
}
#endif

static inline void uniphier_board_reset(void)
{
	support_card_reset();
}

static inline void uniphier_board_init(void)
{
	support_card_init();
}

static inline void uniphier_board_late_init(void)
{
	support_card_late_init();
}

#endif /* ARCH_BOARD_H */
