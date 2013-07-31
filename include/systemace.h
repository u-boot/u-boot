#ifndef __SYSTEMACE_H
#define __SYSTEMACE_H
/*
 * Copyright (c) 2004 Picture Elements, Inc.
 *    Stephen Williams (steve@picturel.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ident "$Id:$"

#ifdef CONFIG_SYSTEMACE

# include  <part.h>

block_dev_desc_t *  systemace_get_dev(int dev);

#endif	/* CONFIG_SYSTEMACE */
#endif	/* __SYSTEMACE_H */
