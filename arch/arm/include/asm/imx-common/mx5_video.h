/*
 * Copyright (C) 2012
 * Anatolij Gustschin, DENX Software Engineering, <agust@denx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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
