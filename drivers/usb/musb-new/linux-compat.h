#ifndef __LINUX_COMPAT_H__
#define __LINUX_COMPAT_H__

#include <malloc.h>
#include <linux/list.h>
#include <linux/compat.h>

#define __init
#define __devinit
#define __devinitdata
#define __devinitconst
#define __iomem
#define __deprecated

struct unused {};
typedef struct unused unused_t;

typedef int irqreturn_t;
typedef unused_t spinlock_t;

struct work_struct {};

struct timer_list {};
struct notifier_block {};

typedef unsigned long dmaaddr_t;

#define spin_lock_init(lock) do {} while (0)
#define spin_lock(lock) do {} while (0)
#define spin_unlock(lock) do {} while (0)
#define spin_lock_irqsave(lock, flags) do {} while (0)
#define spin_unlock_irqrestore(lock, flags) do {} while (0)

#define setup_timer(timer, func, data) do {} while (0)
#define del_timer_sync(timer) do {} while (0)
#define schedule_work(work) do {} while (0)
#define INIT_WORK(work, fun) do {} while (0)

#define cpu_relax() do {} while (0)

#define pr_debug(fmt, args...) debug(fmt, ##args)

#define WARN(condition, fmt, args...) ({	\
	int ret_warn = !!condition;		\
	if (ret_warn)				\
		printf(fmt, ##args);		\
	ret_warn; })

#define pm_runtime_get_sync(dev) do {} while (0)
#define pm_runtime_put(dev) do {} while (0)
#define pm_runtime_put_sync(dev) do {} while (0)
#define pm_runtime_use_autosuspend(dev) do {} while (0)
#define pm_runtime_set_autosuspend_delay(dev, delay) do {} while (0)
#define pm_runtime_enable(dev) do {} while (0)

#define MODULE_DESCRIPTION(desc)
#define MODULE_AUTHOR(author)
#define MODULE_LICENSE(license)
#define MODULE_ALIAS(alias)
#define module_param(name, type, perm)
#define MODULE_PARM_DESC(name, desc)
#define EXPORT_SYMBOL_GPL(name)

#define writesl(a, d, s) __raw_writesl((unsigned long)a, d, s)
#define readsl(a, d, s) __raw_readsl((unsigned long)a, d, s)
#define writesw(a, d, s) __raw_writesw((unsigned long)a, d, s)
#define readsw(a, d, s) __raw_readsw((unsigned long)a, d, s)
#define writesb(a, d, s) __raw_writesb((unsigned long)a, d, s)
#define readsb(a, d, s) __raw_readsb((unsigned long)a, d, s)

#define IRQ_NONE 0
#define IRQ_HANDLED 0

#define dev_set_drvdata(dev, data) do {} while (0)

#define disable_irq_wake(irq) do {} while (0)
#define enable_irq_wake(irq) -EINVAL
#define free_irq(irq, data) do {} while (0)
#define request_irq(nr, f, flags, nm, data) 0

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
