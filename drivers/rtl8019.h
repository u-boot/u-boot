/*
 * Realtek 8019AS Ethernet
 * (C) Copyright 2002-2003
 * Xue Ligong(lgxue@hotmail.com),Wang Kehao, ESLAB, whut.edu.cn
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

/*
 * This code works in 8bit mode.
 * If you need to work in 16bit mode, PLS change it!
 */

#include <asm/types.h>
#include <config.h>


#ifdef CONFIG_DRIVER_RTL8019

#define		RTL8019_REG_00        		(RTL8019_BASE + 0x00)
#define 	RTL8019_REG_01        		(RTL8019_BASE + 0x01)
#define 	RTL8019_REG_02        		(RTL8019_BASE + 0x02)
#define 	RTL8019_REG_03        		(RTL8019_BASE + 0x03)
#define 	RTL8019_REG_04        		(RTL8019_BASE + 0x04)
#define 	RTL8019_REG_05        		(RTL8019_BASE + 0x05)
#define 	RTL8019_REG_06        		(RTL8019_BASE + 0x06)
#define 	RTL8019_REG_07        		(RTL8019_BASE + 0x07)
#define 	RTL8019_REG_08        		(RTL8019_BASE + 0x08)
#define 	RTL8019_REG_09        		(RTL8019_BASE + 0x09)
#define 	RTL8019_REG_0a        		(RTL8019_BASE + 0x0a)
#define 	RTL8019_REG_0b        		(RTL8019_BASE + 0x0b)
#define 	RTL8019_REG_0c        		(RTL8019_BASE + 0x0c)
#define 	RTL8019_REG_0d        		(RTL8019_BASE + 0x0d)
#define 	RTL8019_REG_0e       	 	(RTL8019_BASE + 0x0e)
#define 	RTL8019_REG_0f        		(RTL8019_BASE + 0x0f)
#define 	RTL8019_REG_10        		(RTL8019_BASE + 0x10)
#define 	RTL8019_REG_1f        		(RTL8019_BASE + 0x1f)

#define		RTL8019_COMMAND			RTL8019_REG_00
#define		RTL8019_PAGESTART		RTL8019_REG_01
#define		RTL8019_PAGESTOP		RTL8019_REG_02
#define		RTL8019_BOUNDARY		RTL8019_REG_03
#define		RTL8019_TRANSMITSTATUS		RTL8019_REG_04
#define		RTL8019_TRANSMITPAGE		RTL8019_REG_04
#define		RTL8019_TRANSMITBYTECOUNT0	RTL8019_REG_05
#define		RTL8019_NCR 			RTL8019_REG_05
#define		RTL8019_TRANSMITBYTECOUNT1 	RTL8019_REG_06
#define		RTL8019_INTERRUPTSTATUS		RTL8019_REG_07
#define		RTL8019_CURRENT 		RTL8019_REG_07
#define		RTL8019_REMOTESTARTADDRESS0 	RTL8019_REG_08
#define		RTL8019_CRDMA0  		RTL8019_REG_08
#define		RTL8019_REMOTESTARTADDRESS1 	RTL8019_REG_09
#define		RTL8019_CRDMA1 			RTL8019_REG_09
#define		RTL8019_REMOTEBYTECOUNT0	RTL8019_REG_0a
#define		RTL8019_REMOTEBYTECOUNT1	RTL8019_REG_0b
#define		RTL8019_RECEIVESTATUS		RTL8019_REG_0c
#define		RTL8019_RECEIVECONFIGURATION	RTL8019_REG_0c
#define		RTL8019_TRANSMITCONFIGURATION	RTL8019_REG_0d
#define		RTL8019_FAE_TALLY 		RTL8019_REG_0d
#define		RTL8019_DATACONFIGURATION	RTL8019_REG_0e
#define		RTL8019_CRC_TALLY 		RTL8019_REG_0e
#define		RTL8019_INTERRUPTMASK		RTL8019_REG_0f
#define		RTL8019_MISS_PKT_TALLY		RTL8019_REG_0f
#define		RTL8019_PHYSICALADDRESS0	RTL8019_REG_01
#define 	RTL8019_PHYSICALADDRESS1	RTL8019_REG_02
#define		RTL8019_PHYSICALADDRESS2	RTL8019_REG_03
#define		RTL8019_PHYSICALADDRESS3	RTL8019_REG_04
#define		RTL8019_PHYSICALADDRESS4	RTL8019_REG_05
#define		RTL8019_PHYSICALADDRESS5	RTL8019_REG_06
#define		RTL8019_MULTIADDRESS0		RTL8019_REG_08
#define		RTL8019_MULTIADDRESS1		RTL8019_REG_09
#define		RTL8019_MULTIADDRESS2		RTL8019_REG_0a
#define		RTL8019_MULTIADDRESS3		RTL8019_REG_0b
#define		RTL8019_MULTIADDRESS4		RTL8019_REG_0c
#define		RTL8019_MULTIADDRESS5		RTL8019_REG_0d
#define		RTL8019_MULTIADDRESS6		RTL8019_REG_0e
#define		RTL8019_MULTIADDRESS7		RTL8019_REG_0f
#define		RTL8019_DMA_DATA		RTL8019_REG_10
#define		RTL8019_RESET			RTL8019_REG_1f


#define 	RTL8019_PAGE0               	0x22
#define   	RTL8019_PAGE1               	0x62
#define   	RTL8019_PAGE0DMAWRITE       	0x12
#define   	RTL8019_PAGE2DMAWRITE       	0x92
#define   	RTL8019_REMOTEDMAWR         	0x12
#define   	RTL8019_REMOTEDMARD         	0x0A
#define   	RTL8019_ABORTDMAWR          	0x32
#define   	RTL8019_ABORTDMARD          	0x2A
#define   	RTL8019_PAGE0STOP           	0x21
#define   	RTL8019_PAGE1STOP           	0x61
#define   	RTL8019_TRANSMIT            	0x26
#define   	RTL8019_TXINPROGRESS        	0x04
#define   	RTL8019_SEND		    	0x1A

#define		RTL8019_PSTART			0x4c
#define		RTL8019_PSTOP			0x80
#define		RTL8019_TPSTART			0x40


#endif /*end of CONFIG_DRIVER_RTL8019*/
