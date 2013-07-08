/*
 * Driver for Altera's PIO ip core
 *
 * Copyright (C) 2011  Missing Link Electronics
 *                     Joachim Foerster <joachim@missinglinkelectronics.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * To use this driver, in your board's config. header:
 * #define CONFIG_ALTERA_PIO
 * #define CONFIG_SYS_ALTERA_PIO_NUM <number-of-pio-cores>
 * #define CONFIG_SYS_ALTERA_PIO_GPIO_NUM <total-number-of-gpios>
 * And in your board's early setup routine:
 * altera_pio_init(<baseaddr>, <width>, 'i'|'o'|'t',
 *                 <reset-value>, <neg-mask>, "label");
 *  - 'i'|'o'|'t': PIO is input-only/output-only/tri-state
 *  - <reset-value>: for correct initial status display, output-only
 *  - <neg-mask> is meant to be used to in cases of active-low
 *    GPIOs, such as LEDs and buttons (on/pressed == 0). Each bit
 *    which is 1 in <neg-mask> inverts the corresponding GPIO's value
 *    before set/after get. So: gpio_set_value(gpio, 1) => LED on .
 *
 * Do NOT define CONFIG_SYS_GPIO_BASE !
 *
 * Optionally, in your board's config. header:
 * - To force a GPIO numbering scheme like in Linux ...
 * #define CONFIG_GPIO_DOWNTO_NUMBERING
 * ... starting with 255 (default)
 * #define CONFIG_GPIO_DOWNTO_MAX 255
 */
#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>

#ifdef CONFIG_GPIO_DOWNTO_NUMBERING
#ifndef CONFIG_GPIO_DOWNTO_MAX
#define CONFIG_GPIO_DOWNTO_MAX 255
#endif
#endif

#define ALTERA_PIO_DATA		0x0
#define ALTERA_PIO_DIR		0x4

#define GPIO_LABEL_SIZE		9


static struct altera_pio {
	u32 base;
	u8 width;
	char iot;
	u32 negmask;
	u32 sh_data;
	u32 sh_dir;
	int gidx;
	char label[GPIO_LABEL_SIZE];
} pios[CONFIG_SYS_ALTERA_PIO_NUM];

static int pio_num;

static struct altera_pio_gpio {
	unsigned num;
	struct altera_pio *pio;
	char reqlabel[GPIO_LABEL_SIZE];
} gpios[CONFIG_SYS_ALTERA_PIO_GPIO_NUM];

static int pio_gpio_num;


static int altera_pio_gidx(unsigned gpio)
{
	int i;

	for (i = 0; i < pio_gpio_num; ++i) {
		if (gpio == gpios[i].num)
			break;
	}
	if (i >= pio_gpio_num)
		return -1;
	return i;
}

static struct altera_pio *altera_pio_get_and_mask(unsigned gpio, u32 *mask)
{
	int gidx = altera_pio_gidx(gpio);
	if (gidx < 0)
		return NULL;
	if (mask)
		*mask = 1 << (gidx - gpios[gidx].pio->gidx);
	return gpios[gidx].pio;
}

#define altera_pio_use_gidx(_gidx, _reqlabel) \
	{ strncpy(gpios[_gidx].reqlabel, _reqlabel, GPIO_LABEL_SIZE); }
#define altera_pio_unuse_gidx(_gidx) { gpios[_gidx].reqlabel[0] = '\0'; }
#define altera_pio_is_gidx_used(_gidx) (gpios[_gidx].reqlabel[0] != '\0')

static int altera_pio_gpio_init(struct altera_pio *pio, u8 width)
{
	u8 gidx = pio_gpio_num;
	int i;

	if (!width)
		return -1;
	if ((pio_gpio_num + width) > CONFIG_SYS_ALTERA_PIO_GPIO_NUM)
		return -1;

	for (i = 0; i < width; ++i) {
#ifdef CONFIG_GPIO_DOWNTO_NUMBERING
		gpios[pio_gpio_num + i].num = \
			CONFIG_GPIO_DOWNTO_MAX + 1 - gidx - width + i;
#else
		gpios[pio_gpio_num + i].num = pio_gpio_num + i;
#endif
		gpios[pio_gpio_num + i].pio = pio;
		altera_pio_unuse_gidx(pio_gpio_num + i);
	}
	pio_gpio_num += width;
	return gidx;
}

int altera_pio_init(u32 base, u8 width, char iot, u32 rstval, u32 negmask,
		 const char *label)
{
	if (pio_num >= CONFIG_SYS_ALTERA_PIO_NUM)
		return -1;

	pios[pio_num].base = base;
	pios[pio_num].width = width;
	pios[pio_num].iot = iot;
	switch (iot) {
	case 'i':
		/* input only */
		pios[pio_num].sh_dir = 0;
		pios[pio_num].sh_data = readl(base + ALTERA_PIO_DATA);
		break;
	case 'o':
		/* output only */
		pios[pio_num].sh_dir = 0xffffffff & ((1 << width) - 1);
		pios[pio_num].sh_data = rstval;
		break;
	case 't':
		/* bidir, tri-state */
		pios[pio_num].sh_dir = readl(base + ALTERA_PIO_DIR);
		pios[pio_num].sh_data = readl(base + ALTERA_PIO_DATA);
		break;
	default:
		return -1;
	}
	pios[pio_num].negmask = negmask & ((1 << width) - 1);
	pios[pio_num].gidx = altera_pio_gpio_init(&pios[pio_num], width);
	if (pios[pio_num].gidx < 0)
		return -1;
	strncpy(pios[pio_num].label, label, GPIO_LABEL_SIZE);
	return pio_num++;
}

void altera_pio_info(void)
{
	int i;
	int j;
	int gidx;
	u32 mask;

	for (i = 0; i < pio_num; ++i) {
		printf("Altera PIO % 2d, @0x%08x, "
			"width: %u, label: %s\n",
		       i, pios[i].base, pios[i].width, pios[i].label);
		gidx = pios[i].gidx;
		for (j = gidx; j < (gidx + pios[i].width); ++j) {
			mask = 1 << (j - gidx);
			printf("\tGPIO % 4d: %s %s [%c] %s\n",
				gpios[j].num,
				gpios[j].pio->sh_dir & mask ? "out" : " in",
				gpio_get_value(gpios[j].num) ? "set" : "clr",
				altera_pio_is_gidx_used(j) ? 'x' : ' ',
				gpios[j].reqlabel);
		}
	}
}


int gpio_request(unsigned gpio, const char *label)
{
	int gidx = altera_pio_gidx(gpio);
	if (gidx < 0)
		return gidx;
	if (altera_pio_is_gidx_used(gidx))
		return -1;

	altera_pio_use_gidx(gidx, label);
	return 0;
}

int gpio_free(unsigned gpio)
{
	int gidx = altera_pio_gidx(gpio);
	if (gidx < 0)
		return gidx;
	if (!altera_pio_is_gidx_used(gidx))
		return -1;

	altera_pio_unuse_gidx(gidx);
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	u32 mask;
	struct altera_pio *pio;

	pio = altera_pio_get_and_mask(gpio, &mask);
	if (!pio)
		return -1;
	if (pio->iot == 'o')
		return -1;

	writel(pio->sh_dir &= ~mask, pio->base + ALTERA_PIO_DIR);
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	u32 mask;
	struct altera_pio *pio;

	pio = altera_pio_get_and_mask(gpio, &mask);
	if (!pio)
		return -1;
	if (pio->iot == 'i')
		return -1;

	value = (pio->negmask & mask) ? !value : value;
	if (value)
		pio->sh_data |= mask;
	else
		pio->sh_data &= ~mask;
	writel(pio->sh_data, pio->base + ALTERA_PIO_DATA);
	writel(pio->sh_dir |= mask, pio->base + ALTERA_PIO_DIR);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	u32 mask;
	struct altera_pio *pio;
	u32 val;

	pio = altera_pio_get_and_mask(gpio, &mask);
	if (!pio)
		return -1;

	if ((pio->sh_dir & mask) || (pio->iot == 'o'))
		val = pio->sh_data & mask;
	else
		val = readl(pio->base + ALTERA_PIO_DATA) & mask;
	return (pio->negmask & mask) ? !val : val;
}

void gpio_set_value(unsigned gpio, int value)
{
	u32 mask;
	struct altera_pio *pio;

	pio = altera_pio_get_and_mask(gpio, &mask);
	if (!pio)
		return;
	if (pio->iot == 'i')
		return;

	value = (pio->negmask & mask) ? !value : value;
	if (value)
		pio->sh_data |= mask;
	else
		pio->sh_data &= ~mask;
	writel(pio->sh_data, pio->base + ALTERA_PIO_DATA);
	return;
}

int gpio_is_valid(int number)
{
	int gidx = altera_pio_gidx(number);

	if (gidx < 0)
		return 1;
	return 0;
}
