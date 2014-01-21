/*
 * Copyright 2006-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __ARCH_BLACKFIN_GPIO_H__
#define __ARCH_BLACKFIN_GPIO_H__

#include <asm-generic/gpio.h>
#include <asm/portmux.h>

#define gpio_bank(x)	((x) >> 4)
#define gpio_bit(x)	(1<<((x) & 0xF))
#define gpio_sub_n(x)	((x) & 0xF)

#define GPIO_BANKSIZE	16
#define GPIO_BANK_NUM	DIV_ROUND_UP(MAX_BLACKFIN_GPIOS, GPIO_BANKSIZE)

#define GPIO_0	0
#define GPIO_1	1
#define GPIO_2	2
#define GPIO_3	3
#define GPIO_4	4
#define GPIO_5	5
#define GPIO_6	6
#define GPIO_7	7
#define GPIO_8	8
#define GPIO_9	9
#define GPIO_10	10
#define GPIO_11	11
#define GPIO_12	12
#define GPIO_13	13
#define GPIO_14	14
#define GPIO_15	15
#define GPIO_16	16
#define GPIO_17	17
#define GPIO_18	18
#define GPIO_19	19
#define GPIO_20	20
#define GPIO_21	21
#define GPIO_22	22
#define GPIO_23	23
#define GPIO_24	24
#define GPIO_25	25
#define GPIO_26	26
#define GPIO_27	27
#define GPIO_28	28
#define GPIO_29	29
#define GPIO_30	30
#define GPIO_31	31
#define GPIO_32	32
#define GPIO_33	33
#define GPIO_34	34
#define GPIO_35	35
#define GPIO_36	36
#define GPIO_37	37
#define GPIO_38	38
#define GPIO_39	39
#define GPIO_40	40
#define GPIO_41	41
#define GPIO_42	42
#define GPIO_43	43
#define GPIO_44	44
#define GPIO_45	45
#define GPIO_46	46
#define GPIO_47	47

#define PERIPHERAL_USAGE 1
#define GPIO_USAGE 0
#define MAX_GPIOS MAX_BLACKFIN_GPIOS

#ifndef __ASSEMBLY__

#ifndef CONFIG_ADI_GPIO2
void set_gpio_dir(unsigned, unsigned short);
void set_gpio_inen(unsigned, unsigned short);
void set_gpio_polar(unsigned, unsigned short);
void set_gpio_edge(unsigned, unsigned short);
void set_gpio_both(unsigned, unsigned short);
void set_gpio_data(unsigned, unsigned short);
void set_gpio_maska(unsigned, unsigned short);
void set_gpio_maskb(unsigned, unsigned short);
void set_gpio_toggle(unsigned);
void set_gpiop_dir(unsigned, unsigned short);
void set_gpiop_inen(unsigned, unsigned short);
void set_gpiop_polar(unsigned, unsigned short);
void set_gpiop_edge(unsigned, unsigned short);
void set_gpiop_both(unsigned, unsigned short);
void set_gpiop_data(unsigned, unsigned short);
void set_gpiop_maska(unsigned, unsigned short);
void set_gpiop_maskb(unsigned, unsigned short);
unsigned short get_gpio_dir(unsigned);
unsigned short get_gpio_inen(unsigned);
unsigned short get_gpio_polar(unsigned);
unsigned short get_gpio_edge(unsigned);
unsigned short get_gpio_both(unsigned);
unsigned short get_gpio_maska(unsigned);
unsigned short get_gpio_maskb(unsigned);
unsigned short get_gpio_data(unsigned);
unsigned short get_gpiop_dir(unsigned);
unsigned short get_gpiop_inen(unsigned);
unsigned short get_gpiop_polar(unsigned);
unsigned short get_gpiop_edge(unsigned);
unsigned short get_gpiop_both(unsigned);
unsigned short get_gpiop_maska(unsigned);
unsigned short get_gpiop_maskb(unsigned);
unsigned short get_gpiop_data(unsigned);

struct gpio_port_t {
	unsigned short data;
	unsigned short dummy1;
	unsigned short data_clear;
	unsigned short dummy2;
	unsigned short data_set;
	unsigned short dummy3;
	unsigned short toggle;
	unsigned short dummy4;
	unsigned short maska;
	unsigned short dummy5;
	unsigned short maska_clear;
	unsigned short dummy6;
	unsigned short maska_set;
	unsigned short dummy7;
	unsigned short maska_toggle;
	unsigned short dummy8;
	unsigned short maskb;
	unsigned short dummy9;
	unsigned short maskb_clear;
	unsigned short dummy10;
	unsigned short maskb_set;
	unsigned short dummy11;
	unsigned short maskb_toggle;
	unsigned short dummy12;
	unsigned short dir;
	unsigned short dummy13;
	unsigned short polar;
	unsigned short dummy14;
	unsigned short edge;
	unsigned short dummy15;
	unsigned short both;
	unsigned short dummy16;
	unsigned short inen;
};
#else
extern struct gpio_port_t * const gpio_array[];
#endif

#ifdef ADI_SPECIAL_GPIO_BANKS
void special_gpio_free(unsigned gpio);
int special_gpio_request(unsigned gpio, const char *label);
#endif

void gpio_labels(void);

static inline int gpio_is_valid(int number)
{
	return number >= 0 && number < MAX_GPIOS;
}

#include <linux/ctype.h>

static inline int name_to_gpio(const char *name)
{
	int port_base;

	if (tolower(*name) == 'p') {
		++name;

		switch (tolower(*name)) {
#ifdef GPIO_PA0
		case 'a': port_base = GPIO_PA0; break;
#endif
#ifdef GPIO_PB0
		case 'b': port_base = GPIO_PB0; break;
#endif
#ifdef GPIO_PC0
		case 'c': port_base = GPIO_PC0; break;
#endif
#ifdef GPIO_PD0
		case 'd': port_base = GPIO_PD0; break;
#endif
#ifdef GPIO_PE0
		case 'e': port_base = GPIO_PE0; break;
#endif
#ifdef GPIO_PF0
		case 'f': port_base = GPIO_PF0; break;
#endif
#ifdef GPIO_PG0
		case 'g': port_base = GPIO_PG0; break;
#endif
#ifdef GPIO_PH0
		case 'h': port_base = GPIO_PH0; break;
#endif
#ifdef GPIO_PI0
		case 'i': port_base = GPIO_PI0; break;
#endif
#ifdef GPIO_PJ
		case 'j': port_base = GPIO_PJ0; break;
#endif
		default:  return -1;
		}

		++name;
	} else
		port_base = 0;

	return port_base + simple_strtoul(name, NULL, 10);
}
#define name_to_gpio(n) name_to_gpio(n)

#define gpio_status() gpio_labels()

#endif /* __ASSEMBLY__ */

#endif /* __ARCH_BLACKFIN_GPIO_H__ */
