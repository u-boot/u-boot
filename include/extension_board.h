/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 */

#ifndef __EXTENSION_SUPPORT_H
#define __EXTENSION_SUPPORT_H

#include <linux/list.h>

extern struct list_head extension_list;

/**
 * extension - Description fields of an extension board
 * @list: List head
 * @name: Name of the extension
 * @owner: Owner of the extension
 * @version: Version of the extension
 * @overlay: Devicetree overlay name to be loaded for this extension
 * @other: Other information of this extension
 */
struct extension {
	struct list_head list;
	char name[32];
	char owner[32];
	char version[32];
	char overlay[64];
	char other[32];
};

/**
 * extension_board_scan - Add system-specific function to scan extension board.
 * @param extension_list	List of extension board information to update.
 * Return: the number of extension.
 *
 * This function is called if CONFIG_CMD_EXTENSION is defined.
 * Needs to fill the list extension_list with elements.
 * Each element need to be allocated to an extension structure.
 *
 */
int extension_board_scan(struct list_head *extension_list);

/**
 * extension_apply - Apply extension board overlay to the devicetree
 * @extension: Extension to be applied
 * Return: Zero on success, negative on failure.
 */
int extension_apply(struct extension *extension);

/**
 * extension_scan - Scan extension boards available.
 * @show: Flag to enable verbose log
 * Return: Zero on success, negative on failure.
 */
int extension_scan(bool show);

#endif /* __EXTENSION_SUPPORT_H */
