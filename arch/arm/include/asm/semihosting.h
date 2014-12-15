/*
 * Copyright 2014 Broadcom Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SEMIHOSTING_H__
#define __SEMIHOSTING_H__

/*
 * ARM semihosting functions for loading images to memory. See the source
 * code for more information.
 */
int smh_load(const char *fname, void *memp, int avail, int verbose);
long smh_len(const char *fname);

#endif /* __SEMIHOSTING_H__ */
