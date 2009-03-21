/*
 * This file is part of UBIFS.
 *
 * Copyright (C) 2006-2008 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Authors: Artem Bityutskiy (Битюцкий Артём)
 *          Adrian Hunter
 */

/*
 * This file implements most of the debugging stuff which is compiled in only
 * when it is enabled. But some debugging check functions are implemented in
 * corresponding subsystem, just because they are closely related and utilize
 * various local functions of those subsystems.
 */

#define UBIFS_DBG_PRESERVE_UBI

#include "ubifs.h"

#ifdef CONFIG_UBIFS_FS_DEBUG

DEFINE_SPINLOCK(dbg_lock);

static char dbg_key_buf0[128];
static char dbg_key_buf1[128];

unsigned int ubifs_msg_flags = UBIFS_MSG_FLAGS_DEFAULT;
unsigned int ubifs_chk_flags = UBIFS_CHK_FLAGS_DEFAULT;
unsigned int ubifs_tst_flags;

module_param_named(debug_msgs, ubifs_msg_flags, uint, S_IRUGO | S_IWUSR);
module_param_named(debug_chks, ubifs_chk_flags, uint, S_IRUGO | S_IWUSR);
module_param_named(debug_tsts, ubifs_tst_flags, uint, S_IRUGO | S_IWUSR);

MODULE_PARM_DESC(debug_msgs, "Debug message type flags");
MODULE_PARM_DESC(debug_chks, "Debug check flags");
MODULE_PARM_DESC(debug_tsts, "Debug special test flags");

static const char *get_key_type(int type)
{
	switch (type) {
	case UBIFS_INO_KEY:
		return "inode";
	case UBIFS_DENT_KEY:
		return "direntry";
	case UBIFS_XENT_KEY:
		return "xentry";
	case UBIFS_DATA_KEY:
		return "data";
	case UBIFS_TRUN_KEY:
		return "truncate";
	default:
		return "unknown/invalid key";
	}
}

static void sprintf_key(const struct ubifs_info *c, const union ubifs_key *key,
			char *buffer)
{
	char *p = buffer;
	int type = key_type(c, key);

	if (c->key_fmt == UBIFS_SIMPLE_KEY_FMT) {
		switch (type) {
		case UBIFS_INO_KEY:
			sprintf(p, "(%lu, %s)", (unsigned long)key_inum(c, key),
			       get_key_type(type));
			break;
		case UBIFS_DENT_KEY:
		case UBIFS_XENT_KEY:
			sprintf(p, "(%lu, %s, %#08x)",
				(unsigned long)key_inum(c, key),
				get_key_type(type), key_hash(c, key));
			break;
		case UBIFS_DATA_KEY:
			sprintf(p, "(%lu, %s, %u)",
				(unsigned long)key_inum(c, key),
				get_key_type(type), key_block(c, key));
			break;
		case UBIFS_TRUN_KEY:
			sprintf(p, "(%lu, %s)",
				(unsigned long)key_inum(c, key),
				get_key_type(type));
			break;
		default:
			sprintf(p, "(bad key type: %#08x, %#08x)",
				key->u32[0], key->u32[1]);
		}
	} else
		sprintf(p, "bad key format %d", c->key_fmt);
}

const char *dbg_key_str0(const struct ubifs_info *c, const union ubifs_key *key)
{
	/* dbg_lock must be held */
	sprintf_key(c, key, dbg_key_buf0);
	return dbg_key_buf0;
}

const char *dbg_key_str1(const struct ubifs_info *c, const union ubifs_key *key)
{
	/* dbg_lock must be held */
	sprintf_key(c, key, dbg_key_buf1);
	return dbg_key_buf1;
}

/**
 * ubifs_debugging_init - initialize UBIFS debugging.
 * @c: UBIFS file-system description object
 *
 * This function initializes debugging-related data for the file system.
 * Returns zero in case of success and a negative error code in case of
 * failure.
 */
int ubifs_debugging_init(struct ubifs_info *c)
{
	c->dbg = kzalloc(sizeof(struct ubifs_debug_info), GFP_KERNEL);
	if (!c->dbg)
		return -ENOMEM;

	c->dbg->buf = vmalloc(c->leb_size);
	if (!c->dbg->buf)
		goto out;

	return 0;

out:
	kfree(c->dbg);
	return -ENOMEM;
}

/**
 * ubifs_debugging_exit - free debugging data.
 * @c: UBIFS file-system description object
 */
void ubifs_debugging_exit(struct ubifs_info *c)
{
	vfree(c->dbg->buf);
	kfree(c->dbg);
}

#endif /* CONFIG_UBIFS_FS_DEBUG */
