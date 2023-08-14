/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __CEDIT_H
#define __CEDIT_H

struct expo;
struct video_priv;

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

#endif /* __CEDIT_H */
