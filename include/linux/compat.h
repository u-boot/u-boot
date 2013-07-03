#ifndef _LINUX_COMPAT_H_
#define _LINUX_COMPAT_H_

#define ndelay(x)	udelay(1)

#define dev_dbg(dev, fmt, args...)		\
	debug(fmt, ##args)
#define dev_vdbg(dev, fmt, args...)		\
	debug(fmt, ##args)
#define dev_info(dev, fmt, args...)		\
	printf(fmt, ##args)
#define dev_err(dev, fmt, args...)		\
	printf(fmt, ##args)
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
#define kzalloc(size, flags)	calloc(size, 1)
#define vmalloc(size)		malloc(size)
#define kfree(ptr)		free(ptr)
#define vfree(ptr)		free(ptr)

#define DECLARE_WAITQUEUE(...)	do { } while (0)
#define add_wait_queue(...)	do { } while (0)
#define remove_wait_queue(...)	do { } while (0)

#define KERNEL_VERSION(a,b,c)	(((a) << 16) + ((b) << 8) + (c))

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

#ifndef BUG
#define BUG() do { \
	printf("U-Boot BUG at %s:%d!\n", __FILE__, __LINE__); \
} while (0)

#define BUG_ON(condition) do { if (condition) BUG(); } while(0)
#endif /* BUG */

#define WARN_ON(x) if (x) {printf("WARNING in %s line %d\n" \
				  , __FILE__, __LINE__); }

#define PAGE_SIZE	4096
#endif
