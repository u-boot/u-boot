/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 */

#ifndef _DM_DEVICE_COMPAT_H
#define _DM_DEVICE_COMPAT_H

#include <linux/compat.h>

/*
 * REVISIT:
 * remove the following after resolving conflicts with <linux/compat.h>
 */
#ifdef dev_dbg
#undef dev_dbg
#endif
#ifdef dev_vdbg
#undef dev_vdbg
#endif
#ifdef dev_info
#undef dev_info
#endif
#ifdef dev_err
#undef dev_err
#endif
#ifdef dev_warn
#undef dev_warn
#endif

/*
 * REVISIT:
 * print device name like Linux
 */
#define dev_printk(dev, fmt, ...)				\
({								\
	printk(fmt, ##__VA_ARGS__);				\
})

#define __dev_printk(level, dev, fmt, ...)			\
({								\
	if (level < CONFIG_VAL(LOGLEVEL))			\
		dev_printk(dev, fmt, ##__VA_ARGS__);		\
})

#define dev_emerg(dev, fmt, ...) \
	__dev_printk(0, dev, fmt, ##__VA_ARGS__)
#define dev_alert(dev, fmt, ...) \
	__dev_printk(1, dev, fmt, ##__VA_ARGS__)
#define dev_crit(dev, fmt, ...) \
	__dev_printk(2, dev, fmt, ##__VA_ARGS__)
#define dev_err(dev, fmt, ...) \
	__dev_printk(3, dev, fmt, ##__VA_ARGS__)
#define dev_warn(dev, fmt, ...) \
	__dev_printk(4, dev, fmt, ##__VA_ARGS__)
#define dev_notice(dev, fmt, ...) \
	__dev_printk(5, dev, fmt, ##__VA_ARGS__)
#define dev_info(dev, fmt, ...) \
	__dev_printk(6, dev, fmt, ##__VA_ARGS__)

#ifdef DEBUG
#define dev_dbg(dev, fmt, ...) \
	__dev_printk(7, dev, fmt, ##__VA_ARGS__)
#else
#define dev_dbg(dev, fmt, ...)					\
({								\
	if (0)							\
		__dev_printk(7, dev, fmt, ##__VA_ARGS__);	\
})
#endif

#ifdef VERBOSE_DEBUG
#define dev_vdbg	dev_dbg
#else
#define dev_vdbg(dev, fmt, ...)					\
({								\
	if (0)							\
		__dev_printk(7, dev, fmt, ##__VA_ARGS__);	\
})
#endif

#endif
