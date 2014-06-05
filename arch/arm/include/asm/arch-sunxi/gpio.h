/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_GPIO_H
#define _SUNXI_GPIO_H

#include <linux/types.h>

/*
 * sunxi has 9 banks of gpio, they are:
 * PA0 - PA17 | PB0 - PB23 | PC0 - PC24
 * PD0 - PD27 | PE0 - PE31 | PF0 - PF5
 * PG0 - PG9  | PH0 - PH27 | PI0 - PI12
 */

#define SUNXI_GPIO_A	0
#define SUNXI_GPIO_B	1
#define SUNXI_GPIO_C	2
#define SUNXI_GPIO_D	3
#define SUNXI_GPIO_E	4
#define SUNXI_GPIO_F	5
#define SUNXI_GPIO_G	6
#define SUNXI_GPIO_H	7
#define SUNXI_GPIO_I	8
#define SUNXI_GPIO_BANKS 9

struct sunxi_gpio {
	u32 cfg[4];
	u32 dat;
	u32 drv[2];
	u32 pull[2];
};

/* gpio interrupt control */
struct sunxi_gpio_int {
	u32 cfg[3];
	u32 ctl;
	u32 sta;
	u32 deb;		/* interrupt debounce */
};

struct sunxi_gpio_reg {
	struct sunxi_gpio gpio_bank[SUNXI_GPIO_BANKS];
	u8 res[0xbc];
	struct sunxi_gpio_int gpio_int;
};

#define BANK_TO_GPIO(bank) \
	&((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank]

#define GPIO_BANK(pin)		((pin) >> 5)
#define GPIO_NUM(pin)		((pin) & 0x1f)

#define GPIO_CFG_INDEX(pin)	(((pin) & 0x1f) >> 3)
#define GPIO_CFG_OFFSET(pin)	((((pin) & 0x1f) & 0x7) << 2)

#define GPIO_DRV_INDEX(pin)   (((pin) & 0x1f) >> 4)
#define GPIO_DRV_OFFSET(pin)	((((pin) & 0x1f) & 0xf) << 1)

#define GPIO_PULL_INDEX(pin)	(((pin) & 0x1f) >> 4)
#define GPIO_PULL_OFFSET(pin)	((((pin) & 0x1f) & 0xf) << 1)

/* GPIO bank sizes */
#define SUNXI_GPIO_A_NR		32
#define SUNXI_GPIO_B_NR		32
#define SUNXI_GPIO_C_NR		32
#define SUNXI_GPIO_D_NR		32
#define SUNXI_GPIO_E_NR		32
#define SUNXI_GPIO_F_NR		32
#define SUNXI_GPIO_G_NR		32
#define SUNXI_GPIO_H_NR		32
#define SUNXI_GPIO_I_NR		32

#define SUNXI_GPIO_NEXT(__gpio) \
	((__gpio##_START) + (__gpio##_NR) + 0)

enum sunxi_gpio_number {
	SUNXI_GPIO_A_START = 0,
	SUNXI_GPIO_B_START = SUNXI_GPIO_NEXT(SUNXI_GPIO_A),
	SUNXI_GPIO_C_START = SUNXI_GPIO_NEXT(SUNXI_GPIO_B),
	SUNXI_GPIO_D_START = SUNXI_GPIO_NEXT(SUNXI_GPIO_C),
	SUNXI_GPIO_E_START = SUNXI_GPIO_NEXT(SUNXI_GPIO_D),
	SUNXI_GPIO_F_START = SUNXI_GPIO_NEXT(SUNXI_GPIO_E),
	SUNXI_GPIO_G_START = SUNXI_GPIO_NEXT(SUNXI_GPIO_F),
	SUNXI_GPIO_H_START = SUNXI_GPIO_NEXT(SUNXI_GPIO_G),
	SUNXI_GPIO_I_START = SUNXI_GPIO_NEXT(SUNXI_GPIO_H),
};

/* SUNXI GPIO number definitions */
#define SUNXI_GPA(_nr)	(SUNXI_GPIO_A_START + (_nr))
#define SUNXI_GPB(_nr)	(SUNXI_GPIO_B_START + (_nr))
#define SUNXI_GPC(_nr)	(SUNXI_GPIO_C_START + (_nr))
#define SUNXI_GPD(_nr)	(SUNXI_GPIO_D_START + (_nr))
#define SUNXI_GPE(_nr)	(SUNXI_GPIO_E_START + (_nr))
#define SUNXI_GPF(_nr)	(SUNXI_GPIO_F_START + (_nr))
#define SUNXI_GPG(_nr)	(SUNXI_GPIO_G_START + (_nr))
#define SUNXI_GPH(_nr)	(SUNXI_GPIO_H_START + (_nr))
#define SUNXI_GPI(_nr)	(SUNXI_GPIO_I_START + (_nr))

/* GPIO pin function config */
#define SUNXI_GPIO_INPUT	0
#define SUNXI_GPIO_OUTPUT	1

#define SUNXI_GPA0_EMAC		2
#define SUN7I_GPA0_GMAC		5

#define SUNXI_GPB0_TWI0		2

#define SUN4I_GPB22_UART0_TX	2
#define SUN4I_GPB23_UART0_RX	2

#define SUN5I_GPB19_UART0_TX	2
#define SUN5I_GPB20_UART0_RX	2

#define SUN5I_GPG3_UART1_TX	4
#define SUN5I_GPG4_UART1_RX	4

#define SUNXI_GPC6_SDC2		3

#define SUNXI_GPF0_SDC0		2

#define SUNXI_GPF2_SDC0		2
#define SUNXI_GPF2_UART0_TX	4
#define SUNXI_GPF4_UART0_RX	4

#define SUN4I_GPG0_SDC1		4

#define SUN4I_GPH22_SDC1	5

#define SUN4I_GPI4_SDC3		2

/* GPIO pin pull-up/down config */
#define SUNXI_GPIO_PULL_DISABLE	0
#define SUNXI_GPIO_PULL_UP	1
#define SUNXI_GPIO_PULL_DOWN	2

int sunxi_gpio_set_cfgpin(u32 pin, u32 val);
int sunxi_gpio_get_cfgpin(u32 pin);
int sunxi_gpio_set_drv(u32 pin, u32 val);
int sunxi_gpio_set_pull(u32 pin, u32 val);
int sunxi_name_to_gpio(const char *name);
#define name_to_gpio(name) sunxi_name_to_gpio(name)

#endif /* _SUNXI_GPIO_H */
