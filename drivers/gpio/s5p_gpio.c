/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>

#define S5P_GPIO_GET_PIN(x)	(x % GPIO_PER_BANK)

#define CON_MASK(x)		(0xf << ((x) << 2))
#define CON_SFR(x, v)		((v) << ((x) << 2))

#define DAT_MASK(x)		(0x1 << (x))
#define DAT_SET(x)		(0x1 << (x))

#define PULL_MASK(x)		(0x3 << ((x) << 1))
#define PULL_MODE(x, v)		((v) << ((x) << 1))

#define DRV_MASK(x)		(0x3 << ((x) << 1))
#define DRV_SET(x, m)		((m) << ((x) << 1))
#define RATE_MASK(x)		(0x1 << (x + 16))
#define RATE_SET(x)		(0x1 << (x + 16))

#define name_to_gpio(n) s5p_name_to_gpio(n)
static inline int s5p_name_to_gpio(const char *name)
{
	unsigned num, irregular_set_number, irregular_bank_base;
	const struct gpio_name_num_table *tabp;
	char this_bank, bank_name, irregular_bank_name;
	char *endp;

	/*
	 * The gpio name starts with either 'g' or 'gp' followed by the bank
	 * name character. Skip one or two characters depending on the prefix.
	 */
	if (name[0] == 'g' && name[1] == 'p')
		name += 2;
	else if (name[0] == 'g')
		name++;
	else
		return -1; /* Name must start with 'g' */

	bank_name = *name++;
	if (!*name)
		return -1; /* At least one digit is required/expected. */

	/*
	 * On both exynos5 and exynos5420 architectures there is a bank of
	 * GPIOs which does not fall into the regular address pattern. Those
	 * banks are c4 on Exynos5 and y7 on Exynos5420. The rest of the below
	 * assignments help to handle these irregularities.
	 */
#if defined(CONFIG_EXYNOS4) || defined(CONFIG_EXYNOS5)
	if (cpu_is_exynos5()) {
		if (proid_is_exynos5420()) {
			tabp = exynos5420_gpio_table;
			irregular_bank_name = 'y';
			irregular_set_number = '7';
			irregular_bank_base = EXYNOS5420_GPIO_Y70;
		} else {
			tabp = exynos5_gpio_table;
			irregular_bank_name = 'c';
			irregular_set_number = '4';
			irregular_bank_base = EXYNOS5_GPIO_C40;
		}
	} else {
		if (proid_is_exynos4412())
			tabp = exynos4x12_gpio_table;
		else
			tabp = exynos4_gpio_table;
		irregular_bank_name = 0;
		irregular_set_number = 0;
		irregular_bank_base = 0;
	}
#else
	if (cpu_is_s5pc110())
		tabp = s5pc110_gpio_table;
	else
		tabp = s5pc100_gpio_table;
	irregular_bank_name = 0;
	irregular_set_number = 0;
	irregular_bank_base = 0;
#endif

	this_bank = tabp->bank;
	do {
		if (bank_name == this_bank) {
			unsigned pin_index; /* pin number within the bank */
			if ((bank_name == irregular_bank_name) &&
			    (name[0] == irregular_set_number)) {
				pin_index = name[1] - '0';
				/* Irregular sets have 8 pins. */
				if (pin_index >= GPIO_PER_BANK)
					return -1;
				num = irregular_bank_base + pin_index;
			} else {
				pin_index = simple_strtoul(name, &endp, 8);
				pin_index -= tabp->bank_offset;
				/*
				 * Sanity check: bunk 'z' has no set number,
				 * for all other banks there must be exactly
				 * two octal digits, and the resulting number
				 * should not exceed the number of pins in the
				 * bank.
				 */
				if (((bank_name != 'z') && !name[1]) ||
				    *endp ||
				    (pin_index >= tabp->bank_size))
					return -1;
				num = tabp->base + pin_index;
			}
			return num;
		}
		this_bank = (++tabp)->bank;
	} while (this_bank);

	return -1;
}

static void s5p_gpio_cfg_pin(struct s5p_gpio_bank *bank, int gpio, int cfg)
{
	unsigned int value;

	value = readl(&bank->con);
	value &= ~CON_MASK(gpio);
	value |= CON_SFR(gpio, cfg);
	writel(value, &bank->con);
}

static void s5p_gpio_set_value(struct s5p_gpio_bank *bank, int gpio, int en)
{
	unsigned int value;

	value = readl(&bank->dat);
	value &= ~DAT_MASK(gpio);
	if (en)
		value |= DAT_SET(gpio);
	writel(value, &bank->dat);
}

static void s5p_gpio_direction_output(struct s5p_gpio_bank *bank,
				      int gpio, int en)
{
	s5p_gpio_cfg_pin(bank, gpio, S5P_GPIO_OUTPUT);
	s5p_gpio_set_value(bank, gpio, en);
}

static void s5p_gpio_direction_input(struct s5p_gpio_bank *bank, int gpio)
{
	s5p_gpio_cfg_pin(bank, gpio, S5P_GPIO_INPUT);
}

static unsigned int s5p_gpio_get_value(struct s5p_gpio_bank *bank, int gpio)
{
	unsigned int value;

	value = readl(&bank->dat);
	return !!(value & DAT_MASK(gpio));
}

static void s5p_gpio_set_pull(struct s5p_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->pull);
	value &= ~PULL_MASK(gpio);

	switch (mode) {
	case S5P_GPIO_PULL_DOWN:
	case S5P_GPIO_PULL_UP:
		value |= PULL_MODE(gpio, mode);
		break;
	default:
		break;
	}

	writel(value, &bank->pull);
}

static void s5p_gpio_set_drv(struct s5p_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->drv);
	value &= ~DRV_MASK(gpio);

	switch (mode) {
	case S5P_GPIO_DRV_1X:
	case S5P_GPIO_DRV_2X:
	case S5P_GPIO_DRV_3X:
	case S5P_GPIO_DRV_4X:
		value |= DRV_SET(gpio, mode);
		break;
	default:
		return;
	}

	writel(value, &bank->drv);
}

static void s5p_gpio_set_rate(struct s5p_gpio_bank *bank, int gpio, int mode)
{
	unsigned int value;

	value = readl(&bank->drv);
	value &= ~RATE_MASK(gpio);

	switch (mode) {
	case S5P_GPIO_DRV_FAST:
	case S5P_GPIO_DRV_SLOW:
		value |= RATE_SET(gpio);
		break;
	default:
		return;
	}

	writel(value, &bank->drv);
}

struct s5p_gpio_bank *s5p_gpio_get_bank(unsigned int gpio)
{
	const struct gpio_info *data;
	unsigned int upto;
	int i, count;

	data = get_gpio_data();
	count = get_bank_num();
	upto = 0;

	for (i = 0; i < count; i++) {
		debug("i=%d, upto=%d\n", i, upto);
		if (gpio < data->max_gpio) {
			struct s5p_gpio_bank *bank;
			bank = (struct s5p_gpio_bank *)data->reg_addr;
			bank += (gpio - upto) / GPIO_PER_BANK;
			debug("gpio=%d, bank=%p\n", gpio, bank);
			return bank;
		}

		upto = data->max_gpio;
		data++;
	}

	return NULL;
}

int s5p_gpio_get_pin(unsigned gpio)
{
	return S5P_GPIO_GET_PIN(gpio);
}

/* Common GPIO API */

int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	s5p_gpio_direction_input(s5p_gpio_get_bank(gpio),
				s5p_gpio_get_pin(gpio));
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	s5p_gpio_direction_output(s5p_gpio_get_bank(gpio),
				 s5p_gpio_get_pin(gpio), value);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	return (int) s5p_gpio_get_value(s5p_gpio_get_bank(gpio),
				       s5p_gpio_get_pin(gpio));
}

int gpio_set_value(unsigned gpio, int value)
{
	s5p_gpio_set_value(s5p_gpio_get_bank(gpio),
			  s5p_gpio_get_pin(gpio), value);

	return 0;
}

void gpio_set_pull(int gpio, int mode)
{
	s5p_gpio_set_pull(s5p_gpio_get_bank(gpio),
			  s5p_gpio_get_pin(gpio), mode);
}

void gpio_set_drv(int gpio, int mode)
{
	s5p_gpio_set_drv(s5p_gpio_get_bank(gpio),
			 s5p_gpio_get_pin(gpio), mode);
}

void gpio_cfg_pin(int gpio, int cfg)
{
	s5p_gpio_cfg_pin(s5p_gpio_get_bank(gpio),
			 s5p_gpio_get_pin(gpio), cfg);
}

void gpio_set_rate(int gpio, int mode)
{
	s5p_gpio_set_rate(s5p_gpio_get_bank(gpio),
			  s5p_gpio_get_pin(gpio), mode);
}
