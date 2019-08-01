/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common environment functions
 *
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __ENV_H
#define __ENV_H

#include <stdbool.h>

/**
 * env_get_id() - Gets a sequence number for the environment
 *
 * This value increments every time the environment changes, so can be used an
 * an indication of this
 *
 * @return environment ID
 */
int env_get_id(void);

/**
 * env_complete() - return an auto-complete for environment variables
 *
 * @var: partial name to auto-complete
 * @maxv: Maximum number of matches to return
 * @cmdv: Returns a list of possible matches
 * @maxsz: Size of buffer to use for matches
 * @buf: Buffer to use for matches
 * @dollar_comp: non-zero to wrap each match in ${...}
 * @return number of matches found (in @cmdv)
 */
int env_complete(char *var, int maxv, char *cmdv[], int maxsz, char *buf,
		 bool dollar_comp);

#endif
