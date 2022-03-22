/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ifndef _SEMIHOSTING_H
#define _SEMIHOSTING_H

long smh_open(const char *fname, char *modestr);
long smh_read(long fd, void *memp, size_t len);
long smh_close(long fd);
long smh_flen(long fd);

#endif /* _SEMIHOSTING_H */
