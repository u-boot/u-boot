/*
 * Copyright (C) 2012
 * Anatolij Gustschin, DENX Software Engineering, <agust@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __MX5_VIDEO_H
#define __MX5_VIDEO_H

#ifdef CONFIG_VIDEO
void lcd_enable(void);
void setup_iomux_lcd(void);
#else
static inline void lcd_enable(void) { }
static inline void setup_iomux_lcd(void) { }
#endif

#endif
