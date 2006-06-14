/*
 * (C) Copyright 2006
 * Bryan O'Donoghue, deckard@codehermit.ie, CodeHermit
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* ACM Control Requests */
#define ACM_SEND_ENCAPSULATED_COMMAND	0x00
#define ACM_GET_ENCAPSULATED_RESPONSE	0x01
#define ACM_SET_COMM_FEATURE		0x02
#define ACM_GET_COMM_FEATRUE		0x03
#define ACM_CLEAR_COMM_FEATURE		0x04
#define ACM_SET_LINE_ENCODING		0x20
#define ACM_GET_LINE_ENCODING		0x21
#define ACM_SET_CONTROL_LINE_STATE	0x22
#define ACM_SEND_BREAK			0x23

/* ACM Notification Codes */
#define ACM_NETWORK_CONNECTION		0x00
#define ACM_RESPONSE_AVAILABLE		0x01
#define ACM_SERIAL_STATE		0x20

/* Format of response expected by a ACM_GET_LINE_ENCODING request */
struct rs232_emu{
		unsigned long dter;
		unsigned char stop_bits;
		unsigned char parity;
		unsigned char data_bits;
}__attribute__((packed));
