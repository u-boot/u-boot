#ifndef _LINUX_COMPAT_H_
#define _LINUX_COMPAT_H_

#define __user
#define __iomem

#define ndelay(x)	udelay(1)

#define printk	printf

#define KERN_EMERG
#define KERN_ALERT
#define KERN_CRIT
#define KERN_ERR
#define KERN_WARNING
#define KERN_NOTICE
#define KERN_INFO
#define KERN_DEBUG

#define kmalloc(size, flags)	malloc(size)
#define kfree(ptr)		free(ptr)

/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max at all, of course.
 */
#define min_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })

#define BUG() do { \
	printf("U-Boot BUG at %s:%d!\n", __FILE__, __LINE__); \
} while (0)

#define BUG_ON(condition) do { if (condition) BUG(); } while(0)

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#define PAGE_SIZE	4096
#endif
