/*
 * Copyright (c) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * This is a Linux kernel compatibility layer for USB Gadget
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __LIN_COMPAT_H__
#define __LIN_COMPAT_H__

/* common */
#define spin_lock_init(...)
#define spin_lock(...)
#define spin_lock_irqsave(lock, flags) do { debug("%lu\n", flags); } while (0)
#define spin_unlock(...)
#define spin_unlock_irqrestore(lock, flags) do {flags = 0; } while (0)
#define disable_irq(...)
#define enable_irq(...)

#define mutex_init(...)
#define mutex_lock(...)
#define mutex_unlock(...)

#define WARN_ON(x) if (x) {printf("WARNING in %s line %d\n" \
				  , __FILE__, __LINE__); }

#define KERN_WARNING
#define KERN_ERR
#define KERN_NOTICE
#define KERN_DEBUG

#define GFP_KERNEL	0

#define IRQ_HANDLED	1

#define ENOTSUPP	524	/* Operation is not supported */

#define kmalloc(size, type) memalign(CONFIG_SYS_CACHELINE_SIZE, size)
#define kfree(addr) free(addr)
#define mdelay(n) ({unsigned long msec = (n); while (msec--) udelay(1000); })

#define __iomem
#define min_t min
#define dma_cache_maint(addr, size, mode) cache_flush()
void cache_flush(void);

#endif /* __LIN_COMPAT_H__ */
