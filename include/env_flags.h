/*
 * (C) Copyright 2012
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __ENV_FLAGS_H__
#define __ENV_FLAGS_H__

enum env_flags_vartype {
	env_flags_vartype_string,
	env_flags_vartype_decimal,
	env_flags_vartype_hex,
	env_flags_vartype_bool,
#ifdef CONFIG_CMD_NET
	env_flags_vartype_ipaddr,
	env_flags_vartype_macaddr,
#endif
	env_flags_vartype_end
};

#define ENV_FLAGS_VAR ".flags"
#define ENV_FLAGS_ATTR_MAX_LEN 2
#define ENV_FLAGS_VARTYPE_LOC 0

#ifndef CONFIG_ENV_FLAGS_LIST_STATIC
#define CONFIG_ENV_FLAGS_LIST_STATIC ""
#endif

#define ENV_FLAGS_LIST_STATIC \
	CONFIG_ENV_FLAGS_LIST_STATIC

/*
 * Parse the flags string from a .flags attribute list into the vartype enum.
 */
enum env_flags_vartype env_flags_parse_vartype(const char *flags);

#include <search.h>

/*
 * When adding a variable to the environment, initialize the flags for that
 * variable.
 */
void env_flags_init(ENTRY *var_entry);

/*
 * Validate the newval for to conform with the requirements defined by its flags
 */
int env_flags_validate(const ENTRY *item, const char *newval, enum env_op op,
	int flag);

/*
 * These are the binary flags used in the environment entry->flags variable to
 * decribe properties of veriables in the table
 */
#define ENV_FLAGS_VARTYPE_BIN_MASK	0x00000007
/* The actual variable type values use the enum value (within the mask) */

#endif /* __ENV_FLAGS_H__ */
