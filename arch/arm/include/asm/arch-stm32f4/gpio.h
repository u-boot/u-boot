/*
 * (C) Copyright 2011
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _STM32_GPIO_H_
#define _STM32_GPIO_H_

#if (CONFIG_STM32_USART == 1)
#define STM32_GPIO_PORT_X   STM32_GPIO_PORT_A
#define STM32_GPIO_PIN_TX   STM32_GPIO_PIN_9
#define STM32_GPIO_PIN_RX   STM32_GPIO_PIN_10
#define STM32_GPIO_USART    STM32_GPIO_AF7

#elif (CONFIG_STM32_USART == 2)
#define STM32_GPIO_PORT_X   STM32_GPIO_PORT_D
#define STM32_GPIO_PIN_TX   STM32_GPIO_PIN_5
#define STM32_GPIO_PIN_RX   STM32_GPIO_PIN_6
#define STM32_GPIO_USART    STM32_GPIO_AF7

#elif (CONFIG_STM32_USART == 3)
#define STM32_GPIO_PORT_X   STM32_GPIO_PORT_C
#define STM32_GPIO_PIN_TX   STM32_GPIO_PIN_10
#define STM32_GPIO_PIN_RX   STM32_GPIO_PIN_11
#define STM32_GPIO_USART    STM32_GPIO_AF7

#elif (CONFIG_STM32_USART == 6)
#define STM32_GPIO_PORT_X   STM32_GPIO_PORT_G
#define STM32_GPIO_PIN_TX   STM32_GPIO_PIN_14
#define STM32_GPIO_PIN_RX   STM32_GPIO_PIN_9
#define STM32_GPIO_USART    STM32_GPIO_AF8

#else
#define STM32_GPIO_PORT_X   STM32_GPIO_PORT_A
#define STM32_GPIO_PIN_TX   STM32_GPIO_PIN_9
#define STM32_GPIO_PIN_RX   STM32_GPIO_PIN_10
#define STM32_GPIO_USART    STM32_GPIO_AF7

#endif

enum stm32_gpio_port {
	STM32_GPIO_PORT_A = 0,
	STM32_GPIO_PORT_B,
	STM32_GPIO_PORT_C,
	STM32_GPIO_PORT_D,
	STM32_GPIO_PORT_E,
	STM32_GPIO_PORT_F,
	STM32_GPIO_PORT_G,
	STM32_GPIO_PORT_H,
	STM32_GPIO_PORT_I
};

enum stm32_gpio_pin {
	STM32_GPIO_PIN_0 = 0,
	STM32_GPIO_PIN_1,
	STM32_GPIO_PIN_2,
	STM32_GPIO_PIN_3,
	STM32_GPIO_PIN_4,
	STM32_GPIO_PIN_5,
	STM32_GPIO_PIN_6,
	STM32_GPIO_PIN_7,
	STM32_GPIO_PIN_8,
	STM32_GPIO_PIN_9,
	STM32_GPIO_PIN_10,
	STM32_GPIO_PIN_11,
	STM32_GPIO_PIN_12,
	STM32_GPIO_PIN_13,
	STM32_GPIO_PIN_14,
	STM32_GPIO_PIN_15
};

enum stm32_gpio_mode {
	STM32_GPIO_MODE_IN = 0,
	STM32_GPIO_MODE_OUT,
	STM32_GPIO_MODE_AF,
	STM32_GPIO_MODE_AN
};

enum stm32_gpio_otype {
	STM32_GPIO_OTYPE_PP = 0,
	STM32_GPIO_OTYPE_OD
};

enum stm32_gpio_speed {
	STM32_GPIO_SPEED_2M = 0,
	STM32_GPIO_SPEED_25M,
	STM32_GPIO_SPEED_50M,
	STM32_GPIO_SPEED_100M
};

enum stm32_gpio_pupd {
	STM32_GPIO_PUPD_NO = 0,
	STM32_GPIO_PUPD_UP,
	STM32_GPIO_PUPD_DOWN
};

enum stm32_gpio_af {
	STM32_GPIO_AF0 = 0,
	STM32_GPIO_AF1,
	STM32_GPIO_AF2,
	STM32_GPIO_AF3,
	STM32_GPIO_AF4,
	STM32_GPIO_AF5,
	STM32_GPIO_AF6,
	STM32_GPIO_AF7,
	STM32_GPIO_AF8,
	STM32_GPIO_AF9,
	STM32_GPIO_AF10,
	STM32_GPIO_AF11,
	STM32_GPIO_AF12,
	STM32_GPIO_AF13,
	STM32_GPIO_AF14,
	STM32_GPIO_AF15
};

struct stm32_gpio_dsc {
	enum stm32_gpio_port	port;
	enum stm32_gpio_pin	pin;
};

struct stm32_gpio_ctl {
	enum stm32_gpio_mode	mode;
	enum stm32_gpio_otype	otype;
	enum stm32_gpio_speed	speed;
	enum stm32_gpio_pupd	pupd;
	enum stm32_gpio_af	af;
};

static inline unsigned stm32_gpio_to_port(unsigned gpio)
{
	return gpio / 16;
}

static inline unsigned stm32_gpio_to_pin(unsigned gpio)
{
	return gpio % 16;
}

int stm32_gpio_config(const struct stm32_gpio_dsc *gpio_dsc,
		const struct stm32_gpio_ctl *gpio_ctl);
int stm32_gpout_set(const struct stm32_gpio_dsc *gpio_dsc, int state);

#endif /* _STM32_GPIO_H_ */
