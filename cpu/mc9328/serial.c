/*
 * cpu/mc9328/serial.c 
 * 
 * (c) Copyright 2004
 * Techware Information Technology, Inc.
 * http://www.techware.com.tw/
 *
 * Ming-Len Wu <minglen_wu@techware.com.tw>
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


#include <common.h>
#include <mc9328.h>

#if defined(CONFIG_UART1) 
/* GPIO PORT B 		*/

#define reg_GIUS	MX1_GIUS_C
#define reg_GPR		MX1_GPR_B
#define GPIO_MASK	0xFFFFE1FF
#define UART_BASE	0x00206000


#elif defined (CONFIG_UART2)
/* GPIO PORT C  	*/

#define reg_GIUS	MX1_GIUS_C
#define reg_GPR		MX1_GPR_C
#define GPIO_MASK 	0x0FFFFFFF
#define UART_BASE	0x207000

#endif 

#define reg_URXD	(*((volatile u32 *)(UART_BASE+0x00)))
#define reg_UTXD	(*((volatile u32 *)(UART_BASE+0x40)))
#define reg_UCR1	(*((volatile u32 *)(UART_BASE+0x80)))
#define reg_UCR2	(*((volatile u32 *)(UART_BASE+0x84)))
#define reg_UCR3	(*((volatile u32 *)(UART_BASE+0x88)))
#define reg_UCR4	(*((volatile u32 *)(UART_BASE+0x8C)))
#define reg_UFCR	(*((volatile u32 *)(UART_BASE+0x90)))
#define reg_USR1	(*((volatile u32 *)(UART_BASE+0x94)))
#define reg_USR2	(*((volatile u32 *)(UART_BASE+0x98)))
#define reg_UESC	(*((volatile u32 *)(UART_BASE+0x9C)))
#define reg_UTIM	(*((volatile u32 *)(UART_BASE+0xA0)))
#define reg_UBIR	(*((volatile u32 *)(UART_BASE+0xA4)))
#define reg_UBMR	(*((volatile u32 *)(UART_BASE+0xA8)))
#define reg_UBRC	(*((volatile u32 *)(UART_BASE+0xAC)))

#define TXFE_MASK	0x4000  	/* Tx buffer empty	*/
#define RDR_MASK	0x0001		/* receive data ready	*/


void serial_setbrg (void) {

/* config I/O pins for UART 	*/

	reg_GIUS 	&= GPIO_MASK;
	reg_GPR		&= GPIO_MASK;

/* config UART			*/

	reg_UCR1 	= 5;
	reg_UCR2 	= 0x4027;
	reg_UCR4 	= 1;
	reg_UFCR 	= 0xA81;

	reg_UBIR 	= 0xF;
	reg_UBMR 	= 0x8A;
	reg_UBRC 	= 8;
}



/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
 
int serial_init (void) {
	serial_setbrg ();

	return (0);
}



/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_getc (void) {

	while (!(reg_USR2 & RDR_MASK)) ; 	/* wait until RDR bit set 		*/

	return (u8)reg_URXD;
}


/*
 * Output a single byte to the serial port.
 */
void serial_putc (const char c) {

	while (!(reg_USR2 & TXFE_MASK));	/* wait until TXFE bit set		*/

	reg_UTXD = (u16) c;

	if (c == '\n')	{			/* carriage return ? append line-feed	*/
		while (!(reg_USR2 & TXFE_MASK));	/* wait until TXFE bit set	*/
		reg_UTXD = '\r';
	}

}


/*
 * Test whether a character is in the RX buffer
 */
int serial_tstc (void) {
	return reg_USR2 & RDR_MASK;
}


void serial_puts (const char *s) {
	while (*s) {
		serial_putc (*s++);
	}
}

