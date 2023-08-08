/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016 Marvell International Ltd.
 * https://spdx.org/licenses
 */

#ifndef _CACHE_LLC_H_
#define _CACHE_LLC_H_

/* Armada-7K/8K last level cache */

#define MVEBU_A8K_REGS_BASE_MSB		0xf000
#define LLC_BASE_ADDR			0x8000
#define LLC_CACHE_SYNC			0x700
#define LLC_CACHE_SYNC_COMPLETE		0x730
#define LLC_FLUSH_BY_WAY		0x7fc
#define LLC_WAY_MASK			0xffffffff
#define LLC_CACHE_SYNC_MASK		0x1

#define MVEBU_LLC_BASE			(MVEBU_REGISTER(LLC_BASE_ADDR))
#define LLC_CTRL_REG_OFFSET		0x100
#define LLC_EN				0x1
#define LLC_EXCL_EN			0x100

#endif	/* _CACHE_LLC_H_ */
