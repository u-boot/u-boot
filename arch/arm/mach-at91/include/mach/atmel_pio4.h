/*
 * Copyright (C) 2015 Atmel Corporation.
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ATMEL_PIO4_H
#define __ATMEL_PIO4_H

#ifndef __ASSEMBLY__

struct atmel_pio4_port {
	u32 mskr;		/* 0x00 PIO Mask Register */
	u32 cfgr;		/* 0x04 PIO Configuration Register */
	u32 pdsr;		/* 0x08 PIO Pin Data Status Register */
	u32 locksr;		/* 0x0C PIO Lock Status Register */
	u32 sodr;		/* 0x10 PIO Set Output Data Register */
	u32 codr;		/* 0x14 PIO Clear Output Data Register */
	u32 odsr;		/* 0x18 PIO Output Data Status Register */
	u32 reserved0;
	u32 ier;		/* 0x20 PIO Interrupt Enable Register */
	u32 idr;		/* 0x24 PIO Interrupt Disable Register */
	u32 imr;		/* 0x28 PIO Interrupt Mask Register */
	u32 isr;		/* 0x2C PIO Interrupt Status Register */
	u32 reserved1[3];
	u32 iofr;		/* 0x3C PIO I/O Freeze Register */
};

#endif

#define AT91_PIO_PORTA		0x0
#define AT91_PIO_PORTB		0x1
#define AT91_PIO_PORTC		0x2
#define AT91_PIO_PORTD		0x3

int atmel_pio4_set_gpio(u32 port, u32 pin, u32 use_pullup);
int atmel_pio4_set_a_periph(u32 port, u32 pin, u32 use_pullup);
int atmel_pio4_set_b_periph(u32 port, u32 pin, u32 use_pullup);
int atmel_pio4_set_c_periph(u32 port, u32 pin, u32 use_pullup);
int atmel_pio4_set_d_periph(u32 port, u32 pin, u32 use_pullup);
int atmel_pio4_set_e_periph(u32 port, u32 pin, u32 use_pullup);
int atmel_pio4_set_f_periph(u32 port, u32 pin, u32 use_pullup);
int atmel_pio4_set_g_periph(u32 port, u32 pin, u32 use_pullup);
int atmel_pio4_set_pio_output(u32 port, u32 pin, u32 value);
int atmel_pio4_get_pio_input(u32 port, u32 pin);

#endif
