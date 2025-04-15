/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * UPL handoff command functions
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __UPL_COMMON_H
#define __UPL_COMMON_H

/* Names of bootmodes */
extern const char *const bootmode_names[UPLBM_COUNT];

/* Names of memory usages */
extern const char *const usage_names[UPLUS_COUNT];

/* Names of access types */
extern const char *const access_types[UPLUS_COUNT];

/* Names of graphics formats */
extern const char *const graphics_formats[UPLUS_COUNT];

#endif /* __UPL_COMMON_H */
