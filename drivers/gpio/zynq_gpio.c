/*
 * Xilinx Zynq GPIO device driver
 *
 * Copyright (C) 2015 DAVE Embedded Systems <devel@dave.eu>
 *
 * Most of code taken from linux kernel driver (linux/drivers/gpio/gpio-zynq.c)
 * Copyright (C) 2009 - 2014 Xilinx, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/errno.h>

/**
 * zynq_gpio_get_bank_pin - Get the bank number and pin number within that bank
 * for a given pin in the GPIO device
 * @pin_num:	gpio pin number within the device
 * @bank_num:	an output parameter used to return the bank number of the gpio
 *		pin
 * @bank_pin_num: an output parameter used to return pin number within a bank
 *		  for the given gpio pin
 *
 * Returns the bank number and pin offset within the bank.
 */
static inline void zynq_gpio_get_bank_pin(unsigned int pin_num,
					  unsigned int *bank_num,
					  unsigned int *bank_pin_num)
{
	switch (pin_num) {
	case ZYNQ_GPIO_BANK0_PIN_MIN ... ZYNQ_GPIO_BANK0_PIN_MAX:
		*bank_num = 0;
		*bank_pin_num = pin_num;
		break;
	case ZYNQ_GPIO_BANK1_PIN_MIN ... ZYNQ_GPIO_BANK1_PIN_MAX:
		*bank_num = 1;
		*bank_pin_num = pin_num - ZYNQ_GPIO_BANK1_PIN_MIN;
		break;
	case ZYNQ_GPIO_BANK2_PIN_MIN ... ZYNQ_GPIO_BANK2_PIN_MAX:
		*bank_num = 2;
		*bank_pin_num = pin_num - ZYNQ_GPIO_BANK2_PIN_MIN;
		break;
	case ZYNQ_GPIO_BANK3_PIN_MIN ... ZYNQ_GPIO_BANK3_PIN_MAX:
		*bank_num = 3;
		*bank_pin_num = pin_num - ZYNQ_GPIO_BANK3_PIN_MIN;
		break;
	default:
		printf("invalid GPIO pin number: %u\n", pin_num);
		*bank_num = 0;
		*bank_pin_num = 0;
		break;
	}
}

int gpio_is_valid(unsigned gpio)
{
	return (gpio >= 0) && (gpio < ZYNQ_GPIO_NR_GPIOS);
}

static int check_gpio(unsigned gpio)
{
	if (!gpio_is_valid(gpio)) {
		printf("ERROR : check_gpio: invalid GPIO %d\n", gpio);
		return -1;
	}
	return 0;
}

/**
 * gpio_get_value - Get the state of the specified pin of GPIO device
 * @gpio:	gpio pin number within the device
 *
 * This function reads the state of the specified pin of the GPIO device.
 *
 * Return: 0 if the pin is low, 1 if pin is high.
 */
int gpio_get_value(unsigned gpio)
{
	u32 data;
	unsigned int bank_num, bank_pin_num;

	if (check_gpio(gpio) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num);

	data = readl(ZYNQ_GPIO_BASE_ADDRESS +
			     ZYNQ_GPIO_DATA_RO_OFFSET(bank_num));

	return (data >> bank_pin_num) & 1;
}

/**
 * gpio_set_value - Modify the value of the pin with specified value
 * @gpio:	gpio pin number within the device
 * @value:	value used to modify the value of the specified pin
 *
 * This function calculates the register offset (i.e to lower 16 bits or
 * upper 16 bits) based on the given pin number and sets the value of a
 * gpio pin to the specified value. The value is either 0 or non-zero.
 */
int gpio_set_value(unsigned gpio, int value)
{
	unsigned int reg_offset, bank_num, bank_pin_num;

	if (check_gpio(gpio) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num);

	if (bank_pin_num >= ZYNQ_GPIO_MID_PIN_NUM) {
		/* only 16 data bits in bit maskable reg */
		bank_pin_num -= ZYNQ_GPIO_MID_PIN_NUM;
		reg_offset = ZYNQ_GPIO_DATA_MSW_OFFSET(bank_num);
	} else {
		reg_offset = ZYNQ_GPIO_DATA_LSW_OFFSET(bank_num);
	}

	/*
	 * get the 32 bit value to be written to the mask/data register where
	 * the upper 16 bits is the mask and lower 16 bits is the data
	 */
	value = !!value;
	value = ~(1 << (bank_pin_num + ZYNQ_GPIO_MID_PIN_NUM)) &
		((value << bank_pin_num) | ZYNQ_GPIO_UPPER_MASK);

	writel(value, ZYNQ_GPIO_BASE_ADDRESS + reg_offset);

	return 0;
}

/**
 * gpio_direction_input - Set the direction of the specified GPIO pin as input
 * @gpio:	gpio pin number within the device
 *
 * This function uses the read-modify-write sequence to set the direction of
 * the gpio pin as input.
 *
 * Return: -1 if invalid gpio specified, 0 if successul
 */
int gpio_direction_input(unsigned gpio)
{
	u32 reg;
	unsigned int bank_num, bank_pin_num;

	if (check_gpio(gpio) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num);

	/* bank 0 pins 7 and 8 are special and cannot be used as inputs */
	if (bank_num == 0 && (bank_pin_num == 7 || bank_pin_num == 8))
		return -1;

	/* clear the bit in direction mode reg to set the pin as input */
	reg = readl(ZYNQ_GPIO_BASE_ADDRESS + ZYNQ_GPIO_DIRM_OFFSET(bank_num));
	reg &= ~BIT(bank_pin_num);
	writel(reg, ZYNQ_GPIO_BASE_ADDRESS + ZYNQ_GPIO_DIRM_OFFSET(bank_num));

	return 0;
}

/**
 * gpio_direction_output - Set the direction of the specified GPIO pin as output
 * @gpio:	gpio pin number within the device
 * @value:	value to be written to specified pin
 *
 * This function sets the direction of specified GPIO pin as output, configures
 * the Output Enable register for the pin and uses zynq_gpio_set to set
 * the value of the pin to the value specified.
 *
 * Return: 0 always
 */
int gpio_direction_output(unsigned gpio, int value)
{
	u32 reg;
	unsigned int bank_num, bank_pin_num;

	if (check_gpio(gpio) < 0)
		return -1;

	zynq_gpio_get_bank_pin(gpio, &bank_num, &bank_pin_num);

	/* set the GPIO pin as output */
	reg = readl(ZYNQ_GPIO_BASE_ADDRESS + ZYNQ_GPIO_DIRM_OFFSET(bank_num));
	reg |= BIT(bank_pin_num);
	writel(reg, ZYNQ_GPIO_BASE_ADDRESS + ZYNQ_GPIO_DIRM_OFFSET(bank_num));

	/* configure the output enable reg for the pin */
	reg = readl(ZYNQ_GPIO_BASE_ADDRESS + ZYNQ_GPIO_OUTEN_OFFSET(bank_num));
	reg |= BIT(bank_pin_num);
	writel(reg, ZYNQ_GPIO_BASE_ADDRESS + ZYNQ_GPIO_OUTEN_OFFSET(bank_num));

	/* set the state of the pin */
	gpio_set_value(gpio, value);
	return 0;
}

/**
 * Request a gpio before using it.
 *
 * NOTE: Argument 'label' is unused.
 */
int gpio_request(unsigned gpio, const char *label)
{
	if (check_gpio(gpio) < 0)
		return -1;

	return 0;
}

/**
 * Reset and free the gpio after using it.
 */
int gpio_free(unsigned gpio)
{
	return 0;
}
