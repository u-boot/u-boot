/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _UBOOT_COMPAT_H__
#define _UBOOT_COMPAT_H__


#include <pci.h>
#include <pci_ids.h>
#include <common.h>
#include <malloc.h>
#include <net.h>

#define	__initdata
#define __init
#define __exit

#define netif_stop_queue(x)
#define netif_wake_queue(x)
#define netif_running(x)		0
#define unregister_netdev(x)
#define remove_proc_entry(x,y)

#define dev_addr			enetaddr

#define	spin_lock_irqsave(x,y) y = 0;
#define spin_lock_init(x)
#define spin_lock(x)
#define spin_unlock_irqrestore(x,y)
#define spin_unlock(x)


#define ENODEV				1
#define EAGAIN				2
#define EBUSY				3

#define HZ				CFG_HZ


#define printk				printf
#define KERN_ERR
#define KERN_WARNING
#define KERN_INFO

#define MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT


#define kmalloc(x,y)			malloc(x)
#define kfree(x)			free(x)
#define GFP_ATOMIC			0

#define pci_alloc_consistent(x,y,z)	(void *)(*(dma_addr_t *)(z) = (dma_addr_t)malloc(y))
#define pci_free_consistent(x,y,z,d)	free(z)
#define pci_dma_sync_single(x,y,z,d)
#define pci_unmap_page(x,y,z,d)
#define pci_unmap_single(x,y,z,d)
#define pci_present()			1

struct sk_buff
{
	u8 * data;
	u32 len;
	u8 * data_unaligned;
};

struct sk_buff * alloc_skb(u32 size, int dummy);
void dev_kfree_skb_any(struct sk_buff *skb);
void skb_reserve(struct sk_buff *skb, unsigned int len);
void skb_put(struct sk_buff *skb, unsigned int len);

#define dev_kfree_skb				dev_kfree_skb_any
#define dev_kfree_skb_irq			dev_kfree_skb_any

#define eth_copy_and_sum(dest,src,len,base)	memcpy(dest->data,src,len);


#endif	/* _UBOOT_COMPAT_H__ */
