/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __CEDIT_H
#define __CEDIT_H

struct abuf;
struct expo;
struct scene;
struct video_priv;

enum {
	/* size increment for writing FDT */
	CEDIT_SIZE_INC	= 1024,
};

/* Name of the cedit node in the devicetree */
#define CEDIT_NODE_NAME		"cedit-values"

extern struct expo *cur_exp;

/**
 * cedit_arange() - Arrange objects in a configuration-editor scene
 *
 * @exp: Expo to update
 * @vid_priv: Private info of the video device
 * @scene_id: scene ID to arrange
 * Returns: 0 if OK, -ve on error
 */
int cedit_arange(struct expo *exp, struct video_priv *vid_priv, uint scene_id);

/**
 * cedit_run() - Run a configuration editor
 *
 * This accepts input until the user quits with Escape
 *
 * @exp: Expo to use
 * Returns: 0 if OK, -ve on error
 */
int cedit_run(struct expo *exp);

/**
 * cedit_prepare() - Prepare to run a cedit
 *
 * Set up the video device, select the first scene and highlight the first item.
 * This ensures that all menus have a selected item.
 *
 * @exp: Expo to use
 * @vid_privp: Set to private data for the video device
 * @scnp: Set to the first scene
 * Return: scene ID of first scene if OK, -ve on error
 */
int cedit_prepare(struct expo *exp, struct video_priv **vid_privp,
		  struct scene **scnp);

/**
 * cedit_write_settings() - Write settings in FDT format
 *
 * Sets up an FDT with the settings
 *
 * @exp: Expo to write settings from
 * @buf: Returns abuf containing the settings FDT (inited by this function)
 * Return: 0 if OK, -ve on error
 */
int cedit_write_settings(struct expo *exp, struct abuf *buf);

#endif /* __CEDIT_H */
