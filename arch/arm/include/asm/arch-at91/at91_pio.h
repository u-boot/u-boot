/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91_pio.h]
 *
 * Copyright (C) 2005 Ivan Kokshaysky
 * Copyright (C) SAN People
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 *
 * Parallel I/O Controller (PIO) - System peripherals registers.
 * Based on AT91RM9200 datasheet revision E.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef AT91_PIO_H
#define AT91_PIO_H


#define AT91_ASM_PIO_RANGE	0x200
#define AT91_ASM_PIOC_ASR	\
	(ATMEL_BASE_PIO + AT91_PIO_PORTC * AT91_ASM_PIO_RANGE + 0x70)
#define AT91_ASM_PIOC_BSR	\
	(ATMEL_BASE_PIO + AT91_PIO_PORTC * AT91_ASM_PIO_RANGE + 0x74)
#define AT91_ASM_PIOC_PDR	\
	(ATMEL_BASE_PIO + AT91_PIO_PORTC * AT91_ASM_PIO_RANGE + 0x04)
#define AT91_ASM_PIOC_PUDR	\
	(ATMEL_BASE_PIO + AT91_PIO_PORTC * AT91_ASM_PIO_RANGE + 0x60)

#define AT91_ASM_PIOD_PDR	\
	(ATMEL_BASE_PIO + AT91_PIO_PORTD * AT91_ASM_PIO_RANGE + 0x04)
#define AT91_ASM_PIOD_PUDR	\
	(ATMEL_BASE_PIO + AT91_PIO_PORTD * AT91_ASM_PIO_RANGE + 0x60)
#define AT91_ASM_PIOD_ASR	\
	(ATMEL_BASE_PIO + AT91_PIO_PORTD * AT91_ASM_PIO_RANGE + 0x70)

#ifndef __ASSEMBLY__

typedef struct at91_port {
	u32	per;		/* 0x00 PIO Enable Register */
	u32	pdr;		/* 0x04 PIO Disable Register */
	u32	psr;		/* 0x08 PIO Status Register */
	u32	reserved0;
	u32	oer;		/* 0x10 Output Enable Register */
	u32	odr;		/* 0x14 Output Disable Registerr */
	u32	osr;		/* 0x18 Output Status Register */
	u32	reserved1;
	u32	ifer;		/* 0x20 Input Filter Enable Register */
	u32	ifdr;		/* 0x24 Input Filter Disable Register */
	u32	ifsr;		/* 0x28 Input Filter Status Register */
	u32	reserved2;
	u32	sodr;		/* 0x30 Set Output Data Register */
	u32	codr;		/* 0x34 Clear Output Data Register */
	u32	odsr;		/* 0x38 Output Data Status Register */
	u32	pdsr;		/* 0x3C Pin Data Status Register */
	u32	ier;		/* 0x40 Interrupt Enable Register */
	u32	idr;		/* 0x44 Interrupt Disable Register */
	u32	imr;		/* 0x48 Interrupt Mask Register */
	u32	isr;		/* 0x4C Interrupt Status Register */
	u32	mder;		/* 0x50 Multi-driver Enable Register */
	u32	mddr;		/* 0x54 Multi-driver Disable Register */
	u32	mdsr;		/* 0x58 Multi-driver Status Register */
	u32	reserved3;
	u32	pudr;		/* 0x60 Pull-up Disable Register */
	u32	puer;		/* 0x64 Pull-up Enable Register */
	u32	pusr;		/* 0x68 Pad Pull-up Status Register */
	u32	reserved4;
#if defined(CPU_HAS_PIO3)
	u32	abcdsr1;	/* 0x70 Peripheral ABCD Select Register 1 */
	u32	abcdsr2;	/* 0x74 Peripheral ABCD Select Register 2 */
	u32	reserved5[2];
	u32	ifscdr;		/* 0x80 Input Filter SCLK Disable Register */
	u32	ifscer;		/* 0x84 Input Filter SCLK Enable Register */
	u32	ifscsr;		/* 0x88 Input Filter SCLK Status Register */
	u32	scdr;		/* 0x8C SCLK Divider Debouncing Register */
	u32	ppddr;		/* 0x90 Pad Pull-down Disable Register */
	u32	ppder;		/* 0x94 Pad Pull-down Enable Register */
	u32	ppdsr;		/* 0x98 Pad Pull-down Status Register */
	u32	reserved6;	/*  */
#else
	u32	asr;		/* 0x70 Select A Register */
	u32	bsr;		/* 0x74 Select B Register */
	u32	absr;		/* 0x78 AB Select Status Register */
	u32	reserved5[9];	/*  */
#endif
	u32	ower;		/* 0xA0 Output Write Enable Register */
	u32	owdr;		/* 0xA4 Output Write Disable Register */
	u32	owsr;		/* OxA8 Output Write Status Register */
#if defined(CPU_HAS_PIO3)
	u32	reserved7;	/*  */
	u32	aimer;		/* 0xB0 Additional INT Modes Enable Register */
	u32	aimdr;		/* 0xB4 Additional INT Modes Disable Register */
	u32	aimmr;		/* 0xB8 Additional INT Modes Mask Register */
	u32	reserved8;	/* */
	u32	esr;		/* 0xC0 Edge Select Register */
	u32	lsr;		/* 0xC4 Level Select Register */
	u32	elsr;		/* 0xC8 Edge/Level Status Register */
	u32	reserved9;	/* 0xCC */
	u32	fellsr;		/* 0xD0 Falling /Low Level Select Register */
	u32	rehlsr;		/* 0xD4 Rising /High Level Select Register */
	u32	frlhsr;		/* 0xD8 Fall/Rise - Low/High Status Register */
	u32	reserved10;	/* */
	u32	locksr;		/* 0xE0 Lock Status */
	u32	wpmr;		/* 0xE4 Write Protect Mode Register */
	u32	wpsr;		/* 0xE8 Write Protect Status Register */
	u32	reserved11[5];	/* */
	u32	schmitt;	/* 0x100 Schmitt Trigger Register */
	u32	reserved12[63];
#else
	u32	reserved6[85];
#endif
} at91_port_t;

typedef union at91_pio {
	struct {
		at91_port_t	pioa;
		at91_port_t	piob;
		at91_port_t	pioc;
	#if (ATMEL_PIO_PORTS > 3)
		at91_port_t	piod;
	#endif
	#if (ATMEL_PIO_PORTS > 4)
		at91_port_t	pioe;
	#endif
	} ;
	at91_port_t port[ATMEL_PIO_PORTS];
} at91_pio_t;

#ifdef CONFIG_AT91_GPIO
int at91_set_a_periph(unsigned port, unsigned pin, int use_pullup);
int at91_set_b_periph(unsigned port, unsigned pin, int use_pullup);
#if defined(CPU_HAS_PIO3)
int at91_set_c_periph(unsigned port, unsigned pin, int use_pullup);
int at91_set_d_periph(unsigned port, unsigned pin, int use_pullup);
int at91_set_pio_debounce(unsigned port, unsigned pin, int is_on, int div);
int at91_set_pio_pulldown(unsigned port, unsigned pin, int is_on);
int at91_set_pio_disable_schmitt_trig(unsigned port, unsigned pin);
#endif
int at91_set_pio_input(unsigned port, unsigned pin, int use_pullup);
int at91_set_pio_multi_drive(unsigned port, unsigned pin, int is_on);
int at91_set_pio_output(unsigned port, unsigned pin, int value);
int at91_set_pio_periph(unsigned port, unsigned pin, int use_pullup);
int at91_set_pio_pullup(unsigned port, unsigned pin, int use_pullup);
int at91_set_pio_deglitch(unsigned port, unsigned pin, int is_on);
int at91_set_pio_value(unsigned port, unsigned pin, int value);
int at91_get_pio_value(unsigned port, unsigned pin);
#endif
#endif

#define	AT91_PIO_PORTA		0x0
#define	AT91_PIO_PORTB		0x1
#define	AT91_PIO_PORTC		0x2
#define	AT91_PIO_PORTD		0x3
#define	AT91_PIO_PORTE		0x4

#ifdef CONFIG_AT91_LEGACY

#define PIO_PER		0x00	/* Enable Register */
#define PIO_PDR		0x04	/* Disable Register */
#define PIO_PSR		0x08	/* Status Register */
#define PIO_OER		0x10	/* Output Enable Register */
#define PIO_ODR		0x14	/* Output Disable Register */
#define PIO_OSR		0x18	/* Output Status Register */
#define PIO_IFER	0x20	/* Glitch Input Filter Enable */
#define PIO_IFDR	0x24	/* Glitch Input Filter Disable */
#define PIO_IFSR	0x28	/* Glitch Input Filter Status */
#define PIO_SODR	0x30	/* Set Output Data Register */
#define PIO_CODR	0x34	/* Clear Output Data Register */
#define PIO_ODSR	0x38	/* Output Data Status Register */
#define PIO_PDSR	0x3c	/* Pin Data Status Register */
#define PIO_IER		0x40	/* Interrupt Enable Register */
#define PIO_IDR		0x44	/* Interrupt Disable Register */
#define PIO_IMR		0x48	/* Interrupt Mask Register */
#define PIO_ISR		0x4c	/* Interrupt Status Register */
#define PIO_MDER	0x50	/* Multi-driver Enable Register */
#define PIO_MDDR	0x54	/* Multi-driver Disable Register */
#define PIO_MDSR	0x58	/* Multi-driver Status Register */
#define PIO_PUDR	0x60	/* Pull-up Disable Register */
#define PIO_PUER	0x64	/* Pull-up Enable Register */
#define PIO_PUSR	0x68	/* Pull-up Status Register */
#define PIO_ASR		0x70	/* Peripheral A Select Register */
#define PIO_BSR		0x74	/* Peripheral B Select Register */
#define PIO_ABSR	0x78	/* AB Status Register */
#define PIO_OWER	0xa0	/* Output Write Enable Register */
#define PIO_OWDR	0xa4	/* Output Write Disable Register */
#define PIO_OWSR	0xa8	/* Output Write Status Register */
#endif

#endif
