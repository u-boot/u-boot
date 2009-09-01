/*
 * Header file for UBI support for U-Boot
 *
 * Adaptation from kernel to U-Boot
 *
 *  Copyright (C) 2005-2007 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __UBOOT_UBI_H
#define __UBOOT_UBI_H

#include <common.h>
#include <compiler.h>
#include <malloc.h>
#include <div64.h>
#include <linux/crc32.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/string.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/ubi.h>

#ifdef CONFIG_CMD_ONENAND
#include <onenand_uboot.h>
#endif

#include <asm/errno.h>

#define DPRINTK(format, args...)					\
do {									\
	printf("%s[%d]: " format "\n", __func__, __LINE__, ##args);	\
} while (0)

/* configurable */
#define CONFIG_MTD_UBI_WL_THRESHOLD	4096
#define CONFIG_MTD_UBI_BEB_RESERVE	1
#define UBI_IO_DEBUG			0

/* debug options (Linux: drivers/mtd/ubi/Kconfig.debug) */
#undef CONFIG_MTD_UBI_DEBUG
#undef CONFIG_MTD_UBI_DEBUG_PARANOID
#undef CONFIG_MTD_UBI_DEBUG_MSG
#undef CONFIG_MTD_UBI_DEBUG_MSG_EBA
#undef CONFIG_MTD_UBI_DEBUG_MSG_WL
#undef CONFIG_MTD_UBI_DEBUG_MSG_IO
#undef CONFIG_MTD_UBI_DEBUG_MSG_BLD
#define CONFIG_MTD_UBI_DEBUG_DISABLE_BGT

/* build.c */
#define get_device(...)
#define put_device(...)
#define ubi_sysfs_init(...)		0
#define ubi_sysfs_close(...)		do { } while (0)
static inline int is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

/* FIXME */
#define MKDEV(...)			0
#define MAJOR(dev)			0
#define MINOR(dev)			0

#define alloc_chrdev_region(...)	0
#define unregister_chrdev_region(...)

#define class_create(...)		__builtin_return_address(0)
#define class_create_file(...)		0
#define class_remove_file(...)
#define class_destroy(...)
#define misc_register(...)		0
#define misc_deregister(...)

/* vmt.c */
#define device_register(...)		0
#define volume_sysfs_init(...)		0
#define volume_sysfs_close(...)		do { } while (0)

/* kapi.c */

/* eba.c */

/* io.c */
#define init_waitqueue_head(...)	do { } while (0)
#define wait_event_interruptible(...)	0
#define wake_up_interruptible(...)	do { } while (0)
#define print_hex_dump(...)		do { } while (0)
#define dump_stack(...)			do { } while (0)

/* wl.c */
#define task_pid_nr(x)			0
#define set_freezable(...)		do { } while (0)
#define try_to_freeze(...)		0
#define set_current_state(...)		do { } while (0)
#define kthread_should_stop(...)	0
#define schedule()			do { } while (0)

/* upd.c */
static inline unsigned long copy_from_user(void *dest, const void *src,
					   unsigned long count)
{
	memcpy((void *)dest, (void *)src, count);
	return 0;
}

/* common */
typedef int	spinlock_t;
typedef int	wait_queue_head_t;
#define spin_lock_init(...)
#define spin_lock(...)
#define spin_unlock(...)

#define mutex_init(...)
#define mutex_lock(...)
#define mutex_unlock(...)

#define init_rwsem(...)			do { } while (0)
#define down_read(...)			do { } while (0)
#define down_write(...)			do { } while (0)
#define down_write_trylock(...)		0
#define up_read(...)			do { } while (0)
#define up_write(...)			do { } while (0)

struct kmem_cache { int i; };
#define kmem_cache_create(...)		1
#define kmem_cache_alloc(obj, gfp)	malloc(sizeof(struct ubi_wl_entry))
#define kmem_cache_free(obj, size)	free(size)
#define kmem_cache_destroy(...)

#define cond_resched()			do { } while (0)
#define yield()				do { } while (0)

#define KERN_WARNING
#define KERN_ERR
#define KERN_NOTICE
#define KERN_DEBUG

#define GFP_KERNEL			0
#define GFP_NOFS			1

#define __user
#define __init
#define __exit

#define kthread_create(...)	__builtin_return_address(0)
#define kthread_stop(...)	do { } while (0)
#define wake_up_process(...)	do { } while (0)

#define BUS_ID_SIZE		20

struct rw_semaphore { int i; };
struct device {
	struct device		*parent;
	struct class		*class;
	char	bus_id[BUS_ID_SIZE];	/* position on parent bus */
	dev_t			devt;	/* dev_t, creates the sysfs "dev" */
	void	(*release)(struct device *dev);
};
struct mutex { int i; };
struct kernel_param { int i; };

struct cdev {
	int owner;
	dev_t dev;
};
#define cdev_init(...)		do { } while (0)
#define cdev_add(...)		0
#define cdev_del(...)		do { } while (0)

#define MAX_ERRNO		4095
#define IS_ERR_VALUE(x)		((x) >= (unsigned long)-MAX_ERRNO)

static inline void *ERR_PTR(long error)
{
	return (void *) error;
}

static inline long PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

static inline long IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

/* Force a compilation error if condition is true */
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

/* module */
#define THIS_MODULE		0
#define try_module_get(...)	1
#define module_put(...)		do { } while (0)
#define module_init(...)
#define module_exit(...)
#define EXPORT_SYMBOL(...)
#define EXPORT_SYMBOL_GPL(...)
#define module_param_call(...)
#define MODULE_PARM_DESC(...)
#define MODULE_VERSION(...)
#define MODULE_DESCRIPTION(...)
#define MODULE_AUTHOR(...)
#define MODULE_LICENSE(...)

#ifndef __UBIFS_H__
#include "../drivers/mtd/ubi/ubi.h"
#endif

/* functions */
extern int ubi_mtd_param_parse(const char *val, struct kernel_param *kp);
extern int ubi_init(void);
extern void ubi_exit(void);

extern struct ubi_device *ubi_devices[];

#endif
