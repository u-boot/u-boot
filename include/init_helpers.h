/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _INIT_HELPERS_H_
#define _INIT_HELPERS_H_

/**
 * init_cache_f_r() - Turn on the cache in preparation for relocation
 *
 * @return 0 if OK, -ve on error
 */
int init_cache_f_r(void);

#endif	/* _INIT_HELPERS_H_ */
