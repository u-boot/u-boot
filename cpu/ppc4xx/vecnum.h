/*
*  Copyright (C) 2002 Scott McNutt <smcnutt@artesyncp.com>
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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston,
* MA 02111-1307 USA
*/

/*
 * Interrupt vector number definitions to ease the
 * 405 -- 440 porting pain ;-)
 *
 * NOTE: They're not all here yet ... update as needed.
 *
 */

#ifndef _VECNUMS_H_
#define _VECNUMS_H_

#if defined(CONFIG_440SP)

/* UIC 0 */
#define VECNUM_U0           0           /* UART0                        */
#define VECNUM_U1           1           /* UART1                        */
#define VECNUM_IIC0         2           /* IIC0                         */
#define VECNUM_IIC1         3           /* IIC1                         */
#define VECNUM_PIM          4           /* PCI inbound message          */
#define VECNUM_PCRW         5           /* PCI command reg write        */
#define VECNUM_PPM          6           /* PCI power management         */
#define VECNUM_UIC1NC       30          /* UIC1 non-critical interrupt  */
#define VECNUM_UIC1C        31          /* UIC1 critical interrupt      */

/* UIC 1 */
#define VECNUM_EIR0         (32 + 0)	/* External interrupt 0         */
#define VECNUM_MS           (32 + 1)	/* MAL SERR                     */
#define VECNUM_TXDE         (32 + 2)	/* MAL TXDE                     */
#define VECNUM_RXDE         (32 + 3)	/* MAL RXDE                     */
#define VECNUM_MTE          (32 + 6)	/* MAL Tx EOB                   */
#define VECNUM_MRE          (32 + 7)	/* MAL Rx EOB                   */
#define VECNUM_CT0          (32 + 12)	/* GPT compare timer 0          */
#define VECNUM_CT1          (32 + 13)	/* GPT compare timer 1          */
#define VECNUM_CT2          (32 + 14)	/* GPT compare timer 2          */
#define VECNUM_CT3          (32 + 15)	/* GPT compare timer 3          */
#define VECNUM_CT4          (32 + 16)	/* GPT compare timer 4          */
#define VECNUM_ETH0         (32 + 28)	/* Ethernet interrupt status    */
#define VECNUM_EWU0         (32 + 29)	/* Emac  wakeup                 */

#elif defined(CONFIG_440)

/* UIC 0 */
#define VECNUM_U0           0           /* UART0                        */
#define VECNUM_U1           1           /* UART1                        */
#define VECNUM_IIC0         2           /* IIC0                         */
#define VECNUM_IIC1         3           /* IIC1                         */
#define VECNUM_PIM          4           /* PCI inbound message          */
#define VECNUM_PCRW         5           /* PCI command reg write        */
#define VECNUM_PPM          6           /* PCI power management         */
#define VECNUM_MSI0         7           /* PCI MSI level 0              */
#define VECNUM_MSI1         8           /* PCI MSI level 0              */
#define VECNUM_MSI2         9           /* PCI MSI level 0              */
#define VECNUM_MTE          10          /* MAL TXEOB                    */
#define VECNUM_MRE          11          /* MAL RXEOB                    */
#define VECNUM_D0           12          /* DMA channel 0                */
#define VECNUM_D1           13          /* DMA channel 1                */
#define VECNUM_D2           14          /* DMA channel 2                */
#define VECNUM_D3           15          /* DMA channel 3                */
#define VECNUM_CT0          18          /* GPT compare timer 0          */
#define VECNUM_CT1          19          /* GPT compare timer 1          */
#define VECNUM_CT2          20          /* GPT compare timer 2          */
#define VECNUM_CT3          21          /* GPT compare timer 3          */
#define VECNUM_CT4          22          /* GPT compare timer 4          */
#define VECNUM_EIR0         23          /* External interrupt 0         */
#define VECNUM_EIR1         24          /* External interrupt 1         */
#define VECNUM_EIR2         25          /* External interrupt 2         */
#define VECNUM_EIR3         26          /* External interrupt 3         */
#define VECNUM_EIR4         27          /* External interrupt 4         */
#define VECNUM_EIR5         28          /* External interrupt 5         */
#define VECNUM_EIR6         29          /* External interrupt 6         */
#define VECNUM_UIC1NC       30          /* UIC1 non-critical interrupt  */
#define VECNUM_UIC1C        31          /* UIC1 critical interrupt      */

/* UIC 1 */
#define VECNUM_MS           (32 + 0 )   /* MAL SERR                     */
#define VECNUM_TXDE         (32 + 1 )   /* MAL TXDE                     */
#define VECNUM_RXDE         (32 + 2 )   /* MAL RXDE                     */
#define VECNUM_USBDEV	    (32 + 23)   /* USB 1.1/USB 2.0 Device       */
#define VECNUM_ETH0         (32 + 28)   /* Ethernet 0 interrupt status  */
#define VECNUM_EWU0         (32 + 29)   /* Ethernet 0 wakeup            */

#else /* !defined(CONFIG_440) */

#define VECNUM_U0           0           /* UART0                        */
#define VECNUM_U1           1           /* UART1                        */
#define VECNUM_D0           5           /* DMA channel 0                */
#define VECNUM_D1           6           /* DMA channel 1                */
#define VECNUM_D2           7           /* DMA channel 2                */
#define VECNUM_D3           8           /* DMA channel 3                */
#define VECNUM_EWU0         9           /* Ethernet wakeup              */
#define VECNUM_MS           10          /* MAL SERR                     */
#define VECNUM_MTE          11          /* MAL TXEOB                    */
#define VECNUM_MRE          12          /* MAL RXEOB                    */
#define VECNUM_TXDE         13          /* MAL TXDE                     */
#define VECNUM_RXDE         14          /* MAL RXDE                     */
#define VECNUM_ETH0         15          /* Ethernet interrupt status    */
#define VECNUM_EIR0         25          /* External interrupt 0         */
#define VECNUM_EIR1         26          /* External interrupt 1         */
#define VECNUM_EIR2         27          /* External interrupt 2         */
#define VECNUM_EIR3         28          /* External interrupt 3         */
#define VECNUM_EIR4         29          /* External interrupt 4         */
#define VECNUM_EIR5         30          /* External interrupt 5         */
#define VECNUM_EIR6         31          /* External interrupt 6         */

#endif /* defined(CONFIG_440) */

#endif /* _VECNUMS_H_ */
