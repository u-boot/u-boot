/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __AIROHA_COMMON_HEADER__
#define __AIROHA_COMMON_HEADER__

#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>
#include <linux/pinctrl/pinctrl.h>

#include <dm/device.h>
#include <dm/pinctrl.h>

/* GPIOs */
#define REG_GPIO_CTRL				0x0000
#define REG_GPIO_DATA				0x0004
#define REG_GPIO_INT				0x0008
#define REG_GPIO_INT_EDGE			0x000c
#define REG_GPIO_INT_LEVEL			0x0010
#define REG_GPIO_OE				0x0014
#define REG_GPIO_CTRL1				0x0020
#define REG_GPIO_CTRL2				0x0060
#define REG_GPIO_CTRL3				0x0064
#define REG_GPIO_DATA1				0x0070
#define REG_GPIO_OE1				0x0078
#define REG_GPIO_INT1				0x007c
#define REG_GPIO_INT_EDGE1			0x0080
#define REG_GPIO_INT_EDGE2			0x0084
#define REG_GPIO_INT_EDGE3			0x0088
#define REG_GPIO_INT_LEVEL1			0x008c
#define REG_GPIO_INT_LEVEL2			0x0090
#define REG_GPIO_INT_LEVEL3			0x0094

#define AIROHA_NUM_PINS				64
#define AIROHA_PIN_BANK_SIZE			(AIROHA_NUM_PINS / 2)
#define AIROHA_REG_GPIOCTRL_NUM_PIN		(AIROHA_NUM_PINS / 4)

#define PINCTRL_PIN_GROUP(id, table)					\
	PINCTRL_PINGROUP(id, table##_pins, ARRAY_SIZE(table##_pins))

#define PINCTRL_FUNC_DESC(id, table)					\
	{								\
		.desc = PINCTRL_PINFUNCTION(id, table##_groups,	\
					    ARRAY_SIZE(table##_groups)),\
		.groups = table##_func_group,				\
		.group_size = ARRAY_SIZE(table##_func_group),		\
	}

#define PINCTRL_CONF_DESC(p, offset, mask)				\
	{								\
		.pin = p,						\
		.reg = { offset, mask },				\
	}

struct airoha_pinctrl_reg {
	u32 offset;
	u32 mask;
};

enum airoha_pinctrl_mux_func {
	AIROHA_FUNC_MUX,
	AIROHA_FUNC_PWM_MUX,
	AIROHA_FUNC_PWM_EXT_MUX,
};

struct airoha_pinctrl_func_group {
	const char *name;
	struct {
		enum airoha_pinctrl_mux_func mux;
		u32 offset;
		u32 mask;
		u32 val;
	} regmap[2];
	int regmap_size;
};

struct airoha_pinctrl_func {
	const struct pinfunction desc;
	const struct airoha_pinctrl_func_group *groups;
	u8 group_size;
};

struct airoha_pinctrl_conf {
	u32 pin;
	struct airoha_pinctrl_reg reg;
};

struct airoha_pinctrl_gpiochip {
	/* gpio */
	const u32 *data;
	const u32 *dir;
	const u32 *out;
	/* irq */
	const u32 *status;
	const u32 *level;
	const u32 *edge;

	u32 irq_type[AIROHA_NUM_PINS];
};

struct airoha_pinctrl_confs_info {
	const struct airoha_pinctrl_conf *confs;
	unsigned int num_confs;
};

enum airoha_pinctrl_confs_type {
	AIROHA_PINCTRL_CONFS_PULLUP,
	AIROHA_PINCTRL_CONFS_PULLDOWN,
	AIROHA_PINCTRL_CONFS_DRIVE_E2,
	AIROHA_PINCTRL_CONFS_DRIVE_E4,
	AIROHA_PINCTRL_CONFS_PCIE_RST_OD,

	AIROHA_PINCTRL_CONFS_MAX
};

struct airoha_pinctrl {
	struct udevice *dev;

	struct regmap *chip_scu;
	struct regmap *regmap;

	struct airoha_pinctrl_match_data *data;

	struct airoha_pinctrl_gpiochip gpiochip;
};

struct airoha_pinctrl_match_data {
	const int gpio_offs;
	const int gpio_pin_cnt;
	const char *chip_scu_compatible;
	const struct pinctrl_pin_desc *pins;
	const unsigned int num_pins;
	const struct pingroup *grps;
	const unsigned int num_grps;
	const struct airoha_pinctrl_func *funcs;
	const unsigned int num_funcs;
	const struct airoha_pinctrl_confs_info confs_info[AIROHA_PINCTRL_CONFS_MAX];
};

extern const struct pinctrl_ops airoha_pinctrl_ops;

int airoha_pinctrl_probe(struct udevice *dev);
int airoha_pinctrl_bind(struct udevice *dev);

#endif
