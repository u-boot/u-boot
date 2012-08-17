/*
 * Copyright 2010-2011 Calxeda, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __MENU_H__
#define __MENU_H__

struct menu;

struct menu *menu_create(char *title, int timeout, int prompt,
				void (*item_data_print)(void *));
int menu_default_set(struct menu *m, char *item_key);
int menu_get_choice(struct menu *m, void **choice);
int menu_item_add(struct menu *m, char *item_key, void *item_data);
int menu_destroy(struct menu *m);
void menu_display_statusline(struct menu *m);

#if defined(CONFIG_MENU_SHOW)
int menu_show(int bootdelay);
#endif
#endif /* __MENU_H__ */
