/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Calxeda, Inc.
 */

#ifndef __MENU_H__
#define __MENU_H__

struct menu;

struct menu *menu_create(char *title, int timeout, int prompt,
				void (*display_statusline)(struct menu *),
				void (*item_data_print)(void *),
				char *(*item_choice)(void *),
				void *item_choice_data);
int menu_default_set(struct menu *m, char *item_key);
int menu_get_choice(struct menu *m, void **choice);
int menu_item_add(struct menu *m, char *item_key, void *item_data);
int menu_destroy(struct menu *m);
int menu_default_choice(struct menu *m, void **choice);

/**
 * menu_show() Show a boot menu
 *
 * This shows a menu and lets the user select an option. The menu is defined by
 * environment variables (see README.bootmenu).
 *
 * This function doesn't normally return, but if the users requests the command
 * problem, it will.
 *
 * @bootdelay: Delay to wait before running the default menu option (0 to run
 *		the entry immediately)
 * Return: If it returns, it always returns -1 to indicate that the boot should
 *	be aborted and the command prompt should be provided
 */
int menu_show(int bootdelay);

struct bootmenu_data {
	int delay;			/* delay for autoboot */
	int active;			/* active menu entry */
	int count;			/* total count of menu entries */
	struct bootmenu_entry *first;	/* first menu entry */
};

enum bootmenu_key {
	KEY_NONE = 0,
	KEY_UP,
	KEY_DOWN,
	KEY_SELECT,
	KEY_QUIT,
	KEY_PLUS,
	KEY_MINUS,
	KEY_SPACE,
};

void bootmenu_autoboot_loop(struct bootmenu_data *menu,
			    enum bootmenu_key *key, int *esc);
void bootmenu_loop(struct bootmenu_data *menu,
		   enum bootmenu_key *key, int *esc);

#endif /* __MENU_H__ */
