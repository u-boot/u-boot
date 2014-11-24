/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DM_UTIL_H
#define __DM_UTIL_H

#ifdef CONFIG_DM_WARN
void dm_warn(const char *fmt, ...);
#else
static inline void dm_warn(const char *fmt, ...)
{
}
#endif

#ifdef DEBUG
void dm_dbg(const char *fmt, ...);
#else
static inline void dm_dbg(const char *fmt, ...)
{
}
#endif

struct list_head;

/**
 * list_count_items() - Count number of items in a list
 *
 * @param head:		Head of list
 * @return number of items, or 0 if empty
 */
int list_count_items(struct list_head *head);

#endif
