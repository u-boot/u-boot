/*
 * Declarations for System V style searching functions.
 * Copyright (C) 1995-1999, 2000 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 *
 * The GNU C Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The GNU C Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the GNU C Library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.
 */

/*
 * Based on code from uClibc-0.9.30.3
 * Extensions for use within U-Boot
 * Copyright (C) 2010 Wolfgang Denk <wd@denx.de>
 */

#ifndef _SEARCH_H
#define	_SEARCH_H 1

#include <stddef.h>

#define __set_errno(val) do { errno = val; } while (0)

/* Action which shall be performed in the call the hsearch.  */
typedef enum {
	FIND,
	ENTER
} ACTION;

typedef struct entry {
	const char *key;
	char *data;
} ENTRY;

/* Opaque type for internal use.  */
struct _ENTRY;

/*
 * Family of hash table handling functions.  The functions also
 * have reentrant counterparts ending with _r.  The non-reentrant
 * functions all work on a signle internal hashing table.
 */

/* Data type for reentrant functions.  */
struct hsearch_data {
	struct _ENTRY *table;
	unsigned int size;
	unsigned int filled;
/*
 * Callback function which will check whether the given change for variable
 * "name" from "oldval" to "newval" may be applied or not, and possibly apply
 * such change.
 * When (flag & H_FORCE) is set, it shall not print out any error message and
 * shall force overwriting of write-once variables.
.* Must return 0 for approval, 1 for denial.
 */
	int (*apply)(const char *name, const char *oldval,
			const char *newval, int flag);
};

/* Create a new hashing table which will at most contain NEL elements.  */
extern int hcreate_r(size_t __nel, struct hsearch_data *__htab);

/* Destroy current internal hashing table.  */
extern void hdestroy_r(struct hsearch_data *__htab, int do_apply);

/*
 * Search for entry matching ITEM.key in internal hash table.  If
 * ACTION is `FIND' return found entry or signal error by returning
 * NULL.  If ACTION is `ENTER' replace existing data (if any) with
 * ITEM.data.
 * */
extern int hsearch_r(ENTRY __item, ACTION __action, ENTRY ** __retval,
		     struct hsearch_data *__htab);

/*
 * Search for an entry matching `MATCH'.  Otherwise, Same semantics
 * as hsearch_r().
 */
extern int hmatch_r(const char *__match, int __last_idx, ENTRY ** __retval,
		    struct hsearch_data *__htab);
/*
 * Search for an entry whose key or data contains `MATCH'.  Otherwise,
 * Same semantics as hsearch_r().
 */
extern int hstrstr_r(const char *__match, int __last_idx, ENTRY ** __retval,
		    struct hsearch_data *__htab);

/* Search and delete entry matching ITEM.key in internal hash table. */
extern int hdelete_r(const char *__key, struct hsearch_data *__htab,
			int do_apply);

extern ssize_t hexport_r(struct hsearch_data *__htab,
		     const char __sep, char **__resp, size_t __size,
		     int argc, char * const argv[]);

/*
 * nvars: length of vars array
 * vars: array of strings (variable names) to import (nvars == 0 means all)
 * do_apply: whether to call callback function to check the new argument,
 * and possibly apply changes (false means accept everything)
 */
extern int himport_r(struct hsearch_data *__htab,
		     const char *__env, size_t __size, const char __sep,
		     int __flag, int nvars, char * const vars[], int do_apply);

/* Flags for himport_r() */
#define	H_NOCLEAR	(1 << 0) /* do not clear hash table before importing */
#define	H_FORCE		(1 << 1) /* overwrite read-only/write-once variables */

#endif /* search.h */
