/*
 * (C) Copyright 2011
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
 *
 * Copyright 2015 ATS Advanced Telematics Systems GmbH
 * Copyright 2015 Konsulko Group, Matt Porter <mporter@konsulko.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _STM32_GPIO_H_
#define _STM32_GPIO_H_

enum stm32_gpio_port {
	STM32_GPIO_PORT_A = 0,
	STM32_GPIO_PORT_B,
	STM32_GPIO_PORT_C,
	STM32_GPIO_PORT_D,
	STM32_GPIO_PORT_E,
	STM32_GPIO_PORT_F,
	STM32_GPIO_PORT_G,
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

enum stm32_gpio_icnf {
	STM32_GPIO_ICNF_AN = 0,
	STM32_GPIO_ICNF_IN_FLT,
	STM32_GPIO_ICNF_IN_PUD,
	STM32_GPIO_ICNF_RSVD
};

enum stm32_gpio_ocnf {
	STM32_GPIO_OCNF_GP_PP = 0,
	STM32_GPIO_OCNF_GP_OD,
	STM32_GPIO_OCNF_AF_PP,
	STM32_GPIO_OCNF_AF_OD
};

enum stm32_gpio_pupd {
	STM32_GPIO_PUPD_DOWN = 0,
	STM32_GPIO_PUPD_UP,
};

enum stm32_gpio_mode {
	STM32_GPIO_MODE_IN = 0,
	STM32_GPIO_MODE_OUT_10M,
	STM32_GPIO_MODE_OUT_2M,
	STM32_GPIO_MODE_OUT_50M
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
	enum stm32_gpio_icnf	icnf;
	enum stm32_gpio_ocnf	ocnf;
	enum stm32_gpio_mode	mode;
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
