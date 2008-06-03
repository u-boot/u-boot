/***********************************************************************
 *
 * Copyright (C) 2004 by FS Forth-Systeme GmbH.
 * All rights reserved.
 *
 * $Id: ns9750_bbus.h,v 1.1 2004/02/16 10:37:20 mpietrek Exp $
 * @Author: Markus Pietrek
 * @Descr: Definitions for BBus usage
 * @References: [1] NS9750 Hardware Reference Manual/December 2003 Chap. 10
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
 *
 *
 ***********************************************************************/

#ifndef FS_NS9750_BBUS_H
#define FS_NS9750_BBUS_H

#define NS9750_BBUS_MODULE_BASE		(0x90600000)

#define get_bbus_reg_addr(c) \
	((volatile unsigned int *)(NS9750_BBUS_MODULE_BASE+(unsigned int) (c)))

/* We have support for 50 GPIO pins */

#define get_gpio_cfg_reg_addr(pin) \
	get_bbus_reg_addr( NS9750_BBUS_GPIO_CFG_BASE + (((pin) >> 3) * 4) )

/* To Read/Modify/Write a pin configuration register, use it like
   set_gpio_cfg_reg_val( 12, NS9750_GPIO_CFG_FUNC_GPIO|NS9750_GPIO_CFG_OUTPUT );
   They should be wrapped by cli()/sti() */
#define set_gpio_cfg_reg_val(pin,cfg) \
	*get_gpio_cfg_reg_addr(pin)=(*get_gpio_cfg_reg_addr((pin)) & \
					~NS9750_GPIO_CFG_MASK((pin))) |\
				NS9750_GPIO_CFG_VAL((pin),(cfg));

#define NS9750_GPIO_CFG_MASK(pin)	(NS9750_GPIO_CFG_VAL(pin, \
					 NS9750_GPIO_CFG_MA))
#define NS9750_GPIO_CFG_VAL(pin,cfg)	((cfg) << (((pin) % 8) * 4))

#define NS9750_GPIO_CFG_MA		(0x0F)
#define NS9750_GPIO_CFG_INPUT		(0x00)
#define NS9750_GPIO_CFG_OUTPUT		(0x08)
#define NS9750_GPIO_CFG_FUNC_GPIO	(0x03)
#define NS9750_GPIO_CFG_FUNC_2		(0x02)
#define NS9750_GPIO_CFG_FUNC_1		(0x01)
#define NS9750_GPIO_CFG_FUNC_0		(0x00)

/* the register addresses */

#define NS9750_BBUS_MASTER_RESET	(0x00)
#define NS9750_BBUS_GPIO_CFG_BASE	(0x10)
#define NS9750_BBUS_GPIO_CTRL_BASE	(0x30)
#define NS9750_BBUS_GPIO_STAT_BASE	(0x40)
#define NS9750_BBUS_MONITOR		(0x50)
#define NS9750_BBUS_DMA_INT_STAT	(0x60)
#define NS9750_BBUS_DMA_INT_ENABLE	(0x64)
#define NS9750_BBUS_USB_CFG		(0x70)
#define NS9750_BBUS_ENDIAN_CFG		(0x80)
#define NS9750_BBUS_ARM_WAKE_UP		(0x90)

/* register bit fields */

#define NS9750_BBUS_MASTER_RESET_UTIL	(0x00000100)
#define NS9750_BBUS_MASTER_RESET_I2C	(0x00000080)
#define NS9750_BBUS_MASTER_RESET_1284	(0x00000040)
#define NS9750_BBUS_MASTER_RESET_SER4	(0x00000020)
#define NS9750_BBUS_MASTER_RESET_SER3	(0x00000010)
#define NS9750_BBUS_MASTER_RESET_SER2	(0x00000008)
#define NS9750_BBUS_MASTER_RESET_SER1	(0x00000004)
#define NS9750_BBUS_MASTER_RESET_USB	(0x00000002)
#define NS9750_BBUS_MASTER_RESET_DMA	(0x00000001)

/* BS9750_BBUS_DMA_INT_BINT* are valid for *DMA_INT_STAT and *DMA_INT_ENABLE */

#define NS9750_BBUS_DMA_INT_BINT16	(0x00010000)
#define NS9750_BBUS_DMA_INT_BINT15	(0x00008000)
#define NS9750_BBUS_DMA_INT_BINT14	(0x00004000)
#define NS9750_BBUS_DMA_INT_BINT13	(0x00002000)
#define NS9750_BBUS_DMA_INT_BINT12	(0x00001000)
#define NS9750_BBUS_DMA_INT_BINT11	(0x00000800)
#define NS9750_BBUS_DMA_INT_BINT10	(0x00000400)
#define NS9750_BBUS_DMA_INT_BINT9	(0x00000200)
#define NS9750_BBUS_DMA_INT_BINT8	(0x00000100)
#define NS9750_BBUS_DMA_INT_BINT7	(0x00000080)
#define NS9750_BBUS_DMA_INT_BINT6	(0x00000040)
#define NS9750_BBUS_DMA_INT_BINT5	(0x00000020)
#define NS9750_BBUS_DMA_INT_BINT4	(0x00000010)
#define NS9750_BBUS_DMA_INT_BINT3	(0x00000008)
#define NS9750_BBUS_DMA_INT_BINT2	(0x00000004)
#define NS9750_BBUS_DMA_INT_BINT1	(0x00000002)
#define NS9750_BBUS_DMA_INT_BINT0	(0x00000001)

#define NS9750_BBUS_USB_CFG_OUTEN	(0x00000008)
#define NS9750_BBUS_USB_CFG_SPEED	(0x00000004)
#define NS9750_BBUS_USB_CFG_CFG_MA	(0x00000003)
#define NS9750_BBUS_USB_CFG_CFG_HOST_SOFT (0x00000003)
#define NS9750_BBUS_USB_CFG_CFG_DEVICE	(0x00000002)
#define NS9750_BBUS_USB_CFG_CFG_HOST	(0x00000001)
#define NS9750_BBUS_USB_CFG_CFG_DIS	(0x00000000)

#define NS9750_BBUS_ENDIAN_CFG_AHBM	(0x00001000)
#define NS9750_BBUS_ENDIAN_CFG_I2C	(0x00000080)
#define NS9750_BBUS_ENDIAN_CFG_IEEE1284	(0x00000040)
#define NS9750_BBUS_ENDIAN_CFG_SER4	(0x00000020)
#define NS9750_BBUS_ENDIAN_CFG_SER3	(0x00000010)
#define NS9750_BBUS_ENDIAN_CFG_SER2	(0x00000008)
#define NS9750_BBUS_ENDIAN_CFG_SER1	(0x00000004)
#define NS9750_BBUS_ENDIAN_CFG_USB	(0x00000002)
#define NS9750_BBUS_ENDIAN_CFG_DMA	(0x00000001)

#endif /* FS_NS9750_BBUS_H */
