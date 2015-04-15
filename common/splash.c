/*
 * Copyright (C) 2013, Boundary Devices <info@boundarydevices.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., http://www.fsf.org/about/contact/
 *
 */

#include <common.h>
#include <splash.h>
#include <lcd.h>

__weak int splash_screen_prepare(void)
{
	return 0;
}

#ifdef CONFIG_SPLASH_SCREEN_ALIGN
void splash_get_pos(int *x, int *y)
{
	char *s = getenv("splashpos");

	if (!s)
		return;

	if (s[0] == 'm')
		*x = BMP_ALIGN_CENTER;
	else
		*x = simple_strtol(s, NULL, 0);

	s = strchr(s + 1, ',');
	if (s != NULL) {
		if (s[1] == 'm')
			*y = BMP_ALIGN_CENTER;
		else
			*y = simple_strtol(s + 1, NULL, 0);
	}
}
#endif /* CONFIG_SPLASH_SCREEN_ALIGN */

#if defined(CONFIG_SPLASH_SCREEN) && defined(CONFIG_LCD)
int lcd_splash(ulong addr)
{
	int x = 0, y = 0, ret;

	ret = splash_screen_prepare();
	if (ret)
		return ret;

	splash_get_pos(&x, &y);

	return bmp_display(addr, x, y);
}
#endif
