/*
 * (C) Copyright 2007-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_PPC_GPIO_H
#define __ASM_PPC_GPIO_H

#include <asm/types.h>

/* 4xx PPC's have 2 GPIO controllers */
#if defined(CONFIG_405EZ) ||					\
	defined(CONFIG_440EP) || defined(CONFIG_440GR) ||	\
	defined(CONFIG_440EPX) || defined(CONFIG_440GRX) ||	\
	defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define GPIO_GROUP_MAX	2
#else
#define GPIO_GROUP_MAX	1
#endif

/* GPIO controller */
struct ppc4xx_gpio {
	u32 or;		/* Output Control */
	u32 tcr;	/* Tri-State Control */
	u32 osl;	/* Output Select 16..31 */
	u32 osh;	/* Output Select 0..15 */
	u32 tsl;	/* Tri-State Select 16..31 */
	u32 tsh;	/* Tri-State Select 0..15 */
	u32 odr;	/* Open Drain */
	u32 ir;		/* Input */
	u32 rr1;	/* Receive Register 1 */
	u32 rr2;	/* Receive Register 2 */
	u32 rr3;	/* Receive Register 3 */
	u32 reserved;
	u32 is1l;	/* Input Select 1 16..31 */
	u32 is1h;	/* Input Select 1 0..15 */
	u32 is2l;	/* Input Select 2 16..31 */
	u32 is2h;	/* Input Select 2 0..15 */
	u32 is3l;	/* Input Select 3 16..31 */
	u32 is3h;	/* Input Select 3 0..15 */
};

/* Offsets */
#define GPIOx_OR	0x00		/* GPIO Output Register */
#define GPIOx_TCR	0x04		/* GPIO Three-State Control Register */
#define GPIOx_OSL	0x08		/* GPIO Output Select Register (Bits 0-31) */
#define GPIOx_OSH	0x0C		/* GPIO Ouput Select Register (Bits 32-63) */
#define GPIOx_TSL	0x10		/* GPIO Three-State Select Register (Bits 0-31) */
#define GPIOx_TSH	0x14		/* GPIO Three-State Select Register  (Bits 32-63) */
#define GPIOx_ODR	0x18		/* GPIO Open drain Register */
#define GPIOx_IR	0x1C		/* GPIO Input Register */
#define GPIOx_RR1	0x20		/* GPIO Receive Register 1 */
#define GPIOx_RR2	0x24		/* GPIO Receive Register 2 */
#define GPIOx_RR3	0x28		/* GPIO Receive Register 3 */
#define GPIOx_IS1L	0x30		/* GPIO Input Select Register 1 (Bits 0-31) */
#define GPIOx_IS1H	0x34		/* GPIO Input Select Register 1 (Bits 32-63) */
#define GPIOx_IS2L	0x38		/* GPIO Input Select Register 2 (Bits 0-31) */
#define GPIOx_IS2H	0x3C		/* GPIO Input Select Register 2 (Bits 32-63) */
#define GPIOx_IS3L	0x40		/* GPIO Input Select Register 3 (Bits 0-31) */
#define GPIOx_IS3H	0x44		/* GPIO Input Select Register 3 (Bits 32-63) */

#define GPIO_OR(x)	(x+GPIOx_OR)	/* GPIO Output Register */
#define GPIO_TCR(x)	(x+GPIOx_TCR)	/* GPIO Three-State Control Register */
#define GPIO_OS(x)	(x+GPIOx_OSL)	/* GPIO Output Select Register High or Low */
#define GPIO_TS(x)	(x+GPIOx_TSL)	/* GPIO Three-state Control Reg High or Low */
#define GPIO_IS1(x)	(x+GPIOx_IS1L)	/* GPIO Input register1 High or Low */
#define GPIO_IS2(x)	(x+GPIOx_IS2L)	/* GPIO Input register2 High or Low */
#define GPIO_IS3(x)	(x+GPIOx_IS3L)	/* GPIO Input register3 High or Low */

#define GPIO0		0
#define GPIO1		1

#define GPIO_MAX	32
#define GPIO_ALT1_SEL	0x40000000
#define GPIO_ALT2_SEL	0x80000000
#define GPIO_ALT3_SEL	0xc0000000
#define GPIO_IN_SEL	0x40000000
#define GPIO_MASK	0xc0000000

#define GPIO_VAL(gpio)	(0x80000000 >> (gpio))

#ifndef __ASSEMBLY__
typedef enum gpio_select { GPIO_SEL, GPIO_ALT1, GPIO_ALT2, GPIO_ALT3 } gpio_select_t;
typedef enum gpio_driver { GPIO_DIS, GPIO_IN, GPIO_OUT, GPIO_BI } gpio_driver_t;
typedef enum gpio_out	 { GPIO_OUT_0, GPIO_OUT_1, GPIO_OUT_NO_CHG } gpio_out_t;

typedef struct {
	unsigned long	add;	/* gpio core base address	*/
	gpio_driver_t	in_out;	/* Driver Setting		*/
	gpio_select_t	alt_nb;	/* Selected Alternate		*/
	gpio_out_t	out_val;/* Default Output Value		*/
} gpio_param_s;
#endif

void gpio_config(int pin, int in_out, int gpio_alt, int out_val);
void gpio_write_bit(int pin, int val);
int gpio_read_out_bit(int pin);
int gpio_read_in_bit(int pin);
void gpio_set_chip_configuration(void);

#endif /* __ASM_PPC_GPIO_H */
