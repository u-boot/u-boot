/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _INIT_HELPERS_H_
#define _INIT_HELPERS_H_

int calculate_relocation_address(void);

int init_cache_f_r(void);
int init_bd_struct_r(void);
int init_func_spi(void);
int find_fdt(void);
int prepare_fdt(void);

#endif	/* !_INIT_HELPERS_H_ */
