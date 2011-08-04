/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
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

#ifndef __ASM_IC_ALI512X_H_
#define __ASM_IC_ALI512X_H_

# define ALI_INDEX    0x3f0
# define ALI_DATA     0x3f1

# define ALI_ENABLED  1
# define ALI_DISABLED 0

# define ALI_UART1    0
# define ALI_UART2    1

/* setup functions */
void ali512x_init(void);
void ali512x_set_fdc(int enabled, u16 io, u8 irq, u8 dma_channel);
void ali512x_set_pp(int enabled, u16 io, u8 irq, u8 dma_channel);
void ali512x_set_uart(int enabled, int index, u16 io, u8 irq);
void ali512x_set_rtc(int enabled, u16 io, u8 irq);
void ali512x_set_kbc(int enabled, u8 kbc_irq, u8 mouse_irq);
void ali512x_set_cio(int enabled);


/* common I/O functions */
void ali512x_cio_function(int pin, int special, int inv, int input);
void ali512x_cio_out(int pin, int value);
int ali512x_cio_in(int pin);

/* misc features */
void ali512x_set_uart2_irda(int enabled);

#endif
