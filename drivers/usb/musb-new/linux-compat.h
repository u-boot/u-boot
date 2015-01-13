#ifndef __LINUX_COMPAT_H__
#define __LINUX_COMPAT_H__

#include <malloc.h>
#include <linux/list.h>
#include <linux/compat.h>

#define pr_debug(fmt, args...) debug(fmt, ##args)

#define WARN(condition, fmt, args...) ({	\
	int ret_warn = !!condition;		\
	if (ret_warn)				\
		printf(fmt, ##args);		\
	ret_warn; })

#define writesl(a, d, s) __raw_writesl((unsigned long)a, d, s)
#define readsl(a, d, s) __raw_readsl((unsigned long)a, d, s)
#define writesw(a, d, s) __raw_writesw((unsigned long)a, d, s)
#define readsw(a, d, s) __raw_readsw((unsigned long)a, d, s)
#define writesb(a, d, s) __raw_writesb((unsigned long)a, d, s)
#define readsb(a, d, s) __raw_readsb((unsigned long)a, d, s)

#define device_init_wakeup(dev, a) do {} while (0)

#define platform_data device_data

#ifndef wmb
#define wmb()			asm volatile (""   : : : "memory")
#endif

#define msleep(a)	udelay(a * 1000)

/*
 * Map U-Boot config options to Linux ones
 */
#ifdef CONFIG_OMAP34XX
#define CONFIG_SOC_OMAP3430
#endif

#endif /* __LINUX_COMPAT_H__ */
